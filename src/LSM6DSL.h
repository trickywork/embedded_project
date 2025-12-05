/**
 * @file LSM6DSL.h
 * @brief LSM6DSL 3-axis accelerometer and 3-axis gyroscope driver
 * 
 * This driver provides an interface to communicate with the LSM6DSL sensor
 * via I2C. The sensor is used for motion detection and provides:
 * - 3-axis accelerometer data (±2g, ±4g, ±8g, ±16g ranges)
 * - 3-axis gyroscope data (±125dps to ±2000dps ranges)
 * - Configurable output data rates (ODR) from 12.5Hz to 6.66kHz
 * 
 * The driver handles register configuration, data reading, and unit conversion.
 */

#ifndef LSM6DSL_H
#define LSM6DSL_H

#ifdef MBED_OS
#include "mbed.h"

// LSM6DSL register addresses (from datasheet)
#define LSM6DSL_WHO_AM_I         0x0F  // Device identification register (should read 0x6A)
#define LSM6DSL_CTRL1_XL         0x10  // Accelerometer control register (ODR and full-scale)
#define LSM6DSL_CTRL2_G          0x11  // Gyroscope control register (ODR and full-scale)
#define LSM6DSL_CTRL3_C          0x12  // Control register 3 (BDU, IF_INC, etc.)
#define LSM6DSL_STATUS_REG       0x1E  // Status register (data ready flags)
#define LSM6DSL_OUTX_L_XL        0x28  // Accelerometer X-axis output (low byte)
#define LSM6DSL_OUTX_H_XL        0x29  // Accelerometer X-axis output (high byte)
#define LSM6DSL_OUTY_L_XL        0x2A  // Accelerometer Y-axis output (low byte)
#define LSM6DSL_OUTY_H_XL        0x2B  // Accelerometer Y-axis output (high byte)
#define LSM6DSL_OUTZ_L_XL        0x2C  // Accelerometer Z-axis output (low byte)
#define LSM6DSL_OUTZ_H_XL        0x2D  // Accelerometer Z-axis output (high byte)
#define LSM6DSL_OUTX_L_G         0x22  // Gyroscope X-axis output (low byte)
#define LSM6DSL_OUTX_H_G         0x23  // Gyroscope X-axis output (high byte)
#define LSM6DSL_OUTY_L_G         0x24  // Gyroscope Y-axis output (low byte)
#define LSM6DSL_OUTY_H_G         0x25  // Gyroscope Y-axis output (high byte)
#define LSM6DSL_OUTZ_L_G         0x26  // Gyroscope Z-axis output (low byte)
#define LSM6DSL_OUTZ_H_G         0x27  // Gyroscope Z-axis output (high byte)

// LSM6DSL I2C addresses
// Note: B-L475E-IOT01A1 board may use 0xD6 or 0xD4 depending on SA0 pin configuration
#define LSM6DSL_I2C_ADDRESS      0xD6  // SA0 = 1 (default address, 7-bit: 0x6B)
#define LSM6DSL_I2C_ADDRESS_ALT  0xD4  // SA0 = 0 (alternate address, 7-bit: 0x6A)

// Accelerometer full-scale ranges and sensitivity values
// Sensitivity values in mg/LSB (milligrams per least significant bit)
#define LSM6DSL_ACCEL_FS_2G      0x00  // ±2g full-scale range
#define LSM6DSL_ACCEL_FS_4G      0x08  // ±4g full-scale range
#define LSM6DSL_ACCEL_FS_8G      0x0C  // ±8g full-scale range
#define LSM6DSL_ACCEL_FS_16G     0x04  // ±16g full-scale range
#define LSM6DSL_ACCEL_SENSITIVITY_2G   0.061f   // mg/LSB for ±2g range
#define LSM6DSL_ACCEL_SENSITIVITY_4G   0.122f   // mg/LSB for ±4g range
#define LSM6DSL_ACCEL_SENSITIVITY_8G   0.244f   // mg/LSB for ±8g range
#define LSM6DSL_ACCEL_SENSITIVITY_16G  0.488f   // mg/LSB for ±16g range

