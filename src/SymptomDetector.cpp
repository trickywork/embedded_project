#include "SymptomDetector.h"
#include "FFTProcessor.h"
#include <cmath>
#include <algorithm>

SymptomDetector::SymptomDetector() : lastStepTime(0), stepCount(0), cadence(0) {
}

void SymptomDetector::begin() {
    lastStepTime = 0;
    stepCount = 0;
    cadence = 0;
}

SymptomResults SymptomDetector::analyze(float* accelX, float* accelY, float* accelZ,
                                        float* gyroX, float* gyroY, float* gyroZ,
                                        int windowSize) {
    SymptomResults results = {false, 0.0f, false, 0.0f, false, 0.0f};
    
    // 预处理：去均值（去除DC分量）
    float* processedX = new float[windowSize];
    float* processedY = new float[windowSize];
    float* processedZ = new float[windowSize];
    
    float meanX = 0.0f, meanY = 0.0f, meanZ = 0.0f;
    for (int i = 0; i < windowSize; i++) {
        meanX += accelX[i];
        meanY += accelY[i];
        meanZ += accelZ[i];
    }
    meanX /= windowSize;
    meanY /= windowSize;
    meanZ /= windowSize;
    
    for (int i = 0; i < windowSize; i++) {
        processedX[i] = accelX[i] - meanX;
        processedY[i] = accelY[i] - meanY;
        processedZ[i] = accelZ[i] - meanZ;
    }
    
    // 计算加速度幅值
    float* accelMagnitude = new float[windowSize];
    for (int i = 0; i < windowSize; i++) {
        accelMagnitude[i] = sqrt(accelX[i]*accelX[i] + 
                                accelY[i]*accelY[i] + 
                                accelZ[i]*accelZ[i]);
    }
    
    // 检测震颤 (3-5Hz)
    results.tremorIntensity = calculateIntensity(processedX, processedY, processedZ, windowSize, 3.0f, 5.0f);
    // 使用绝对阈值，但检查是否明显高于背景噪声
    float backgroundNoise = calculateIntensity(processedX, processedY, processedZ, windowSize, 0.0f, 2.0f);
    results.tremorDetected = (results.tremorIntensity > 0.25f) && 
                            (results.tremorIntensity > backgroundNoise * 1.2f);
    
    // 检测运动障碍 (5-7Hz)
    results.dyskinesiaIntensity = calculateIntensity(processedX, processedY, processedZ, windowSize, 5.0f, 7.0f);
    // 使用绝对阈值，但检查是否明显高于背景噪声
    results.dyskinesiaDetected = (results.dyskinesiaIntensity > 0.25f) && 
                                (results.dyskinesiaIntensity > backgroundNoise * 1.2f);
    
    // 分析步态
    analyzeGait(accelX, accelY, accelZ, windowSize);
    
    // 检测冻结步态
    results.fogDetected = detectFOG(accelX, accelY, accelZ, 
                                   gyroX, gyroY, gyroZ, windowSize);
    results.fogIntensity = calculateFOGIntensity(accelMagnitude, windowSize);
    
    delete[] processedX;
    delete[] processedY;
    delete[] processedZ;
    delete[] accelMagnitude;
    
    return results;
}

bool SymptomDetector::detectTremor(float* accelX, float* accelY, float* accelZ, int size) {
    // 已移至analyze函数中，保留此函数以保持接口兼容
    float intensity = calculateIntensity(accelX, accelY, accelZ, size, 3.0f, 5.0f);
    return intensity > 0.15f;
}

bool SymptomDetector::detectDyskinesia(float* accelX, float* accelY, float* accelZ, int size) {
    // 已移至analyze函数中，保留此函数以保持接口兼容
    float intensity = calculateIntensity(accelX, accelY, accelZ, size, 5.0f, 7.0f);
    return intensity > 0.15f;
}

bool SymptomDetector::detectFOG(float* accelX, float* accelY, float* accelZ,
                                float* gyroX, float* gyroY, float* gyroZ, int size) {
    // 冻结步态检测逻辑：
    // 1. 之前有步态活动（步频 > 0.5步/秒）
    // 2. 突然停止（加速度和角速度都变得很小）
    // 3. 检查后半段数据（模拟突然停止）
    
    // 将数据分成三段：前1/3（行走），中1/3（过渡），后1/3（可能冻结）
    int thirdSize = size / 3;
    
    // 前1/3的方差（行走阶段）
    float* accelXFirst = accelX;
    float* accelYFirst = accelY;
    float* accelZFirst = accelZ;
    float accelVarianceFirst = calculateVariance(accelXFirst, accelYFirst, accelZFirst, thirdSize);
    
    // 后1/3的方差（可能冻结阶段）
    float* accelXLast = accelX + 2 * thirdSize;
    float* accelYLast = accelY + 2 * thirdSize;
    float* accelZLast = accelZ + 2 * thirdSize;
    float* gyroXLast = gyroX + 2 * thirdSize;
    float* gyroYLast = gyroY + 2 * thirdSize;
    float* gyroZLast = gyroZ + 2 * thirdSize;
    
    float accelVarianceLast = calculateVariance(accelXLast, accelYLast, accelZLast, thirdSize);
    float gyroVarianceLast = calculateVariance(gyroXLast, gyroYLast, gyroZLast, thirdSize);
    
    // 如果之前有步态活动，且后半段运动突然变小，可能是冻结
    bool wasWalking = (cadence > 0.3f); // 步频大于0.3步/秒（降低要求）
    bool isFrozen = (accelVarianceLast < 0.01f && gyroVarianceLast < 0.01f); // 后段方差很小
    bool suddenStop = (accelVarianceLast < accelVarianceFirst * 0.5f); // 后段方差明显小于前段（放宽要求）
    
    return wasWalking && isFrozen && suddenStop;
}

