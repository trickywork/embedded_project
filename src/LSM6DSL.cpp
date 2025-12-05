#ifdef MBED_OS

#include "LSM6DSL.h"
#include <cstdio>

LSM6DSL::LSM6DSL(I2C* i2c) : _i2c(i2c), _address(LSM6DSL_I2C_ADDRESS),
    _accelSensitivity(LSM6DSL_ACCEL_SENSITIVITY_2G),
    _gyroSensitivity(LSM6DSL_GYRO_SENSITIVITY_250DPS) {
}

bool LSM6DSL::init() {
    // 检查WHO_AM_I寄存器
    uint8_t whoAmI;
    if (!readRegister(LSM6DSL_WHO_AM_I, whoAmI)) {
        printf("LSM6DSL: 无法读取WHO_AM_I寄存器 (地址0x%02X)\r\n", _address);
        // 尝试备用地址
        if (_address == LSM6DSL_I2C_ADDRESS) {
            _address = LSM6DSL_I2C_ADDRESS_ALT;
            printf("LSM6DSL: 尝试备用地址 0x%02X\r\n", _address);
            if (!readRegister(LSM6DSL_WHO_AM_I, whoAmI)) {
                return false;
            }
        } else {
            return false;
        }
    }
    
    if (whoAmI != 0x6A) {
        printf("LSM6DSL: WHO_AM_I错误 (期望0x6A, 得到0x%02X)\r\n", whoAmI);
        return false;
    }
    
    // 配置加速度计: ±2g, 52Hz
    uint8_t ctrl1_xl = (LSM6DSL_ODR_52_HZ << 4) | (LSM6DSL_ACCEL_FS_2G << 2);
    if (!writeRegister(LSM6DSL_CTRL1_XL, ctrl1_xl)) {
        printf("LSM6DSL: 无法配置加速度计\r\n");
        return false;
    }
    _accelSensitivity = LSM6DSL_ACCEL_SENSITIVITY_2G;
    
    // 配置陀螺仪: ±250dps, 52Hz
    uint8_t ctrl2_g = (LSM6DSL_ODR_52_HZ << 4) | (LSM6DSL_GYRO_FS_250DPS << 2);
    if (!writeRegister(LSM6DSL_CTRL2_G, ctrl2_g)) {
        printf("LSM6DSL: 无法配置陀螺仪\r\n");
        return false;
    }
    _gyroSensitivity = LSM6DSL_GYRO_SENSITIVITY_250DPS;
    
    // 配置CTRL3_C: BDU (Block Data Update) 启用
    if (!writeRegister(LSM6DSL_CTRL3_C, 0x44)) {
        printf("LSM6DSL: 无法配置CTRL3_C\r\n");
        return false;
    }
    
    printf("LSM6DSL: 初始化成功\r\n");
    return true;
}

bool LSM6DSL::readAccel(float& x, float& y, float& z) {
    int16_t rawX = read16BitRegister(LSM6DSL_OUTX_L_XL);
    int16_t rawY = read16BitRegister(LSM6DSL_OUTY_L_XL);
    int16_t rawZ = read16BitRegister(LSM6DSL_OUTZ_L_XL);
    
    // 转换为g单位
    x = (rawX * _accelSensitivity) / 1000.0f;
    y = (rawY * _accelSensitivity) / 1000.0f;
    z = (rawZ * _accelSensitivity) / 1000.0f;
    
    return true;
}

bool LSM6DSL::readGyro(float& x, float& y, float& z) {
    int16_t rawX = read16BitRegister(LSM6DSL_OUTX_L_G);
    int16_t rawY = read16BitRegister(LSM6DSL_OUTY_L_G);
    int16_t rawZ = read16BitRegister(LSM6DSL_OUTZ_L_G);
    
    // 转换为deg/s单位
    x = (rawX * _gyroSensitivity) / 1000.0f;
    y = (rawY * _gyroSensitivity) / 1000.0f;
    z = (rawZ * _gyroSensitivity) / 1000.0f;
    
    return true;
}

bool LSM6DSL::dataReady() {
    uint8_t status;
    if (!readRegister(LSM6DSL_STATUS_REG, status)) {
        return false;
    }
    // 检查加速度计和陀螺仪数据就绪标志
    return (status & 0x03) == 0x03;
}

bool LSM6DSL::writeRegister(uint8_t reg, uint8_t value) {
    char data[2] = {reg, value};
    if (_i2c->write(_address, data, 2) != 0) {
        return false;
    }
    return true;
}

bool LSM6DSL::readRegister(uint8_t reg, uint8_t& value) {
    char regAddr = reg;
    if (_i2c->write(_address, &regAddr, 1, true) != 0) {
        return false;
    }
    char data;
    if (_i2c->read(_address, &data, 1) != 0) {
        return false;
    }
    value = data;
    return true;
}

bool LSM6DSL::readRegisters(uint8_t reg, uint8_t* data, int length) {
    char regAddr = reg;
    if (_i2c->write(_address, &regAddr, 1, true) != 0) {
        return false;
    }
    if (_i2c->read(_address, (char*)data, length) != 0) {
        return false;
    }
    return true;
}

int16_t LSM6DSL::read16BitRegister(uint8_t regLow) {
    uint8_t low, high;
    if (!readRegister(regLow, low) || !readRegister(regLow + 1, high)) {
        return 0;
    }
    return (int16_t)((high << 8) | low);
}

#endif // MBED_OS

