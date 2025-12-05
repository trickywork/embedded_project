/**
 * @file SymptomDetector.cpp
 * @brief Implementation of symptom detection algorithms
 * 
 * This file contains the core detection algorithms for Parkinson's symptoms:
 * - Frequency domain analysis using FFT
 * - Statistical analysis for gait detection
 * - Multi-condition detection logic
 */

#include "SymptomDetector.h"
#include "FFTProcessor.h"
#include <cmath>
#include <algorithm>

/**
 * @brief Constructor - Initialize symptom detector
 * 
 * Initializes gait analysis variables to zero.
 */
SymptomDetector::SymptomDetector() : lastStepTime(0), stepCount(0), cadence(0) {
}

/**
 * @brief Initialize symptom detector
 * 
 * Resets all gait analysis variables for a fresh start.
 */
void SymptomDetector::begin() {
    lastStepTime = 0;
    stepCount = 0;
    cadence = 0;
}

/**
 * @brief Main analysis function - Detect all symptoms in a data window
 * 
 * This is the core function that processes a 3-second data window and detects
 * all three symptoms. The analysis pipeline:
 * 
 * 1. Data Preprocessing: Remove DC component (mean) from accelerometer data
 * 2. Magnitude Calculation: Compute acceleration magnitude for gait analysis
 * 3. Tremor Detection: FFT analysis in 3-5Hz range with background noise comparison
 * 4. Dyskinesia Detection: FFT analysis in 5-7Hz range with background noise comparison
 * 5. Gait Analysis: Detect steps and calculate cadence
 * 6. FOG Detection: Analyze gait pattern and sudden movement stop
 * 
 * @param accelX, accelY, accelZ Accelerometer data arrays (g)
 * @param gyroX, gyroY, gyroZ Gyroscope data arrays (deg/s)
 * @param windowSize Number of samples (156 for 3 seconds at 52Hz)
 * @return SymptomResults structure with detection results and intensities
 */
SymptomResults SymptomDetector::analyze(float* accelX, float* accelY, float* accelZ,
                                        float* gyroX, float* gyroY, float* gyroZ,
                                        int windowSize) {
    // Initialize results structure (all false, intensities 0.0)
    SymptomResults results = {false, 0.0f, false, 0.0f, false, 0.0f};
    
    // Step 1: Data Preprocessing - Remove DC component (mean removal)
    // This removes gravity and sensor offset, leaving only AC signal for FFT analysis
    float* processedX = new float[windowSize];
    float* processedY = new float[windowSize];
    float* processedZ = new float[windowSize];
    
    // Calculate mean (DC component) for each axis
    float meanX = 0.0f, meanY = 0.0f, meanZ = 0.0f;
    for (int i = 0; i < windowSize; i++) {
        meanX += accelX[i];
        meanY += accelY[i];
        meanZ += accelZ[i];
    }
    meanX /= windowSize;
    meanY /= windowSize;
    meanZ /= windowSize;
    
    // Subtract mean to remove DC component
    for (int i = 0; i < windowSize; i++) {
        processedX[i] = accelX[i] - meanX;
        processedY[i] = accelY[i] - meanY;
        processedZ[i] = accelZ[i] - meanZ;
    }
    
    // Step 2: Calculate acceleration magnitude for gait analysis
    // Magnitude = sqrt(X² + Y² + Z²) - represents overall movement intensity
    float* accelMagnitude = new float[windowSize];
    for (int i = 0; i < windowSize; i++) {
        accelMagnitude[i] = sqrt(accelX[i]*accelX[i] + 
                                accelY[i]*accelY[i] + 
                                accelZ[i]*accelZ[i]);
    }
    
    // Step 3: Tremor Detection (3-5Hz frequency range)
    // Calculate energy in tremor frequency band
    results.tremorIntensity = calculateIntensity(processedX, processedY, processedZ, windowSize, 3.0f, 5.0f);
    // Calculate background noise (0-2Hz) for comparison
    float backgroundNoise = calculateIntensity(processedX, processedY, processedZ, windowSize, 0.0f, 2.0f);
    // Detection criteria: intensity > 0.25 AND > 1.2x background noise
    results.tremorDetected = (results.tremorIntensity > 0.25f) && 
                            (results.tremorIntensity > backgroundNoise * 1.2f);
    
    // Step 4: Dyskinesia Detection (5-7Hz frequency range)
    // Calculate energy in dyskinesia frequency band
    results.dyskinesiaIntensity = calculateIntensity(processedX, processedY, processedZ, windowSize, 5.0f, 7.0f);
    // Detection criteria: intensity > 0.25 AND > 1.2x background noise
    results.dyskinesiaDetected = (results.dyskinesiaIntensity > 0.25f) && 
                                (results.dyskinesiaIntensity > backgroundNoise * 1.2f);
    
    // Step 5: Gait Analysis
    // Detect steps and calculate cadence (steps per second)
    analyzeGait(accelX, accelY, accelZ, windowSize);
    
    // Step 6: Freezing of Gait Detection
    // Analyze gait pattern and detect sudden movement stop
    results.fogDetected = detectFOG(accelX, accelY, accelZ, 
                                   gyroX, gyroY, gyroZ, windowSize);
    results.fogIntensity = calculateFOGIntensity(accelMagnitude, windowSize);
    
    // Clean up allocated memory
    delete[] processedX;
    delete[] processedY;
    delete[] processedZ;
    delete[] accelMagnitude;
    
    return results;
}

