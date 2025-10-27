package com.example.openmicstream.ui.main

import android.Manifest
import android.content.Intent
import android.net.Uri
import android.os.Build
import android.provider.Settings
import androidx.compose.foundation.layout.*
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.platform.LocalContext
import androidx.compose.ui.text.style.TextAlign
import androidx.compose.ui.unit.dp
import androidx.hilt.navigation.compose.hiltViewModel
import com.example.openmicstream.service.AudioService
import com.google.accompanist.permissions.ExperimentalPermissionsApi
import com.google.accompanist.permissions.MultiplePermissionsState
import com.google.accompanist.permissions.rememberMultiplePermissionsState

@OptIn(ExperimentalPermissionsApi::class)
@Composable
fun MainScreen(
    viewModel: MainViewModel = hiltViewModel()
) {
    val context = LocalContext.current
    val uiState by viewModel.uiState.collectAsState()

    // Group permissions together: Audio and (on newer devices) Notifications
    val permissions = remember {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.TIRAMISU) {
            listOf(Manifest.permission.RECORD_AUDIO, Manifest.permission.POST_NOTIFICATIONS)
        } else {
            listOf(Manifest.permission.RECORD_AUDIO)
        }
    }
    val permissionsState = rememberMultiplePermissionsState(permissions = permissions)

    // This LaunchedEffect will trigger the permission request when the screen is first composed
    // if the permissions have not already been granted. This fixes the initial launch issue.
    LaunchedEffect(permissionsState) {
        if (!permissionsState.allPermissionsGranted) {
            permissionsState.launchMultiplePermissionRequest()
        }
    }

    Surface(
        modifier = Modifier.fillMaxSize(),
        color = MaterialTheme.colorScheme.background
    ) {
        Column(
            modifier = Modifier
                .fillMaxSize()
                .padding(16.dp),
            verticalArrangement = Arrangement.Center,
            horizontalAlignment = Alignment.CenterHorizontally
        ) {
            if (permissionsState.allPermissionsGranted) {
                // Permissions are granted, show the main controls
                MainControls(isStreaming = uiState.isStreaming) {
                    val intent = Intent(context, AudioService::class.java)
                    if (uiState.isStreaming) {
                        context.stopService(intent)
                    } else {
                        // For Android 12+, we must start foreground services from the foreground
                        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.S) {
                            context.startForegroundService(intent)
                        } else {
                            context.startService(intent)
                        }
                    }
                }
            } else {
                // If we're here, it means the user has explicitly denied permission at least once.
                PermissionRationale(permissionsState)
            }
        }
    }
}

@Composable
fun MainControls(isStreaming: Boolean, onToggleStreaming: () -> Unit) {
    Text(
        text = if (isStreaming) "Streaming..." else "Ready to stream",
        style = MaterialTheme.typography.headlineSmall,
        color = if (isStreaming) MaterialTheme.colorScheme.primary else MaterialTheme.colorScheme.onSurface
    )
    Spacer(modifier = Modifier.height(32.dp))
    Button(
        onClick = onToggleStreaming,
        modifier = Modifier.size(width = 200.dp, height = 50.dp)
    ) {
        Text(if (isStreaming) "Stop Streaming" else "Start Streaming")
    }
}

@OptIn(ExperimentalPermissionsApi::class)
@Composable
fun PermissionRationale(permissionsState: MultiplePermissionsState) {
    val context = LocalContext.current

    // After the initial denial, shouldShowRationale will be true. If the user
    // denies again with "Don't ask again", it will become false.
    // This logic now correctly handles the post-denial flow.
    val isPermanentlyDenied = !permissionsState.shouldShowRationale && !permissionsState.allPermissionsGranted

    val text = if (isPermanentlyDenied) {
        "Audio and Notification permissions are essential for this app to function. You have permanently denied them. Please enable them in the system settings to continue."
    } else {
        "To use your phone as a microphone, this app needs access to record audio and show notifications for the background service. Please grant these permissions."
    }
    val buttonText = if (isPermanentlyDenied) "Open Settings" else "Grant Permissions"

    Text(text = "Permissions Required", style = MaterialTheme.typography.headlineSmall)
    Spacer(modifier = Modifier.height(16.dp))
    Text(text = text, textAlign = TextAlign.Center)
    Spacer(modifier = Modifier.height(32.dp))
    Button(onClick = {
        if (isPermanentlyDenied) {
            // Open app settings
            val intent = Intent(Settings.ACTION_APPLICATION_DETAILS_SETTINGS).apply {
                data = Uri.fromParts("package", context.packageName, null)
            }
            context.startActivity(intent)
        } else {
            permissionsState.launchMultiplePermissionRequest()
        }
    }) {
        Text(buttonText)
    }
}