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
        // Get BLE instance
        ble = &BLE::Instance();
        
        // Initialize BLE stack
        ble_error_t error = ble->init();
        if (error != BLE_ERROR_NONE) {
            printf("BLE initialization failed: %d\r\n", error);
            return false;
        }
        
        // Configure GAP (Generic Access Profile) parameters
        // Set device name that will appear in BLE scans
        ble->gap().setDeviceName((const uint8_t*)DEVICE_NAME);
        // Set advertising parameters (connectable, undirected)
        ble->gap().setAdvertisingParameters(
            GapAdvertisingParams(GapAdvertisingParams::ADV_CONNECTABLE_UNDIRECTED)
        );
        
        // Create UUIDs for service and characteristics
        UUID serviceUUID(SERVICE_UUID, sizeof(SERVICE_UUID));
        UUID tremorUUID(TREMOR_CHAR_UUID, sizeof(TREMOR_CHAR_UUID));
        UUID dyskinesiaUUID(DYSKINESIA_CHAR_UUID, sizeof(DYSKINESIA_CHAR_UUID));
        UUID fogUUID(FOG_CHAR_UUID, sizeof(FOG_CHAR_UUID));
        
        // Create characteristics (readable and notifiable)
        // Each characteristic stores 1 byte: detection status (0 or 1)
        tremorChar = new GattCharacteristic(
            tremorUUID,              // Characteristic UUID
            &tremorStatus,           // Initial value pointer
            1,                       // Current length (bytes)
            1,                       // Maximum length (bytes)
            GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_READ |   // Can be read
            GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_NOTIFY   // Can send notifications
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
        
        // Create service containing all three characteristics
        GattCharacteristic* characteristics[] = {tremorChar, dyskinesiaChar, fogChar};
        symptomService = new GattService(serviceUUID, characteristics, 3);
        
        // Add service to GATT server
        error = ble->gattServer().addService(*symptomService);
        if (error != BLE_ERROR_NONE) {
            printf("Failed to add BLE service: %d\r\n", error);
            return false;
        }
        
        // Start BLE advertising (makes device discoverable)
        error = ble->gap().startAdvertising();
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
        if (ble) {
            ble->processEvents();  // Process pending BLE events
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
        if (ble && initialized) {
            // Update characteristics and notify connected clients
            ble_error_t error;
            
            // Write tremor status to characteristic
            error = ble->gattServer().write(
                tremorChar->getValueHandle(),  // Characteristic handle
                &tremorStatus,                 // Data to write
                1                               // Data length (bytes)
            );
            if (error == BLE_ERROR_NONE) {
                ble->gattServer().handleDataSent();  // Trigger notification if subscribed
            }
            
            // Write dyskinesia status to characteristic
            error = ble->gattServer().write(
                dyskinesiaChar->getValueHandle(),
                &dyskinesiaStatus,
                1
            );
            if (error == BLE_ERROR_NONE) {
                ble->gattServer().handleDataSent();
            }
            
            // Write FOG status to characteristic
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
        // Print data in simulation mode
        if (simulationMode) {
            printf("[BLE Simulation] Tremor:%d(%.2f) Dyskinesia:%d(%.2f) FOG:%d(%.2f)\r\n",
                   tremorStatus, tremorIntensity,
                   dyskinesiaStatus, dyskinesiaIntensity,
                   fogStatus, fogIntensity);
        }
    #endif
}

