#include "OutputManager.h"

void printStatus(SystemState &state) {
    Serial.printf("G: %.2f | T1: %.1fC | HR: %d | SpO2: %d%% | T2: %.1fC | %s\n", 
      state.gForce, 
      state.temperature,  // Board Temp
      state.heartRate, 
      state.spo2, 
      state.healthTemp,   // Skin Temp (New)
      state.seizureDetected ? "SEIZURE!" : "OK");
}