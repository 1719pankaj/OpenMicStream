package com.example.openmicstream.ui.main

import androidx.lifecycle.ViewModel
import androidx.lifecycle.viewModelScope
import com.example.openmicstream.service.AudioServiceManager
import com.example.openmicstream.service.AudioServiceState
import dagger.hilt.android.lifecycle.HiltViewModel
import kotlinx.coroutines.flow.SharingStarted
import kotlinx.coroutines.flow.StateFlow
import kotlinx.coroutines.flow.stateIn
import javax.inject.Inject

@HiltViewModel
class MainViewModel @Inject constructor(
    audioServiceManager: AudioServiceManager
) : ViewModel() {

    // Expose the service state as UI state
    val uiState: StateFlow<AudioServiceState> = audioServiceManager.serviceState
        .stateIn(
            scope = viewModelScope,
            started = SharingStarted.WhileSubscribed(5000),
            initialValue = AudioServiceState()
        )
}