#ifndef OPENMICSTREAM_AUDIOENGINE_H
#define OPENMICSTREAM_AUDIOENGINE_H

#include <oboe/Oboe.h>
#include <android/log.h>
#include <mutex>
#include "opus.h" // Include the Opus header

class AudioEngine : public oboe::AudioStreamDataCallback, public oboe::AudioStreamErrorCallback {
public:
    AudioEngine();
    ~AudioEngine();

    int32_t start();
    void stop();

    // Oboe callbacks
    oboe::DataCallbackResult onAudioReady(
            oboe::AudioStream *oboeStream,
            void *audioData,
            int32_t numFrames) override;

    void onErrorBeforeClose(oboe::AudioStream *oboeStream, oboe::Result error) override;
    void onErrorAfterClose(oboe::AudioStream *oboeStream, oboe::Result error) override;

private:
    // Audio constants
    static constexpr int32_t kSampleRate = 48000;
    static constexpr int32_t kChannelCount = 1; // Mono
    static constexpr int32_t kFrameSize = 960; // 20ms at 48kHz (48000 * 0.020)

    // Oboe
    oboe::ManagedStream mStream;
    std::mutex mLock;

    // Opus
    OpusEncoder *mEncoder;
    // Buffer to hold encoded Opus data. 1500 is a safe size for one frame.
    unsigned char mOpusPacket[1500];
};

#endif //OPENMICSTREAM_AUDIOENGINE_H