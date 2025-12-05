/**
 * @file BLEManager.h
 * @brief BLE communication manager for transmitting detection results
 * 
 * This class manages Bluetooth Low Energy (BLE) communication to transmit
 * symptom detection results to mobile devices. It creates a GATT service
 * with three characteristics, one for each symptom type.
 * 
 * BLE Architecture:
 * - Service: Container for related characteristics
 * - Characteristics: Individual data points (tremor, dyskinesia, FOG)
 * - Notifications: Push updates to connected devices
 */

#ifndef BLE_MANAGER_H
#define BLE_MANAGER_H

#ifdef MBED_OS
#include <mbed.h>
// Include BLE headers in correct order
#include "ble/BLE.h"
#include "ble/gatt/GattCharacteristic.h"
#include "ble/gatt/GattService.h"
#else
#include "mbed_compat.h"
#endif

/**
 * @class BLEManager
 * @brief Manages BLE communication for symptom detection results
 * 
 * Creates and manages a BLE service with three characteristics for
 * transmitting detection results wirelessly to mobile devices.
 */
class BLEManager {
public:
    BLEManager();
    
    /**
     * @brief Initialize BLE and create service/characteristics
     * @return true if initialization successful, false otherwise
     */
    bool begin();
    
    /**
     * @brief Process BLE events (must be called regularly)
     * 
     * Handles BLE stack events such as connections, disconnections,
     * and data transmission. Should be called in main loop.
     */
    void update();
    
    /**
     * @brief Update BLE characteristics with latest detection results
     * 
     * Converts detection results to bytes and writes to BLE characteristics.
     * Connected devices will receive notifications if subscribed.
     * 
     * @param tremorDetected, tremorIntensity Tremor detection result
     * @param dyskinesiaDetected, dyskinesiaIntensity Dyskinesia detection result
     * @param fogDetected, fogIntensity FOG detection result
     */
    void updateCharacteristics(bool tremorDetected, float tremorIntensity,
                               bool dyskinesiaDetected, float dyskinesiaIntensity,
                               bool fogDetected, float fogIntensity);
    
private:
    bool initialized;        // Flag indicating if BLE is initialized
    bool simulationMode;      // Flag for simulation mode (no hardware)
    
    // BLE characteristic values (stored as bytes for transmission)
    uint8_t tremorStatus;           // Tremor detection status (0 or 1)
    uint8_t tremorIntensityByte;    // Tremor intensity (0-255, normalized from 0.0-1.0)
    uint8_t dyskinesiaStatus;       // Dyskinesia detection status (0 or 1)
    uint8_t dyskinesiaIntensityByte;// Dyskinesia intensity (0-255)
    uint8_t fogStatus;              // FOG detection status (0 or 1)
    uint8_t fogIntensityByte;       // FOG intensity (0-255)
    
    // Hardware-specific BLE objects (only compiled for MBED_OS)
    // Using void* to avoid header inclusion issues, cast in .cpp file
    #ifdef MBED_OS
    void* ble;  // BLE instance pointer (cast to ble::BLE* in implementation)
    
    // BLE Service and Characteristic UUIDs (128-bit UUIDs)
    // Service UUID: Main container for all characteristics
    static const char* DEVICE_NAME;
    static const uint8_t SERVICE_UUID[];
    // Characteristic UUIDs: One for each symptom type
    static const uint8_t TREMOR_CHAR_UUID[];
    static const uint8_t DYSKINESIA_CHAR_UUID[];
    static const uint8_t FOG_CHAR_UUID[];
    
    // GATT objects for BLE communication (cast in implementation)
    void* tremorChar;      // Tremor characteristic (cast to ble::GattCharacteristic*)
    void* dyskinesiaChar;  // Dyskinesia characteristic
    void* fogChar;          // FOG characteristic
    void* symptomService;  // Main service (cast to ble::GattService*)
    #endif
};

#endif