/**
 * @brief Detect tremor in accelerometer data
 * 
 * Legacy function - detection logic has been moved to analyze() function.
 * Kept for interface compatibility.
 * 
 * @param accelX, accelY, accelZ Accelerometer data arrays
 * @param size Number of samples
 * @return true if tremor detected, false otherwise
 */
bool SymptomDetector::detectTremor(float* accelX, float* accelY, float* accelZ, int size) {
    // Detection logic moved to analyze() function, kept here for compatibility
    float intensity = calculateIntensity(accelX, accelY, accelZ, size, 3.0f, 5.0f);
    return intensity > 0.15f;
}

/**
 * @brief Detect dyskinesia in accelerometer data
 * 
 * Legacy function - detection logic has been moved to analyze() function.
 * Kept for interface compatibility.
 * 
 * @param accelX, accelY, accelZ Accelerometer data arrays
 * @param size Number of samples
 * @return true if dyskinesia detected, false otherwise
 */
bool SymptomDetector::detectDyskinesia(float* accelX, float* accelY, float* accelZ, int size) {
    // Detection logic moved to analyze() function, kept here for compatibility
    float intensity = calculateIntensity(accelX, accelY, accelZ, size, 5.0f, 7.0f);
    return intensity > 0.15f;
}

/**
 * @brief Detect Freezing of Gait (FOG) in sensor data
 * 
 * FOG detection algorithm based on three conditions:
 * 1. Previous walking activity: Cadence > 0.3 steps/second
 * 2. Sudden movement stop: Low variance in latter third of data window
 * 3. Variance reduction: Latter third variance < 50% of first third variance
 * 
 * The algorithm divides the 3-second window into three segments:
 * - First third: Walking phase (high variance expected)
 * - Middle third: Transition phase
 * - Last third: Potential freezing phase (low variance expected)
 * 
 * @param accelX, accelY, accelZ Accelerometer data arrays
 * @param gyroX, gyroY, gyroZ Gyroscope data arrays
 * @param size Number of samples
 * @return true if FOG detected, false otherwise
 */
bool SymptomDetector::detectFOG(float* accelX, float* accelY, float* accelZ,
                                float* gyroX, float* gyroY, float* gyroZ, int size) {
    // FOG Detection Logic:
    // 1. Previous gait activity (cadence > 0.3 steps/sec)
    // 2. Sudden stop (acceleration and angular velocity become very small)
    // 3. Check latter third of data (simulating sudden stop)
    
    // Divide data into three segments: first 1/3 (walking), middle 1/3 (transition), last 1/3 (potential freezing)
    int thirdSize = size / 3;
    
    // Calculate variance of first third (walking phase)
    // High variance indicates active movement
    float* accelXFirst = accelX;
    float* accelYFirst = accelY;
    float* accelZFirst = accelZ;
    float accelVarianceFirst = calculateVariance(accelXFirst, accelYFirst, accelZFirst, thirdSize);
    
    // Calculate variance of last third (potential freezing phase)
    // Low variance indicates minimal movement (freezing)
    float* accelXLast = accelX + 2 * thirdSize;
    float* accelYLast = accelY + 2 * thirdSize;
    float* accelZLast = accelZ + 2 * thirdSize;
    float* gyroXLast = gyroX + 2 * thirdSize;
    float* gyroYLast = gyroY + 2 * thirdSize;
    float* gyroZLast = gyroZ + 2 * thirdSize;
    
    float accelVarianceLast = calculateVariance(accelXLast, accelYLast, accelZLast, thirdSize);
    float gyroVarianceLast = calculateVariance(gyroXLast, gyroYLast, gyroZLast, thirdSize);
    
    // Three conditions must all be true for FOG detection:
    // 1. Was walking: Cadence > 0.3 steps/second (lowered threshold)
    // 2. Is frozen: Very low variance in last third (< 0.01)
    // 3. Sudden stop: Last third variance < 50% of first third variance (relaxed requirement)
    bool wasWalking = (cadence > 0.3f);
    bool isFrozen = (accelVarianceLast < 0.01f && gyroVarianceLast < 0.01f);
    bool suddenStop = (accelVarianceLast < accelVarianceFirst * 0.5f);
    
    return wasWalking && isFrozen && suddenStop;
}

