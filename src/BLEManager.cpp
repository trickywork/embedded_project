/**
 * @file BLEManager.cpp
 * @brief Implementation of BLE communication manager
 * 
 * Handles BLE initialization, service/characteristic creation, and
 * data transmission to mobile devices.
 */

#include "BLEManager.h"
#ifdef MBED_OS
#include "ble/BLE.h"
#include "ble/Gap.h"
#include "ble/GattServer.h"
#include "ble/gatt/GattCharacteristic.h"
#include "ble/gatt/GattService.h"
#include "ble/common/UUID.h"
#include "ble/gap/AdvertisingDataSimpleBuilder.h"
#include "ble/gap/AdvertisingParameters.h"
#include "ble/common/BLETypes.h"
#endif

/**
 * @brief Constructor - Initialize BLE manager
 * 
 * Initializes all characteristic values to zero.
 */
BLEManager::BLEManager() : initialized(false), simulationMode(false) {
    tremorStatus = 0;
    tremorIntensityByte = 0;
    dyskinesiaStatus = 0;
    dyskinesiaIntensityByte = 0;
    fogStatus = 0;
    fogIntensityByte = 0;
    #ifdef MBED_OS
    ble = nullptr;
    tremorChar = nullptr;
    dyskinesiaChar = nullptr;
    fogChar = nullptr;
    symptomService = nullptr;
    #endif
}

/**
 * @brief Initialize BLE and create GATT service/characteristics
 * 
 * Initialization steps:
 * 1. Initialize BLE stack
 * 2. Set device name and advertising parameters
 * 3. Create UUIDs for service and characteristics
 * 4. Create three characteristics (tremor, dyskinesia, FOG)
 * 5. Create service containing all characteristics
 * 6. Add service to GATT server
 * 7. Start BLE advertising
 * 
 * @return true if initialization successful, false otherwise
 */
