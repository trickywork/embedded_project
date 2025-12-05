#include "SensorManager.h"
#include <cstdlib>
#include <cmath>
#ifdef NATIVE_TEST_MODE
#include <sys/time.h>
#endif

SensorManager::SensorManager() : simulationMode(false) {
    simulatedData = {0, 0, 0, 0, 0, 0};
    #ifdef NATIVE_TEST_MODE
    simTimerStarted = false;
    gettimeofday(&simStartTime, nullptr);
    simTimerStarted = true;
    #endif
}

bool SensorManager::begin() {
    #ifdef MBED_OS
        // STM32硬件初始化
        initHardware();
        return true;
    #else
        // 原生模式（电脑测试）- 使用模拟数据
        #ifdef NATIVE_TEST_MODE
        printf("运行在模拟模式（电脑测试）\n");
        #else
        printf("运行在模拟模式（电脑测试）\r\n");
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

SensorData SensorManager::read() {
    if (simulationMode) {
        #ifdef NATIVE_TEST_MODE
            // 生成模拟的传感器数据（包含不同频率的信号用于测试）
            int timeMs = getSimTimeMs();
            
            // 基础值 + 噪声
            float noise = (rand() % 20 - 10) / 1000.0f;
            
            SensorData data;
            // 可以在这里添加不同频率的信号来测试检测算法
            // 例如：4Hz的震颤信号，6Hz的运动障碍信号等
            data.accelX = 0.1f * sin(2.0f * M_PI * 4.0f * timeMs / 1000.0f) + noise;
            data.accelY = 0.1f * sin(2.0f * M_PI * 4.0f * timeMs / 1000.0f + M_PI/4) + noise;
            data.accelZ = 0.1f + noise;
            data.gyroX = noise * 10;
            data.gyroY = noise * 10;
            data.gyroZ = noise * 10;
            
            return data;
        #else
            // 返回模拟数据（添加一些噪声使其更真实）
            SensorData data = simulatedData;
            // 添加小的随机噪声
            data.accelX += (rand() % 20 - 10) / 1000.0f;
            data.accelY += (rand() % 20 - 10) / 1000.0f;
            data.accelZ += (rand() % 20 - 10) / 1000.0f;
            return data;
        #endif
    }
    
    #ifdef MBED_OS
        // 实际硬件读取
        // TODO: 实现实际的传感器读取代码
        // STM32 L475VG IoT Discovery板有LSM6DSL传感器
        SensorData data = {0, 0, 0, 0, 0, 0};
        // 这里需要添加实际的传感器读取代码
        return data;
    #else
        return simulatedData;
    #endif
}

#ifdef MBED_OS
void SensorManager::initHardware() {
    // STM32硬件初始化
    // TODO: 根据STM32 L475VG IoT Discovery板添加传感器初始化代码
    // 该板有LSM6DSL加速度计/陀螺仪传感器
    // 需要配置I2C接口和传感器寄存器
}
#endif

#ifdef NATIVE_TEST_MODE
int SensorManager::getSimTimeMs() {
    if (!simTimerStarted) return 0;
    struct timeval currentTime;
    gettimeofday(&currentTime, nullptr);
    return (currentTime.tv_sec - simStartTime.tv_sec) * 1000 + 
           (currentTime.tv_usec - simStartTime.tv_usec) / 1000;
}

float SensorManager::generateSimulatedValue(float base, float amplitude, 
                                            float frequency, int timeMs) {
    return base + amplitude * sin(2.0f * M_PI * frequency * timeMs / 1000.0f);
}
#endif
