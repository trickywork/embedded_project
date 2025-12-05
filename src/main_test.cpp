/**
 * @file main_test.cpp
 * @brief Test program for computer-side algorithm validation
 * 
 * This program tests symptom detection algorithms without hardware.
 * It generates simulated sensor data with known frequencies to verify
 * that detection algorithms work correctly.
 * 
 * Test scenarios:
 * 1. Normal data (low-amplitude random motion)
 * 2. Tremor signal (4Hz)
 * 3. Dyskinesia signal (6Hz)
 * 4. Freezing of Gait (walking then freezing)
 */

#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <ctime>
#include <unistd.h>
#include "SensorManager.h"
#include "SymptomDetector.h"
#include "BLEManager.h"
#include "mbed_compat.h"

/**
 * @brief Generate tremor test data (4Hz signal)
 * 
 * Generates a 4Hz sinusoidal signal to test tremor detection.
 * This simulates typical Parkinson's tremor frequency.
 * 
 * @param sensor Sensor manager reference
 * @param durationMs Duration of test signal in milliseconds
 */
void generateTremorData(SensorManager& sensor, int durationMs) {
    printf("Generating tremor test data (4Hz, duration %dms)...\n", durationMs);
    Timer timer;
    timer.start();
    
    while (timer.read_ms() < durationMs) {
        int timeMs = timer.read_ms();
        // Generate 4Hz sinusoidal signal (tremor frequency)
        float accelX = 0.2f * sin(2.0f * M_PI * 4.0f * timeMs / 1000.0f);
        float accelY = 0.2f * sin(2.0f * M_PI * 4.0f * timeMs / 1000.0f + M_PI/4);
        float accelZ = 1.0f; // Gravity component
        
        sensor.setSimulationData(accelX, accelY, accelZ, 0, 0, 0);
        thread_sleep_for(1000 / 52); // 52Hz sampling rate
    }
}

/**
 * @brief Generate dyskinesia test data (6Hz signal)
 * 
 * Generates a 6Hz sinusoidal signal to test dyskinesia detection.
 * This simulates typical dyskinetic movement frequency.
 * 
 * @param sensor Sensor manager reference
 * @param durationMs Duration of test signal in milliseconds
 */
void generateDyskinesiaData(SensorManager& sensor, int durationMs) {
    printf("Generating dyskinesia test data (6Hz, duration %dms)...\n", durationMs);
    Timer timer;
    timer.start();
    
    while (timer.read_ms() < durationMs) {
        int timeMs = timer.read_ms();
        // Generate 6Hz sinusoidal signal (dyskinesia frequency)
        float accelX = 0.3f * sin(2.0f * M_PI * 6.0f * timeMs / 1000.0f);
        float accelY = 0.3f * sin(2.0f * M_PI * 6.0f * timeMs / 1000.0f + M_PI/3);
        float accelZ = 1.0f;
        
        sensor.setSimulationData(accelX, accelY, accelZ, 0, 0, 0);
        thread_sleep_for(1000 / 52);
    }
}

/**
 * @brief Generate Freezing of Gait test data
 * 
 * Generates data that simulates walking then sudden freezing:
 * - First half: Walking pattern (oscillating signal)
 * - Second half: Freezing (minimal movement)
 * 
 * @param sensor Sensor manager reference
 * @param durationMs Duration of test signal in milliseconds
 */
void generateFOGData(SensorManager& sensor, int durationMs) {
    printf("Generating FOG test data (walking then freezing, duration %dms)...\n", durationMs);
    Timer timer;
    timer.start();
    
    while (timer.read_ms() < durationMs) {
        int timeMs = timer.read_ms();
        float accelX, accelY, accelZ;
        
        if (timeMs < durationMs / 2) {
            // First half: Simulate walking (oscillating signal at 2Hz)
            accelX = 0.5f * sin(2.0f * M_PI * 2.0f * timeMs / 1000.0f);
            accelY = 0.5f * sin(2.0f * M_PI * 2.0f * timeMs / 1000.0f + M_PI/2);
            accelZ = 1.0f;
        } else {
            // Second half: Freezing (minimal movement)
            accelX = 0.01f;
            accelY = 0.01f;
            accelZ = 1.0f;
        }
        
        sensor.setSimulationData(accelX, accelY, accelZ, 0, 0, 0);
        thread_sleep_for(1000 / 52);
    }
}

/**
 * @brief Generate normal data (low-amplitude random motion)
 * 
 * Generates random low-amplitude motion to test that normal movement
 * does not trigger false positives.
 * 
 * @param sensor Sensor manager reference
 * @param durationMs Duration of test signal in milliseconds
 */
void generateNormalData(SensorManager& sensor, int durationMs) {
    printf("Generating normal data (low-amplitude random motion, duration %dms)...\n", durationMs);
    Timer timer;
    timer.start();
    srand(time(nullptr));
    
    while (timer.read_ms() < durationMs) {
        // Generate low-amplitude random motion (simulating normal activity)
        float accelX = (rand() % 20 - 10) / 100.0f;  // -0.1 to 0.1 g
        float accelY = (rand() % 20 - 10) / 100.0f;
        float accelZ = 1.0f + (rand() % 10 - 5) / 100.0f;  // Gravity with small variation
        
        sensor.setSimulationData(accelX, accelY, accelZ, 0, 0, 0);
        thread_sleep_for(1000 / 52);
    }
}

