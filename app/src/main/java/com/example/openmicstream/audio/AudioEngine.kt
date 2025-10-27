package com.example.openmicstream.audio

import android.util.Log
import javax.inject.Inject
import javax.inject.Singleton

private const val TAG = "AudioEngine"

/**
 * A Kotlin wrapper for the native C++ AudioEngine.
 * This class manages the lifecycle of the native engine instance.
 */
@Singleton
class AudioEngine @Inject constructor() {

    // This holds the memory address of the C++ AudioEngine instance.
    private var nativeHandle: Long = 0

    val isStarted: Boolean
        get() = nativeHandle != 0L

    fun start(): Boolean {
        if (isStarted) {
            Log.w(TAG, "Attempted to start an already running engine.")
            return true
        }
        nativeHandle = native_create()
        if (nativeHandle == 0L) {
            Log.e(TAG, "Failed to create native engine.")
            return false
        }

        val result = native_start(nativeHandle)
        if (result != 0) {
            Log.e(TAG, "Failed to start native engine. Error code: $result")
            // Clean up if start fails
            native_destroy(nativeHandle)
            nativeHandle = 0
            return false
        }
        Log.i(TAG, "AudioEngine started successfully.")
        return true
    }

    fun stop() {
        if (!isStarted) {
            Log.w(TAG, "Attempted to stop an engine that is not running.")
            return
        }
        native_stop(nativeHandle)
        native_destroy(nativeHandle)
        nativeHandle = 0
        Log.i(TAG, "AudioEngine stopped.")
    }

    // --- Native Method Declarations ---
    // These link to the JNI functions in native-lib.cpp

    private external fun native_create(): Long
    private external fun native_start(engineHandle: Long): Int
    private external fun native_stop(engineHandle: Long)
    private external fun native_destroy(engineHandle: Long)
}