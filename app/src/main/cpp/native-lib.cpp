#include <jni.h>
#include <string>
#include "AudioEngine.h"

// JNI entry points
extern "C" {

// Create and return a pointer to the AudioEngine.
JNIEXPORT jlong JNICALL
Java_com_example_openmicstream_audio_AudioEngine_native_1create(JNIEnv *env, jobject thiz) {
    auto* engine = new AudioEngine();
    // We cast the pointer to a jlong (which is a 64-bit integer) to return it to Kotlin.
    return reinterpret_cast<jlong>(engine);
}

// Start the audio engine.
JNIEXPORT jint JNICALL
Java_com_example_openmicstream_audio_AudioEngine_native_1start(JNIEnv *env, jobject thiz, jlong engine_handle) {
    auto* engine = reinterpret_cast<AudioEngine*>(engine_handle);
    if (engine == nullptr) {
        return -1; // Indicate error
    }
    return engine->start();
}

// Stop the audio engine.
JNIEXPORT void JNICALL
Java_com_example_openmicstream_audio_AudioEngine_native_1stop(JNIEnv *env, jobject thiz, jlong engine_handle) {
    auto* engine = reinterpret_cast<AudioEngine*>(engine_handle);
    if (engine != nullptr) {
        engine->stop();
    }
}

// Destroy the audio engine instance.
JNIEXPORT void JNICALL
Java_com_example_openmicstream_audio_AudioEngine_native_1destroy(JNIEnv *env, jobject thiz, jlong engine_handle) {
    auto* engine = reinterpret_cast<AudioEngine*>(engine_handle);
    if (engine != nullptr) {
        delete engine;
    }
}

} // extern "C"