/**
 * @brief Main test program entry point
 * 
 * Runs a series of test scenarios to validate detection algorithms:
 * 1. Normal data test (should not detect symptoms)
 * 2. Tremor test (should detect tremor)
 * 3. Dyskinesia test (should detect dyskinesia)
 * 4. FOG test (should detect freezing of gait)
 * 
 * Each test generates simulated sensor data and runs detection analysis.
 */
int main() {
    printf("========================================\n");
    printf("  Parkinson's Symptom Detection System\n");
    printf("         Computer-Side Testing\n");
    printf("========================================\n\n");
    
    // Initialize random number seed
    srand(time(nullptr));
    
    // Create system components
    SensorManager sensorManager;
    SymptomDetector symptomDetector;
    BLEManager bleManager;
    
    // Initialize components
    printf("Initializing system components...\n");
    sensorManager.begin();
    symptomDetector.begin();
    bleManager.begin();
    
    // Enable simulation mode
    sensorManager.setSimulationMode(true);
    printf("System switched to simulation mode\n\n");
    
    // Data buffer for 3-second window (156 samples at 52Hz)
    const int WINDOW_SIZE = 156; // 3 seconds * 52 Hz
    float accelX[WINDOW_SIZE];
    float accelY[WINDOW_SIZE];
    float accelZ[WINDOW_SIZE];
    float gyroX[WINDOW_SIZE];
    float gyroY[WINDOW_SIZE];
    float gyroZ[WINDOW_SIZE];
    
    // Test 1: Normal data (should not detect symptoms)
    printf("========== Test 1: Normal Data ==========\n");
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
    
    printf("Detection Results:\n");
    printf("  Tremor: %s (Intensity: %.2f)\n", 
           results.tremorDetected ? "DETECTED" : "NOT DETECTED",
           results.tremorIntensity);
    printf("  Dyskinesia: %s (Intensity: %.2f)\n", 
           results.dyskinesiaDetected ? "DETECTED" : "NOT DETECTED",
           results.dyskinesiaIntensity);
    printf("  Freezing of Gait: %s (Intensity: %.2f)\n\n", 
           results.fogDetected ? "DETECTED" : "NOT DETECTED",
           results.fogIntensity);
    
    // Test 2: Tremor detection
    printf("========== Test 2: Tremor Detection (4Hz) ==========\n");
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
    
    printf("Detection Results:\n");
    printf("  Tremor: %s (Intensity: %.2f) %s\n", 
           results.tremorDetected ? "DETECTED" : "NOT DETECTED",
           results.tremorIntensity,
           results.tremorDetected ? "✓" : "✗");
    printf("  Dyskinesia: %s (Intensity: %.2f)\n", 
           results.dyskinesiaDetected ? "DETECTED" : "NOT DETECTED",
           results.dyskinesiaIntensity);
    printf("  Freezing of Gait: %s (Intensity: %.2f)\n\n", 
           results.fogDetected ? "DETECTED" : "NOT DETECTED",
           results.fogIntensity);
    
    // Test 3: Dyskinesia detection
    printf("========== Test 3: Dyskinesia Detection (6Hz) ==========\n");
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
    
    printf("Detection Results:\n");
    printf("  Tremor: %s (Intensity: %.2f)\n", 
           results.tremorDetected ? "DETECTED" : "NOT DETECTED",
           results.tremorIntensity);
    printf("  Dyskinesia: %s (Intensity: %.2f) %s\n", 
           results.dyskinesiaDetected ? "DETECTED" : "NOT DETECTED",
           results.dyskinesiaIntensity,
           results.dyskinesiaDetected ? "✓" : "✗");
    printf("  Freezing of Gait: %s (Intensity: %.2f)\n\n", 
           results.fogDetected ? "DETECTED" : "NOT DETECTED",
           results.fogIntensity);
    
    // Test 4: Freezing of Gait detection
    printf("========== Test 4: Freezing of Gait Detection ==========\n");
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
    
    printf("Detection Results:\n");
    printf("  Tremor: %s (Intensity: %.2f)\n", 
           results.tremorDetected ? "DETECTED" : "NOT DETECTED",
           results.tremorIntensity);
    printf("  Dyskinesia: %s (Intensity: %.2f)\n", 
           results.dyskinesiaDetected ? "DETECTED" : "NOT DETECTED",
           results.dyskinesiaIntensity);
    printf("  Freezing of Gait: %s (Intensity: %.2f) %s\n\n", 
           results.fogDetected ? "DETECTED" : "NOT DETECTED",
           results.fogIntensity,
           results.fogDetected ? "✓" : "✗");
    
    printf("========================================\n");
    printf("  All tests completed!\n");
    printf("========================================\n");
    
    return 0;
}

