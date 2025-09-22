

#pragma once  // or use traditional include guards

#include <JuceHeader.h>
#include "../../PluginEditor.h"

class PluginDropZone : public juce::Component, public juce::DragAndDropTarget, public juce::AudioProcessorParameter::Listener, private juce::Timer
{
public:
    PluginDropZone(ChainBuilderAudioProcessor& proc);
    ~PluginDropZone() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

    // --- DragAndDropTarget callbacks ---
    bool isInterestedInDragSource(const juce::DragAndDropTarget::SourceDetails& /*details*/) override;
    void itemDragEnter(const juce::DragAndDropTarget::SourceDetails& /*details*/) override;
    void itemDragExit(const juce::DragAndDropTarget::SourceDetails& /*details*/) override;
    void itemDropped(const juce::DragAndDropTarget::SourceDetails& /*details*/) override;

    // --- Implement listener methods ---
    void parameterValueChanged(int parameterIndex, float newValue) override;
    void parameterGestureChanged(int parameterIndex, bool gestureIsStarting) override;

    juce::Array<juce::AudioProcessorParameter*> parameters;

    ChainBuilderAudioProcessor& audioProcessor; // store reference to processor
    float dashPhase = 0.0f; // 0..1, updated every timer tick

    bool params_loaded = false;
private:
    void mouseDown(const juce::MouseEvent& event) override;
    bool isClickOnPlus(const juce::Point<int>& pos);

    bool isDragOver = false;
    juce::VST3PluginFormat pluginFormat;
    juce::AudioPluginFormatManager formatManager;
    juce::KnownPluginList pluginList;

    // Handle Plugin View
    std::unique_ptr<juce::AudioPluginInstance> pluginInstance;
    juce::String selectedPluginName;

    juce::AudioBuffer<float>* hostedPluginBuffer = nullptr; // pointer to buffer from hosted plugin
    void timerCallback() override;
};

class Listener
{
public:
    virtual ~Listener() = default;

    virtual void parameterValueChanged(int parameterIndex, float newValue) = 0;
    virtual void parameterGestureChanged(int parameterIndex, bool gestureIsStarting) = 0;
};

class ParameterDisplay : public juce::Component,
    private juce::Timer   // Timer for updating value
{
public:
    // Constructor
    ParameterDisplay(juce::AudioProcessorParameter* param);

    // Destructor
    ~ParameterDisplay() override = default;

    // JUCE overrides
    void resized() override;
    void paint(juce::Graphics& g) override {}
    void timerCallback() override;

    juce::AudioProcessorParameter* parameter; // parameter to display

    
private:


    juce::Label nameLabel;  // small font for name
    juce::Label valueLabel; // larger font for value
};