/**
 * @brief Calculate signal intensity in a specific frequency range (single axis)
 * 
 * Uses FFT to analyze frequency content and calculates energy in the specified range.
 * Combines peak energy and average energy for robust detection.
 * 
 * @param data Input data array (single axis, DC-removed)
 * @param size Number of samples
 * @param minFreq Minimum frequency of interest (Hz)
 * @param maxFreq Maximum frequency of interest (Hz)
 * @return Normalized intensity value (0.0 - 1.0)
 */
float SymptomDetector::calculateIntensity(float* data, int size, float minFreq, float maxFreq) {
    // Use FFT to calculate energy in specified frequency range (single axis)
    FFTProcessor fft;
    fft.process(data, size, 52.0f); // 52Hz sampling rate
    
    float maxEnergy = 0.0f;    // Peak energy in frequency range
    float totalEnergy = 0.0f;  // Total energy in frequency range
    int count = 0;              // Number of frequency bins in range
    
    // Iterate through FFT bins and accumulate energy in target frequency range
    for (int i = 0; i < size / 2; i++) {
        float freq = fft.getFrequency(i, 52.0f, size);
        if (freq >= minFreq && freq <= maxFreq) {
            float magnitude = fft.getMagnitude(i);
            totalEnergy += magnitude;
            maxEnergy = std::max(maxEnergy, magnitude);
            count++;
        }
    }
    
    if (count == 0) return 0.0f;  // No data in frequency range
    
    // Combine peak energy and average energy (weighted towards peak)
    // Peak energy (80% weight): Captures dominant frequency component
    // Average energy (20% weight): Captures overall energy distribution
    float avgEnergy = totalEnergy / count;
    float combinedEnergy = (maxEnergy * 0.8f + avgEnergy * 0.2f);
    
    // Normalize to 0-1 range (adjusted normalization factor for better sensitivity)
    return std::min(1.0f, combinedEnergy / 1.2f);
}

/**
 * @brief Calculate signal intensity in a specific frequency range (three axes)
 * 
 * Calculates intensity for each axis separately and returns the maximum.
 * This ensures detection if symptom appears in any axis.
 * 
 * @param dataX, dataY, dataZ Input data arrays (three axes, DC-removed)
 * @param size Number of samples
 * @param minFreq Minimum frequency of interest (Hz)
 * @param maxFreq Maximum frequency of interest (Hz)
 * @return Maximum intensity across all three axes (0.0 - 1.0)
 */
float SymptomDetector::calculateIntensity(float* dataX, float* dataY, float* dataZ, int size, float minFreq, float maxFreq) {
    // Calculate intensity for each axis separately
    float intensityX = calculateIntensity(dataX, size, minFreq, maxFreq);
    float intensityY = calculateIntensity(dataY, size, minFreq, maxFreq);
    float intensityZ = calculateIntensity(dataZ, size, minFreq, maxFreq);
    
    // Return maximum intensity across all three axes
    // This ensures detection if symptom appears in any direction
    return std::max({intensityX, intensityY, intensityZ});
}

/**
 * @brief Calculate Freezing of Gait (FOG) intensity
 * 
 * Calculates variance of acceleration magnitude. Lower variance indicates
 * less movement, which corresponds to higher freezing intensity.
 * 
 * Uses the latter half of the data window for more accurate freezing detection,
 * as freezing typically occurs after a period of walking.
 * 
 * @param accelMagnitude Acceleration magnitude array
 * @param size Number of samples
 * @return FOG intensity (0.0 - 1.0), where 1.0 = completely frozen
 */
