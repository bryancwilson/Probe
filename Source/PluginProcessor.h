/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "Metrics/Metrics.h"


//==============================================================================
/**
*/
class ChainBuilderAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    ChainBuilderAudioProcessor();
    ~ChainBuilderAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    std::unique_ptr<juce::AudioPluginInstance> hostedPlugin = nullptr;
    bool pluginPrepared = false;

    enum
    {
        fftOrder = 12,              // this designates the size of the fft 2 ^ fft_Order
        fftSize = 1 << fftOrder,    // calculating the fftSize
        scopeSize = 2048             // number of points in the visual representation
    };

    float fifo[fftSize]; // contains our incoming audio data in samples
    float fftData[2 * fftSize]; // will contain the results of our fft
    int fifoIndex = 0;
    juce::dsp::FFT forwardFFT; // instantiating (FFT) class to perform the forward FFT on
    juce::dsp::WindowingFunction<float> window; // instantiating (window) class to apply windowing function on
    void pushNextSampleIntoFifo(float sample) noexcept;
    
    /* Metric Variables */
    float spectral_centroid;
    float spectral_rolloff;
    float spectral_flatness;
    float resonance_score;
    float harmonic_to_noise;
    float rms;
    float lufs;
    float peak;
    float crest_factor;
    float transient_sharpness;
    float decay_time;
    float stereo_correlation;
    float modulation_depth;
private:
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ChainBuilderAudioProcessor)
};
