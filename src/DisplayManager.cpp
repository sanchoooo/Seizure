#include "DisplayManager.h"

#ifdef HAS_OLED
    #include <U8x8lib.h>

    // --- CONSTRUCTOR ---
    // Software I2C: Clock=15, Data=4, Reset=16 (Standard Heltec V1)
    U8X8_SSD1306_128X64_NONAME_SW_I2C u8x8(15, 4, 16);
#endif

void initDisplay() {
    #ifdef HAS_OLED
        Serial.print("Init OLED (U8x8 SW)... ");

        // 1. HARDWARE RESET
        pinMode(16, OUTPUT);
        digitalWrite(16, LOW);
        delay(50);
        digitalWrite(16, HIGH);
        delay(50);

        // 2. START
        if (u8x8.begin()) {
            Serial.println("OK!");
        } else {
            Serial.println("FAILED!");
        }
        
        // 3. WAKE & CONFIG
        u8x8.setPowerSave(0); 
        u8x8.setFlipMode(1); 
        u8x8.setFont(u8x8_font_chroma48medium8_r);
        
        u8x8.clearDisplay();
        u8x8.drawString(0, 0, "K9 MONITOR");
        u8x8.drawString(0, 2, "System Init...");
    #endif
}

void updateDisplay(SystemState &state) {
    #ifdef HAS_OLED
        u8x8.setFont(u8x8_font_chroma48medium8_r);

        // --- LINE 0: NETWORK STATUS ---
        if (state.apMode) {
             u8x8.drawString(0, 0, "AP MODE        ");
        } else if (state.wifiConnected) {
             u8x8.drawString(0, 0, "WIFI: CONNECTED");
        } else {
             u8x8.drawString(0, 0, "CONNECTING...  ");
        }

        // --- LINE 2: IP ADDRESS HEADER ---
        u8x8.drawString(0, 2, "IP ADDRESS:");

        // --- LINE 3: THE ACTUAL IP (SMALLER FONT) ---
        // Using the standard font ensures 192.168.xxx.xxx fits perfectly
        u8x8.drawString(0, 3, state.ipAddress.c_str());

        // --- LINE 5: STATUS ---
        if (state.seizureDetected) {
            u8x8.inverse(); 
            u8x8.drawString(0, 5, "!! SEIZURE !! ");
            u8x8.noInverse();
        } else if (state.isPaused) {
            u8x8.drawString(0, 5, "STATUS: PAUSED");
        } else {
            u8x8.drawString(0, 5, "STATUS: ACTIVE");
        }

        // --- LINE 7: VITALS ---
        char buf[20];
        snprintf(buf, sizeof(buf), "HR:%-3d SP:%-3d%%", (int)state.heartRate, (int)state.spo2);
        u8x8.drawString(0, 7, buf);

    #endif
}