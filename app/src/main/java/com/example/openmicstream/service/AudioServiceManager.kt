package com.example.openmicstream.service

import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.asStateFlow
import javax.inject.Inject
import javax.inject.Singleton

/**
 * Manages the state of the AudioService and exposes it to the UI layer.
 * This acts as the single source of truth for whether streaming is active.
 */
@Singleton
class AudioServiceManager @Inject constructor() {
    private val _serviceState = MutableStateFlow(AudioServiceState())
    val serviceState = _serviceState.asStateFlow()

    fun setStreaming(isStreaming: Boolean) {
        _serviceState.value = _serviceState.value.copy(isStreaming = isStreaming)
    }
}