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
    dropZone = new PluginDropZone(audioProcessor, *this);


}

ChainBuilderAudioProcessorEditor::~ChainBuilderAudioProcessorEditor()
{
}

//==============================================================================

// This ensures any click in the editor window grabs keyboard focus
void ChainBuilderAudioProcessorEditor::mouseDown (const juce::MouseEvent&)
{
    grabKeyboardFocus();
}

void ChainBuilderAudioProcessorEditor::paint (juce::Graphics& g)
{
    // Fill the background black
    g.fillAll (juce::Colours::black);

    // Draw “Hello” in white, centred horizontally, 20 px down from the top
    g.setColour (juce::Colours::white);
    g.setFont   (20.0f);
   
    // ChainBuilderAudioProcessorEditor::updateLicenseState();
    
    // Display Functions
    display_metrics();
    display_params();
    // testParameterDisplayOffsets();

    set_textbox(4);

    // Position and show your drop zone
    dropZone->setBounds (0, 0, getWidth() * 0.2f, getHeight() - 80);
    addAndMakeVisible (dropZone);
}

void ChainBuilderAudioProcessorEditor::togglePromptSidebar(bool shouldShow)
{
    if (sidebarVisible == shouldShow)
        return; // no change needed

    sidebarVisible = shouldShow;

    int currentWidth = getWidth();
    int targetWidth = sidebarVisible ? currentWidth + sidebarWidth : currentWidth - sidebarWidth;

    // Animate the window resize (optional)
    static juce::ComponentAnimator animator;
    animator.animateComponent(this,
        { getX(), getY(), targetWidth, getHeight() },
        1.0f,
        300,   // duration ms
        false, // don't use proxy (animate actual component)
        0.0f,
        0.0f);


    resized();

}
void ChainBuilderAudioProcessorEditor::resized()
{

    auto area = getLocalBounds(); // full plugin area

    // ================== Display Slidebar =======================
    if (sidebarVisible)
    {
        auto sidebar = area.removeFromRight(sidebarWidth);
        auto buttonHeight = 30;
        int padding = 10;

        // --- Translate header & text box (top half) ---
        juce::String fontName = "Arial";
        int labelHeight = 25;
        int textBoxHeight = 30;

        // Calculate vertical positioning
        auto translateArea = sidebar.reduced(padding);
        int yOffset = translateArea.getCentreY() - (labelHeight + textBoxHeight + padding) / 2;

        // Place Translate label
        Translate.setFont(juce::Font(fontName, 16.0f, juce::Font::bold));
        Translate.setText("Translate", juce::dontSendNotification);
        Translate.setColour(juce::Label::textColourId, juce::Colours::white);
        Translate.setJustificationType(juce::Justification::centred);

        Translate.setBounds(translateArea.getX(),
            yOffset,
            translateArea.getWidth(),
            labelHeight);
        addAndMakeVisible(Translate);

        // --- Creative Response Header ---
        creative_response.setFont(juce::Font(fontName, 15.0f, juce::Font::plain));
        creative_response.setText("AI Response:", juce::dontSendNotification);
        creative_response.setColour(juce::Label::textColourId, juce::Colours::white);
        creative_response.setJustificationType(juce::Justification::centredLeft);
        creative_response.setBounds(sidebar.getX() + padding,
            sendButton.getBottom() + (padding * 2),
            sidebar.getWidth() - 2 * padding,
            labelHeight);

        addAndMakeVisible(creative_response);
        // Place text box directly below
        textBox.setBounds(translateArea.getX() + padding,
            yOffset + labelHeight + padding,
            translateArea.getWidth() - 2 * padding,
            textBoxHeight);
        textBox.setColour(juce::TextEditor::backgroundColourId, juce::Colours::black);
        textBox.setColour(juce::TextEditor::textColourId, juce::Colours::white);
        textBox.setColour(juce::TextEditor::outlineColourId, juce::Colour(color_4));
        textBox.setColour(juce::TextEditor::focusedOutlineColourId, juce::Colour(color_2));
        textBox.setWantsKeyboardFocus(true);
        textBox.setInterceptsMouseClicks(true, true);
        textBox.grabKeyboardFocus();

        addAndMakeVisible(textBox);

        return;
    }

    // =========== Display Section Titles =================
    int labelWidth = 100;   // desired width of the label
    int labelHeight = 25;   // desired height
    int creativeWidth = 200;   // desired width of the label
    int creativeHeight = 100;
    
    int x_translate = (area.getWidth() - labelWidth) / 2.8f;  // center horizontally
    int x_discuss = (area.getWidth() - labelWidth) / 1.3f;  // center horizontally
    int y = 10;  // small top margin

    std::string font = "Arial";
    Translate.setFont(juce::Font(font, 15.0f, 0));
    Translate.setText("Translate", juce::dontSendNotification);
    Translate.setColour(juce::Label::textColourId, juce::Colours::white);
    Translate.setJustificationType(juce::Justification::centred);
    Translate.setBounds(x_translate, y, labelWidth, labelHeight);
    addAndMakeVisible(Translate);

    Discuss.setFont(juce::Font(font, 15.0f, 0));
    Discuss.setText("Discuss", juce::dontSendNotification);
    Discuss.setColour(juce::Label::textColourId, juce::Colours::white);
    Discuss.setJustificationType(juce::Justification::centred);
    Discuss.setBounds(x_discuss, y, labelWidth, labelHeight);
    addAndMakeVisible(Discuss);
    
    // After you set Translate bounds
    auto translateBounds = Translate.getBounds();
    int x_creative_text = translateBounds.getCentreX() - creativeWidth / 2;
    
    creative_response.setFont(juce::Font(font, 15.0f, 0));
    creative_response.setColour(juce::Label::textColourId, juce::Colours::white);
    creative_response.setJustificationType(juce::Justification::centred);
    creative_response.setBounds(x_creative_text, y + 300, creativeWidth, creativeHeight);
    addAndMakeVisible(creative_response);

    // ================ Textboxes ======================
    int componentWidth = 208;
    int x_component = translateBounds.getCentreX() - componentWidth / 2;

    textBox.setBounds(x_component, getHeight() * 0.80, componentWidth, 30);
    textBox.setColour(juce::TextEditor::backgroundColourId, juce::Colours::black);
    textBox.setColour(juce::CaretComponent::caretColourId, juce::Colours::white);
    textBox.setColour(juce::TextEditor::outlineColourId, juce::Colour(color_4));
    textBox.setColour(juce::TextEditor::focusedOutlineColourId, juce::Colour(color_2));
    
    // ================ AI Creative Response ===================
   

}
