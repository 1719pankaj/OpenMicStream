#ifndef OPENMICSTREAM_AUDIOENGINE_H
#define OPENMICSTREAM_AUDIOENGINE_H

#include <oboe/Oboe.h>
#include <android/log.h>
#include <mutex>
#include <chrono> // For timestamps
#include "opus.h"

// For networking
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>


// Our custom packet header
#pragma pack(push, 1)
struct PacketHeader {
    uint8_t version = 1;
    uint8_t flags = 0;
    uint16_t sequence = 0;
    uint32_t timestamp = 0;
    uint32_t auth_id = 12345; // Placeholder auth ID
};
#pragma pack(pop)


class AudioEngine : public oboe::AudioStreamDataCallback, public oboe::AudioStreamErrorCallback {
public:
    AudioEngine();
    ~AudioEngine();

    // Now takes target IP and port
    int32_t start(const char* targetIp, int targetPort);
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
    static constexpr int32_t kChannelCount = 1;
    static constexpr int32_t kFrameSize = 960;

    // Oboe
    oboe::ManagedStream mStream;
    std::mutex mLock;

    // Opus
    OpusEncoder *mEncoder;

    // Network
    int mSocket = -1;
    struct sockaddr_in mTargetAddress{};
    uint16_t mSequenceNumber = 0;
    std::chrono::time_point<std::chrono::steady_clock> mStartTime;

    // Combined buffer for header + opus data
    unsigned char mSendBuffer[1500];
};

#endif //OPENMICSTREAM_AUDIOENGINE_H