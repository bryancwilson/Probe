
#include "../../PluginEditor.h"
#include "Display.h"
#include "../../Metrics/Metrics.h"


// JUCE_WINDOWS true

//juce::FileSearchPath searchPaths;
// #if JUCE_WINDOWS
//searchPaths.add("C:\\Program Files\\VST3");
//searchPaths.add("C:\\Program Files\\Steinberg\\VST3");
//#elif JUCE_MAC
//searchPaths.add("/Library/Audio/Plug-Ins/VST3");
//searchPaths.add("~/Library/Audio/Plug-Ins/VST3");
//#endif


void ChainBuilderAudioProcessorEditor::initWindowSize_Editor()
{
    // Grab the window instance and create a rectangle
    juce::Rectangle<int> r = juce::Desktop::getInstance().getDisplays().getPrimaryDisplay()->userArea;

    // Using the width is more useful than the height. because we know the height will always be < than width
    int x = r.getWidth();

    auto width = 0;

    if (r.getWidth() <= 1920)
    {
        // small screen size
        width = x * 0.5;
    }
    else
    {
        // large screen size
        width = x * 0.25;
    }

    auto height = width * 0.5;

    // Making the window resizable by aspect ration and setting size
    // AudioProcessorEditor::setResizable(true, true);
    // AudioProcessorEditor::setResizeLimits(width * 1.5f, height * 1.5f, width * 2.5f, height * 2.5f);
    // AudioProcessorEditor::getConstrainer()->setFixedAspectRatio(2.0);

    setSize(width * 1.5, height * 1.5);
}

// CLASSES
PluginDropZone::PluginDropZone(ChainBuilderAudioProcessor& proc)
    : audioProcessor(proc)
{
    formatManager.addDefaultFormats(); // VST, AU, etc.
    startTimerHz(60); // repaint 60 times per second
}

PluginDropZone::~PluginDropZone() {
        if (pluginInstance)
        {
            auto& params = pluginInstance->getParameters();
            for (auto* p : params)
                p->removeListener(this);
        }   
}

void PluginDropZone::timerCallback()
{
    dashPhase += 0.5f;
    if (dashPhase > 6.0f) // reset after one dash length
        dashPhase = 0.0f;

    repaint();
}

void PluginDropZone::paint(juce::Graphics& g)
{

    // Slide amount
    float offsetX = 0.0f; // how far to move everything right

    float centerX = getWidth() * .5f + offsetX;

    // Create path for vertical line
    juce::Path linePath;
    linePath.startNewSubPath(centerX, 0.0f);
    linePath.lineTo(centerX, getHeight());
    juce::PathStrokeType stroke(2.0f);

    // Dashed line
    float dashLengths[] = { 6.0f, 6.0f };
    juce::Path dashed;
    stroke.createDashedStroke(dashed, linePath, dashLengths, 2,
        juce::AffineTransform::translation(0.0f, dashPhase),
        3.0f);

    g.setColour(juce::Colours::white);
    g.fillPath(dashed);

    // ----- Draw plugin box -----
    if (selectedPluginName.isNotEmpty())
    {
        g.setFont(16.0f);
        auto textWidth = g.getCurrentFont().getStringWidthFloat(selectedPluginName);
        auto textHeight = g.getCurrentFont().getHeight();

        float paddingX = 15.0f;
        float paddingY = 6.0f;

        juce::Rectangle<float> box(
            (getWidth() - (textWidth + 2.0f * paddingX)) / 2.0f + offsetX, // slide box
            (getHeight() - (textHeight + 2.0f * paddingY)) / 2.0f,
            textWidth + 2.0f * paddingX,
            textHeight + 2.0f * paddingY
        );

        g.setColour(juce::Colours::darkgrey.withAlpha(0.7f));
        g.fillRoundedRectangle(box, 8.0f);

        g.setColour(juce::Colours::white);
        g.drawRoundedRectangle(box, 8.0f, 2.0f);

        g.drawText(selectedPluginName, box.toNearestInt(), juce::Justification::centred);
    }
    else
    {
        // Draw "+" text inside a centered box that also slides
        juce::Rectangle<float> plusBox(
            getWidth() / 2.0f - 15.0f + offsetX,
            getHeight() / 2.0f - 15.0f,
            30.0f,
            30.0f
        );

        g.setColour(juce::Colours::white);
        g.setFont(16.0f);
        g.drawText("+", plusBox.toNearestInt(), juce::Justification::centred);
    }
}




void PluginDropZone::resized()
{
    // Nothing special
}


// Check if click is on plus
bool PluginDropZone::isClickOnPlus(const juce::Point<int>& pos)
{
    auto area = getLocalBounds();
    float centerX = area.getWidth() / 2.0f;
    float centerY = area.getHeight() / 2.0f;
    float threshold = 15.0f;
    return (std::abs(pos.x - centerX) <= threshold && std::abs(pos.y - centerY) <= threshold);
}

