#pragma once

#include <JuceHeader.h>

// Namespace to avoid clutter
namespace Metrics
{
    // =============================
    // Frequency-domain metrics
    // =============================
    float computeSpectralCentroid(float* fftData, double sampleRate);
    float computeSpectralRolloff(float* fftData, double sampleRate, float rolloffPercent = 0.95f);
    float computeSpectralFlatness(float* fftData);
    std::array<float, 3> computeBandEnergy(float* fftData, double sampleRate); // low, mid, high
    float computeResonanceScore(float* fftData, double sampleRate);
    juce::Array<float> computeTonalBalance(const juce::AudioBuffer<float>& buffer, double sampleRate);
    float computeHarmonicToNoiseRatio(float* fftData, double sampleRate);

    // =============================
    // Time-domain metrics
    // =============================
    float computeRMS(const juce::AudioBuffer<float>& buffer);
    float computeLUFS(const juce::AudioBuffer<float>& buffer, double sampleRate);
    float computePeakLevel(const juce::AudioBuffer<float>& buffer);
    float computeCrestFactor(const juce::AudioBuffer<float>& buffer);
    float computeTransientSharpness(const juce::AudioBuffer<float>& buffer, double sampleRate);
    float computeDecayTime(const juce::AudioBuffer<float>& buffer, double sampleRate);
    juce::Array<float> computeEnvelope(const juce::AudioBuffer<float>& buffer, double sampleRate);
    float computeStereoCorrelation(const juce::AudioBuffer<float>& buffer);

    // =============================
    // Cross-domain metrics
    // =============================
    float computeTHD(const juce::AudioBuffer<float>& buffer, double sampleRate);
    float computeIntermodulationDistortion(const juce::AudioBuffer<float>& buffer, double sampleRate);
    juce::Array<float> computeSpectralDynamics(const juce::AudioBuffer<float>& buffer, double sampleRate);
    float computeModulationDepth(const juce::AudioBuffer<float>& buffer);
    float computeModulationRate(const juce::AudioBuffer<float>& buffer, double sampleRate);

    enum
    {
        fftOrder = 12,              // this designates the size of the fft 2 ^ fft_Order
        fftSize = 1 << fftOrder,    // calculating the fftSize
        scopeSize = 2048             // number of points in the visual representation
    };

    // juce::dsp::FFT::ComplexArray performFFT(const juce::AudioBuffer<float>& buffer, int fftOrder);
}
