#ifndef OPENMICSTREAM_AUDIOENGINE_H
#define OPENMICSTREAM_AUDIOENGINE_H

#include <oboe/Oboe.h>
#include <android/log.h>
#include <mutex>

class AudioEngine : public oboe::AudioStreamDataCallback, public oboe::AudioStreamErrorCallback {
public:
    AudioEngine();
    ~AudioEngine();

    int32_t start();
    void stop();

    // Oboe Callbacks
    oboe::DataCallbackResult onAudioReady(
            oboe::AudioStream *oboeStream,
            void *audioData,
            int32_t numFrames) override;

    void onErrorBeforeClose(oboe::AudioStream *oboeStream, oboe::Result error) override;
    void onErrorAfterClose(oboe::AudioStream *oboeStream, oboe::Result error) override;


private:
    oboe::ManagedStream mStream;
    std::mutex mLock;
};

#endif //OPENMICSTREAM_AUDIOENGINE_H