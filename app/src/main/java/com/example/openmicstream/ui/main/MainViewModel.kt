package com.example.openmicstream.ui.main

import androidx.lifecycle.ViewModel
import androidx.lifecycle.viewModelScope
import com.example.openmicstream.data.SettingsRepository
import com.example.openmicstream.service.AudioServiceManager
import dagger.hilt.android.lifecycle.HiltViewModel
import kotlinx.coroutines.flow.*
import javax.inject.Inject

/**
 * Represents the complete state of the main UI.
 */
data class MainUiState(
    val isStreaming: Boolean = false,
    val targetIp: String = ""
)

@HiltViewModel
class MainViewModel @Inject constructor(private val settingsRepository: SettingsRepository, audioServiceManager: AudioServiceManager) : ViewModel() {
    private val _targetIp = MutableStateFlow(settingsRepository.getTargetIp())

    // Combine the streaming state and IP state into a single UI state flow.
    val uiState: StateFlow<MainUiState> = combine(
        audioServiceManager.serviceState,
        _targetIp
    ) { serviceState, ip ->
        MainUiState(
            isStreaming = serviceState.isStreaming,
            targetIp = ip
        )
    }.stateIn(
        scope = viewModelScope,
        started = SharingStarted.WhileSubscribed(5000),
        initialValue = MainUiState()
    )

    /**
     * Called by the UI when the user types in the IP address field.
     */
    fun onTargetIpChanged(newIp: String) {
        _targetIp.value = newIp
        settingsRepository.saveTargetIp(newIp)
    }
}