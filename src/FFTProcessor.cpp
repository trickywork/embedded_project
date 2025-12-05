#include "FFTProcessor.h"
#include <algorithm>

FFTProcessor::FFTProcessor() : fftResult(nullptr), fftSize(0) {
}

FFTProcessor::~FFTProcessor() {
    if (fftResult != nullptr) {
        delete[] fftResult;
    }
}

void FFTProcessor::process(float* data, int size, float samplingFreq) {
    // 分配内存
    if (fftSize != size) {
        if (fftResult != nullptr) {
            delete[] fftResult;
        }
        fftResult = new std::complex<float>[size];
        fftSize = size;
    }
    
    // 复制数据到复数数组
    for (int i = 0; i < size; i++) {
        fftResult[i] = std::complex<float>(data[i], 0.0f);
    }
    
    // 执行FFT
    fft(fftResult, size);
}

float FFTProcessor::getFrequency(int bin, float samplingFreq, int size) {
    return (bin * samplingFreq) / size;
}

float FFTProcessor::getMagnitude(int bin) {
    if (bin >= 0 && bin < fftSize) {
        return std::abs(fftResult[bin]);
    }
    return 0.0f;
}

// Cooley-Tukey FFT算法实现
void FFTProcessor::fft(std::complex<float>* x, int n) {
    // 检查是否为2的幂
    if (n <= 1) return;
    
    // 如果不是2的幂，使用零填充或截断
    int pow2 = 1;
    while (pow2 < n) pow2 *= 2;
    
    if (pow2 != n) {
        // 零填充到2的幂
        std::complex<float>* padded = new std::complex<float>[pow2];
        for (int i = 0; i < n; i++) {
            padded[i] = x[i];
        }
        for (int i = n; i < pow2; i++) {
            padded[i] = std::complex<float>(0.0f, 0.0f);
        }
        fft(padded, pow2);
        // 复制回原数组（只取前n个）
        for (int i = 0; i < n; i++) {
            x[i] = padded[i];
        }
        delete[] padded;
        return;
    }
    
    // 分治
    std::complex<float>* even = new std::complex<float>[n/2];
    std::complex<float>* odd = new std::complex<float>[n/2];
    
    for (int i = 0; i < n/2; i++) {
        even[i] = x[i*2];
        odd[i] = x[i*2+1];
    }
    
    fft(even, n/2);
    fft(odd, n/2);
    
    // 合并
    for (int k = 0; k < n/2; k++) {
        float angle = -2.0f * static_cast<float>(M_PI) * static_cast<float>(k) / static_cast<float>(n);
        std::complex<float> t = std::polar(1.0f, angle) * odd[k];
        x[k] = even[k] + t;
        x[k + n/2] = even[k] - t;
    }
    
    delete[] even;
    delete[] odd;
}

void FFTProcessor::ifft(std::complex<float>* x, int n) {
    // 共轭
    for (int i = 0; i < n; i++) {
        x[i] = std::conj(x[i]);
    }
    
    // FFT
    fft(x, n);
    
    // 共轭并归一化
    for (int i = 0; i < n; i++) {
        x[i] = std::conj(x[i]) / float(n);
    }
}

