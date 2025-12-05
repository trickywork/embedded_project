/**
 * @file SensorManager.h
 * @brief Sensor management interface for accelerometer and gyroscope data acquisition
 * 
 * This class provides a unified interface for reading sensor data from either:
 * - Hardware: LSM6DSL sensor on ST B-L475E-IOT01A1 board (via I2C)
 * - Simulation: Generated test data for algorithm development and testing
 * 
 * The class automatically handles initialization, data reading, and provides
 * simulation mode for testing without hardware.
 */

#ifndef SENSOR_MANAGER_H
#define SENSOR_MANAGER_H

#include "mbed_compat.h"

/**
 * @struct SensorData
 * @brief Structure containing accelerometer and gyroscope readings
 * 
 * All values are in standard units:
 * - Accelerometer: g (gravitational acceleration, 1g = 9.81 m/s²)
 * - Gyroscope: deg/s (degrees per second)
 */
struct SensorData {
    float accelX, accelY, accelZ;  // Accelerometer data in g (X, Y, Z axes)
    float gyroX, gyroY, gyroZ;     // Gyroscope data in deg/s (X, Y, Z axes)
};

/**
 * @class SensorManager
 * @brief Manages sensor initialization and data acquisition
 * 
 * Supports both hardware and simulation modes:
 * - Hardware mode: Reads from LSM6DSL sensor via I2C
 * - Simulation mode: Generates test data for algorithm validation
 */
class SensorManager {
public:
    SensorManager();
    
    /**
     * @brief Initialize sensor hardware or simulation mode
     * @return true if initialization successful, false otherwise
     */
    bool begin();
    
    /**
     * @brief Read current sensor data
     * @return SensorData structure containing accelerometer and gyroscope readings
     */
    SensorData read();
    
    /**
     * @brief Enable or disable simulation mode
     * @param enabled true to enable simulation, false for hardware mode
     */
    void setSimulationMode(bool enabled);
    
    /**
     * @brief Set simulated sensor data values (for testing)
     * @param accelX, accelY, accelZ Accelerometer values in g
     * @param gyroX, gyroY, gyroZ Gyroscope values in deg/s
     */
    void setSimulationData(float accelX, float accelY, float accelZ,
                          float gyroX, float gyroY, float gyroZ);
    
private:
    bool simulationMode;        // Flag indicating if simulation mode is active
    SensorData simulatedData;    // Stored simulated data values
    
    // Hardware-specific members (only compiled for MBED_OS)
    #ifdef MBED_OS
    void initHardware();         // Initialize I2C and LSM6DSL sensor
    I2C* i2c;                    // I2C interface pointer
    class LSM6DSL* lsm6dsl;      // LSM6DSL sensor driver instance
    #endif
    
    // Simulation mode helpers (only compiled for native test mode)
    #ifdef NATIVE_TEST_MODE
    float generateSimulatedValue(float base, float amplitude, float frequency, int timeMs);
    struct timeval simStartTime; // Start time for simulation timer
    bool simTimerStarted;        // Flag indicating if simulation timer is active
    int getSimTimeMs();          // Get elapsed time in milliseconds for simulation
    #endif
};

#endif
