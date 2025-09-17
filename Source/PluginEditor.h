/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "GUI/Display/Display.h"

//==============================================================================
/**
*/
class ChainBuilderAudioProcessorEditor  : public juce::AudioProcessorEditor, public juce::DragAndDropContainer
{
public:
    ChainBuilderAudioProcessorEditor (ChainBuilderAudioProcessor&);
    ~ChainBuilderAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

    // Display Functions
    void initWindowSize_Editor();
    PluginDropZone* dropZone;


    // Branding Colours
    uint32_t color_1 = 0xff443627;
    uint32_t color_2 = 0xffd98324;
    uint32_t color_4 = 0xfff2f6d0;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    ChainBuilderAudioProcessor& audioProcessor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ChainBuilderAudioProcessorEditor)
};
