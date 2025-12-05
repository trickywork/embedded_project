/**
 * @file SymptomDetector.h
 * @brief Symptom detection algorithms for Parkinson's disease
 * 
 * This class implements detection algorithms for three Parkinson's symptoms:
 * - Tremor: 3-5Hz frequency analysis using FFT
 * - Dyskinesia: 5-7Hz frequency analysis using FFT
 * - Freezing of Gait (FOG): Gait analysis + sudden movement stop detection
 * 
 * The detection is based on frequency domain analysis (FFT) and statistical
 * analysis of accelerometer and gyroscope data.
 */

#ifndef SYMPTOM_DETECTOR_H
#define SYMPTOM_DETECTOR_H

#include "mbed_compat.h"

/**
 * @struct SymptomResults
 * @brief Structure containing detection results for all three symptoms
 * 
 * Each symptom has:
 * - Detection flag: true if symptom detected, false otherwise
 * - Intensity: Normalized value from 0.0 to 1.0 indicating symptom severity
 */
struct SymptomResults {
    bool tremorDetected;           // True if tremor detected in 3-5Hz range
    float tremorIntensity;        // Tremor intensity (0.0 - 1.0)
    
    bool dyskinesiaDetected;      // True if dyskinesia detected in 5-7Hz range
    float dyskinesiaIntensity;    // Dyskinesia intensity (0.0 - 1.0)
    
    bool fogDetected;             // True if freezing of gait detected
    float fogIntensity;           // FOG intensity (0.0 - 1.0)
};

/**
 * @class SymptomDetector
 * @brief Main class for symptom detection algorithms
 * 
 * Analyzes sensor data using FFT and statistical methods to detect
 * Parkinson's disease symptoms. Processes 3-second data windows.
 */
class SymptomDetector {
public:
    SymptomDetector();
    
    /**
     * @brief Initialize symptom detector
     * 
     * Resets gait analysis variables (step count, cadence, etc.)
     */
    void begin();
    
    /**
     * @brief Analyze sensor data and detect symptoms
     * 
     * Main analysis function that processes a 3-second data window:
     * 1. Preprocesses data (removes DC component)
     * 2. Performs FFT frequency analysis
     * 3. Detects tremor (3-5Hz)
     * 4. Detects dyskinesia (5-7Hz)
     * 5. Analyzes gait and detects FOG
     * 
     * @param accelX, accelY, accelZ Accelerometer data arrays (g)
     * @param gyroX, gyroY, gyroZ Gyroscope data arrays (deg/s)
     * @param windowSize Number of samples (typically 156 for 3 seconds at 52Hz)
     * @return SymptomResults structure with detection results
     */
    SymptomResults analyze(float* accelX, float* accelY, float* accelZ,
                          float* gyroX, float* gyroY, float* gyroZ,
                          int windowSize);
    
private:
    // Gait analysis variables
    float lastStepTime;    // Timestamp of last detected step
    int stepCount;         // Number of steps detected in current window
    float cadence;         // Steps per second (gait cadence)
    
    // Detection methods for individual symptoms
    bool detectTremor(float* accelX, float* accelY, float* accelZ, int size);
    bool detectDyskinesia(float* accelX, float* accelY, float* accelZ, int size);
    bool detectFOG(float* accelX, float* accelY, float* accelZ, 
                  float* gyroX, float* gyroY, float* gyroZ, int size);
    
    // Frequency analysis and intensity calculation
    float calculateIntensity(float* data, int size, float minFreq, float maxFreq);
    float calculateIntensity(float* dataX, float* dataY, float* dataZ, int size, float minFreq, float maxFreq);
    float calculateFOGIntensity(float* accelMagnitude, int size);
    float calculateVariance(float* x, float* y, float* z, int size);
    
    // Gait analysis methods
    void analyzeGait(float* accelX, float* accelY, float* accelZ, int size);
    int detectSteps(float* accelMagnitude, int size);
};

#endif
