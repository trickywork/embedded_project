#include "BLEManager.h"

BLEManager::BLEManager() : initialized(false), simulationMode(false) {
    tremorStatus = 0;
    tremorIntensityByte = 0;
    dyskinesiaStatus = 0;
    dyskinesiaIntensityByte = 0;
    fogStatus = 0;
    fogIntensityByte = 0;
}

bool BLEManager::begin() {
    #ifdef MBED_OS
        // TODO: 初始化实际的BLE
        // 创建BLE服务和特征值
        // 服务UUID: 例如 "19B10000-E8F2-537E-4F6C-D104768A1214"
        // 三个特征值用于震颤、运动障碍、冻结步态
        initialized = true;
        return true;
    #else
        // 模拟模式
        printf("BLE运行在模拟模式\r\n");
        simulationMode = true;
        initialized = true;
        return true;
    #endif
}

void BLEManager::update() {
    #ifdef MBED_OS
        // TODO: 处理BLE事件
        // ble.processEvents();
    #else
        // 模拟模式下不需要处理事件
    #endif
}

void BLEManager::updateCharacteristics(bool tremorDetected, float tremorIntensity,
                                      bool dyskinesiaDetected, float dyskinesiaIntensity,
                                      bool fogDetected, float fogIntensity) {
    // 更新特征值
    tremorStatus = tremorDetected ? 1 : 0;
    tremorIntensityByte = (uint8_t)(tremorIntensity * 255);
    
    dyskinesiaStatus = dyskinesiaDetected ? 1 : 0;
    dyskinesiaIntensityByte = (uint8_t)(dyskinesiaIntensity * 255);
    
    fogStatus = fogDetected ? 1 : 0;
    fogIntensityByte = (uint8_t)(fogIntensity * 255);
    
    #ifdef MBED_OS
        // TODO: 通过BLE发送数据
        // tremorCharacteristic.writeValue(&tremorStatus, 1);
        // tremorIntensityCharacteristic.writeValue(&tremorIntensityByte, 1);
        // ...
    #else
        // 模拟模式下打印数据
        if (simulationMode) {
            printf("[BLE模拟] 震颤:%d(%.2f) 运动障碍:%d(%.2f) 冻结:%d(%.2f)\r\n",
                   tremorStatus, tremorIntensity,
                   dyskinesiaStatus, dyskinesiaIntensity,
                   fogStatus, fogIntensity);
        }
    #endif
}

