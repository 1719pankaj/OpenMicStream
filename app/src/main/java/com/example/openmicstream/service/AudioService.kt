package com.example.openmicstream.service

import android.app.Notification
import android.app.NotificationChannel
import android.app.NotificationManager
import android.app.PendingIntent
import android.content.Intent
import android.os.Build
import android.os.IBinder
import androidx.core.app.NotificationCompat
import com.example.openmicstream.MainActivity
import com.example.openmicstream.R
import com.example.openmicstream.audio.AudioEngine
import dagger.hilt.android.AndroidEntryPoint
import javax.inject.Inject

@AndroidEntryPoint
class AudioService : android.app.Service() {

    @Inject
    lateinit var serviceManager: AudioServiceManager

    @Inject
    lateinit var audioEngine: AudioEngine

    private lateinit var notificationManager: NotificationManager

    companion object {
        const val NOTIFICATION_ID = 1
        const val NOTIFICATION_CHANNEL_ID = "OpenMicStreamChannel"
        const val ACTION_STOP = "com.example.openmicstream.ACTION_STOP"
    }

    override fun onCreate() {
        super.onCreate()
        notificationManager = getSystemService(NOTIFICATION_SERVICE) as NotificationManager
        createNotificationChannel()
    }

    override fun onStartCommand(intent: Intent?, flags: Int, startId: Int): Int {
        // Handle the stop action from the notification
        if (intent?.action == ACTION_STOP) {
            // Tell the system to stop the service. This will trigger onDestroy().
            stopSelf()
            return START_NOT_STICKY
        }

        // Attempt to start the native audio engine
        if (!audioEngine.isStarted && !audioEngine.start()) {
            // If the engine fails to start, stop the service immediately.
            stopSelf()
            return START_NOT_STICKY
        }

        // If engine started successfully, promote to foreground service
        startForeground(NOTIFICATION_ID, createNotification())
        serviceManager.setStreaming(true)

        return START_STICKY
    }

    // THIS IS THE GUARANTEED CLEANUP METHOD
    override fun onDestroy() {
        // Stop and release the native AudioEngine FIRST.
        if (audioEngine.isStarted) {
            audioEngine.stop()
        }

        // Update the global state.
        serviceManager.setStreaming(false)

        // Clean up the foreground service state.
        stopForeground(STOP_FOREGROUND_REMOVE)

        super.onDestroy()
    }

    private fun createNotification(): Notification {
        val pendingIntent = Intent(this, MainActivity::class.java).let { notificationIntent ->
            PendingIntent.getActivity(this, 0, notificationIntent, PendingIntent.FLAG_IMMUTABLE)
        }

        val stopSelf = Intent(this, AudioService::class.java).apply {
            action = ACTION_STOP
        }
        val pStopSelf = PendingIntent.getService(this, 0, stopSelf, PendingIntent.FLAG_IMMUTABLE or PendingIntent.FLAG_CANCEL_CURRENT)

        return NotificationCompat.Builder(this, NOTIFICATION_CHANNEL_ID)
            .setContentTitle("OpenMicStream Active")
            .setContentText("Streaming audio to your PC...")
            .setSmallIcon(R.drawable.ic_launcher_foreground)
            .setContentIntent(pendingIntent)
            .addAction(R.drawable.ic_launcher_foreground, "Stop", pStopSelf)
            .setOngoing(true)
            .build()
    }
    private fun createNotificationChannel() {
        val serviceChannel = NotificationChannel(
            NOTIFICATION_CHANNEL_ID,
            "OpenMicStream Service Channel",
            NotificationManager.IMPORTANCE_LOW
        )
        notificationManager.createNotificationChannel(serviceChannel)
    }

    override fun onBind(intent: Intent?): IBinder? {
        return null
    }
}