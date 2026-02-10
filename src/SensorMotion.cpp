#include "SensorMotion.h"
#include <Wire.h>

Adafruit_LSM6DSOX lsm;
float baselineGravity = 9.8;

void initMotionSensor() {
    Serial.print("Init Motion (LSM6DSOX)... ");
    if (!lsm.begin_I2C()) {
        Serial.printf("FAILED! Check Pin SDA: %d & SCL: %d.\n", I2C_SDA, I2C_SCL);
        while (1) delay(100);
    }
    Serial.println("OK!");
}

void calibrateMotionSensor() {
    Serial.println(">> Calibrating Motion (Keep Still)...");
    float sum = 0;
    int samples = 50;
    for (int i = 0; i < samples; i++) {
        sensors_event_t accel, gyro, temp;
        lsm.getEvent(&accel, &gyro, &temp);
        float m = sqrt(pow(accel.acceleration.x, 2) + pow(accel.acceleration.y, 2) + pow(accel.acceleration.z, 2));
        sum += m;
        delay(10);
    }
    baselineGravity = sum / samples;
    Serial.printf(">> Calibration Complete. Baseline: %.2f\n", baselineGravity);
}

void updateMotionSensor(SystemState &state) {
    sensors_event_t accel, gyro, temp;
    lsm.getEvent(&accel, &gyro, &temp);

    // 1. Calculate raw data
    float totalAccel = sqrt(pow(accel.acceleration.x, 2) + pow(accel.acceleration.y, 2) + pow(accel.acceleration.z, 2));
    
    // 2. Update State
    state.gForce = abs(totalAccel - baselineGravity);
    state.gyroX = gyro.gyro.x;
    state.gyroY = gyro.gyro.y;
    state.gyroZ = gyro.gyro.z;
    state.temperature = temp.temperature;

    // 3. Seizure Logic
    if (!state.isPaused && state.gForce > SEIZURE_G_FORCE_THRESH) {
        if (state.seizureStartTime == 0) state.seizureStartTime = millis();
        if (millis() - state.seizureStartTime > SEIZURE_DURATION_MS) {
            state.seizureDetected = true;
        }
    } else {
        if (state.gForce <= SEIZURE_G_FORCE_THRESH) {
            state.seizureDetected = false;
            state.seizureStartTime = 0;
        }
    }
}