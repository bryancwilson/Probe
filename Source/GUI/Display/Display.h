#pragma once  // or use traditional include guards

#include <JuceHeader.h>
#include "../../PluginEditor.h"
#include <unordered_map>

class ChainBuilderAudioProcessorEditor; // forward declaration

// ===============================================================================================================
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
    std::unordered_map<int, std::pair<juce::String, juce::String>> changedParameters;

    bool params_loaded = false;

private:
    // ====================================== MOUSE & UI EVENTS ======================================
    void mouseDown(const juce::MouseEvent& event) override;
    void mouseEnter(const juce::MouseEvent& event) override;
    void mouseMove(const juce::MouseEvent& event) override;
    void mouseExit(const juce::MouseEvent& event) override;
    
    void openPluginEditor(int index);
    void handleRoutingClick(int index);
    bool isClickOnPlus(const juce::Rectangle<float>& loadPluginBox, const juce::Point<int>& pos);

    // ====================================== INTERNAL STATE ======================================
    bool isDragOver = false;
    bool in_vertical_bounds = false;
    bool closed_clicked = false;
    bool emptyPluginBoxHover = false;
    int addClicked = -1;

    // Animation State Variables
    float newBoxX = 0.0f;
    float newBoxWidth = 0.0f;
    float newShrinkage = 0.0f;
    bool init_var_for_click_anim = false;

    enum class SlideState { Hidden, AnimatingIn, Shown, AnimatingOut, AnimatingAway };
    SlideState slideState = SlideState::Hidden;
    
    enum class AddState { Default, SlidingOut, Shown, SlidingIn };
    AddState addState = AddState::Default;

    // ====================================== PLUGIN MANAGEMENT ======================================
    juce::VST3PluginFormat pluginFormat;
    juce::AudioPluginFormatManager formatManager;
    juce::KnownPluginList pluginList;
    std::unique_ptr<juce::AudioPluginInstance> pluginInstance;
    juce::String selectedPluginName;
    juce::StringArray selectedPluginNames;

    std::unique_ptr<juce::AudioProcessorEditor> editor; // hosted editor pointer

    
    // ====================================== UI COMPONENTS ======================================
    juce::TextEditor promptBox;
    juce::TextButton sendButton;
    juce::Label outputLabel;

    juce::Rectangle<float> loadPluginBox;
    juce::Array<juce::Rectangle<float>> loadPluginBoxes;
    
    // Hit Test Rects
    juce::Array<juce::Rectangle<float>> xButtonRects;
    juce::Array<juce::Rectangle<float>> bypassButtonRects;
    juce::Array<juce::Rectangle<float>> visualButtonRects;

    int hoveredPluginIndex = -1;
    int previouslyHoveredPluginIndex = -1;
    int visualHoveredIndex = -1;
    int bypassHoveredIndex = -1;
    int visualClickedIndex = -1;
    int bypassClickedIndex = -1;

    // ====================================== ANIMATION VALUES ======================================
    juce::AudioBuffer<float>* hostedPluginBuffer = nullptr;
    void timerCallback() override;

    float hoverAnim = 0.0f;
    float clickAnim = 0.0f;
    float clickAnimInv = 1.0f;
    float visualClickAnim = 0.0f;
    float visualClickAnimInv = 0.0f;
    float slideAnim = 0.0f;
    
    bool triggerClickAnim = false;

    // Arrow Connection Variables
    juce::Array<juce::Point<float>> arrow_beg_ends;
    juce::Array<int> connection_array;
    
    // Helper to draw curved arrow
    void drawCurvedArrow (juce::Graphics& g,
                                 juce::Point<float> from,
                                 juce::Point<float> to,
                                 float curvature,
                                 float shaftThickness,
                                 float headLength,
                                 float headWidth,
                                 juce::Colour colour);
    void updateSlideAnimation(bool& needsRepaint);
    void handlePluginRemoval();

    juce::Label titleLabel;
};

// ===============================================================================================================
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

// ===============================================================================================================
class Listener
{
public:
    virtual ~Listener() = default;

    virtual void parameterValueChanged(int parameterIndex, float newValue) = 0;
    virtual void parameterGestureChanged(int parameterIndex, bool gestureIsStarting) = 0;
};

// ===============================================================================================================
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
    void applyDelta(float delta);
    
private:


    juce::Label nameLabel;  // small font for name
    juce::Label valueLabel; // larger font for value
    juce::Label offsetLabel; // small offset label
};
