#include "PluginEditor.h"

using json = nlohmann::json;

void ChainBuilderAudioProcessorEditor::set_textbox(int id)
{
    // Set up the text editor
    textBox.setMultiLine(false);
    textBox.setReturnKeyStartsNewLine(false);
    textBox.setReadOnly(false);
    textBox.setScrollbarsShown(false);
    textBox.setTextToShowWhenEmpty("Type here then click enter...", juce::Colours::grey);


    // textBox.setText("Type here...");
    textBox.onReturnKey = [this, id] { handleTextInput(id); }; // Handle when Enter is pressed

    addAndMakeVisible(textBox);

}

void ChainBuilderAudioProcessorEditor::handleTextInput(int id)
{
    if (id == 1) // LIVE MODE
    {
        /* 
        
        This should function somewhat similarly to Everett 1.0
        
        */
    }
    else if (id == 2) // MANUAL MODE
    {
        /*

        This should function somewhat similarly to Everett 1.0

        */
    }
    else if (id == 3) // INPUT LICENSE
    {
        license_input_text = textBox.getText().toStdString();
        state = LicenseState::Uninitialized;
        saveLicenseID(license_input_text);
        textBox.clear();

    }
    else if (id == 4) // CREATIVE MODE
    {
        /*

        This should give you the specific parameters that you need to tweak and the values

        This could be a scrollable section in the plugin that has all of the needed parameters to change and the new value next to a smaller "what is the offset"
        for example new gain of -2dB (-.2). Then below is a textual summary of overall what the producer should do and why.

        */
        creative_text = textBox.getText().toStdString();
        textBox.clear();
        prompt_gen();

    }

}