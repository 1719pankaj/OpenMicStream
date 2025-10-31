package com.example.openmicstream.data

import android.content.Context
import dagger.hilt.android.qualifiers.ApplicationContext
import javax.inject.Inject
import javax.inject.Singleton

private const val PREFS_NAME = "OpenMicStreamPrefs"
private const val KEY_TARGET_IP = "target_ip"
private const val DEFAULT_IP = "192.168.29.11"

@Singleton
class SettingsRepository @Inject constructor(
    @ApplicationContext private val context: Context
) {
    private val sharedPreferences = context.getSharedPreferences(PREFS_NAME, Context.MODE_PRIVATE)

    fun getTargetIp(): String {
        return sharedPreferences.getString(KEY_TARGET_IP, DEFAULT_IP) ?: DEFAULT_IP
    }

    fun saveTargetIp(ipAddress: String) {
        sharedPreferences.edit().putString(KEY_TARGET_IP, ipAddress).apply()
    }
}