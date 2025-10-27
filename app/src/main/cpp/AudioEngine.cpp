#include "AudioEngine.h"

// Helper to get monotonic milliseconds
uint32_t get_monotonic_time_ms() {
    auto now = std::chrono::steady_clock::now();
    return std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
}

AudioEngine::AudioEngine() : mEncoder(nullptr) {
    __android_log_print(ANDROID_LOG_INFO, "AudioEngine", "Instance created");
}

AudioEngine::~AudioEngine() {
    // Ensure cleanup happens
    stop();
    __android_log_print(ANDROID_LOG_INFO, "AudioEngine", "Instance destroyed");
}

int32_t AudioEngine::start(const char* targetIp, int targetPort) {
    std::lock_guard<std::mutex> lock(mLock);
    if (mStream) return 0;

    // 1. Create UDP Socket
    mSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (mSocket < 0) {
        __android_log_print(ANDROID_LOG_ERROR, "AudioEngine", "Failed to create socket");
        return -1;
    }

    memset(&mTargetAddress, 0, sizeof(mTargetAddress));
    mTargetAddress.sin_family = AF_INET;
    mTargetAddress.sin_port = htons(targetPort);
    if (inet_pton(AF_INET, targetIp, &mTargetAddress.sin_addr) <= 0) {
        __android_log_print(ANDROID_LOG_ERROR, "AudioEngine", "Invalid target IP address");
        close(mSocket);
        mSocket = -1;
        return -2;
    }
    __android_log_print(ANDROID_LOG_INFO, "AudioEngine", "Socket created for %s:%d", targetIp, targetPort);

    // 2. Create Opus Encoder
    int error;
    mEncoder = opus_encoder_create(kSampleRate, kChannelCount, OPUS_APPLICATION_AUDIO, &error);
    if (error != OPUS_OK) {
        __android_log_print(ANDROID_LOG_ERROR, "AudioEngine", "Failed to create Opus encoder: %s", opus_strerror(error));
        close(mSocket);
        mSocket = -1;
        return -3;
    }
    opus_encoder_ctl(mEncoder, OPUS_SET_BITRATE(64000));
    opus_encoder_ctl(mEncoder, OPUS_SET_VBR(1));

    // 3. Create and start Oboe stream
    oboe::AudioStreamBuilder builder;
    builder.setDirection(oboe::Direction::Input)
            ->setPerformanceMode(oboe::PerformanceMode::LowLatency)
            ->setSharingMode(oboe::SharingMode::Exclusive)
            ->setFormat(oboe::AudioFormat::I16)
            ->setChannelCount(kChannelCount)
            ->setSampleRate(kSampleRate)
            ->setFramesPerCallback(kFrameSize)
            ->setDataCallback(this)
            ->setErrorCallback(this);

    oboe::Result result = builder.openManagedStream(mStream);
    if (result != oboe::Result::OK) {
        __android_log_print(ANDROID_LOG_ERROR, "AudioEngine", "Failed to create stream. Error: %s", oboe::convertToText(result));
        // Cleanup already created resources
        opus_encoder_destroy(mEncoder);
        mEncoder = nullptr;
        close(mSocket);
        mSocket = -1;
        return -4;
    }

    result = mStream->requestStart();
    if (result != oboe::Result::OK) {
        __android_log_print(ANDROID_LOG_ERROR, "AudioEngine", "Failed to start stream. Error: %s", oboe::convertToText(result));
        mStream->close();
        mStream.reset();
        opus_encoder_destroy(mEncoder);
        mEncoder = nullptr;
        close(mSocket);
        mSocket = -1;
        return -5;
    }

    mStartTime = std::chrono::steady_clock::now();
    mSequenceNumber = 0;
    __android_log_print(ANDROID_LOG_INFO, "AudioEngine", "Streaming pipeline started successfully");
    return 0;
}

void AudioEngine::stop() {
    std::lock_guard<std::mutex> lock(mLock);
    if (!mStream) return;

    mStream->stop();
    mStream->close();
    mStream.reset();

    if (mEncoder) {
        opus_encoder_destroy(mEncoder);
        mEncoder = nullptr;
    }

    if (mSocket >= 0) {
        close(mSocket);
        mSocket = -1;
    }
    __android_log_print(ANDROID_LOG_INFO, "AudioEngine", "Streaming pipeline stopped");
}

oboe::DataCallbackResult AudioEngine::onAudioReady(
        oboe::AudioStream *oboeStream,
        void *audioData,
        int32_t numFrames) {

    auto* pcmData = static_cast<const opus_int16*>(audioData);

    // Point the opus payload buffer to just after the header in our send buffer
    unsigned char* opusPayload = mSendBuffer + sizeof(PacketHeader);
    const opus_int32 opusPayloadMaxSize = sizeof(mSendBuffer) - sizeof(PacketHeader);

    opus_int32 encodedBytes = opus_encode(
            mEncoder,
            pcmData,
            numFrames,
            opusPayload,
            opusPayloadMaxSize
    );

    if (encodedBytes < 0) {
        __android_log_print(ANDROID_LOG_ERROR, "AudioEngine", "Opus encode failed: %s", opus_strerror(encodedBytes));
        return oboe::DataCallbackResult::Continue;
    }

    if (encodedBytes > 0) {
        // Create the header
        PacketHeader header;
        header.sequence = htons(mSequenceNumber++); // Use network byte order
        header.timestamp = htonl(get_monotonic_time_ms()); // Use network byte order

        // Copy header into the start of the send buffer
        memcpy(mSendBuffer, &header, sizeof(PacketHeader));

        // Send the complete packet (header + opus data)
        ssize_t bytesSent = sendto(mSocket, mSendBuffer, sizeof(PacketHeader) + encodedBytes, 0,
                                   (struct sockaddr *) &mTargetAddress, sizeof(mTargetAddress));

        if (bytesSent < 0) {
            __android_log_print(ANDROID_LOG_WARN, "AudioEngine", "Network send failed: %s", strerror(errno));
        }
    }

    return oboe::DataCallbackResult::Continue;
}

void AudioEngine::onErrorBeforeClose(oboe::AudioStream *oboeStream, oboe::Result error) {
    __android_log_print(ANDROID_LOG_ERROR, "AudioEngine", "Error before close: %s", oboe::convertToText(error));
}

void AudioEngine::onErrorAfterClose(oboe::AudioStream *oboeStream, oboe::Result error) {
    __android_log_print(ANDROID_LOG_ERROR, "AudioEngine", "Error after close: %s", oboe::convertToText(error));
    stop();
}