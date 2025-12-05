#ifndef LSM6DSL_H
#define LSM6DSL_H

#ifdef MBED_OS
#include "mbed.h"

// LSM6DSL寄存器地址
#define LSM6DSL_WHO_AM_I         0x0F
#define LSM6DSL_CTRL1_XL         0x10  // 加速度计控制寄存器
#define LSM6DSL_CTRL2_G          0x11  // 陀螺仪控制寄存器
#define LSM6DSL_CTRL3_C          0x12
#define LSM6DSL_STATUS_REG       0x1E
#define LSM6DSL_OUTX_L_XL        0x28  // 加速度计X轴低字节
#define LSM6DSL_OUTX_H_XL        0x29
#define LSM6DSL_OUTY_L_XL        0x2A
#define LSM6DSL_OUTY_H_XL        0x2B
#define LSM6DSL_OUTZ_L_XL        0x2C
#define LSM6DSL_OUTZ_H_XL        0x2D
#define LSM6DSL_OUTX_L_G         0x22  // 陀螺仪X轴低字节
#define LSM6DSL_OUTX_H_G         0x23
#define LSM6DSL_OUTY_L_G         0x24
#define LSM6DSL_OUTY_H_G         0x25
#define LSM6DSL_OUTZ_L_G         0x26
#define LSM6DSL_OUTZ_H_G         0x27

// LSM6DSL I2C地址
// 注意: B-L475E-IOT01A1板可能使用0xD6或0xD4，取决于SA0引脚
#define LSM6DSL_I2C_ADDRESS      0xD6  // SA0 = 1 (默认)
#define LSM6DSL_I2C_ADDRESS_ALT 0xD4  // SA0 = 0 (备用地址)

// 加速度计量程和灵敏度
#define LSM6DSL_ACCEL_FS_2G      0x00
#define LSM6DSL_ACCEL_FS_4G      0x08
#define LSM6DSL_ACCEL_FS_8G      0x0C
#define LSM6DSL_ACCEL_FS_16G     0x04
#define LSM6DSL_ACCEL_SENSITIVITY_2G   0.061f   // mg/LSB
#define LSM6DSL_ACCEL_SENSITIVITY_4G   0.122f
#define LSM6DSL_ACCEL_SENSITIVITY_8G   0.244f
#define LSM6DSL_ACCEL_SENSITIVITY_16G  0.488f

// 陀螺仪量程和灵敏度
#define LSM6DSL_GYRO_FS_125DPS   0x02
#define LSM6DSL_GYRO_FS_250DPS   0x00
#define LSM6DSL_GYRO_FS_500DPS   0x04
#define LSM6DSL_GYRO_FS_1000DPS  0x08
#define LSM6DSL_GYRO_FS_2000DPS  0x0C
#define LSM6DSL_GYRO_SENSITIVITY_125DPS  4.375f   // mdps/LSB
#define LSM6DSL_GYRO_SENSITIVITY_250DPS  8.75f
#define LSM6DSL_GYRO_SENSITIVITY_500DPS  17.5f
#define LSM6DSL_GYRO_SENSITIVITY_1000DPS 35.0f
#define LSM6DSL_GYRO_SENSITIVITY_2000DPS 70.0f

// 输出数据速率 (ODR)
#define LSM6DSL_ODR_POWER_DOWN   0x00
#define LSM6DSL_ODR_12_5_HZ      0x01
#define LSM6DSL_ODR_26_HZ        0x02
#define LSM6DSL_ODR_52_HZ        0x03  // 我们需要的52Hz
#define LSM6DSL_ODR_104_HZ       0x04
#define LSM6DSL_ODR_208_HZ       0x05
#define LSM6DSL_ODR_416_HZ       0x06
#define LSM6DSL_ODR_833_HZ       0x07
#define LSM6DSL_ODR_1_66K_HZ     0x08
#define LSM6DSL_ODR_3_33K_HZ     0x09
#define LSM6DSL_ODR_6_66K_HZ     0x0A

class LSM6DSL {
public:
    LSM6DSL(I2C* i2c);
    bool init();
    bool readAccel(float& x, float& y, float& z);
    bool readGyro(float& x, float& y, float& z);
    bool dataReady();
    
private:
    I2C* _i2c;
    uint8_t _address;
    float _accelSensitivity;
    float _gyroSensitivity;
    
    bool writeRegister(uint8_t reg, uint8_t value);
    bool readRegister(uint8_t reg, uint8_t& value);
    bool readRegisters(uint8_t reg, uint8_t* data, int length);
    int16_t read16BitRegister(uint8_t regLow);
};

#endif // LSM6DSL_H

#endif // MBED_OS

