/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "Metrics/Metrics.h"

//==============================================================================
ChainBuilderAudioProcessor::ChainBuilderAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ),
forwardFFT(fftOrder),
window(fftSize, juce::dsp::WindowingFunction<float>::hann)
#endif
{
}

ChainBuilderAudioProcessor::~ChainBuilderAudioProcessor()
{
}

//==============================================================================
const juce::String ChainBuilderAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool ChainBuilderAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool ChainBuilderAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool ChainBuilderAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double ChainBuilderAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int ChainBuilderAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int ChainBuilderAudioProcessor::getCurrentProgram()
{
    return 0;
}

void ChainBuilderAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String ChainBuilderAudioProcessor::getProgramName (int index)
{
    return {};
}

void ChainBuilderAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void ChainBuilderAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
}

void ChainBuilderAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool ChainBuilderAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void ChainBuilderAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // Clear any output channels that don't contain input data
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    bool useWhiteNoiseForFifo = true;
    // ======================= White Noise Test ==========================
    if (useWhiteNoiseForFifo)
    {
        if (buffer.getNumChannels() > 0)
        {
            for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
            {
                auto* writePointer = buffer.getWritePointer(channel);
                for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
                {
                    float noise = juce::Random::getSystemRandom().nextFloat() * 2.0f - 1.0f;
                    writePointer[sample] = noise;
                }
            }
        }

        // 2. Pass through hosted EQ
        if (hostedPlugin != nullptr)
        {
            hostedPlugin->processBlock(buffer, midiMessages);
        }

        // 3. Analyze *post-EQ* signal (first channel is fine for FFT)
        if (buffer.getNumChannels() > 0)
        {
            auto* channelData = buffer.getReadPointer(0);
            for (int i = 0; i < buffer.getNumSamples(); ++i)
            {
                pushNextSampleIntoFifo(channelData[i], buffer);
            }
        }

    }
    // Live From DAW
    else if (buffer.getNumChannels() > 0)
    {
        auto* channelData = buffer.getReadPointer(0); // pointer to first channel
        for (int i = 0; i < buffer.getNumSamples(); ++i)
        {
            pushNextSampleIntoFifo(channelData[i], buffer);
        }
    }



}

void ChainBuilderAudioProcessor::pushNextSampleIntoFifo(float sample, juce::AudioBuffer<float>& buffer) noexcept
{
    // if the fifo contains enough data, set a flag to say that the next frame should now be rendered..
    if (fifoIndex == fftSize)
    {
        juce::zeromem(fftData, sizeof(fftData));    // clear fftData buffer
        memcpy(fftData, fifo, sizeof(fifo));        // copy FIFO into fftData

        // Time Based Functions
        rms = Metrics::computeRMS(buffer);
        lufs = Metrics::computeLUFS(buffer, getSampleRate());
        peak = Metrics::computePeakLevel(buffer);
        crest_factor = Metrics::computeCrestFactor(buffer);
        transient_sharpness = Metrics::computeTransientSharpness(buffer, getSampleRate());
        decay_time = Metrics::computeDecayTime(buffer, getSampleRate());
        stereo_correlation = Metrics::computeStereoCorrelation(buffer);

        // Frequency Based Functions
        window.multiplyWithWindowingTable(fftData, fftSize);        // Window the signal first
        forwardFFT.performRealOnlyForwardTransform(fftData, fftData);   // Perform forward FFT including phase

        spectral_centroid = Metrics::computeSpectralCentroid(fftData, getSampleRate());
        spectral_rolloff = Metrics::computeSpectralRolloff(fftData, getSampleRate(), 0.95f);
        spectral_flatness = Metrics::computeSpectralFlatness(fftData);
        resonance_score = Metrics::computeResonanceScore(fftData, getSampleRate());
        harmonic_to_noise = Metrics::computeHarmonicToNoiseRatio(fftData, getSampleRate());

        //DBG("Spectral Centroid: " << juce::String(spectral_centroid)
        //<< " Spectral Rollof: " << juce::String(spectral_rolloff)
        //<< " Spectral Flatness: " << juce::String(spectral_flatness)
        //<< " Resonance Score: " << juce::String(resonance_score)
        //<< " Harmonic to Noise Ratio: " << juce::String(harmonic_to_noise));
        fifoIndex = 0;                              // reset FIFO index
    }

    fifo[fifoIndex++] = sample;

}

//==============================================================================
bool ChainBuilderAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* ChainBuilderAudioProcessor::createEditor()
{
    return new ChainBuilderAudioProcessorEditor (*this);
}

//==============================================================================
void ChainBuilderAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void ChainBuilderAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ChainBuilderAudioProcessor();
}
