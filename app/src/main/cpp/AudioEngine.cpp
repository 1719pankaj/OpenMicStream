#include "AudioEngine.h"

AudioEngine::AudioEngine() {
    __android_log_print(ANDROID_LOG_INFO, "AudioEngine", "Instance created");
}

AudioEngine::~AudioEngine() {
    __android_log_print(ANDROID_LOG_INFO, "AudioEngine", "Instance destroyed");
}

int32_t AudioEngine::start() {
    std::lock_guard<std::mutex> lock(mLock);

    oboe::AudioStreamBuilder builder;
    builder.setDirection(oboe::Direction::Input)
            ->setPerformanceMode(oboe::PerformanceMode::LowLatency)
            ->setSharingMode(oboe::SharingMode::Exclusive)
            ->setFormat(oboe::AudioFormat::I16)
            ->setChannelCount(oboe::ChannelCount::Mono)
            ->setSampleRate(48000)
            ->setDataCallback(this)
            ->setErrorCallback(this);

    // THIS IS THE CORRECT METHOD CALL
    oboe::Result result = builder.openManagedStream(mStream);
    if (result != oboe::Result::OK) {
        __android_log_print(ANDROID_LOG_ERROR, "AudioEngine", "Failed to create stream. Error: %s",
                            oboe::convertToText(result));
        return -1;
    }

    result = mStream->requestStart();
    if (result != oboe::Result::OK) {
        __android_log_print(ANDROID_LOG_ERROR, "AudioEngine", "Failed to start stream. Error: %s",
                            oboe::convertToText(result));
        return -2;
    }

    __android_log_print(ANDROID_LOG_INFO, "AudioEngine", "Stream started successfully");
    return 0; // Success
}

void AudioEngine::stop() {
    std::lock_guard<std::mutex> lock(mLock);
    if (mStream) {
        mStream->stop();
        mStream->close();
    }
    __android_log_print(ANDROID_LOG_INFO, "AudioEngine", "Stream stopped");
}

oboe::DataCallbackResult AudioEngine::onAudioReady(
        oboe::AudioStream *oboeStream,
        void *audioData,
        int32_t numFrames) {

    __android_log_print(ANDROID_LOG_VERBOSE, "AudioEngine", "onAudioReady received %d frames", numFrames);

    auto* pcmData = static_cast<int16_t*>(audioData);
    // TODO: Pass pcmData to the Opus encoder.
    return oboe::DataCallbackResult::Continue;
}

void AudioEngine::onErrorBeforeClose(oboe::AudioStream *oboeStream, oboe::Result error) {
    __android_log_print(ANDROID_LOG_ERROR, "AudioEngine", "Error before close: %s",
                        oboe::convertToText(error));
}

void AudioEngine::onErrorAfterClose(oboe::AudioStream *oboeStream, oboe::Result error) {
    __android_log_print(ANDROID_LOG_ERROR, "AudioEngine", "Error after close: %s",
                        oboe::convertToText(error));
}