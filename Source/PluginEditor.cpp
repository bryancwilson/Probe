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
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll(juce::Colours::black); // Make the background All Black

    dropZone->setBounds(0, 40, getWidth() * .2, getHeight() - 80);
    addAndMakeVisible(dropZone);
    
}

void ChainBuilderAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..


}
