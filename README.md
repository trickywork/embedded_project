# Parkinson's Disease Symptom Detection System

An embedded system based on STM32 and Mbed framework for real-time detection of Parkinson's disease symptoms using accelerometer and gyroscope data.

## Overview

This project implements a wearable device that monitors and detects three primary symptoms of Parkinson's disease:

- **Tremor**: Rhythmic oscillations in the frequency range of 3-5Hz, typically affecting the dominant arm
- **Dyskinesia**: Dance-like movements in the frequency range of 5-7Hz, often caused by excessive dopamine medication
- **Freezing of Gait (FOG)**: Sudden freezing of the body after a period of walking, a late-stage symptom

## Key Features

1. **High-Frequency Data Acquisition**: 52Hz sampling rate with 3-second analysis windows (156 samples)
2. **FFT-Based Frequency Analysis**: Fast Fourier Transform to detect frequency-specific symptoms
3. **Real-Time Symptom Detection**: Continuous monitoring with immediate detection results
4. **BLE Communication**: Wireless transmission of detection results to mobile devices via three BLE characteristics
5. **Simulation Mode**: Complete testing environment without hardware for algorithm development

## Project Structure

```
embedded/
├── platformio.ini          # PlatformIO configuration file
├── src/
│   ├── main.cpp            # Main program for hardware deployment
│   ├── main_test.cpp       # Test program for computer-side testing
│   ├── SensorManager.h/cpp # Sensor management (hardware + simulation)
│   ├── LSM6DSL.h/cpp       # LSM6DSL sensor driver (I2C communication)
│   ├── SymptomDetector.h/cpp # Symptom detection algorithms
│   ├── FFTProcessor.h/cpp  # FFT frequency analysis implementation
│   ├── BLEManager.h/cpp    # BLE communication management
│   └── mbed_compat.h       # Mbed compatibility layer for native testing
├── test/
│   └── main_test.cpp       # Original test file
└── README.md
```

## Hardware Requirements

- **Development Board**: ST B-L475E-IOT01A1 (STM32L475VG IoT Discovery)
- **Onboard Sensors**:
  - LSM6DSL: 3-axis accelerometer and 3-axis gyroscope (I2C interface)
- **Communication**: Integrated BLE module for wireless data transmission
- **Power**: Can be powered by a simple power bank

## Algorithm Description

### Tremor Detection (3-5Hz)
- Uses FFT to analyze accelerometer data
- Detects energy in the 3-5Hz frequency range
- Threshold: Intensity > 0.25 and > 1.2x background noise
- Typical frequency: 4Hz for Parkinson's tremor

### Dyskinesia Detection (5-7Hz)
- Uses FFT to analyze accelerometer data
- Detects energy in the 5-7Hz frequency range
- Threshold: Intensity > 0.25 and > 1.2x background noise
- Typical frequency: 6Hz for dyskinetic movements

### Freezing of Gait (FOG) Detection
1. **Gait Analysis**: Detects steps and calculates cadence (steps per second)
2. **Variance Analysis**: Calculates variance of acceleration and gyroscope data
3. **Freezing Detection**: Identifies sudden stop after walking activity
   - Condition 1: Previous walking activity (cadence > 0.3 steps/sec)
   - Condition 2: Sudden stop (low variance in latter third of data window)
   - Condition 3: Variance reduction (latter third < 50% of first third)

## Building and Running

### Computer-Side Testing (Simulation Mode)

Test all algorithms without hardware:

```bash
# Compile test program
pio run -e native

# Run tests
pio run -e native -t exec
```

The test program automatically runs four test scenarios:
1. Normal data (low-amplitude random motion)
2. Tremor detection (4Hz signal)
3. Dyskinesia detection (6Hz signal)
4. Freezing of gait (walking then freezing)

### Hardware Deployment (ST B-L475E-IOT01A1)

```bash
# Compile for STM32
pio run -e disco_l475vg_iot01a

# Upload to development board
pio run -e disco_l475vg_iot01a -t upload

# Monitor serial output (115200 baud)
pio device monitor
```

## BLE Communication

### Service and Characteristics

- **Service UUID**: `19B10000-E8F2-537E-4F6C-D104768A1214`
- **Tremor Characteristic**: `19B10001-E8F2-537E-4F6C-D104768A1214`
- **Dyskinesia Characteristic**: `19B10002-E8F2-537E-4F6C-D104768A1214`
- **FOG Characteristic**: `19B10003-E8F2-537E-4F6C-D104768A1214`

### Connecting with Mobile Device

1. **Using nRF Connect (Recommended)**:
   - Download nRF Connect app
   - Scan for "ParkinsonDetector"
   - Connect and subscribe to notifications
   - View real-time detection results

2. **Data Format**:
   - Each characteristic contains:
     - Status byte: 0 = not detected, 1 = detected
     - Intensity byte: 0-255 (0.0-1.0 normalized)

## Configuration

### Sensor Configuration (LSM6DSL)

- **Accelerometer**: ±2g range, 52Hz ODR
- **Gyroscope**: ±250dps range, 52Hz ODR
- **I2C Speed**: 400kHz
- **I2C Address**: Auto-detected (0xD6 or 0xD4)

### Detection Thresholds

Current thresholds (can be adjusted in `SymptomDetector.cpp`):
- Tremor: Intensity > 0.25 and > 1.2x background noise
- Dyskinesia: Intensity > 0.25 and > 1.2x background noise
- FOG: Cadence > 0.3 steps/sec, variance reduction > 50%

## Testing

### Test Scenarios

The test program (`main_test.cpp`) includes:

1. **Normal Data Test**: Verifies system doesn't false-positive on normal movement
2. **Tremor Test**: Generates 4Hz signal to verify tremor detection
3. **Dyskinesia Test**: Generates 6Hz signal to verify dyskinesia detection
4. **FOG Test**: Simulates walking then freezing to verify FOG detection

### Expected Results

- ✅ Tremor detection: Successfully detects 4Hz signals
- ✅ Dyskinesia detection: Successfully detects 6Hz signals
- ⚠️ FOG detection: May require further optimization based on real-world data

## Troubleshooting

### Sensor Initialization Fails

**Symptoms**: Serial output shows "Sensor initialization failed!"

**Solutions**:
1. Code automatically tries multiple I2C configurations
2. Check hardware connections
3. Verify sensor is functioning (check WHO_AM_I register)

### BLE Initialization Fails

**Symptoms**: Serial output shows "BLE initialization failed"

**Solutions**:
1. Check Mbed BLE library is properly linked
2. Verify UUIDs are unique
3. Restart development board

### Compilation Errors

**Solutions**:
```bash
# Clean and rebuild
pio run -e disco_l475vg_iot01a -t clean
pio run -e disco_l475vg_iot01a
```

## Future Enhancements

1. **LED Indicators**: Visual feedback for detected symptoms
2. **Button Control**: Start/stop detection via onboard buttons
3. **Data Logging**: Store detection results to Flash memory
4. **Low Power Optimization**: Extend battery life
5. **Calibration**: Sensor calibration functionality
6. **Machine Learning**: Advanced pattern recognition for improved accuracy

## License

This project is developed for educational purposes as part of an embedded systems course.

## References

- ST B-L475E-IOT01A1 Discovery Board User Manual
- LSM6DSL Datasheet
- Mbed OS Documentation
- PlatformIO Documentation
