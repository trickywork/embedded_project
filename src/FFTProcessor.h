/**
 * @file FFTProcessor.h
 * @brief Fast Fourier Transform (FFT) processor for frequency domain analysis
 * 
 * This class implements FFT to convert time-domain sensor data into frequency domain.
 * Used to detect specific frequency components (3-5Hz for tremor, 5-7Hz for dyskinesia).
 * 
 * Implements Cooley-Tukey FFT algorithm for efficient computation.
 */

#ifndef FFT_PROCESSOR_H
#define FFT_PROCESSOR_H

#include "mbed_compat.h"
#include <complex>
#include <cmath>

/**
 * @class FFTProcessor
 * @brief FFT processor for frequency domain analysis
 * 
 * Converts time-domain signals to frequency domain using FFT.
 * Provides methods to query frequency and magnitude at specific bins.
 */
class FFTProcessor {
public:
    FFTProcessor();
    ~FFTProcessor();
    
    /**
     * @brief Process data and perform FFT
     * 
     * Converts input time-domain data to frequency domain.
     * 
     * @param data Input time-domain data array
     * @param size Number of samples
     * @param samplingFreq Sampling frequency in Hz (52Hz for this project)
     */
    void process(float* data, int size, float samplingFreq);
    
    /**
     * @brief Get frequency corresponding to a specific FFT bin
     * 
     * @param bin FFT bin index (0 to size/2)
     * @param samplingFreq Sampling frequency in Hz
     * @param size Number of samples
     * @return Frequency in Hz
     */
    float getFrequency(int bin, float samplingFreq, int size);
    
    /**
     * @brief Get magnitude (energy) at a specific FFT bin
     * 
     * @param bin FFT bin index
     * @return Magnitude value (higher = more energy at that frequency)
     */
    float getMagnitude(int bin);
    
private:
    std::complex<float>* fftResult;  // FFT output (complex numbers)
    int fftSize;                      // Size of FFT (number of samples)
    
    // FFT implementation using Cooley-Tukey algorithm (divide and conquer)
    void fft(std::complex<float>* x, int n);
    void ifft(std::complex<float>* x, int n);  // Inverse FFT (not used in this project)
};

#endif