// Mouse click handler
void PluginDropZone::mouseDown(const juce::MouseEvent& event)
{
    if (!isClickOnPlus(event.getPosition()))
        return;

    // If a plugin is already loaded -> open its GUI
    if (audioProcessor.hostedPlugin != nullptr)
    {
        if (auto* editor = audioProcessor.hostedPlugin->createEditorIfNeeded())
        {
            auto* window = new juce::DialogWindow(
                audioProcessor.hostedPlugin->getName(),
                juce::Colours::black,
                true
            );

            window->setContentOwned(editor, true);
            window->centreWithSize(editor->getWidth(), editor->getHeight());
            window->setVisible(true);
        }

        auto* processor = audioProcessor.hostedPlugin.get();

        DBG("Name: " << processor->getName());
        DBG("Inputs: " << processor->getTotalNumInputChannels());
        DBG("Outputs: " << processor->getTotalNumOutputChannels());

        DBG("========================== Plugin Parameters =============================");
        parameters.clear();

        for (auto* param : audioProcessor.hostedPlugin->getParameters())
        {
            param->addListener(this); // Listen to changes
            parameters.add(param);

            DBG("Parameter: " << param->getName(100)
                << ", Value: " << param->getValue()
                << ", Default: " << param->getDefaultValue()
                << ", Label: " << param->getLabel());
        }
    }

    juce::FileSearchPath searchPaths;
    searchPaths.add(juce::File("C:\\Program Files\\Common Files\\VST3"));
    searchPaths.add(juce::File("C:\\Program Files\\Steinberg\\VST3"));

    // Path to store scan state
    juce::File deadMansPedal = juce::File::getSpecialLocation(juce::File::userDocumentsDirectory)
        .getChildFile("pluginScanState.tmp");

    // Scan VST3 plugins (or cached scan)
    juce::PluginDirectoryScanner scanner(pluginList, pluginFormat, searchPaths, true, deadMansPedal, false);
    juce::String pluginNames = "";

    bool finished = false;
    while (!finished)
        finished = !scanner.scanNextFile(true, pluginNames);

    // Build popup menu
    juce::PopupMenu menu;
    auto pluginTypes = pluginList.getTypes();

    for (int i = 0; i < pluginTypes.size(); ++i)
    {
        menu.addItem(i + 1, pluginTypes[i].name);
    }

    // Show menu asynchronously, handle selection in lambda
    int samplerate = 44100;
    int blockSize = 512;

    menu.showMenuAsync(juce::PopupMenu::Options(),
        [this, pluginTypes, samplerate, blockSize](int result)
        {
            if (result > 0)
            {
                auto selectedPlugin = pluginTypes[result - 1];
                DBG("User chose plugin: " << selectedPlugin.name);

                // Example: create plugin instance
                juce::String errorMessage;
                auto instance = formatManager.createPluginInstance(
                    selectedPlugin,
                    samplerate,
                    blockSize,
                    errorMessage
                );

                if (instance == false)
                {
                    DBG("Failed to load plugin: " << errorMessage);
                }
                else
                {
                    DBG("Plugin loaded successfully!");

                    audioProcessor.hostedPlugin = std::move(instance);
                    audioProcessor.hostedPlugin->prepareToPlay(audioProcessor.getSampleRate(), audioProcessor.getBlockSize());
                    // audioProcessor.pluginPrepared = false;

                    selectedPluginName = selectedPlugin.name;
                    repaint();
                }
            }
        });

}

void PluginDropZone::parameterValueChanged(int parameterIndex, float newValue)
{
    if (parameterIndex < parameters.size())
    {
        auto* param = parameters[parameterIndex];
        juce::String displayText = param->getName(100) + ": " + param->getText(newValue, 100);
        DBG(displayText);

        // Optionally repaint your GUI with the new value
        // e.g., update a label or slider associated with this parameter
    }
}

void PluginDropZone::parameterGestureChanged(int parameterIndex, bool gestureIsStarting)
{
    DBG("Parameter " + juce::String(parameterIndex) + " gesture "
        + juce::String(gestureIsStarting ? "started" : "ended"));
}


// --- DragAndDropTarget callbacks ---
bool PluginDropZone::isInterestedInDragSource(const juce::DragAndDropTarget::SourceDetails& /*details*/)
{
    return true; // accept any drag for now
}

void PluginDropZone::itemDragEnter(const juce::DragAndDropTarget::SourceDetails& /*details*/)
{
    isDragOver = true;
    repaint();
}

void PluginDropZone::itemDragExit(const juce::DragAndDropTarget::SourceDetails& /*details*/)
{
    isDragOver = false;
    repaint();
}

void PluginDropZone::itemDropped(const juce::DragAndDropTarget::SourceDetails& /*details*/)
{
    isDragOver = false;
    // Here you can handle the dropped plugin
    DBG("Plugin dropped!");
    repaint();
}
