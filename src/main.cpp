/**
 * Dog Seizure Monitor v7.0 - Modular Refactor
 */

#include <Arduino.h>
#include <Wire.h>
#include "Config.h"
#include "SensorMotion.h"
#include "SensorHealth.h"
#include "NetworkManager.h"
#include "BLEManager.h"
#include "OutputManager.h"
#include "DisplayManager.h"

// The Master State Object
SystemState state;

void setup() {
    Serial.begin(115200);
    while(!Serial) delay(10);
    Serial.println("\n--- K9 MONITOR STARTING ---");

    // Init Display (Show Boot Screen)
    initDisplay();

    // Init BLE First (Memory Reservation)
    initBLE(state);

    // Init I2C and Sensors
    Wire.begin(I2C_SDA, I2C_SCL);
    initMotionSensor();
    initHealthSensor();
    calibrateMotionSensor();

    // Init Network (Last to prevent timeout blocking)
    initNetwork(state);
    
    Serial.println("--- SYSTEM READY ---");
}

void loop() {
    // 1. High Speed Polling (Sensors)
    updateMotionSensor(state);
    updateHealthSensor(state);

    // 2. Low Speed Updates (Output/Network) - Every 1 second
    static unsigned long lastUpdate = 0;
    if (millis() - lastUpdate > 1000) {
        updateWebClients(state);
        updateBLE(state);
        printStatus(state);
        updateDisplay(state);
        lastUpdate = millis();
    }
}