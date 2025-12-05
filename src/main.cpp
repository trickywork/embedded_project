#include "mbed_compat.h"
#include "SensorManager.h"
#include "SymptomDetector.h"
#include "BLEManager.h"

// 全局对象
SensorManager sensorManager;
SymptomDetector symptomDetector;
BLEManager bleManager;

// 数据缓冲区
const int WINDOW_SIZE = 156; // 3秒 * 52Hz
float accelX[WINDOW_SIZE];
float accelY[WINDOW_SIZE];
float accelZ[WINDOW_SIZE];
float gyroX[WINDOW_SIZE];
float gyroY[WINDOW_SIZE];
float gyroZ[WINDOW_SIZE];

Timer timer;
const int SAMPLE_INTERVAL_MS = 1000 / 52; // 52Hz采样率

int main() {
    printf("=== 帕金森症状检测系统启动 ===\r\n");
    
    // 初始化传感器
    if (!sensorManager.begin()) {
        printf("错误: 传感器初始化失败!\r\n");
        return -1;
    }
    
    // 初始化症状检测器
    symptomDetector.begin();
    
    // 初始化BLE
    if (!bleManager.begin()) {
        printf("警告: BLE初始化失败，继续运行（模拟模式）\r\n");
    }
    
    printf("系统初始化完成，开始数据采集...\r\n");
    
    timer.start();
    int sampleIndex = 0;
    int lastSampleTime = 0;
    
    while (true) {
        int currentTime = timer.read_ms();
        
        // 按52Hz采样率采集数据
        if (currentTime - lastSampleTime >= SAMPLE_INTERVAL_MS) {
            // 读取传感器数据
            SensorData data = sensorManager.read();
            
            // 存储到缓冲区
            accelX[sampleIndex] = data.accelX;
            accelY[sampleIndex] = data.accelY;
            accelZ[sampleIndex] = data.accelZ;
            gyroX[sampleIndex] = data.gyroX;
            gyroY[sampleIndex] = data.gyroY;
            gyroZ[sampleIndex] = data.gyroZ;
            
            sampleIndex++;
            
            // 当收集满3秒数据时进行分析
            if (sampleIndex >= WINDOW_SIZE) {
                sampleIndex = 0;
                
                // 进行症状检测
                SymptomResults results = symptomDetector.analyze(
                    accelX, accelY, accelZ,
                    gyroX, gyroY, gyroZ,
                    WINDOW_SIZE
                );
                
                // 打印结果
                printf("\r\n=== 检测结果 ===\r\n");
                printf("震颤: %s (强度: %.2f)\r\n", 
                       results.tremorDetected ? "是" : "否",
                       results.tremorIntensity);
                
                printf("运动障碍: %s (强度: %.2f)\r\n", 
                       results.dyskinesiaDetected ? "是" : "否",
                       results.dyskinesiaIntensity);
                
                printf("冻结步态: %s (强度: %.2f)\r\n", 
                       results.fogDetected ? "是" : "否",
                       results.fogIntensity);
                
                // 通过BLE发送数据
                bleManager.updateCharacteristics(
                    results.tremorDetected,
                    results.tremorIntensity,
                    results.dyskinesiaDetected,
                    results.dyskinesiaIntensity,
                    results.fogDetected,
                    results.fogIntensity
                );
            }
            
            lastSampleTime = currentTime;
        }
        
        // 处理BLE事件
        bleManager.update();
        
        thread_sleep_for(1);
    }
    
    return 0;
}
