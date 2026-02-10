#include "BLEManager.h"
#include <NimBLEDevice.h>

// UUIDs
#define SERVICE_UUID           "6E400001-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_TX "6E400003-B5A3-F393-E0A9-E50E24DCCA9E" 

NimBLEServer* pServer = NULL;
NimBLECharacteristic* pCharacteristic = NULL;
bool deviceConnected = false;

// Server Callbacks
class MyServerCallbacks: public NimBLEServerCallbacks {
    void onConnect(NimBLEServer* pServer) {
        deviceConnected = true;
    };
    void onDisconnect(NimBLEServer* pServer) {
        deviceConnected = false;
    }
};

void initBLE(SystemState &state) {
    Serial.print("Init BLE... ");
    
    // 1. Initialize
    NimBLEDevice::init("K9 Monitor");
    
    // 2. Create Server
    pServer = NimBLEDevice::createServer();
    pServer->setCallbacks(new MyServerCallbacks());
    
    // 3. Create Service
    NimBLEService *pService = pServer->createService(SERVICE_UUID);
    
    // 4. Create Characteristic
    // FIXED: Changed NIMBLE_PROPERTY_NOTIFY to NIMBLE_PROPERTY::NOTIFY
    pCharacteristic = pService->createCharacteristic(
        CHARACTERISTIC_UUID_TX,
        NIMBLE_PROPERTY::NOTIFY 
    );
    
    // 5. Start
    pService->start();
    NimBLEAdvertising *pAdvertising = NimBLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(SERVICE_UUID);
    pAdvertising->setScanResponse(true);
    pAdvertising->start();
    
    Serial.println("OK!");
}

void updateBLE(SystemState &state) {
    state.bleConnected = deviceConnected;

    if (deviceConnected) {
        char buf[128]; 
        
        // JSON Format:
        // g: G-Force
        // x/y/z: Gyro Vectors
        // t1: Board Temp | t2: Skin Temp
        // hr: Heart Rate | sp: SpO2
        // sz: Seizure (0/1)
        
        snprintf(buf, sizeof(buf), 
            "{\"g\":%.2f,\"x\":%.2f,\"y\":%.2f,\"z\":%.2f,\"t1\":%.1f,\"t2\":%.1f,\"hr\":%d,\"sp\":%d,\"sz\":%d}", 
            state.gForce,
            state.gyroX, state.gyroY, state.gyroZ,
            state.temperature, 
            state.healthTemp,
            (int)state.heartRate, 
            (int)state.spo2,
            state.seizureDetected ? 1 : 0
        );

        pCharacteristic->setValue((uint8_t*)buf, strlen(buf));
        pCharacteristic->notify();
    }
}