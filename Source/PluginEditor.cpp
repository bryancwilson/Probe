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
   
    // ChainBuilderAudioProcessorEditor::updateLicenseState();
    
    // Display Metrics
    display_metrics();

    // Position and show your drop zone
    dropZone->setBounds (0, 40, getWidth() * 0.2f, getHeight() - 80);
    addAndMakeVisible (dropZone);

    // =============== Display Parameters ===================
    if (audioProcessor.hostedPlugin != nullptr && dropZone->params_loaded)
    {
        if (!loaded_params)
        {
            for (auto* param : dropZone->parameters)
            {
                auto* display = new ParameterDisplay(param);
                parameterDisplays.add(display);
                addAndMakeVisible(display);
            }
            loaded_params = true;
        }

        auto bounds = getLocalBounds().toFloat();

        // Example: scale only the x coordinates
        float xScale = 0.35f;
        float left = bounds.getX();
        float width = bounds.getWidth() * xScale; // shrink width
        float rightOffset = (bounds.getWidth() - width) / 3.303f; // optional center

        bounds.setX(left + rightOffset);
        bounds.setWidth(width);

        int columns = 3;
        int rows = (parameterDisplays.size() + columns - 1) / columns;

        // use fractions of width/height instead of fixed pixels
        float topMargin = bounds.getHeight() * 0.10f;   // 5% of total height
        float rowSpacing = bounds.getHeight() * 0.01f;   // 1% of total height
        float colSpacing = bounds.getWidth() * -0.02f;  // -2% of total width (tighten columns)

        // shrink by top margin
        bounds.removeFromTop(topMargin);

        // compute sizes
        float colWidth = (bounds.getWidth() - (columns - 1) * colSpacing) / columns;
        float rowHeight = (bounds.getHeight() - (rows - 1) * rowSpacing) / rows;

        // for (int i = 0; i < parameterDisplays.size(); ++i)
        for (int i = 0; i < 6; ++i)
        {
            int row = i / columns;
            int col = i % columns;

            float x = bounds.getX() + col * (colWidth + colSpacing);
            float y = bounds.getY() + row * (rowHeight + rowSpacing);

            parameterDisplays[i]->setBounds(x, y, colWidth, rowHeight);
        }


    }
}

void ChainBuilderAudioProcessorEditor::resized()
{

    auto area = getLocalBounds(); // full plugin area

    int labelWidth = 100;   // desired width of the label
    int labelHeight = 25;   // desired height

    int x_translate = (area.getWidth() - labelWidth) / 2.8f;  // center horizontally
    int x_discuss = (area.getWidth() - labelWidth) / 1.3f;  // center horizontally
    int y = 10;  // small top margin

    // =========== Display Section Titles =================
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



}
