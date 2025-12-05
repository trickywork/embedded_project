#include "BLEManager.h"
#ifdef MBED_OS
#include "ble/BLE.h"
#include "ble/Gap.h"
#include "ble/GattServer.h"
#endif

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
        ble = &BLE::Instance();
        
        // 初始化BLE
        ble_error_t error = ble->init();
        if (error != BLE_ERROR_NONE) {
            printf("BLE初始化失败: %d\r\n", error);
            return false;
        }
        
        // 设置GAP参数
        ble->gap().setDeviceName((const uint8_t*)DEVICE_NAME);
        ble->gap().setAdvertisingParameters(
            GapAdvertisingParams(GapAdvertisingParams::ADV_CONNECTABLE_UNDIRECTED)
        );
        
        // 创建服务和特征值
        UUID serviceUUID(SERVICE_UUID, sizeof(SERVICE_UUID));
        UUID tremorUUID(TREMOR_CHAR_UUID, sizeof(TREMOR_CHAR_UUID));
        UUID dyskinesiaUUID(DYSKINESIA_CHAR_UUID, sizeof(DYSKINESIA_CHAR_UUID));
        UUID fogUUID(FOG_CHAR_UUID, sizeof(FOG_CHAR_UUID));
        
        // 创建特征值（可读可通知）
        tremorChar = new GattCharacteristic(
            tremorUUID,
            &tremorStatus,
            1,  // 长度
            1,  // 最大长度
            GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_READ | 
            GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_NOTIFY
        );
        
        dyskinesiaChar = new GattCharacteristic(
            dyskinesiaUUID,
            &dyskinesiaStatus,
            1,
            1,
            GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_READ | 
            GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_NOTIFY
        );
        
        fogChar = new GattCharacteristic(
            fogUUID,
            &fogStatus,
            1,
            1,
            GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_READ | 
            GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_NOTIFY
        );
        
        // 创建服务
        GattCharacteristic* characteristics[] = {tremorChar, dyskinesiaChar, fogChar};
        symptomService = new GattService(serviceUUID, characteristics, 3);
        
        // 添加服务到GATT服务器
        error = ble->gattServer().addService(*symptomService);
        if (error != BLE_ERROR_NONE) {
            printf("添加BLE服务失败: %d\r\n", error);
            return false;
        }
        
        // 开始广播
        error = ble->gap().startAdvertising();
        if (error != BLE_ERROR_NONE) {
            printf("开始BLE广播失败: %d\r\n", error);
            return false;
        }
        
        printf("BLE初始化成功，设备名称: %s\r\n", DEVICE_NAME);
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
        if (ble) {
            ble->processEvents();
        }
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
        if (ble && initialized) {
            // 更新特征值并通知客户端
            ble_error_t error;
            
            error = ble->gattServer().write(
                tremorChar->getValueHandle(),
                &tremorStatus,
                1
            );
            if (error == BLE_ERROR_NONE) {
                ble->gattServer().handleDataSent();
            }
            
            error = ble->gattServer().write(
                dyskinesiaChar->getValueHandle(),
                &dyskinesiaStatus,
                1
            );
            if (error == BLE_ERROR_NONE) {
                ble->gattServer().handleDataSent();
            }
            
            error = ble->gattServer().write(
                fogChar->getValueHandle(),
                &fogStatus,
                1
            );
            if (error == BLE_ERROR_NONE) {
                ble->gattServer().handleDataSent();
            }
        }
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

