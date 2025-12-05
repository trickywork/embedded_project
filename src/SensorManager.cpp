/**
 * @file SensorManager.cpp
 * @brief Implementation of sensor management for accelerometer and gyroscope
 */

#include "SensorManager.h"
#include <cstdlib>
#include <cmath>
#ifdef NATIVE_TEST_MODE
#include <sys/time.h>
#endif
#ifdef MBED_OS
#include "LSM6DSL.h"
#endif

/**
 * @brief Constructor - Initialize sensor manager
 * 
 * Sets up simulation mode flags and initializes hardware pointers to null.
 * For native test mode, initializes simulation timer.
 */
SensorManager::SensorManager() : simulationMode(false) {
    simulatedData = {0, 0, 0, 0, 0, 0};
    #ifdef NATIVE_TEST_MODE
    simTimerStarted = false;
    gettimeofday(&simStartTime, nullptr);
    simTimerStarted = true;
    #endif
    #ifdef MBED_OS
    i2c = nullptr;
    lsm6dsl = nullptr;
    #endif
}

/**
 * @brief Initialize sensor hardware or simulation mode
 * 
 * For MBED_OS: Initializes I2C interface and LSM6DSL sensor
 * For native mode: Enables simulation mode for testing
 * 
 * @return true if initialization successful, false otherwise
 */
bool SensorManager::begin() {
    #ifdef MBED_OS
        // Initialize STM32 hardware (I2C and LSM6DSL sensor)
        initHardware();
        return true;
    #else
        // Native mode (computer testing) - use simulated data
        #ifdef NATIVE_TEST_MODE
        printf("Running in simulation mode (computer testing)\n");
        #else
        printf("Running in simulation mode (computer testing)\r\n");
        #endif
        simulationMode = true;
        return true;
    #endif
}

void SensorManager::setSimulationMode(bool enabled) {
    simulationMode = enabled;
}

void SensorManager::setSimulationData(float accelX, float accelY, float accelZ,
                                      float gyroX, float gyroY, float gyroZ) {
    simulatedData.accelX = accelX;
    simulatedData.accelY = accelY;
    simulatedData.accelZ = accelZ;
    simulatedData.gyroX = gyroX;
    simulatedData.gyroY = gyroY;
    simulatedData.gyroZ = gyroZ;
}

/**
 * @brief Read current sensor data
 * 
 * Returns data from either hardware sensor or simulation mode.
 * In simulation mode, generates test signals with different frequencies
 * for algorithm validation.
 * 
 * @return SensorData structure with accelerometer and gyroscope readings
 */
SensorData SensorManager::read() {
    if (simulationMode) {
        #ifdef NATIVE_TEST_MODE
            // Generate simulated sensor data with different frequency signals for testing
            // This allows testing detection algorithms without hardware
            int timeMs = getSimTimeMs();
            
            // Base value + noise for realistic simulation
            float noise = (rand() % 20 - 10) / 1000.0f;
            
            SensorData data;
            // Generate test signals at different frequencies:
            // - 4Hz signal for tremor testing
            // - 6Hz signal for dyskinesia testing
            // Add phase offset for Y-axis to simulate 3D movement
            data.accelX = 0.1f * sin(2.0f * M_PI * 4.0f * timeMs / 1000.0f) + noise;
            data.accelY = 0.1f * sin(2.0f * M_PI * 4.0f * timeMs / 1000.0f + M_PI/4) + noise;
            data.accelZ = 0.1f + noise;  // Gravity component + noise
            data.gyroX = noise * 10;      // Small gyroscope noise
            data.gyroY = noise * 10;
            data.gyroZ = noise * 10;
            
            return data;
        #else
            // Return simulated data with added noise for realism
            SensorData data = simulatedData;
            // Add small random noise to simulate sensor noise
            data.accelX += (rand() % 20 - 10) / 1000.0f;
            data.accelY += (rand() % 20 - 10) / 1000.0f;
            data.accelZ += (rand() % 20 - 10) / 1000.0f;
            return data;
        #endif
    }
    
    #ifdef MBED_OS
        // Read from actual hardware sensor (LSM6DSL)
        if (lsm6dsl == nullptr) {
            // Sensor not initialized, return zero data
            SensorData data = {0, 0, 0, 0, 0, 0};
            return data;
        }
        
        SensorData data;
        // Read accelerometer data (X, Y, Z axes in g)
        if (!lsm6dsl->readAccel(data.accelX, data.accelY, data.accelZ)) {
            // Read failed, return zero data
            data = {0, 0, 0, 0, 0, 0};
            return data;
        }
        
        // Read gyroscope data (X, Y, Z axes in deg/s)
        if (!lsm6dsl->readGyro(data.gyroX, data.gyroY, data.gyroZ)) {
            // Read failed, set gyroscope to zero but keep accelerometer data
            data.gyroX = data.gyroY = data.gyroZ = 0;
        }
        
        return data;
    #else
        return simulatedData;
    #endif
}

