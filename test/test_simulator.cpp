#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <ctime>
#include "../src/SensorManager.h"
#include "../src/SymptomDetector.h"
#include "../src/BLEManager.h"

// 测试数据生成函数
void generateTremorData(SensorManager& sensor, int durationMs) {
    printf("生成震颤测试数据 (4Hz, 持续%dms)...\r\n", durationMs);
    Timer timer;
    timer.start();
    
    while (timer.read_ms() < durationMs) {
        int timeMs = timer.read_ms();
        float accelX = 0.2f * sin(2.0f * M_PI * 4.0f * timeMs / 1000.0f);
        float accelY = 0.2f * sin(2.0f * M_PI * 4.0f * timeMs / 1000.0f + M_PI/4);
        float accelZ = 1.0f; // 重力
        
        sensor.setSimulationData(accelX, accelY, accelZ, 0, 0, 0);
        thread_sleep_for(1000 / 52); // 52Hz
    }
}

void generateDyskinesiaData(SensorManager& sensor, int durationMs) {
    printf("生成运动障碍测试数据 (6Hz, 持续%dms)...\r\n", durationMs);
    Timer timer;
    timer.start();
    
    while (timer.read_ms() < durationMs) {
        int timeMs = timer.read_ms();
        float accelX = 0.3f * sin(2.0f * M_PI * 6.0f * timeMs / 1000.0f);
        float accelY = 0.3f * sin(2.0f * M_PI * 6.0f * timeMs / 1000.0f + M_PI/3);
        float accelZ = 1.0f;
        
        sensor.setSimulationData(accelX, accelY, accelZ, 0, 0, 0);
        thread_sleep_for(1000 / 52);
    }
}

void generateFOGData(SensorManager& sensor, int durationMs) {
    printf("生成冻结步态测试数据 (先行走后冻结, 持续%dms)...\r\n", durationMs);
    Timer timer;
    timer.start();
    
    while (timer.read_ms() < durationMs) {
        int timeMs = timer.read_ms();
        float accelX, accelY, accelZ;
        
        if (timeMs < durationMs / 2) {
            // 前半段：模拟行走
            accelX = 0.5f * sin(2.0f * M_PI * 2.0f * timeMs / 1000.0f);
            accelY = 0.5f * sin(2.0f * M_PI * 2.0f * timeMs / 1000.0f + M_PI/2);
            accelZ = 1.0f;
        } else {
            // 后半段：冻结
            accelX = 0.01f;
            accelY = 0.01f;
            accelZ = 1.0f;
        }
        
        sensor.setSimulationData(accelX, accelY, accelZ, 0, 0, 0);
        thread_sleep_for(1000 / 52);
    }
}

int main() {
    printf("=== 帕金森症状检测系统 - 电脑端测试 ===\r\n\r\n");
    
    SensorManager sensorManager;
    SymptomDetector symptomDetector;
    BLEManager bleManager;
    
    // 初始化
    sensorManager.begin();
    symptomDetector.begin();
    bleManager.begin();
    
    sensorManager.setSimulationMode(true);
    
    const int WINDOW_SIZE = 156;
    float accelX[WINDOW_SIZE];
    float accelY[WINDOW_SIZE];
    float accelZ[WINDOW_SIZE];
    float gyroX[WINDOW_SIZE];
    float gyroY[WINDOW_SIZE];
    float gyroZ[WINDOW_SIZE];
    
    // 测试1: 震颤检测
    printf("\r\n========== 测试1: 震颤检测 ==========\r\n");
    generateTremorData(sensorManager, 3000);
    
    // 收集数据
    for (int i = 0; i < WINDOW_SIZE; i++) {
        SensorData data = sensorManager.read();
        accelX[i] = data.accelX;
        accelY[i] = data.accelY;
        accelZ[i] = data.accelZ;
        gyroX[i] = data.gyroX;
        gyroY[i] = data.gyroY;
        gyroZ[i] = data.gyroZ;
        thread_sleep_for(1000 / 52);
    }
    
    SymptomResults results = symptomDetector.analyze(
        accelX, accelY, accelZ, gyroX, gyroY, gyroZ, WINDOW_SIZE);
    
    printf("结果: 震颤=%s (强度=%.2f)\r\n", 
           results.tremorDetected ? "检测到" : "未检测到",
           results.tremorIntensity);
    
    // 测试2: 运动障碍检测
    printf("\r\n========== 测试2: 运动障碍检测 ==========\r\n");
    generateDyskinesiaData(sensorManager, 3000);
    
    for (int i = 0; i < WINDOW_SIZE; i++) {
        SensorData data = sensorManager.read();
        accelX[i] = data.accelX;
        accelY[i] = data.accelY;
        accelZ[i] = data.accelZ;
        gyroX[i] = data.gyroX;
        gyroY[i] = data.gyroY;
        gyroZ[i] = data.gyroZ;
        thread_sleep_for(1000 / 52);
    }
    
    results = symptomDetector.analyze(
        accelX, accelY, accelZ, gyroX, gyroY, gyroZ, WINDOW_SIZE);
    
    printf("结果: 运动障碍=%s (强度=%.2f)\r\n", 
           results.dyskinesiaDetected ? "检测到" : "未检测到",
           results.dyskinesiaIntensity);
    
    // 测试3: 冻结步态检测
    printf("\r\n========== 测试3: 冻结步态检测 ==========\r\n");
    generateFOGData(sensorManager, 3000);
    
    for (int i = 0; i < WINDOW_SIZE; i++) {
        SensorData data = sensorManager.read();
        accelX[i] = data.accelX;
        accelY[i] = data.accelY;
        accelZ[i] = data.accelZ;
        gyroX[i] = data.gyroX;
        gyroY[i] = data.gyroY;
        gyroZ[i] = data.gyroZ;
        thread_sleep_for(1000 / 52);
    }
    
    results = symptomDetector.analyze(
        accelX, accelY, accelZ, gyroX, gyroY, gyroZ, WINDOW_SIZE);
    
    printf("结果: 冻结步态=%s (强度=%.2f)\r\n", 
           results.fogDetected ? "检测到" : "未检测到",
           results.fogIntensity);
    
    printf("\r\n========== 所有测试完成 ==========\r\n");
    
    return 0;
}

