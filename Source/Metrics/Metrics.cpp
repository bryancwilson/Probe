#include "metrics.h"
#include <juce_dsp/juce_dsp.h>
#include <numeric>
#include <complex>
#include <cmath>

namespace Metrics
{

    // =============================
    // Frequency-domain metrics
    // =============================

    float computeSpectralCentroid(float* fftData, double sampleRate)
    {

        float numerator = 0.0f, denominator = 0.0f;
        for (int k = 0; k < fftSize / 2; ++k)
        {
            float magnitude = std::abs(fftData[k]);
            numerator += k * magnitude;
            denominator += magnitude;
        }

        if (denominator == 0.0f) return 0.0f;
        return (numerator / denominator) * (sampleRate / fftSize);
    }

    float computeSpectralRolloff(float* fftData, double sampleRate, float rolloffPercent)
    {

        std::vector<float> magnitudes(fftSize / 2);
        float totalEnergy = 0.0f;
        for (int k = 0; k < fftSize / 2; ++k)
        {
            magnitudes[k] = std::abs(fftData[k]);
            totalEnergy += magnitudes[k];
        }

        float threshold = totalEnergy * rolloffPercent;
        float cumulative = 0.0f;
        for (int k = 0; k < fftSize / 2; ++k)
        {
            cumulative += magnitudes[k];
            if (cumulative >= threshold)
                return (float)k * (sampleRate / fftSize);
        }
        return 0.0f;
    }

    float computeSpectralFlatness(float* fftData)
    {

        float geoMean = 1.0f, arithMean = 0.0f;
        int N = fftSize / 2;

        for (int k = 0; k < N; ++k)
        {
            float mag = std::abs(fftData[k]) + 1e-12f;
            geoMean *= mag;
            arithMean += mag;
        }
        geoMean = std::pow(geoMean, 1.0f / N);
        arithMean /= N;

        return arithMean > 0.0f ? geoMean / arithMean : 0.0f;
    }

    std::array<float, 3> computeBandEnergy(float* fftData, double sampleRate)
    {

        return {};
    }

    float computeResonanceScore(float* fftData, double sampleRate)
    {

        float maxPeak = 0.0f, avg = 0.0f;
        for (int k = 0; k < fftSize / 2; ++k)
        {
            float mag = std::abs(fftData[k]);
            maxPeak = std::max(maxPeak, mag);
            avg += mag;
        }
        avg /= (fftSize / 2);
        return avg > 0.0f ? maxPeak / avg : 0.0f;
    }

    juce::Array<float> computeTonalBalance(const juce::AudioBuffer<float>& buffer, double sampleRate)
    {

        return 0;
    }

    float computeHarmonicToNoiseRatio(float* fftData, double sampleRate)
    {
        // Simplified version: compare strongest harmonic peak to average noise floor
        float maxPeak = 0.0f, noiseSum = 0.0f;
        int N = fftSize / 2;

        for (int k = 1; k < N; ++k)
        {
            float mag = std::abs(fftData[k]);
            maxPeak = std::max(maxPeak, mag);
            noiseSum += mag;
        }
        float noiseAvg = (noiseSum - maxPeak) / (N - 1);

        return noiseAvg > 0.0f ? maxPeak / noiseAvg : 0.0f;
    }

    // =============================
    // Time-domain metrics
    // =============================

    float computeRMS(const juce::AudioBuffer<float>& buffer)
    {
        double sumSquares = 0.0;
        int totalSamples = 0;

        for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
        {
            auto* data = buffer.getReadPointer(ch);
            for (int n = 0; n < buffer.getNumSamples(); ++n)
            {
                sumSquares += data[n] * data[n];
                ++totalSamples;
            }
        }

        return totalSamples > 0 ? std::sqrt(sumSquares / totalSamples) : 0.0f;
    }

    float computeLUFS(const juce::AudioBuffer<float>& buffer, double sampleRate)
    {
        // Placeholder: A proper LUFS implementation requires gating & weighting filters
        return juce::Decibels::gainToDecibels(computeRMS(buffer));
    }

