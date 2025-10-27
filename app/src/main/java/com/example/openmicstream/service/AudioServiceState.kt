package com.example.openmicstream.service

/**
 * A simple data class to hold the current state of the audio service.
 */
data class AudioServiceState(
    val isStreaming: Boolean = false
)