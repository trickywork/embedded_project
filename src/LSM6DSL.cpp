/**
 * @file LSM6DSL.cpp
 * @brief Implementation of LSM6DSL sensor driver
 */

#ifdef MBED_OS

#include "LSM6DSL.h"
#include <cstdio>

/**
 * @brief Constructor - Initialize LSM6DSL driver
 * @param i2c Pointer to I2C interface object
 * 
 * Sets default I2C address and sensitivity values for ±2g accelerometer
 * and ±250dps gyroscope ranges.
 */
LSM6DSL::LSM6DSL(I2C* i2c) : _i2c(i2c), _address(LSM6DSL_I2C_ADDRESS),
    _accelSensitivity(LSM6DSL_ACCEL_SENSITIVITY_2G),
    _gyroSensitivity(LSM6DSL_GYRO_SENSITIVITY_250DPS) {
}

/**
 * @brief Initialize sensor and configure registers
 * 
 * Performs the following initialization steps:
 * 1. Verifies sensor presence by reading WHO_AM_I register (should be 0x6A)
 * 2. Tries alternate I2C address if first attempt fails
 * 3. Configures accelerometer: ±2g range, 52Hz ODR
 * 4. Configures gyroscope: ±250dps range, 52Hz ODR
 * 5. Enables Block Data Update (BDU) to prevent reading partially updated data
 * 
 * @return true if initialization successful, false otherwise
 */
bool LSM6DSL::init() {
    // Check WHO_AM_I register to verify sensor presence
    uint8_t whoAmI;
    if (!readRegister(LSM6DSL_WHO_AM_I, whoAmI)) {
        printf("LSM6DSL: Cannot read WHO_AM_I register (address 0x%02X)\r\n", _address);
        // Try alternate address if first attempt failed
        if (_address == LSM6DSL_I2C_ADDRESS) {
            _address = LSM6DSL_I2C_ADDRESS_ALT;
            printf("LSM6DSL: Trying alternate address 0x%02X\r\n", _address);
            if (!readRegister(LSM6DSL_WHO_AM_I, whoAmI)) {
                return false;
            }
        } else {
            return false;
        }
    }
    
    // Verify WHO_AM_I value (should be 0x6A for LSM6DSL)
    if (whoAmI != 0x6A) {
        printf("LSM6DSL: WHO_AM_I error (expected 0x6A, got 0x%02X)\r\n", whoAmI);
        return false;
    }
    
    // Configure accelerometer: ±2g full-scale, 52Hz output data rate
    // CTRL1_XL format: [ODR3:ODR0][FS1_XL:FS0_XL][BW1_XL:BW0_XL]
    uint8_t ctrl1_xl = (LSM6DSL_ODR_52_HZ << 4) | (LSM6DSL_ACCEL_FS_2G << 2);
    if (!writeRegister(LSM6DSL_CTRL1_XL, ctrl1_xl)) {
        printf("LSM6DSL: Cannot configure accelerometer\r\n");
        return false;
    }
    _accelSensitivity = LSM6DSL_ACCEL_SENSITIVITY_2G;
    
    // Configure gyroscope: ±250dps full-scale, 52Hz output data rate
    // CTRL2_G format: [ODR3_G:ODR0_G][FS1_G:FS0_G][FS_125]
    uint8_t ctrl2_g = (LSM6DSL_ODR_52_HZ << 4) | (LSM6DSL_GYRO_FS_250DPS << 2);
    if (!writeRegister(LSM6DSL_CTRL2_G, ctrl2_g)) {
        printf("LSM6DSL: Cannot configure gyroscope\r\n");
        return false;
    }
    _gyroSensitivity = LSM6DSL_GYRO_SENSITIVITY_250DPS;
    
    // Configure CTRL3_C: Enable BDU (Block Data Update)
    // BDU ensures that upper and lower register parts are updated simultaneously
    // 0x44 = BDU enabled (bit 6) + IF_INC enabled (bit 2)
    if (!writeRegister(LSM6DSL_CTRL3_C, 0x44)) {
        printf("LSM6DSL: Cannot configure CTRL3_C\r\n");
        return false;
    }
    
    printf("LSM6DSL: Initialization successful\r\n");
    return true;
}

/**
 * @brief Read accelerometer data from sensor
 * 
 * Reads 16-bit values from accelerometer output registers and converts
 * them to g units using the configured sensitivity value.
 * 
 * @param x Reference to store X-axis acceleration (g)
 * @param y Reference to store Y-axis acceleration (g)
 * @param z Reference to store Z-axis acceleration (g)
 * @return true if read successful, false otherwise
 */
