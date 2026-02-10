#include "DisplayManager.h"

#ifdef HAS_OLED
    #include <U8x8lib.h>

    // --- CONSTRUCTOR SELECTOR ---
    // TRY THIS FIRST (Standard Heltec V1/V2):
    // Syntax: Clock=15, Data=4, Reset=16
    //U8X8_SSD1306_128X64_NONAME_SW_I2C u8x8(15, 4, 16);
    
    // IF THE SCREEN IS STILL BLACK, COMMENT OUT THE LINE ABOVE
    // AND UNCOMMENT THIS ONE (Some older boards use SH1106):
     U8X8_SH1106_128X64_NONAME_SW_I2C u8x8(15, 4, 16);
#endif

void initDisplay() {
    #ifdef HAS_OLED
        Serial.print("Init OLED (U8x8)... ");

        // --- 1. MANUAL HARDWARE RESET (CRITICAL FOR V1) ---
        // The library sometimes misses this timing, so we force it.
        pinMode(16, OUTPUT);
        digitalWrite(16, LOW);  // Reset
        delay(50);              // Hold
        digitalWrite(16, HIGH); // Release
        delay(50);              // Wait for boot
        
        // --- 2. INITIALIZE ---
        if (!u8x8.begin()) {
            Serial.println("FAILED!");
        } else {
            Serial.println("OK!");
        }
        
        // --- 3. CONFIGURATION ---
        u8x8.setPowerSave(0); // Wake up
        u8x8.setFlipMode(1);  // Flips screen 180 (Common for Heltec)
        u8x8.setFont(u8x8_font_chroma48medium8_r);
        
        // --- 4. IMMEDIATE TEST PATTERN ---
        u8x8.clearDisplay();
        
        // Fill the screen with 'X' to prove it's alive
        for(int i=0; i<16; i++) {
            u8x8.drawString(0, i, "XXXXXXXXXXXXXXXX"); 
        }
        delay(500); // Pause to let you see it
        
        u8x8.clearDisplay();
        u8x8.drawString(0, 0, "K9 MONITOR");
        u8x8.drawString(0, 2, "Display Active");
    #endif
}

void updateDisplay(SystemState &state) {
    #ifdef HAS_OLED
        // Do not use clearDisplay() here if possible to avoid flickering.
        // Instead, pad strings with spaces to overwrite old data.
        
        u8x8.setFont(u8x8_font_chroma48medium8_r);

        // Line 0: Network
        if (state.apMode) {
             u8x8.drawString(0, 0, "AP MODE        ");
        } else if (state.wifiConnected) {
             u8x8.drawString(0, 0, "WIFI: CONNECTED");
        } else {
             u8x8.drawString(0, 0, "CONNECTING...  ");
        }

        // Line 2: IP Address (Big Font)
        u8x8.setFont(u8x8_font_profont29_2x3_r); 
        u8x8.setCursor(0, 2);
        u8x8.print(state.ipAddress);

        // Line 5: Status
        u8x8.setFont(u8x8_font_chroma48medium8_r);
        if (state.seizureDetected) {
            u8x8.inverse(); 
            u8x8.drawString(0, 5, "!! SEIZURE !! ");
            u8x8.noInverse();
        } else if (state.isPaused) {
            u8x8.drawString(0, 5, "STATUS: PAUSED");
        } else {
            u8x8.drawString(0, 5, "STATUS: ACTIVE");
        }

        // Line 7: Vitals
        char buf[20];
        snprintf(buf, sizeof(buf), "HR:%-3d SP:%-3d%%", (int)state.heartRate, (int)state.spo2);
        u8x8.drawString(0, 7, buf);
    #endif
}