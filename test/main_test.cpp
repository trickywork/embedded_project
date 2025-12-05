#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <ctime>
#include <unistd.h>
#include "../src/SensorManager.h"
#include "../src/SymptomDetector.h"
#include "../src/BLEManager.h"
#include "../src/mbed_compat.h"

// 测试数据生成函数
void generateTremorData(SensorManager& sensor, int durationMs) {
    printf("生成震颤测试数据 (4Hz, 持续%dms)...\n", durationMs);
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
    printf("生成运动障碍测试数据 (6Hz, 持续%dms)...\n", durationMs);
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
    printf("生成冻结步态测试数据 (先行走后冻结, 持续%dms)...\n", durationMs);
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

void generateNormalData(SensorManager& sensor, int durationMs) {
    printf("生成正常数据 (低幅度随机运动, 持续%dms)...\n", durationMs);
    Timer timer;
    timer.start();
    srand(time(nullptr));
    
    while (timer.read_ms() < durationMs) {
        // 生成低幅度的随机运动
        float accelX = (rand() % 20 - 10) / 100.0f;
        float accelY = (rand() % 20 - 10) / 100.0f;
        float accelZ = 1.0f + (rand() % 10 - 5) / 100.0f;
        
        sensor.setSimulationData(accelX, accelY, accelZ, 0, 0, 0);
        thread_sleep_for(1000 / 52);
    }
}

int main() {
    printf("========================================\n");
    printf("  帕金森症状检测系统 - 电脑端测试\n");
    printf("========================================\n\n");
    
    // 初始化随机数种子
    srand(time(nullptr));
    
    SensorManager sensorManager;
    SymptomDetector symptomDetector;
    BLEManager bleManager;
    
    // 初始化
    printf("初始化系统组件...\n");
    sensorManager.begin();
    symptomDetector.begin();
    bleManager.begin();
    
    sensorManager.setSimulationMode(true);
    printf("系统已切换到模拟模式\n\n");
    
    const int WINDOW_SIZE = 156; // 3秒 * 52Hz
    float accelX[WINDOW_SIZE];
    float accelY[WINDOW_SIZE];
    float accelZ[WINDOW_SIZE];
    float gyroX[WINDOW_SIZE];
    float gyroY[WINDOW_SIZE];
    float gyroZ[WINDOW_SIZE];
    
    // 测试1: 正常数据（不应检测到症状）
    printf("========== 测试1: 正常数据 ==========\n");
    generateNormalData(sensorManager, 3000);
    
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
    
    printf("检测结果:\n");
    printf("  震颤: %s (强度: %.2f)\n", 
           results.tremorDetected ? "检测到" : "未检测到",
           results.tremorIntensity);
    printf("  运动障碍: %s (强度: %.2f)\n", 
           results.dyskinesiaDetected ? "检测到" : "未检测到",
           results.dyskinesiaIntensity);
    printf("  冻结步态: %s (强度: %.2f)\n\n", 
           results.fogDetected ? "检测到" : "未检测到",
           results.fogIntensity);
    
    // 测试2: 震颤检测
    printf("========== 测试2: 震颤检测 (4Hz) ==========\n");
    generateTremorData(sensorManager, 3000);
    
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
    
    printf("检测结果:\n");
    printf("  震颤: %s (强度: %.2f) %s\n", 
           results.tremorDetected ? "检测到" : "未检测到",
           results.tremorIntensity,
           results.tremorDetected ? "✓" : "✗");
    printf("  运动障碍: %s (强度: %.2f)\n", 
           results.dyskinesiaDetected ? "检测到" : "未检测到",
           results.dyskinesiaIntensity);
    printf("  冻结步态: %s (强度: %.2f)\n\n", 
           results.fogDetected ? "检测到" : "未检测到",
           results.fogIntensity);
    
    // 测试3: 运动障碍检测
    printf("========== 测试3: 运动障碍检测 (6Hz) ==========\n");
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
    
    printf("检测结果:\n");
    printf("  震颤: %s (强度: %.2f)\n", 
           results.tremorDetected ? "检测到" : "未检测到",
           results.tremorIntensity);
    printf("  运动障碍: %s (强度: %.2f) %s\n", 
           results.dyskinesiaDetected ? "检测到" : "未检测到",
           results.dyskinesiaIntensity,
           results.dyskinesiaDetected ? "✓" : "✗");
    printf("  冻结步态: %s (强度: %.2f)\n\n", 
           results.fogDetected ? "检测到" : "未检测到",
           results.fogIntensity);
    
    // 测试4: 冻结步态检测
    printf("========== 测试4: 冻结步态检测 ==========\n");
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
    
    printf("检测结果:\n");
    printf("  震颤: %s (强度: %.2f)\n", 
           results.tremorDetected ? "检测到" : "未检测到",
           results.tremorIntensity);
    printf("  运动障碍: %s (强度: %.2f)\n", 
           results.dyskinesiaDetected ? "检测到" : "未检测到",
           results.dyskinesiaIntensity);
    printf("  冻结步态: %s (强度: %.2f) %s\n\n", 
           results.fogDetected ? "检测到" : "未检测到",
           results.fogIntensity,
           results.fogDetected ? "✓" : "✗");
    
    printf("========================================\n");
    printf("  所有测试完成！\n");
    printf("========================================\n");
    
    return 0;
}

