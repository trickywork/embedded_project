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
    // TODO: 添加实际的BLE对象
    // BLEDevice ble;
    #endif
};

#endif