float SymptomDetector::calculateIntensity(float* data, int size, float minFreq, float maxFreq) {
    // 使用FFT计算指定频率范围内的能量（单轴）
    FFTProcessor fft;
    fft.process(data, size, 52.0f); // 52Hz采样率
    
    float maxEnergy = 0.0f;
    float totalEnergy = 0.0f;
    int count = 0;
    
    for (int i = 0; i < size / 2; i++) {
        float freq = fft.getFrequency(i, 52.0f, size);
        if (freq >= minFreq && freq <= maxFreq) {
            float magnitude = fft.getMagnitude(i);
            totalEnergy += magnitude;
            maxEnergy = std::max(maxEnergy, magnitude);
            count++;
        }
    }
    
    if (count == 0) return 0.0f;
    
    // 使用峰值能量和平均能量的组合（更重视峰值）
    float avgEnergy = totalEnergy / count;
    float combinedEnergy = (maxEnergy * 0.8f + avgEnergy * 0.2f);
    
    // 归一化到0-1范围（调整归一化因子，使其对信号更敏感）
    return std::min(1.0f, combinedEnergy / 1.2f);
}

float SymptomDetector::calculateIntensity(float* dataX, float* dataY, float* dataZ, int size, float minFreq, float maxFreq) {
    // 计算三轴数据的组合强度（取最大值）
    float intensityX = calculateIntensity(dataX, size, minFreq, maxFreq);
    float intensityY = calculateIntensity(dataY, size, minFreq, maxFreq);
    float intensityZ = calculateIntensity(dataZ, size, minFreq, maxFreq);
    
    // 返回三轴中的最大强度
    return std::max({intensityX, intensityY, intensityZ});
}

float SymptomDetector::calculateFOGIntensity(float* accelMagnitude, int size) {
    // 计算加速度幅值的方差，方差越小，冻结程度越高
    float mean = 0.0f;
    for (int i = 0; i < size; i++) {
        mean += accelMagnitude[i];
    }
    mean /= size;
    
    float variance = 0.0f;
    for (int i = 0; i < size; i++) {
        float diff = accelMagnitude[i] - mean;
        variance += diff * diff;
    }
    variance /= size;
    
    // 检查后半段的方差（更准确反映冻结状态）
    float meanHalf = 0.0f;
    for (int i = size / 2; i < size; i++) {
        meanHalf += accelMagnitude[i];
    }
    meanHalf /= (size - size / 2);
    
    float varianceHalf = 0.0f;
    for (int i = size / 2; i < size; i++) {
        float diff = accelMagnitude[i] - meanHalf;
        varianceHalf += diff * diff;
    }
    varianceHalf /= (size - size / 2);
    
    // 使用后半段方差，更严格
    return std::min(1.0f, std::max(0.0f, (0.005f - varianceHalf) / 0.005f));
}

void SymptomDetector::analyzeGait(float* accelX, float* accelY, float* accelZ, int size) {
    // 计算加速度幅值
    float* accelMagnitude = new float[size];
    for (int i = 0; i < size; i++) {
        accelMagnitude[i] = sqrt(accelX[i]*accelX[i] + 
                                accelY[i]*accelY[i] + 
                                accelZ[i]*accelZ[i]);
    }
    
    // 检测步数
    int steps = detectSteps(accelMagnitude, size);
    
    // 计算步频（步/秒）
    // 3秒窗口
    cadence = steps / 3.0f;
    
    delete[] accelMagnitude;
}

int SymptomDetector::detectSteps(float* accelMagnitude, int size) {
    // 简单的峰值检测算法
    int steps = 0;
    float threshold = 0.0f;
    
    // 计算阈值（平均值 + 标准差）
    float mean = 0.0f;
    for (int i = 0; i < size; i++) {
        mean += accelMagnitude[i];
    }
    mean /= size;
    
    float stdDev = 0.0f;
    for (int i = 0; i < size; i++) {
        float diff = accelMagnitude[i] - mean;
        stdDev += diff * diff;
    }
    stdDev = sqrt(stdDev / size);
    
    threshold = mean + stdDev * 0.5f;
    
    // 检测峰值
    bool wasAbove = false;
    for (int i = 1; i < size - 1; i++) {
        bool isAbove = accelMagnitude[i] > threshold;
        if (isAbove && !wasAbove && 
            accelMagnitude[i] > accelMagnitude[i-1] &&
            accelMagnitude[i] > accelMagnitude[i+1]) {
            steps++;
        }
        wasAbove = isAbove;
    }
    
    return steps;
}

float SymptomDetector::calculateVariance(float* x, float* y, float* z, int size) {
    // 计算三轴数据的组合方差
    float* magnitude = new float[size];
    for (int i = 0; i < size; i++) {
        magnitude[i] = sqrt(x[i]*x[i] + y[i]*y[i] + z[i]*z[i]);
    }
    
    float mean = 0.0f;
    for (int i = 0; i < size; i++) {
        mean += magnitude[i];
    }
    mean /= size;
    
    float variance = 0.0f;
    for (int i = 0; i < size; i++) {
        float diff = magnitude[i] - mean;
        variance += diff * diff;
    }
    variance /= size;
    
    delete[] magnitude;
    return variance;
}

