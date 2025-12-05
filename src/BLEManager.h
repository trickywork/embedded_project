#ifndef BLE_MANAGER_H
#define BLE_MANAGER_H

#include "mbed_compat.h"

class BLEManager {
public:
    BLEManager();
    bool begin();
    void update();
    
    void updateCharacteristics(bool tremorDetected, float tremorIntensity,
                               bool dyskinesiaDetected, float dyskinesiaIntensity,
                               bool fogDetected, float fogIntensity);
    
private:
    bool initialized;
    bool simulationMode;
    
    // BLE特征值
    uint8_t tremorStatus;
    uint8_t tremorIntensityByte;
    uint8_t dyskinesiaStatus;
    uint8_t dyskinesiaIntensityByte;
    uint8_t fogStatus;
    uint8_t fogIntensityByte;
    
    #ifdef MBED_OS
    #include "ble/BLE.h"
    #include "ble/Gap.h"
    #include "ble/services/HealthThermometerService.h"
    
    BLE* ble;
    // BLE服务和特征值UUID
    static const char* DEVICE_NAME = "ParkinsonDetector";
    static const uint8_t SERVICE_UUID[] = {0x19, 0xB1, 0x00, 0x00, 0xE8, 0xF2, 0x53, 0x7E, 
                                           0x4F, 0x6C, 0xD1, 0x04, 0x76, 0x8A, 0x12, 0x14};
    static const uint8_t TREMOR_CHAR_UUID[] = {0x19, 0xB1, 0x00, 0x01, 0xE8, 0xF2, 0x53, 0x7E,
                                               0x4F, 0x6C, 0xD1, 0x04, 0x76, 0x8A, 0x12, 0x14};
    static const uint8_t DYSKINESIA_CHAR_UUID[] = {0x19, 0xB1, 0x00, 0x02, 0xE8, 0xF2, 0x53, 0x7E,
                                                  0x4F, 0x6C, 0xD1, 0x04, 0x76, 0x8A, 0x12, 0x14};
    static const uint8_t FOG_CHAR_UUID[] = {0x19, 0xB1, 0x00, 0x03, 0xE8, 0xF2, 0x53, 0x7E,
                                           0x4F, 0x6C, 0xD1, 0x04, 0x76, 0x8A, 0x12, 0x14};
    
    GattCharacteristic* tremorChar;
    GattCharacteristic* dyskinesiaChar;
    GattCharacteristic* fogChar;
    GattService* symptomService;
    #endif
};

#endif

