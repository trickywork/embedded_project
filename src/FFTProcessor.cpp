/**
 * @file FFTProcessor.cpp
 * @brief Implementation of Fast Fourier Transform processor
 *
 * Implements Cooley-Tukey FFT algorithm for efficient frequency domain analysis.
 * Converts time-domain signals to frequency domain for symptom detection.
 */

#include "FFTProcessor.h"
#include <algorithm>

/**
 * @brief Constructor - Initialize FFT processor
 *
 * Initializes FFT result buffer to null and size to zero.
 */
FFTProcessor::FFTProcessor() : fftResult(nullptr), fftSize(0)
{
}

/**
 * @brief Destructor - Clean up allocated memory
 *
 * Frees FFT result buffer if allocated.
 */
FFTProcessor::~FFTProcessor()
{
    if (fftResult != nullptr)
    {
        delete[] fftResult;
    }
}

/**
 * @brief Process input data and perform FFT
 *
 * Converts time-domain data to frequency domain:
 * 1. Allocates memory for complex FFT result
 * 2. Converts real input data to complex format (imaginary part = 0)
 * 3. Performs FFT transformation
 *
 * @param data Input time-domain data array (real numbers)
 * @param size Number of samples
 * @param samplingFreq Sampling frequency in Hz (not used in this function, but stored for getFrequency)
 */
void FFTProcessor::process(float *data, int size, float samplingFreq)
{
    // Allocate memory for FFT result if size changed
    if (fftSize != size)
    {
        if (fftResult != nullptr)
        {
            delete[] fftResult;
        }
        fftResult = new std::complex<float>[size];
        fftSize = size;
    }

    // Copy real data to complex array (imaginary part = 0)
    // FFT requires complex input, but our sensor data is real
    for (int i = 0; i < size; i++)
    {
        fftResult[i] = std::complex<float>(data[i], 0.0f);
    }

    // Perform FFT transformation
    fft(fftResult, size);
}

/**
 * @brief Get frequency corresponding to an FFT bin
 *
 * FFT bins represent frequency components. This function converts
 * bin index to actual frequency in Hz.
 *
 * Formula: frequency = (bin_index * sampling_frequency) / total_samples
 *
 * @param bin FFT bin index (0 to size/2)
 * @param samplingFreq Sampling frequency in Hz (52Hz for this project)
 * @param size Total number of samples
 * @return Frequency in Hz
 */
float FFTProcessor::getFrequency(int bin, float samplingFreq, int size)
{
    return (bin * samplingFreq) / size;
}

/**
 * @brief Get magnitude (energy) at a specific FFT bin
 *
 * Magnitude represents the energy/amplitude at a specific frequency.
 * Higher magnitude = stronger signal at that frequency.
 *
 * @param bin FFT bin index
 * @return Magnitude value (absolute value of complex number)
 */
float FFTProcessor::getMagnitude(int bin)
{
    if (bin >= 0 && bin < fftSize)
    {
        return std::abs(fftResult[bin]); // Absolute value of complex number
    }
    return 0.0f;
}

/**
 * @brief Fast Fourier Transform implementation using Cooley-Tukey algorithm
 *
 * Recursive divide-and-conquer algorithm for efficient FFT computation.
 *
 * Algorithm steps:
 * 1. Check if input size is power of 2 (required for algorithm)
 * 2. If not, zero-pad to next power of 2
 * 3. Divide: Split data into even and odd indices
 * 4. Conquer: Recursively compute FFT of even and odd parts
 * 5. Combine: Merge results using twiddle factors
 *
 * Time complexity: O(n log n) vs O(n²) for naive DFT
 *
 * @param x Input/output complex array (modified in place)
 * @param n Number of samples (should be power of 2)
 */
void FFTProcessor::fft(std::complex<float> *x, int n)
{
    // Base case: if size <= 1, no computation needed
    if (n <= 1)
        return;

    // Check if n is power of 2 (required for Cooley-Tukey algorithm)
    // If not, zero-pad to next power of 2
    int pow2 = 1;
    while (pow2 < n)
        pow2 *= 2;

    if (pow2 != n)
    {
        // Zero-pad to power of 2
        std::complex<float> *padded = new std::complex<float>[pow2];
        // Copy original data
        for (int i = 0; i < n; i++)
        {
            padded[i] = x[i];
        }
        // Zero-pad remaining elements
        for (int i = n; i < pow2; i++)
        {
            padded[i] = std::complex<float>(0.0f, 0.0f);
        }
        // Recursive FFT on padded data
        fft(padded, pow2);
        // Copy back to original array (only first n elements)
        for (int i = 0; i < n; i++)
        {
            x[i] = padded[i];
        }
        delete[] padded;
        return;
    }

    // Divide: Split into even and odd indexed samples
    std::complex<float> *even = new std::complex<float>[n / 2];
    std::complex<float> *odd = new std::complex<float>[n / 2];

    for (int i = 0; i < n / 2; i++)
    {
        even[i] = x[i * 2];    // Even indices: 0, 2, 4, ...
        odd[i] = x[i * 2 + 1]; // Odd indices: 1, 3, 5, ...
    }

    // Conquer: Recursively compute FFT of even and odd parts
    fft(even, n / 2);
    fft(odd, n / 2);

    // Combine: Merge results using twiddle factors
    // Twiddle factor: e^(-2πik/n) = complex exponential
    for (int k = 0; k < n / 2; k++)
    {
        // Calculate twiddle factor angle
        float angle = -2.0f * static_cast<float>(M_PI) * static_cast<float>(k) / static_cast<float>(n);
        // Apply twiddle factor to odd part
        std::complex<float> t = std::polar(1.0f, angle) * odd[k];
        // Combine even and odd parts
        x[k] = even[k] + t;         // First half
        x[k + n / 2] = even[k] - t; // Second half (negative frequency)
    }

    delete[] even;
    delete[] odd;
}

/**
 * @brief Inverse Fast Fourier Transform (IFFT)
 *
 * Converts frequency domain back to time domain.
 * Not used in this project, but included for completeness.
 *
 * Algorithm:
 * 1. Take complex conjugate
 * 2. Perform FFT
 * 3. Take complex conjugate again and normalize
 *
 * @param x Input/output complex array (modified in place)
 * @param n Number of samples
 */
void FFTProcessor::ifft(std::complex<float> *x, int n)
{
    // Take complex conjugate
    for (int i = 0; i < n; i++)
    {
        x[i] = std::conj(x[i]);
    }

    // Perform FFT
    fft(x, n);

    // Take complex conjugate again and normalize
    for (int i = 0; i < n; i++)
    {
        x[i] = std::conj(x[i]) / float(n);
    }
}