// Gyroscope full-scale ranges and sensitivity values
// Sensitivity values in mdps/LSB (millidegrees per second per LSB)
#define LSM6DSL_GYRO_FS_125DPS   0x02  // ±125dps full-scale range
#define LSM6DSL_GYRO_FS_250DPS   0x00  // ±250dps full-scale range
#define LSM6DSL_GYRO_FS_500DPS   0x04  // ±500dps full-scale range
#define LSM6DSL_GYRO_FS_1000DPS  0x08  // ±1000dps full-scale range
#define LSM6DSL_GYRO_FS_2000DPS  0x0C  // ±2000dps full-scale range
#define LSM6DSL_GYRO_SENSITIVITY_125DPS  4.375f   // mdps/LSB for ±125dps range
#define LSM6DSL_GYRO_SENSITIVITY_250DPS  8.75f   // mdps/LSB for ±250dps range
#define LSM6DSL_GYRO_SENSITIVITY_500DPS  17.5f   // mdps/LSB for ±500dps range
#define LSM6DSL_GYRO_SENSITIVITY_1000DPS 35.0f   // mdps/LSB for ±1000dps range
#define LSM6DSL_GYRO_SENSITIVITY_2000DPS 70.0f   // mdps/LSB for ±2000dps range

// Output Data Rate (ODR) settings
// These values configure the sampling frequency of the sensor
#define LSM6DSL_ODR_POWER_DOWN   0x00  // Power-down mode
#define LSM6DSL_ODR_12_5_HZ      0x01  // 12.5 Hz output data rate
#define LSM6DSL_ODR_26_HZ        0x02  // 26 Hz output data rate
#define LSM6DSL_ODR_52_HZ        0x03  // 52 Hz output data rate (required for this project)
#define LSM6DSL_ODR_104_HZ       0x04  // 104 Hz output data rate
#define LSM6DSL_ODR_208_HZ       0x05  // 208 Hz output data rate
#define LSM6DSL_ODR_416_HZ       0x06  // 416 Hz output data rate
#define LSM6DSL_ODR_833_HZ       0x07  // 833 Hz output data rate
#define LSM6DSL_ODR_1_66K_HZ     0x08  // 1.66 kHz output data rate
#define LSM6DSL_ODR_3_33K_HZ     0x09  // 3.33 kHz output data rate
#define LSM6DSL_ODR_6_66K_HZ     0x0A  // 6.66 kHz output data rate

/**
 * @class LSM6DSL
 * @brief Driver class for LSM6DSL accelerometer and gyroscope sensor
 * 
 * Provides methods to initialize, configure, and read data from the LSM6DSL sensor.
 * Handles I2C communication, register configuration, and data conversion to
 * standard units (g for accelerometer, deg/s for gyroscope).
 */
class LSM6DSL {
public:
    /**
     * @brief Constructor
     * @param i2c Pointer to I2C interface object
     */
    LSM6DSL(I2C* i2c);
    
    /**
     * @brief Initialize sensor and configure registers
     * 
     * Configures accelerometer and gyroscope for 52Hz ODR and appropriate
     * full-scale ranges. Verifies sensor presence by reading WHO_AM_I register.
     * 
     * @return true if initialization successful, false otherwise
     */
    bool init();
    
    /**
     * @brief Read accelerometer data
     * @param x Reference to store X-axis acceleration (g)
     * @param y Reference to store Y-axis acceleration (g)
     * @param z Reference to store Z-axis acceleration (g)
     * @return true if read successful, false otherwise
     */
    bool readAccel(float& x, float& y, float& z);
    
    /**
     * @brief Read gyroscope data
     * @param x Reference to store X-axis angular velocity (deg/s)
     * @param y Reference to store Y-axis angular velocity (deg/s)
     * @param z Reference to store Z-axis angular velocity (deg/s)
     * @return true if read successful, false otherwise
     */
    bool readGyro(float& x, float& y, float& z);
    
    /**
     * @brief Check if new sensor data is ready
     * @return true if both accelerometer and gyroscope data are ready
     */
    bool dataReady();
    
private:
    I2C* _i2c;                    // I2C interface pointer
    uint8_t _address;              // I2C device address
    float _accelSensitivity;      // Accelerometer sensitivity (mg/LSB) for current range
    float _gyroSensitivity;       // Gyroscope sensitivity (mdps/LSB) for current range
    
    // I2C register access methods
    bool writeRegister(uint8_t reg, uint8_t value);              // Write single register
    bool readRegister(uint8_t reg, uint8_t& value);              // Read single register
    bool readRegisters(uint8_t reg, uint8_t* data, int length);  // Read multiple registers
    int16_t read16BitRegister(uint8_t regLow);                    // Read 16-bit register (low byte address)
};

#endif // LSM6DSL_H

#endif // MBED_OS