float SymptomDetector::calculateFOGIntensity(float* accelMagnitude, int size) {
    // Calculate variance of acceleration magnitude
    // Lower variance = less movement = higher freezing intensity
    
    // Calculate mean of entire window
    float mean = 0.0f;
    for (int i = 0; i < size; i++) {
        mean += accelMagnitude[i];
    }
    mean /= size;
    
    // Calculate variance of entire window
    float variance = 0.0f;
    for (int i = 0; i < size; i++) {
        float diff = accelMagnitude[i] - mean;
        variance += diff * diff;
    }
    variance /= size;
    
    // Check variance of latter half (more accurately reflects freezing state)
    // Freezing typically occurs in the latter part of the window
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
    
    // Use latter half variance for stricter detection
    // Invert and normalize: variance 0.005 -> intensity 1.0, variance > 0.005 -> intensity 0.0
    return std::min(1.0f, std::max(0.0f, (0.005f - varianceHalf) / 0.005f));
}

/**
 * @brief Analyze gait pattern and calculate cadence
 * 
 * Detects steps in the acceleration magnitude signal and calculates
 * cadence (steps per second). This is used for FOG detection to determine
 * if there was previous walking activity.
 * 
 * @param accelX, accelY, accelZ Accelerometer data arrays
 * @param size Number of samples
 */
void SymptomDetector::analyzeGait(float* accelX, float* accelY, float* accelZ, int size) {
    // Calculate acceleration magnitude (overall movement intensity)
    float* accelMagnitude = new float[size];
    for (int i = 0; i < size; i++) {
        accelMagnitude[i] = sqrt(accelX[i]*accelX[i] + 
                                accelY[i]*accelY[i] + 
                                accelZ[i]*accelZ[i]);
    }
    
    // Detect steps using peak detection algorithm
    int steps = detectSteps(accelMagnitude, size);
    
    // Calculate cadence (steps per second)
    // Window is 3 seconds, so cadence = steps / 3.0
    cadence = steps / 3.0f;
    
    delete[] accelMagnitude;
}

/**
 * @brief Detect steps in acceleration magnitude signal
 * 
 * Uses a simple peak detection algorithm:
 * 1. Calculate adaptive threshold (mean + 0.5 * standard deviation)
 * 2. Detect peaks that cross threshold and are local maxima
 * 3. Count peaks as steps
 * 
 * @param accelMagnitude Acceleration magnitude array
 * @param size Number of samples
 * @return Number of steps detected
 */
int SymptomDetector::detectSteps(float* accelMagnitude, int size) {
    // Simple peak detection algorithm
    int steps = 0;
    float threshold = 0.0f;
    
    // Calculate adaptive threshold (mean + 0.5 * standard deviation)
    // This adapts to different movement intensities
    float mean = 0.0f;
    for (int i = 0; i < size; i++) {
        mean += accelMagnitude[i];
    }
    mean /= size;
    
    // Calculate standard deviation
    float stdDev = 0.0f;
    for (int i = 0; i < size; i++) {
        float diff = accelMagnitude[i] - mean;
        stdDev += diff * diff;
    }
    stdDev = sqrt(stdDev / size);
    
    // Threshold = mean + 0.5 * standard deviation
    threshold = mean + stdDev * 0.5f;
    
    // Detect peaks: value crosses threshold and is local maximum
    bool wasAbove = false;  // Track if previous value was above threshold
    for (int i = 1; i < size - 1; i++) {
        bool isAbove = accelMagnitude[i] > threshold;
        // Peak detected if: crosses threshold upward AND is local maximum
        if (isAbove && !wasAbove && 
            accelMagnitude[i] > accelMagnitude[i-1] &&
            accelMagnitude[i] > accelMagnitude[i+1]) {
            steps++;
        }
        wasAbove = isAbove;
    }
    
    return steps;
}

/**
 * @brief Calculate variance of 3-axis sensor data
 * 
 * Combines X, Y, Z axes into magnitude and calculates variance.
 * Variance measures how much the signal varies, which is useful for
 * detecting movement vs. stillness.
 * 
 * @param x, y, z Input data arrays (three axes)
 * @param size Number of samples
 * @return Variance value (higher = more variation = more movement)
 */
float SymptomDetector::calculateVariance(float* x, float* y, float* z, int size) {
    // Calculate combined variance of 3-axis data
    // First compute magnitude for each sample
    float* magnitude = new float[size];
    for (int i = 0; i < size; i++) {
        magnitude[i] = sqrt(x[i]*x[i] + y[i]*y[i] + z[i]*z[i]);
    }
    
    // Calculate mean
    float mean = 0.0f;
    for (int i = 0; i < size; i++) {
        mean += magnitude[i];
    }
    mean /= size;
    
    // Calculate variance: average of squared differences from mean
    float variance = 0.0f;
    for (int i = 0; i < size; i++) {
        float diff = magnitude[i] - mean;
        variance += diff * diff;
    }
    variance /= size;
    
    delete[] magnitude;
    return variance;
}

