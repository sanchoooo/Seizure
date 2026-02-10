#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>


// --- PINS ---
#if defined(ESP32C3)
//ESP32c3
    #define I2C_SDA 6
    #define I2C_SCL 7
    #define PULSE_INT_PIN 3 
#elif defined(HELTEC_V1)
// Heltec WiFi Kit 32 V1
    #define I2C_SDA 21
    #define I2C_SCL 22
    #define PULSE_INT_PIN 4  // Free GPIO
    #define OLED_SDA 4
    #define OLED_SCL 15
    #define OLED_RST 16
    #define HAS_OLED
    #pragma message "Compiling for Heltec V1 (SDA:21, SCL:22)"
#else
    // Fallback or Unknown Board
    #error "Board type not defined! Please select heltec_v2 or heltec_v3 in platformio.ini"
#endif

// --- WIFI ---
#define WIFI_SSID "HereKittyKitty"
#define WIFI_PASS "xboxrocks"
#define AP_SSID   "K9-Monitor"

// --- LOGIC THRESHOLDS ---
#define SEIZURE_G_FORCE_THRESH 3.5
#define SEIZURE_DURATION_MS    4000

// --- SHARED DATA STRUCTURE ---
struct SystemState {
    bool isPaused = false;
    bool seizureDetected = false;
    unsigned long seizureStartTime = 0;
    
    // Motion Sensor Data (LSM6DSOX)
    float gForce = 0.0;
    float gyroX = 0.0;
    float gyroY = 0.0;
    float gyroZ = 0.0;
    float temperature = 0.0; // Ambient / Board Temp
    
    // Health Sensor Data (MAX30102)
    int32_t heartRate = 0;
    int32_t spo2 = 0;
    int8_t validHR = 0; 
    int8_t validSPO2 = 0;
    float healthTemp = 0.0;  // <--- NEW: Skin Temperature
    
    bool wifiConnected = false;
    bool bleConnected = false;
    bool apMode = false;          
    String ipAddress = "0.0.0.0";
};

#endif