/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "GUI/Display/Display.h"

#include <string>
#include <iostream>
#include <curl/curl.h>
#include <nlohmann/json.hpp>
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
    void display_metrics();
    void showText();
    juce::Label Translate;
    juce::Label Discuss;
    juce::Label metrics_text;
    juce::Label main_text;
    std::string b_text;
    juce::TextEditor textBox;
    juce::Rectangle<float> text_box_bounds;
    PluginDropZone* dropZone;

    // LLM API Functions
    std::string prompt_gen();
    void api_func();

    // License Functions
    juce::ApplicationProperties appProperties;
    bool first_pass_valid = false;
    bool valid = false;
    bool loaded_params = false;
    std::string returned_licenseID = "";
    std::string license_input_text = "";
    std::string returned_accountID = "";
    std::string account_input_text = "";
    enum class LicenseState
    {
        Uninitialized,     // Haven't checked anything yet
        AwaitingInput,     // User needs to enter a license
        ValidatingLicense, // Checking license validity
        RegisteringMachine,// Registering this device
        Licensed,          // All good
        Invalid            // License invalid / machine not allowed
    };
    LicenseState state = LicenseState::Uninitialized;
    void updateLicenseState();
    void saveLicenseID(const std::string& licenseID);
    std::string loadLicenseID();

    // Input Functions
    void set_textbox(int id);
    void handleTextInput(int id);

    // Branding Colours
    uint32_t color_1 = 0xff443627;
    uint32_t color_2 = 0xffd98324;
    uint32_t color_4 = 0xfff2f6d0;

    // In your component header
    juce::Rectangle<int> paramBoxDebug;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    ChainBuilderAudioProcessor& audioProcessor;

    juce::OwnedArray<ParameterDisplay> parameterDisplays;


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ChainBuilderAudioProcessorEditor)
};
