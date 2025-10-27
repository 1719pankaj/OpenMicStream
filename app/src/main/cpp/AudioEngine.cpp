#include "AudioEngine.h"

AudioEngine::AudioEngine() : mEncoder(nullptr) { // Initialize mEncoder to null
    __android_log_print(ANDROID_LOG_INFO, "AudioEngine", "Instance created");
}

AudioEngine::~AudioEngine() {
    // Ensure cleanup happens even if stop() wasn't called
    if (mEncoder) {
        opus_encoder_destroy(mEncoder);
    }
    __android_log_print(ANDROID_LOG_INFO, "AudioEngine", "Instance destroyed");
}

int32_t AudioEngine::start() {
    std::lock_guard<std::mutex> lock(mLock);
    if (mStream) return 0; // Already running

    // 1. Create Opus Encoder
    int error;
    mEncoder = opus_encoder_create(kSampleRate, kChannelCount, OPUS_APPLICATION_AUDIO, &error);
    if (error != OPUS_OK) {
        __android_log_print(ANDROID_LOG_ERROR, "AudioEngine", "Failed to create Opus encoder: %s", opus_strerror(error));
        return -1;
    }
    // Set a bitrate. 64kbps is a good starting point for high quality voice.
    opus_encoder_ctl(mEncoder, OPUS_SET_BITRATE(64000));
    opus_encoder_ctl(mEncoder, OPUS_SET_VBR(1)); // Enable VBR

    // 2. Create and start Oboe stream
    oboe::AudioStreamBuilder builder;
    builder.setDirection(oboe::Direction::Input)
            ->setPerformanceMode(oboe::PerformanceMode::LowLatency)
            ->setSharingMode(oboe::SharingMode::Exclusive)
            ->setFormat(oboe::AudioFormat::I16)
            ->setChannelCount(kChannelCount)
            ->setSampleRate(kSampleRate)
                    // Request a specific frame count for consistent buffer sizes
            ->setFramesPerCallback(kFrameSize)
            ->setDataCallback(this)
            ->setErrorCallback(this);

    oboe::Result result = builder.openManagedStream(mStream);
    if (result != oboe::Result::OK) {
        __android_log_print(ANDROID_LOG_ERROR, "AudioEngine", "Failed to create stream. Error: %s", oboe::convertToText(result));
        return -2;
    }

    result = mStream->requestStart();
    if (result != oboe::Result::OK) {
        __android_log_print(ANDROID_LOG_ERROR, "AudioEngine", "Failed to start stream. Error: %s", oboe::convertToText(result));
        return -3;
    }

    __android_log_print(ANDROID_LOG_INFO, "AudioEngine", "Stream and encoder started successfully");
    return 0;
}

void AudioEngine::stop() {
    std::lock_guard<std::mutex> lock(mLock);
    if (!mStream) return; // Already stopped

    mStream->stop();
    mStream->close();
    mStream.reset(); // Important: release the managed stream

    if (mEncoder) {
        opus_encoder_destroy(mEncoder);
        mEncoder = nullptr;
    }
    __android_log_print(ANDROID_LOG_INFO, "AudioEngine", "Stream and encoder stopped");
}

oboe::DataCallbackResult AudioEngine::onAudioReady(
        oboe::AudioStream *oboeStream,
        void *audioData,
        int32_t numFrames) {

    if (numFrames != kFrameSize) {
        __android_log_print(ANDROID_LOG_WARN, "AudioEngine", "Unexpected frame count: %d", numFrames);
    }

    // The PCM data from Oboe
    auto* pcmData = static_cast<const opus_int16*>(audioData);

    // Encode the PCM data into our packet buffer
    opus_int32 encodedBytes = opus_encode(
            mEncoder,
            pcmData,
            numFrames,
            mOpusPacket,
            sizeof(mOpusPacket)
    );

    if (encodedBytes < 0) {
        __android_log_print(ANDROID_LOG_ERROR, "AudioEngine", "Opus encode failed: %s", opus_strerror(encodedBytes));
    } else if (encodedBytes > 0) {
        // SUCCESS: We have an encoded packet.
        // For now, we just log its size. In the next step, we'll send it.
        __android_log_print(ANDROID_LOG_VERBOSE, "AudioEngine", "Encoded %d PCM frames into %d Opus bytes", numFrames, encodedBytes);
        // TODO: Pass mOpusPacket (and encodedBytes) to the network sender.
    }

    return oboe::DataCallbackResult::Continue;
}

void AudioEngine::onErrorBeforeClose(oboe::AudioStream *oboeStream, oboe::Result error) {
    __android_log_print(ANDROID_LOG_ERROR, "AudioEngine", "Error before close: %s", oboe::convertToText(error));
}

void AudioEngine::onErrorAfterClose(oboe::AudioStream *oboeStream, oboe::Result error) {
    __android_log_print(ANDROID_LOG_ERROR, "AudioEngine", "Error after close: %s", oboe::convertToText(error));
    // We can attempt to restart the stream here if needed
    stop(); // Stop completely on error
}