bool BLEManager::begin() {
    #ifdef MBED_OS
        using namespace ble;
        
        // Get BLE instance (cast void* to ble::BLE*)
        BLE* bleInstance = &BLE::Instance();
        this->ble = static_cast<void*>(bleInstance);
        
        // Initialize BLE stack
        ble_error_t error = bleInstance->init();
        if (error != BLE_ERROR_NONE) {
            printf("BLE initialization failed: %d\r\n", error);
            return false;
        }
        
        // Configure GAP (Generic Access Profile) parameters
        // Set advertising parameters (connectable, undirected)
        AdvertisingParameters advParams;
        advParams.setType(advertising_type_t::CONNECTABLE_UNDIRECTED);
        error = bleInstance->gap().setAdvertisingParameters(LEGACY_ADVERTISING_HANDLE, advParams);
        if (error != BLE_ERROR_NONE) {
            printf("Failed to set advertising parameters: %d\r\n", error);
            return false;
        }
        
        // Set advertising payload with device name
        error = bleInstance->gap().setAdvertisingPayload(
            LEGACY_ADVERTISING_HANDLE,
            AdvertisingDataSimpleBuilder<LEGACY_ADVERTISING_MAX_SIZE>()
                .setFlags()
                .setName(DEVICE_NAME, true)
                .getAdvertisingData()
        );
        if (error != BLE_ERROR_NONE) {
            printf("Failed to set advertising payload: %d\r\n", error);
            return false;
        }
        
        // Create UUIDs for service and characteristics (128-bit UUIDs)
        // UUID format: 16 bytes, MSB (Most Significant Byte first)
        UUID::LongUUIDBytes_t serviceUUIDBytes;
        UUID::LongUUIDBytes_t tremorUUIDBytes;
        UUID::LongUUIDBytes_t dyskinesiaUUIDBytes;
        UUID::LongUUIDBytes_t fogUUIDBytes;
        
        // Copy UUID arrays
        memcpy(serviceUUIDBytes, SERVICE_UUID, 16);
        memcpy(tremorUUIDBytes, TREMOR_CHAR_UUID, 16);
        memcpy(dyskinesiaUUIDBytes, DYSKINESIA_CHAR_UUID, 16);
        memcpy(fogUUIDBytes, FOG_CHAR_UUID, 16);
        
        UUID serviceUUID(serviceUUIDBytes, UUID::MSB);
        UUID tremorUUID(tremorUUIDBytes, UUID::MSB);
        UUID dyskinesiaUUID(dyskinesiaUUIDBytes, UUID::MSB);
        UUID fogUUID(fogUUIDBytes, UUID::MSB);
        
        // Create characteristics (readable and notifiable)
        // Each characteristic stores 1 byte: detection status (0 or 1)
        GattCharacteristic* tremorCharPtr = new GattCharacteristic(
            tremorUUID,              // Characteristic UUID
            &tremorStatus,           // Initial value pointer
            1,                       // Current length (bytes)
            1,                       // Maximum length (bytes)
            GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_READ |   // Can be read
            GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_NOTIFY   // Can send notifications
        );
        this->tremorChar = static_cast<void*>(tremorCharPtr);
        
        GattCharacteristic* dyskinesiaCharPtr = new GattCharacteristic(
            dyskinesiaUUID,
            &dyskinesiaStatus,
            1,
            1,
            GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_READ | 
            GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_NOTIFY
        );
        this->dyskinesiaChar = static_cast<void*>(dyskinesiaCharPtr);
        
        GattCharacteristic* fogCharPtr = new GattCharacteristic(
            fogUUID,
            &fogStatus,
            1,
            1,
            GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_READ | 
            GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_NOTIFY
        );
        this->fogChar = static_cast<void*>(fogCharPtr);
        
        // Create service containing all three characteristics
        GattCharacteristic* characteristics[] = {tremorCharPtr, dyskinesiaCharPtr, fogCharPtr};
        GattService* servicePtr = new GattService(serviceUUID, characteristics, 3);
        this->symptomService = static_cast<void*>(servicePtr);
        
        // Add service to GATT server
        error = bleInstance->gattServer().addService(*servicePtr);
        if (error != BLE_ERROR_NONE) {
            printf("Failed to add BLE service: %d\r\n", error);
            return false;
        }
        
        // Start BLE advertising (makes device discoverable)
        error = bleInstance->gap().startAdvertising(LEGACY_ADVERTISING_HANDLE);
        if (error != BLE_ERROR_NONE) {
            printf("Failed to start BLE advertising: %d\r\n", error);
            return false;
        }
        
        printf("BLE initialization successful, device name: %s\r\n", DEVICE_NAME);
        initialized = true;
        return true;
    #else
        // Simulation mode (for computer testing)
        printf("BLE running in simulation mode\r\n");
        simulationMode = true;
        initialized = true;
        return true;
    #endif
}

/**
 * @brief Process BLE events
 * 
 * Must be called regularly in main loop to handle BLE stack events:
 * - Connection/disconnection events
 * - Data transmission events
 * - Notification delivery
 */
void BLEManager::update() {
    #ifdef MBED_OS
        using namespace ble;
        if (this->ble) {
            BLE* bleInstance = static_cast<BLE*>(this->ble);
            bleInstance->processEvents();  // Process pending BLE events
        }
    #else
        // No event processing needed in simulation mode
    #endif
}

/**
 * @brief Update BLE characteristics with latest detection results
 * 
 * Converts detection results to bytes and writes to BLE characteristics.
 * If a mobile device is connected and subscribed to notifications,
 * it will automatically receive updates.
 * 
 * Data format:
 * - Status: 1 byte (0 = not detected, 1 = detected)
 * - Intensity: 1 byte (0-255, normalized from 0.0-1.0 float)
 * 
 * @param tremorDetected, tremorIntensity Tremor detection result and intensity
 * @param dyskinesiaDetected, dyskinesiaIntensity Dyskinesia detection result and intensity
 * @param fogDetected, fogIntensity FOG detection result and intensity
 */
