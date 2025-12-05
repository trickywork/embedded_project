#ifndef SYMPTOM_DETECTOR_H
#define SYMPTOM_DETECTOR_H

#include "mbed_compat.h"

// 检测结果结构
struct SymptomResults {
    bool tremorDetected;
    float tremorIntensity;      // 0.0 - 1.0
    
    bool dyskinesiaDetected;
    float dyskinesiaIntensity;  // 0.0 - 1.0
    
    bool fogDetected;
    float fogIntensity;         // 0.0 - 1.0
};

class SymptomDetector {
public:
    SymptomDetector();
    void begin();
    
    SymptomResults analyze(float* accelX, float* accelY, float* accelZ,
                          float* gyroX, float* gyroY, float* gyroZ,
                          int windowSize);
    
private:
    // 步态检测相关
    float lastStepTime;
    int stepCount;
    float cadence;  // 步频
    
    // 检测方法
    bool detectTremor(float* accelX, float* accelY, float* accelZ, int size);
    bool detectDyskinesia(float* accelX, float* accelY, float* accelZ, int size);
    bool detectFOG(float* accelX, float* accelY, float* accelZ, 
                  float* gyroX, float* gyroY, float* gyroZ, int size);
    
    float calculateIntensity(float* data, int size, float minFreq, float maxFreq);
    float calculateFOGIntensity(float* accelMagnitude, int size);
    float calculateVariance(float* x, float* y, float* z, int size);
    
    // 步态分析
    void analyzeGait(float* accelX, float* accelY, float* accelZ, int size);
    int detectSteps(float* accelMagnitude, int size);
};

#endif
