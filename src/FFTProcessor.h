#ifndef FFT_PROCESSOR_H
#define FFT_PROCESSOR_H

#include "mbed_compat.h"
#include <complex>
#include <cmath>

class FFTProcessor {
public:
    FFTProcessor();
    ~FFTProcessor();
    
    // 处理数据并执行FFT
    void process(float* data, int size, float samplingFreq);
    
    // 获取频率
    float getFrequency(int bin, float samplingFreq, int size);
    
    // 获取幅值
    float getMagnitude(int bin);
    
private:
    std::complex<float>* fftResult;
    int fftSize;
    
    // 简单的FFT实现（使用Cooley-Tukey算法）
    void fft(std::complex<float>* x, int n);
    void ifft(std::complex<float>* x, int n);
};

#endif