bool LSM6DSL::readAccel(float& x, float& y, float& z) {
    // Read 16-bit raw values from accelerometer output registers
    int16_t rawX = read16BitRegister(LSM6DSL_OUTX_L_XL);
    int16_t rawY = read16BitRegister(LSM6DSL_OUTY_L_XL);
    int16_t rawZ = read16BitRegister(LSM6DSL_OUTZ_L_XL);
    
    // Convert raw values to g units using sensitivity (mg/LSB -> g)
    // Sensitivity is in mg/LSB, divide by 1000 to get g
    x = (rawX * _accelSensitivity) / 1000.0f;
    y = (rawY * _accelSensitivity) / 1000.0f;
    z = (rawZ * _accelSensitivity) / 1000.0f;
    
    return true;
}

/**
 * @brief Read gyroscope data from sensor
 * 
 * Reads 16-bit values from gyroscope output registers and converts
 * them to deg/s units using the configured sensitivity value.
 * 
 * @param x Reference to store X-axis angular velocity (deg/s)
 * @param y Reference to store Y-axis angular velocity (deg/s)
 * @param z Reference to store Z-axis angular velocity (deg/s)
 * @return true if read successful, false otherwise
 */
bool LSM6DSL::readGyro(float& x, float& y, float& z) {
    // Read 16-bit raw values from gyroscope output registers
    int16_t rawX = read16BitRegister(LSM6DSL_OUTX_L_G);
    int16_t rawY = read16BitRegister(LSM6DSL_OUTY_L_G);
    int16_t rawZ = read16BitRegister(LSM6DSL_OUTZ_L_G);
    
    // Convert raw values to deg/s units using sensitivity (mdps/LSB -> deg/s)
    // Sensitivity is in mdps/LSB, divide by 1000 to get deg/s
    x = (rawX * _gyroSensitivity) / 1000.0f;
    y = (rawY * _gyroSensitivity) / 1000.0f;
    z = (rawZ * _gyroSensitivity) / 1000.0f;
    
    return true;
}

/**
 * @brief Check if new sensor data is ready to be read
 * 
 * Reads the status register and checks data ready flags for both
 * accelerometer and gyroscope. Both must be ready for valid readings.
 * 
 * @return true if both accelerometer and gyroscope data are ready
 */
bool LSM6DSL::dataReady() {
    uint8_t status;
    if (!readRegister(LSM6DSL_STATUS_REG, status)) {
        return false;
    }
    // Check accelerometer (bit 0) and gyroscope (bit 1) data ready flags
    // Both bits must be set (0x03) for data to be ready
    return (status & 0x03) == 0x03;
}

/**
 * @brief Write a value to a sensor register via I2C
 * 
 * @param reg Register address to write to
 * @param value Value to write
 * @return true if write successful, false otherwise
 */
bool LSM6DSL::writeRegister(uint8_t reg, uint8_t value) {
    char data[2] = {reg, value};
    if (_i2c->write(_address, data, 2) != 0) {
        return false;
    }
    return true;
}

/**
 * @brief Read a value from a sensor register via I2C
 * 
 * Uses I2C write-then-read transaction:
 * 1. Write register address (with repeated start)
 * 2. Read register value
 * 
 * @param reg Register address to read from
 * @param value Reference to store read value
 * @return true if read successful, false otherwise
 */
bool LSM6DSL::readRegister(uint8_t reg, uint8_t& value) {
    char regAddr = reg;
    // Write register address with repeated start (true parameter)
    if (_i2c->write(_address, &regAddr, 1, true) != 0) {
        return false;
    }
    // Read register value
    char data;
    if (_i2c->read(_address, &data, 1) != 0) {
        return false;
    }
    value = data;
    return true;
}

/**
 * @brief Read multiple consecutive registers via I2C
 * 
 * Reads a block of registers starting from the specified address.
 * 
 * @param reg Starting register address
 * @param data Pointer to buffer to store read data
 * @param length Number of registers to read
 * @return true if read successful, false otherwise
 */
bool LSM6DSL::readRegisters(uint8_t reg, uint8_t* data, int length) {
    char regAddr = reg;
    // Write starting register address with repeated start
    if (_i2c->write(_address, &regAddr, 1, true) != 0) {
        return false;
    }
    // Read multiple bytes
    if (_i2c->read(_address, (char*)data, length) != 0) {
        return false;
    }
    return true;
}

/**
 * @brief Read a 16-bit register value (low and high bytes)
 * 
 * Reads two consecutive 8-bit registers and combines them into a 16-bit value.
 * The low byte is at regLow, high byte is at regLow+1.
 * 
 * @param regLow Address of the low byte register
 * @return 16-bit signed integer value, or 0 if read fails
 */
int16_t LSM6DSL::read16BitRegister(uint8_t regLow) {
    uint8_t low, high;
    // Read low byte and high byte
    if (!readRegister(regLow, low) || !readRegister(regLow + 1, high)) {
        return 0;
    }
    // Combine bytes: high byte in upper 8 bits, low byte in lower 8 bits
    return (int16_t)((high << 8) | low);
}

#endif // MBED_OS

