package com.example.openmicstream

import android.app.Application
import dagger.hilt.android.HiltAndroidApp

@HiltAndroidApp
class OpenMicStreamApplication : Application() {
    companion object {
        init {
            // Load the native library as soon as the app starts.
            System.loadLibrary("openmicstream")
        }
    }
}