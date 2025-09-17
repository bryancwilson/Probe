/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
ChainBuilderAudioProcessorEditor::ChainBuilderAudioProcessorEditor (ChainBuilderAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    initWindowSize_Editor();
    dropZone = new PluginDropZone(audioProcessor);
}

ChainBuilderAudioProcessorEditor::~ChainBuilderAudioProcessorEditor()
{
}

//==============================================================================
void ChainBuilderAudioProcessorEditor::paint (juce::Graphics& g)
{
    // Fill the background black
    g.fillAll (juce::Colours::black);

    // Draw “Hello” in white, centred horizontally, 20 px down from the top
    g.setColour (juce::Colours::white);
    g.setFont   (20.0f);
    
    juce::String metrics_display =
        "Spectral Centroid: " + juce::String (audioProcessor.spectral_centroid, 2) + "\n"
        "Spectral Rolloff: "  + juce::String (audioProcessor.spectral_rolloff, 2)  + "\n"
        "Spectral Flatness: " + juce::String (audioProcessor.spectral_flatness, 2) + "\n"
        "Resonance Score: "   + juce::String (audioProcessor.resonance_score, 2)   + "\n"
        "Harmonic-to-Noise: " + juce::String (audioProcessor.harmonic_to_noise, 2);
    
    g.drawText (metrics_display,
                0, 0, getWidth(), 400,               // x, y, width, height of the text area
                juce::Justification::centred,       // centre the text
                true);                              // use ellipses if it doesn’t fit

    // Position and show your drop zone
    dropZone->setBounds (0, 40, getWidth() * 0.2f, getHeight() - 80);
    addAndMakeVisible (dropZone);
}

void ChainBuilderAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..


}