    float computePeakLevel(const juce::AudioBuffer<float>& buffer)
    {
        float peak = 0.0f;
        for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
        {
            auto* data = buffer.getReadPointer(ch);
            for (int n = 0; n < buffer.getNumSamples(); ++n)
                peak = std::max(peak, std::abs(data[n]));
        }
        return peak;
    }

    float computeCrestFactor(const juce::AudioBuffer<float>& buffer)
    {
        float peak = computePeakLevel(buffer);
        float rms = computeRMS(buffer);
        return rms > 0.0f ? peak / rms : 0.0f;
    }

    float computeTransientSharpness(const juce::AudioBuffer<float>& buffer, double sampleRate)
    {
        // Approx: ratio of maximum sample difference
        float maxDelta = 0.0f;
        auto* data = buffer.getReadPointer(0);

        for (int n = 1; n < buffer.getNumSamples(); ++n)
            maxDelta = std::max(maxDelta, std::abs(data[n] - data[n - 1]));

        return maxDelta;
    }

    float computeDecayTime(const juce::AudioBuffer<float>& buffer, double sampleRate)
    {
        // Rough estimate: time it takes RMS to drop by 60 dB
        float rmsStart = computeRMS(buffer);
        if (rmsStart <= 0.0f) return 0.0f;

        auto* data = buffer.getReadPointer(0);
        int N = buffer.getNumSamples();
        for (int n = 0; n < N; ++n)
        {
            float windowRMS = std::abs(data[n]);
            if (juce::Decibels::gainToDecibels(windowRMS / rmsStart) <= -60.0f)
                return (float)n / sampleRate;
        }
        return (float)N / sampleRate;
    }

    juce::Array<float> computeEnvelope(const juce::AudioBuffer<float>& buffer, double sampleRate)
    {
        juce::Array<float> env;
        int hopSize = (int)(sampleRate * 0.01); // 10 ms
        for (int n = 0; n < buffer.getNumSamples(); n += hopSize)
        {
            float rms = 0.0f;
            int count = 0;
            for (int i = 0; i < hopSize && (n + i) < buffer.getNumSamples(); ++i)
            {
                float sample = buffer.getSample(0, n + i);
                rms += sample * sample;
                ++count;
            }
            env.add(count > 0 ? std::sqrt(rms / count) : 0.0f);
        }
        return env;
    }

    float computeStereoCorrelation(const juce::AudioBuffer<float>& buffer)
    {
        if (buffer.getNumChannels() < 2) return 1.0f;

        auto* L = buffer.getReadPointer(0);
        auto* R = buffer.getReadPointer(1);
        int N = buffer.getNumSamples();

        double sumLR = 0, sumL2 = 0, sumR2 = 0;
        for (int n = 0; n < N; ++n)
        {
            sumLR += L[n] * R[n];
            sumL2 += L[n] * L[n];
            sumR2 += R[n] * R[n];
        }

        return (float)(sumLR / (std::sqrt(sumL2) * std::sqrt(sumR2) + 1e-12));
    }

    // =============================
    // Cross-domain metrics
    // =============================

    float computeTHD(const juce::AudioBuffer<float>& buffer, double sampleRate)
    {
        // Placeholder: proper THD needs fundamental detection & harmonic energy sum
        return 0.0f;
    }

    float computeIntermodulationDistortion(const juce::AudioBuffer<float>& buffer, double sampleRate)
    {
        // Placeholder
        return 0.0f;
    }

    juce::Array<float> computeSpectralDynamics(const juce::AudioBuffer<float>& buffer, double sampleRate)
    {
        // Placeholder: could return per-band RMS over time
        return {};
    }

    float computeModulationDepth(const juce::AudioBuffer<float>& buffer)
    {
        // Placeholder: ratio of max-min envelope
        auto env = computeEnvelope(buffer, 44100.0);
        if (env.isEmpty()) return 0.0f;

        float maxEnv = *std::max_element(env.begin(), env.end());
        float minEnv = *std::min_element(env.begin(), env.end());
        return maxEnv - minEnv;
    }

    float computeModulationRate(const juce::AudioBuffer<float>& buffer, double sampleRate)
    {
        // Placeholder: would require FFT of envelope
        return 0.0f;
    }
}
