#ifndef SENSOR_MANAGER_H
#define SENSOR_MANAGER_H

#include "mbed_compat.h"

// 传感器数据结构
struct SensorData {
    float accelX, accelY, accelZ;  // 加速度 (g)
    float gyroX, gyroY, gyroZ;     // 角速度 (deg/s)
};

class SensorManager {
public:
    SensorManager();
    bool begin();
    SensorData read();
    
    // 用于测试的模拟模式
    void setSimulationMode(bool enabled);
    void setSimulationData(float accelX, float accelY, float accelZ,
                          float gyroX, float gyroY, float gyroZ);
    
private:
    bool simulationMode;
    SensorData simulatedData;
    
    // 实际硬件初始化（STM32）
    #ifdef MBED_OS
    void initHardware();
    I2C* i2c;
    class LSM6DSL* lsm6dsl;
    #endif
    
    // 模拟数据生成（用于测试）
    #ifdef NATIVE_TEST_MODE
    float generateSimulatedValue(float base, float amplitude, float frequency, int timeMs);
    struct timeval simStartTime;
    bool simTimerStarted;
    int getSimTimeMs();
    #endif
};

#endif
