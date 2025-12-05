#include "SensorManager.h"
#include <cstdlib>
#include <cmath>
#ifdef NATIVE_TEST_MODE
#include <sys/time.h>
#endif
#ifdef MBED_OS
#include "LSM6DSL.h"
#endif

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
        if (lsm6dsl == nullptr) {
            SensorData data = {0, 0, 0, 0, 0, 0};
            return data;
        }
        
        SensorData data;
        if (!lsm6dsl->readAccel(data.accelX, data.accelY, data.accelZ)) {
            data = {0, 0, 0, 0, 0, 0};
            return data;
        }
        
        if (!lsm6dsl->readGyro(data.gyroX, data.gyroY, data.gyroZ)) {
            data.gyroX = data.gyroY = data.gyroZ = 0;
        }
        
        return data;
    #else
        return simulatedData;
    #endif
}

#ifdef MBED_OS
void SensorManager::initHardware() {
    // ST B-L475E-IOT01A1板使用I2C1接口连接LSM6DSL
    // 根据板级支持包，使用预定义的I2C引脚
    // 如果预定义不可用，尝试: PB6(SCL), PB7(SDA) 或 PC0(SCL), PC1(SDA)
    
    // 方法1: 尝试使用Mbed预定义的I2C引脚（如果板级支持包提供）
    // i2c = new I2C(I2C_SDA, I2C_SCL);
    
    // 方法2: 使用标准I2C1引脚 (PB6=SCL, PB7=SDA)
    i2c = new I2C(PB_7, PB_6);  // SDA, SCL (I2C1)
    
    // 如果上述失败，可以尝试I2C3 (PC0=SCL, PC1=SDA)
    // i2c = new I2C(PC_1, PC_0);  // SDA, SCL (I2C3)
    
    i2c->frequency(400000);  // 400kHz I2C速度
    
    // 创建LSM6DSL对象
    lsm6dsl = new LSM6DSL(i2c);
    
    // 初始化传感器
    if (!lsm6dsl->init()) {
        printf("传感器初始化失败! 尝试备用I2C引脚...\r\n");
        // 如果I2C1失败，尝试I2C3
        delete i2c;
        delete lsm6dsl;
        
        i2c = new I2C(PC_1, PC_0);  // I2C3: SDA, SCL
        i2c->frequency(400000);
        lsm6dsl = new LSM6DSL(i2c);
        
        if (!lsm6dsl->init()) {
            printf("传感器初始化完全失败!\r\n");
            delete lsm6dsl;
            lsm6dsl = nullptr;
            delete i2c;
            i2c = nullptr;
            return;
        }
    }
    
    printf("传感器初始化成功 (I2C地址: 0x%02X)\r\n", LSM6DSL_I2C_ADDRESS);
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
