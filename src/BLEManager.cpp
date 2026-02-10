#include "BLEManager.h"

NimBLECharacteristic *pStatusChar;
NimBLECharacteristic *pControlChar;
SystemState* _bleStateRef = nullptr;

class ControlCallbacks : public NimBLECharacteristicCallbacks {
    void onWrite(NimBLECharacteristic* pChar) {
        if(!_bleStateRef) return;
        std::string v = pChar->getValue();
        if (v.length() > 0) {
            if (v[0] == '1') _bleStateRef->isPaused = true;
            if (v[0] == '0') _bleStateRef->isPaused = false;
        }
    }
};

void initBLE(SystemState &state) {
    _bleStateRef = &state;
    NimBLEDevice::init("K9-Monitor");
    NimBLEServer *pServer = NimBLEDevice::createServer();
    NimBLEService *pService = pServer->createService("180D");
    
    pStatusChar = pService->createCharacteristic("2A37", NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::NOTIFY);
    pControlChar = pService->createCharacteristic("2A38", NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::READ);
    pControlChar->setCallbacks(new ControlCallbacks());
    
    pService->start();
    NimBLEDevice::getAdvertising()->addServiceUUID("180D");
    NimBLEDevice::getAdvertising()->start();
    Serial.println(">> BLE Ready.");
}

void updateBLE(SystemState &state) {
    String msg = state.isPaused ? "PAUSED" : (state.seizureDetected ? "SEIZURE" : "OK");
    pStatusChar->setValue(msg);
    pStatusChar->notify();
}