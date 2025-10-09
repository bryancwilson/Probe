

#pragma once  // or use traditional include guards

#include <JuceHeader.h>
#include "../../PluginEditor.h"

class ChainBuilderAudioProcessorEditor; // forward declaration


class PluginDropZone : public juce::Component, public juce::DragAndDropTarget, public juce::AudioProcessorParameter::Listener, private juce::Timer
{
public:
    PluginDropZone(ChainBuilderAudioProcessor& proc, ChainBuilderAudioProcessorEditor& editorRef);
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

    // Flags
    bool plugin_painted = false;
    
    juce::Array<juce::AudioProcessorParameter*> parameters;
    juce::String param_list = "";
    
    ChainBuilderAudioProcessor& audioProcessor; // store reference to processor
    ChainBuilderAudioProcessorEditor& hostEditor;             // reference to editor

    float dashPhase = 0.0f; // 0..1, updated every timer tick

    bool params_loaded = false;
private:
    void mouseDown(const juce::MouseEvent& event) override;
    bool isClickOnPlus(const juce::Point<int>& pos);

    bool isDragOver = false;
    juce::VST3PluginFormat pluginFormat;
    juce::AudioPluginFormatManager formatManager;
    juce::KnownPluginList pluginList;

    std::unique_ptr<juce::AudioProcessorEditor> editor; // hosted editor pointer

    juce::TextEditor promptBox;
    juce::TextButton sendButton;
    juce::Label outputLabel;

    // Handle Plugin View
    std::unique_ptr<juce::AudioPluginInstance> pluginInstance;
    juce::String selectedPluginName;

    juce::AudioBuffer<float>* hostedPluginBuffer = nullptr; // pointer to buffer from hosted plugin
    void timerCallback() override;
};

struct NonFocusableWrapper : public juce::Component
{
    NonFocusableWrapper(juce::AudioProcessorEditor* editorIn)
        : editor(editorIn)
    {
        jassert(editor != nullptr);
        addAndMakeVisible(editor);
        setWantsKeyboardFocus(false);       // wrapper itself never takes focus
        editor->setWantsKeyboardFocus(false); // prevent child from requesting focus
    }

    void resized() override
    {
        if (editor != nullptr)
            editor->setBounds(getLocalBounds());
    }

private:
    juce::AudioProcessorEditor* editor;
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

    juce::AudioProcessorParameter* getParameter() const { return parameter; }
    juce::AudioProcessorParameter* parameter; // parameter to display

    std::optional<float> targetValue; // LLM target midpoint
    void setTargetValue(float newTarget);
    
private:


    juce::Label nameLabel;  // small font for name
    juce::Label valueLabel; // larger font for value
    juce::Label offsetLabel; // small offset label
};
