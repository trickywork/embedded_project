/**
 * @file main.cpp
 * @brief Main program for Parkinson's Disease Symptom Detection System
 * 
 * This program implements a real-time symptom detection system for Parkinson's disease
 * using accelerometer and gyroscope data. It detects three main symptoms:
 * - Tremor: 3-5Hz rhythmic oscillations
 * - Dyskinesia: 5-7Hz dance-like movements
 * - Freezing of Gait (FOG): Sudden body freezing after walking
 * 
 * The system samples sensor data at 52Hz, collects 3-second windows (156 samples),
 * and uses FFT analysis to detect frequency-specific symptoms.
 */

#include "mbed_compat.h"
#include "SensorManager.h"
#include "SymptomDetector.h"
#include "BLEManager.h"

// Global objects for sensor management, symptom detection, and BLE communication
SensorManager sensorManager;
SymptomDetector symptomDetector;
BLEManager bleManager;

// Data buffers for storing sensor readings
// Window size: 3 seconds * 52 Hz = 156 samples
const int WINDOW_SIZE = 156;
float accelX[WINDOW_SIZE];  // Accelerometer X-axis data (g)
float accelY[WINDOW_SIZE];  // Accelerometer Y-axis data (g)
float accelZ[WINDOW_SIZE];  // Accelerometer Z-axis data (g)
float gyroX[WINDOW_SIZE];   // Gyroscope X-axis data (deg/s)
float gyroY[WINDOW_SIZE];   // Gyroscope Y-axis data (deg/s)
float gyroZ[WINDOW_SIZE];   // Gyroscope Z-axis data (deg/s)

Timer timer;  // Timer for precise sampling rate control
const int SAMPLE_INTERVAL_MS = 1000 / 52;  // Sampling interval: ~19.23ms for 52Hz

/**
 * @brief Main program entry point
 * 
 * Initializes all system components, then enters the main loop which:
 * 1. Samples sensor data at 52Hz
 * 2. Collects 3-second data windows
 * 3. Analyzes data for symptom detection
 * 4. Transmits results via BLE
 * 
 * @return int Exit code (0 for success, -1 for initialization failure)
 */
int main() {
    printf("=== Parkinson's Disease Symptom Detection System ===\r\n");
    
    // Initialize sensor hardware (LSM6DSL on ST B-L475E-IOT01A1)
    if (!sensorManager.begin()) {
        printf("ERROR: Sensor initialization failed!\r\n");
        return -1;
    }
    
    // Initialize symptom detection algorithm
    symptomDetector.begin();
    
    // Initialize BLE communication for transmitting detection results
    if (!bleManager.begin()) {
        printf("WARNING: BLE initialization failed, continuing in simulation mode\r\n");
    }
    
    printf("System initialization complete. Starting data acquisition...\r\n");
    
    // Start timer and initialize sampling variables
    timer.start();
    int sampleIndex = 0;      // Current position in data buffer
    int lastSampleTime = 0;   // Timestamp of last sample
    
    // Main processing loop
    while (true) {
        int currentTime = timer.read_ms();
        
        // Sample data at 52Hz (every ~19.23ms)
        if (currentTime - lastSampleTime >= SAMPLE_INTERVAL_MS) {
            // Read sensor data (accelerometer and gyroscope)
            SensorData data = sensorManager.read();
            
            // Store data in circular buffer
            accelX[sampleIndex] = data.accelX;
            accelY[sampleIndex] = data.accelY;
            accelZ[sampleIndex] = data.accelZ;
            gyroX[sampleIndex] = data.gyroX;
            gyroY[sampleIndex] = data.gyroY;
            gyroZ[sampleIndex] = data.gyroZ;
            
            sampleIndex++;
            
            // When buffer is full (3 seconds of data collected), perform analysis
            if (sampleIndex >= WINDOW_SIZE) {
                sampleIndex = 0;  // Reset buffer index for next window
                
                // Perform symptom detection analysis on collected data
                SymptomResults results = symptomDetector.analyze(
                    accelX, accelY, accelZ,
                    gyroX, gyroY, gyroZ,
                    WINDOW_SIZE
                );
                
                // Print detection results to serial console
                printf("\r\n=== Detection Results ===\r\n");
                printf("Tremor: %s (Intensity: %.2f)\r\n", 
                       results.tremorDetected ? "YES" : "NO",
                       results.tremorIntensity);
                
                printf("Dyskinesia: %s (Intensity: %.2f)\r\n", 
                       results.dyskinesiaDetected ? "YES" : "NO",
                       results.dyskinesiaIntensity);
                
                printf("Freezing of Gait: %s (Intensity: %.2f)\r\n", 
                       results.fogDetected ? "YES" : "NO",
                       results.fogIntensity);
                
                // Transmit detection results via BLE to connected mobile device
                bleManager.updateCharacteristics(
                    results.tremorDetected,
                    results.tremorIntensity,
                    results.dyskinesiaDetected,
                    results.dyskinesiaIntensity,
                    results.fogDetected,
                    results.fogIntensity
                );
            }
            
            lastSampleTime = currentTime;
        }
        
        // Process BLE events (handle connections, notifications, etc.)
        bleManager.update();
        
        // Small delay to prevent CPU spinning
        thread_sleep_for(1);
    }
    
    return 0;
}