#ifdef MBED_OS
/**
 * @brief Initialize hardware I2C interface and LSM6DSL sensor
 * 
 * This function initializes the I2C communication interface and the LSM6DSL
 * sensor on the ST B-L475E-IOT01A1 board. It automatically tries multiple
 * I2C configurations if the first attempt fails:
 * 
 * 1. First attempt: I2C1 on PB6 (SCL) and PB7 (SDA)
 * 2. Fallback: I2C3 on PC0 (SCL) and PC1 (SDA)
 * 
 * The sensor is configured for:
 * - Accelerometer: ±2g range, 52Hz output data rate
 * - Gyroscope: ±250dps range, 52Hz output data rate
 * - I2C speed: 400kHz
 */
void SensorManager::initHardware() {
    // ST B-L475E-IOT01A1 board uses I2C1 interface to connect LSM6DSL sensor
    // According to board support package, use predefined I2C pins
    // If predefined pins not available, try: PB6(SCL), PB7(SDA) or PC0(SCL), PC1(SDA)
    
    // Method 1: Try using Mbed predefined I2C pins (if board support package provides)
    // i2c = new I2C(I2C_SDA, I2C_SCL);
    
    // Method 2: Use standard I2C1 pins (PB6=SCL, PB7=SDA)
    i2c = new I2C(PB_7, PB_6);  // SDA, SCL (I2C1)
    
    // If above fails, can try I2C3 (PC0=SCL, PC1=SDA)
    // i2c = new I2C(PC_1, PC_0);  // SDA, SCL (I2C3)
    
    i2c->frequency(400000);  // Set I2C speed to 400kHz (fast mode)
    
    // Create LSM6DSL sensor driver object
    lsm6dsl = new LSM6DSL(i2c);
    
    // Initialize sensor (configure registers, verify WHO_AM_I)
    if (!lsm6dsl->init()) {
        printf("Sensor initialization failed! Trying alternate I2C pins...\r\n");
        // If I2C1 fails, try I2C3 as fallback
        delete i2c;
        delete lsm6dsl;
        
        i2c = new I2C(PC_1, PC_0);  // I2C3: SDA, SCL
        i2c->frequency(400000);
        lsm6dsl = new LSM6DSL(i2c);
        
        if (!lsm6dsl->init()) {
            printf("Sensor initialization completely failed!\r\n");
            delete lsm6dsl;
            lsm6dsl = nullptr;
            delete i2c;
            i2c = nullptr;
            return;
        }
    }
    
    printf("Sensor initialization successful (I2C address: 0x%02X)\r\n", LSM6DSL_I2C_ADDRESS);
}
#endif

#ifdef NATIVE_TEST_MODE
/**
 * @brief Get elapsed simulation time in milliseconds
 * 
 * Used for generating time-based test signals in simulation mode.
 * 
 * @return Elapsed time in milliseconds since simulation started
 */
int SensorManager::getSimTimeMs() {
    if (!simTimerStarted) return 0;
    struct timeval currentTime;
    gettimeofday(&currentTime, nullptr);
    return (currentTime.tv_sec - simStartTime.tv_sec) * 1000 + 
           (currentTime.tv_usec - simStartTime.tv_usec) / 1000;
}

/**
 * @brief Generate simulated sensor value with sinusoidal signal
 * 
 * Creates a sine wave signal for testing detection algorithms.
 * 
 * @param base Base value (DC offset)
 * @param amplitude Signal amplitude
 * @param frequency Signal frequency in Hz
 * @param timeMs Current time in milliseconds
 * @return Generated signal value
 */
float SensorManager::generateSimulatedValue(float base, float amplitude, 
                                            float frequency, int timeMs) {
    return base + amplitude * sin(2.0f * M_PI * frequency * timeMs / 1000.0f);
}
#endif
