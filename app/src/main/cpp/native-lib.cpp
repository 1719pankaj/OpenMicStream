#include <jni.h>
#include <string>
#include "AudioEngine.h"

extern "C" {

JNIEXPORT jlong JNICALL
Java_com_example_openmicstream_audio_AudioEngine_native_1create(JNIEnv *env, jobject thiz) {
    auto* engine = new AudioEngine();
    return reinterpret_cast<jlong>(engine);
}

// MODIFIED: Now accepts IP and port
JNIEXPORT jint JNICALL
Java_com_example_openmicstream_audio_AudioEngine_native_1start(
        JNIEnv *env,
        jobject thiz,
        jlong engine_handle,
        jstring target_ip,
        jint target_port) {
    auto* engine = reinterpret_cast<AudioEngine*>(engine_handle);
    if (engine == nullptr) {
        return -1;
    }
    // Convert jstring to C-style string
    const char* ip = env->GetStringUTFChars(target_ip, nullptr);
    int32_t result = engine->start(ip, target_port);
    // Release the string memory
    env->ReleaseStringUTFChars(target_ip, ip);
    return result;
}

JNIEXPORT void JNICALL
Java_com_example_openmicstream_audio_AudioEngine_native_1stop(JNIEnv *env, jobject thiz, jlong engine_handle) {
    auto* engine = reinterpret_cast<AudioEngine*>(engine_handle);
    if (engine != nullptr) {
        engine->stop();
    }
}

JNIEXPORT void JNICALL
Java_com_example_openmicstream_audio_AudioEngine_native_1destroy(JNIEnv *env, jobject thiz, jlong engine_handle) {
    auto* engine = reinterpret_cast<AudioEngine*>(engine_handle);
    if (engine != nullptr) {
        delete engine;
    }
}

} // extern "C"