void BLEManager::updateCharacteristics(bool tremorDetected, float tremorIntensity,
                                      bool dyskinesiaDetected, float dyskinesiaIntensity,
                                      bool fogDetected, float fogIntensity) {
    // Update characteristic values
    // Convert boolean to byte (0 or 1)
    tremorStatus = tremorDetected ? 1 : 0;
    // Convert float (0.0-1.0) to byte (0-255)
    tremorIntensityByte = (uint8_t)(tremorIntensity * 255);
    
    dyskinesiaStatus = dyskinesiaDetected ? 1 : 0;
    dyskinesiaIntensityByte = (uint8_t)(dyskinesiaIntensity * 255);
    
    fogStatus = fogDetected ? 1 : 0;
    fogIntensityByte = (uint8_t)(fogIntensity * 255);
    
    #ifdef MBED_OS
        using namespace ble;
        if (this->ble && initialized && this->tremorChar && this->dyskinesiaChar && this->fogChar) {
            BLE* bleInstance = static_cast<BLE*>(this->ble);
            GattCharacteristic* tremorCharPtr = static_cast<GattCharacteristic*>(this->tremorChar);
            GattCharacteristic* dyskinesiaCharPtr = static_cast<GattCharacteristic*>(this->dyskinesiaChar);
            GattCharacteristic* fogCharPtr = static_cast<GattCharacteristic*>(this->fogChar);
            
            // Update characteristics and notify connected clients
            // Write tremor status to characteristic
            // Notifications are sent automatically if client is subscribed
            (void)bleInstance->gattServer().write(
                tremorCharPtr->getValueHandle(),  // Characteristic handle
                &tremorStatus,                 // Data to write
                1                               // Data length (bytes)
            );
            
            // Write dyskinesia status to characteristic
            (void)bleInstance->gattServer().write(
                dyskinesiaCharPtr->getValueHandle(),
                &dyskinesiaStatus,
                1
            );
            
            // Write FOG status to characteristic
            (void)bleInstance->gattServer().write(
                fogCharPtr->getValueHandle(),
                &fogStatus,
                1
            );
        }
    #else
        // Print data in simulation mode
        if (simulationMode) {
            printf("[BLE Simulation] Tremor:%d(%.2f) Dyskinesia:%d(%.2f) FOG:%d(%.2f)\r\n",
                   tremorStatus, tremorIntensity,
                   dyskinesiaStatus, dyskinesiaIntensity,
                   fogStatus, fogIntensity);
        }
    #endif
}

// UUID and device name definitions
#ifdef MBED_OS
const char* BLEManager::DEVICE_NAME = "ParkinsonDetector";

// 128-bit UUIDs for service and characteristics
// Service UUID: 19B10000-E8F2-537E-4F6C-D104768A1214
const uint8_t BLEManager::SERVICE_UUID[] = {
    0x19, 0xB1, 0x00, 0x00, 0xE8, 0xF2, 0x53, 0x7E,
    0x4F, 0x6C, 0xD1, 0x04, 0x76, 0x8A, 0x12, 0x14
};

// Tremor Characteristic UUID: 19B10001-E8F2-537E-4F6C-D104768A1214
const uint8_t BLEManager::TREMOR_CHAR_UUID[] = {
    0x19, 0xB1, 0x00, 0x01, 0xE8, 0xF2, 0x53, 0x7E,
    0x4F, 0x6C, 0xD1, 0x04, 0x76, 0x8A, 0x12, 0x14
};

// Dyskinesia Characteristic UUID: 19B10002-E8F2-537E-4F6C-D104768A1214
const uint8_t BLEManager::DYSKINESIA_CHAR_UUID[] = {
    0x19, 0xB1, 0x00, 0x02, 0xE8, 0xF2, 0x53, 0x7E,
    0x4F, 0x6C, 0xD1, 0x04, 0x76, 0x8A, 0x12, 0x14
};

// FOG Characteristic UUID: 19B10003-E8F2-537E-4F6C-D104768A1214
const uint8_t BLEManager::FOG_CHAR_UUID[] = {
    0x19, 0xB1, 0x00, 0x03, 0xE8, 0xF2, 0x53, 0x7E,
    0x4F, 0x6C, 0xD1, 0x04, 0x76, 0x8A, 0x12, 0x14
};
#endif

