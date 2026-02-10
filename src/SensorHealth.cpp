#include "SensorHealth.h"
#include <Wire.h>

MAX30105 particleSensor;

const long IR_THRESHOLD = 50000; 

// Buffers
uint32_t irBuffer[100];
uint32_t redBuffer[100];
int32_t bufferLength = 100;
int bufferIndex = 0;

unsigned long lastTempRead = 0;

void initHealthSensor() {
    Serial.print("Init Health (MAX30105)... ");
    
    if (!particleSensor.begin(Wire, I2C_SPEED_FAST)) {
        Serial.println("FAILED! Check wiring.");
    } else {
        Serial.println("OK!");
        
        // --- CONFIGURATION ---
        byte ledBrightness = 18; // Back to ~35% power (192 might be saturating)
        byte sampleAverage = 4;    
        byte ledMode = 2;          
        int sampleRate = 400;      // 100Hz Effective
        int pulseWidth = 411;      // Back to 411 for better resolution
        int adcRange = 4096;       
        
        particleSensor.setup(ledBrightness, sampleAverage, ledMode, sampleRate, pulseWidth, adcRange);
        
        particleSensor.enableDIETEMPRDY();
    }
}

void updateHealthSensor(SystemState &state) {
    particleSensor.check(); 
    
    while (particleSensor.available()) {
        uint32_t red = particleSensor.getRed();
        uint32_t ir = particleSensor.getIR();
        particleSensor.nextSample();

        // 1. FINGER CHECK
        if (ir < IR_THRESHOLD) {
            state.heartRate = 0;
            state.spo2 = 0;
            state.validHR = 0;
            state.validSPO2 = 0;
            state.healthTemp = 0.0;
            bufferIndex = 0;
            return;
        }

        // 2. IMMEDIATE TEMP READ (Every 1 second)
        if (millis() - lastTempRead > 1000) {
            float t = particleSensor.readTemperature();
            if (t > 0) state.healthTemp = t;
            lastTempRead = millis();
        }

        // 3. FILL BUFFER
        redBuffer[bufferIndex] = red;
        irBuffer[bufferIndex] = ir;
        bufferIndex++;

        // 4. PROCESS BATCH
        if (bufferIndex == bufferLength) {
            
            int32_t calcHR, calcSPO2;
            int8_t validHR, validSPO2;

            maxim_heart_rate_and_oxygen_saturation(
                irBuffer, bufferLength, redBuffer, 
                &calcSPO2, &validSPO2, 
                &calcHR, &validHR
            );

            // Filter Results
            if (validHR == 1 && calcHR > 40 && calcHR < 220) {
                state.heartRate = calcHR;
                state.validHR = 1;
            } else {
                state.validHR = 0;
            }

            if (validSPO2 == 1 && calcSPO2 > 70 && calcSPO2 <= 100) {
                state.spo2 = calcSPO2;
                state.validSPO2 = 1;
            }

            // 5. CRITICAL FIX: THE SLIDING WINDOW
            // Instead of wiping the buffer (bufferIndex=0), we keep the newest data
            // and shift it to the front. This ensures the "Heartbeat" signal isn't cut in half.
            
            // Move the last 75 samples to the front (0-74)
            for (byte i = 25; i < 100; i++) {
                redBuffer[i - 25] = redBuffer[i];
                irBuffer[i - 25] = irBuffer[i];
            }

            // Set index to 75. 
            // We only need to collect 25 new samples (0.25 seconds) to run the math again.
            bufferIndex = 75; 
        }
    }
}