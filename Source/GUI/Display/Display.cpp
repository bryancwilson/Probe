#include "../../PluginEditor.h"
#include "Display.h"
#include "../../Metrics/Metrics.h"

// FUNCTIONS
void ChainBuilderAudioProcessorEditor::initWindowSize_Editor()
{
    // Grab the window instance and create a rectangle
    juce::Rectangle<int> r = juce::Desktop::getInstance().getDisplays().getPrimaryDisplay()->userArea;

    // Using the width is more useful than the height. because we know the height will always be < than width
    int x = r.getWidth();

    auto width = 0;
    auto width_a = 0;

    if (r.getWidth() <= 1920)
    {
        // small screen size
        width = x * 0.5;
        width_a = x * 0.5;
    }
    else
    {
        // large screen size
        width = x * 0.25;
        width_a = x * 0.25;
    }

    auto height = width * 0.5;

    // Making the window resizable by aspect ration and setting size
    // AudioProcessorEditor::setResizable(true, true);
    // AudioProcessorEditor::setResizeLimits(width * 1.5f, height * 1.5f, width * 2.5f, height * 2.5f);
    // AudioProcessorEditor::getConstrainer()->setFixedAspectRatio(2.0);

    setSize(width_a * 1.5, height * 1.5);
}

void ChainBuilderAudioProcessorEditor::showText()
{

    main_text.setText(b_text, juce::dontSendNotification);

    // Set the font for the label
    std::string font = "Arial";
    main_text.setFont(juce::Font(font, 15.0f, juce::Font::plain));

    // Set the label's text color
    main_text.setColour(juce::Label::textColourId, juce::Colours::white);

    // Set label justification
    main_text.setJustificationType(juce::Justification::centred);

    // Set the bounds of the label (position and size)
    main_text.setBounds(text_box_bounds.getX(), text_box_bounds.getY(), text_box_bounds.getWidth(), text_box_bounds.getHeight() / 1.5);

    // Add the label to the editor
    addAndMakeVisible(main_text);
}

void ChainBuilderAudioProcessorEditor::display_params(juce::Rectangle<int> boundsToUse)
{
    // =============== Display Parameters ===================

    // Live constants for layout tuning
    //int maxVisibleParams = JUCE_LIVE_CONSTANT(9);
    //float padding = JUCE_LIVE_CONSTANT(5.0f);
    //float rowSpacing = JUCE_LIVE_CONSTANT(8.0f);
    //float columnSpacing = JUCE_LIVE_CONSTANT(17.0f);
    //int columns = JUCE_LIVE_CONSTANT(2);
    
    int maxVisibleParams = 8;
    float padding = 5.0f;
    float rowSpacing = 8.0f;
    float columnSpacing = 17.0f;
    int columns = 2;

    if (audioProcessor.hostedPlugin != nullptr && dropZone->params_loaded)
    {
        if (chosen_parameters.size() != 0)
        {
            for (auto* param : chosen_parameters)
            {
                 auto* display = new ParameterDisplay(param);
                parameterDisplays.add(display);
                addAndMakeVisible(display);
            }
            loaded_params = true;
        }
        else
        {
            return;
        }

        auto bounds = boundsToUse.reduced(padding).toFloat();
        int totalParams = std::min<int>(maxVisibleParams, chosen_parameters.size());

        // Clamp columns to at least 1
        int actualColumns = columns;
        int rows = (int)std::ceil(maxVisibleParams / (float)actualColumns);

        float totalColumnSpacing = (actualColumns - 1) * columnSpacing;
        float colWidth = (bounds.getWidth() - totalColumnSpacing) / actualColumns;

        float totalRowSpacing = (rows - 1) * rowSpacing;
        float rowHeight = (bounds.getHeight() - totalRowSpacing) / rows;

        for (int i = 0; i < totalParams; ++i)
        {
            int col = i % actualColumns;
            int row = i / actualColumns;

            float x = bounds.getX() + col * (colWidth + columnSpacing);
            float y = bounds.getY() + row * (rowHeight + rowSpacing);

            parameterDisplays[i]->setBounds(x, y, colWidth, rowHeight);
            parameterDisplays[i]->applyDelta(chosen_deltas[i]);

        }
    }
}

void ChainBuilderAudioProcessorEditor::testParameterDisplayOffsets()
{
    DBG("=== Testing ParameterDisplay Offsets ===");

    // Assign test target values (simulate ChatGPT midpoints)
    for (int i = 0; i < parameterDisplays.size(); ++i)
    {
        auto* display = parameterDisplays[i];

        // Example: set target to 0.5 for float params
        display->setTargetValue(0.5f);

        // Log current value and offset
        float currentValue = 0.0f;
        if (auto* floatParam = dynamic_cast<juce::AudioParameterFloat*>(display->getParameter()))
        {
            currentValue = floatParam->get();
        }

        float offset = currentValue - 0.5f; // target = 0.5

        DBG("Parameter " << display->getParameter()->getName(100)
            << " | Current: " << currentValue
            << " | Target: 0.5"
            << " | Offset: " << offset);
    }

    DBG("=== End Test ===");
}


void ChainBuilderAudioProcessorEditor::display_metrics()
{
    juce::String metrics_display =
        "Spectral Centroid: " + juce::String(audioProcessor.spectral_centroid, 2) + "\n"
        "Spectral Rolloff: " + juce::String(audioProcessor.spectral_rolloff, 2) + "\n"
        "Spectral Flatness: " + juce::String(audioProcessor.spectral_flatness, 2) + "\n"
        "Resonance Score: " + juce::String(audioProcessor.resonance_score, 2) + "\n"
        "Harmonic-to-Noise: " + juce::String(audioProcessor.harmonic_to_noise, 2);

    auto area = getLocalBounds();

    // Let's say you want metrics_text to take half the width and 1/5 of the height
    auto metricsWidth = area.getWidth() / 2;
    auto metricsHeight = area.getHeight() / 5;

    metrics_text.setText(metrics_display, juce::dontSendNotification);
    std::string font = "Arial";
    metrics_text.setFont(juce::Font(font, 15.0f, juce::Font::plain));
    metrics_text.setColour(juce::Label::textColourId, juce::Colours::white);
    metrics_text.setJustificationType(juce::Justification::centred);
    metrics_text.setBounds(
        (area.getWidth() - metricsWidth) / 2,  // X: center
        (area.getHeight() - metricsHeight) / 2,  // Y: center
        metricsWidth,
        metricsHeight
    );
    // addAndMakeVisible(metrics_text);
}

// CLASSES
class NonFocusableDialog : public juce::DialogWindow
{
public:
    NonFocusableDialog (const juce::String& title,
                        juce::Colour background,
                        bool escapeKeyCloses)
        : DialogWindow (title, background, escapeKeyCloses)
    {
        setWantsKeyboardFocus (false);
    }

    int getDesktopWindowStyleFlags() const override
    {
        // Start with the default flags that DialogWindow wants
        int flags = DialogWindow::getDesktopWindowStyleFlags();

        // Add the “ignore key presses” bit so the OS never gives this
        // native window keyboard focus
        flags |= juce::ComponentPeer::windowIgnoresKeyPresses;

        return flags;
    }
};

ParameterDisplay::ParameterDisplay(juce::AudioProcessorParameter* p)
    : parameter(p)
{
    // Name label (smaller font)
    nameLabel.setText(parameter->getName(100), juce::dontSendNotification);
    nameLabel.setFont(juce::Font(12.0f, juce::Font::plain));
    nameLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(nameLabel);

    // Value label (bigger font)
    valueLabel.setFont(juce::Font(18.0f, juce::Font::bold));
    valueLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(valueLabel);

    // Offset label (tiny number, different colour)
    offsetLabel.setFont(juce::Font(11.0f, juce::Font::italic));
    offsetLabel.setColour(juce::Label::textColourId, juce::Colours::orange);
    offsetLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(offsetLabel);

    startTimerHz(10); // Update 10 times per second
}

void ParameterDisplay::resized()
{
    auto bounds = getLocalBounds();

    // Top: name label (smaller font)
    auto nameArea = bounds.removeFromTop(bounds.getHeight() * 0.25f);
    nameLabel.setBounds(nameArea);

    // Bottom: value + offset
    // Give value most of the space, offset just enough to show the number
    float valueFraction = 0.60f; // 85% for main value
    auto valueArea = bounds.removeFromLeft(bounds.getWidth() * valueFraction);
    valueLabel.setBounds(valueArea);

    auto offsetArea = bounds; // remaining 15% for offset
    offsetLabel.setBounds(offsetArea);
}

void ParameterDisplay::applyDelta(float delta)
{

    setTargetValue(delta);
    
}

void ParameterDisplay::timerCallback()
{
    if (parameter != nullptr)
    {
        // Find current value of parameter
        float normCurVal = parameter->getValue();
        juce::String unnormCurVal = parameter->getText(normCurVal, 100);
        
        // Show current value
        valueLabel.setText(unnormCurVal + " ", juce::dontSendNotification);
        
        // Show offset if target exists
        // 3 + 1 = 4
        if (targetValue.has_value())
        {
            float diff = targetValue.value() - unnormCurVal.getFloatValue();
            if (std::abs(diff) > 0.01f)
                offsetLabel.setText((diff > 0 ? "+" : "") + juce::String(diff, 2), juce::dontSendNotification);
            else
                offsetLabel.setText("", juce::dontSendNotification);
        }
        else
        {
            offsetLabel.setText("", juce::dontSendNotification);
        }
    }
}

void ParameterDisplay::setTargetValue(float newTarget)
{
    targetValue = newTarget;
}



PluginDropZone::PluginDropZone(ChainBuilderAudioProcessor& proc, ChainBuilderAudioProcessorEditor& editorRef)
    : audioProcessor(proc), hostEditor(editorRef) {
    formatManager.addDefaultFormats(); // VST, AU, etc.
    startTimerHz(60); // repaint 60 times per second
}

PluginDropZone::~PluginDropZone() {
    for (auto* instance : audioProcessor.pluginInstances)
    {
        if (instance)
        {
            auto& params = instance->getParameters();
            for (auto* p : params)
                p->removeListener(this);
        }
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
    loadPluginBoxes.clear();
    int maxPlugins = 3;
    int numBoxes = std::min(maxPlugins, selectedPluginNames.size() + 1);
    float boxWidth = 100.0f;
    float boxHeight = 50.0f;
    float spacing = 20.0f;
    float totalHeight = numBoxes * boxHeight + (numBoxes - 1) * spacing;
    float startY = getHeight() / 2.0f - totalHeight / 2.0f;
    float centerX = getWidth() / 2.0f;
    for (int i = 0; i < numBoxes; ++i)
    {
        float boxX = centerX - boxWidth / 2.0f;
        float boxY = startY + i * (boxHeight + spacing);
        juce::Rectangle<float> box(boxX, boxY, boxWidth, boxHeight);
        loadPluginBoxes.add(box);
        g.setColour(juce::Colours::white);
        g.setFont(16.0f);
        if (i < selectedPluginNames.size())
            g.drawText(selectedPluginNames[i], box.toNearestInt(), juce::Justification::centred);
        else
            g.drawText("Load Plugin", box.toNearestInt(), juce::Justification::centred);
        g.drawRoundedRectangle(box, 8.0f, 2.0f);
    }
}

void PluginDropZone::resized()
{

}



bool PluginDropZone::isClickOnPlus(const juce::Rectangle<float>& loadPluginBox, const juce::Point<int>& pos)
{
    return loadPluginBox.contains((float)pos.x, (float)pos.y);
}

// Mouse click handler
void PluginDropZone::mouseDown(const juce::MouseEvent& event)
{
    // Check which plugin box was clicked
    for (int i = 0; i < loadPluginBoxes.size(); ++i)
    {
        if (loadPluginBoxes[i].contains((float)event.x, (float)event.y))
        {
            if (i < selectedPluginNames.size())
            {
                // Open plugin editor for loaded plugin
                auto* instance = audioProcessor.pluginInstances[i];
                if (instance != nullptr)
                {
                    if (auto* ed = instance->createEditorIfNeeded())
                    {
                        editor.reset(ed);
                        ed->setWantsKeyboardFocus(false);
                        ed->setInterceptsMouseClicks(true, false);
                        addAndMakeVisible(editor.get());
                        static juce::ComponentAnimator animator;
                        auto pluginArea = hostEditor.getLocalBounds();
                        ed->setBounds(pluginArea.withY(0).withHeight(1));
                        animator.animateComponent(editor.get(), pluginArea, 1.0f, 300, true, 0.0f, 0.0f);
                        int requiredHeight = pluginArea.getHeight();
                        int requiredWidth = pluginArea.getWidth();
                        auto hostEditorBounds = hostEditor.getBounds();
                        hostEditor.setBounds(hostEditorBounds.withHeight(requiredHeight).withWidth(requiredWidth));
                        hostEditor.extend_panel = true;
                        hostEditor.togglePromptSidebar(hostEditor.extend_panel);
                    }
                }
            }
            else
            {
                // Load new plugin if less than 3
                if (selectedPluginNames.size() < 3)
                {
                    juce::FileSearchPath searchPaths;
                    #if JUCE_WINDOWS
                        searchPaths.add(juce::File("C:\\Program Files\\Common Files\\VST3"));
                        searchPaths.add(juce::File("C:\\Program Files\\Steinberg\\VST3"));
                    #elif JUCE_MAC
                        searchPaths.add(juce::File("/Library/Audio/Plug-Ins/VST3"));
                        searchPaths.add(juce::File("~/Library/Audio/Plug-Ins/VST3"));
                    #elif JUCE_LINUX
                        searchPaths.add(juce::File("/usr/lib/vst3"));
                        searchPaths.add(juce::File("/usr/local/lib/vst3"));
                    #endif
                    juce::File deadMansPedal = juce::File::getSpecialLocation(juce::File::userDocumentsDirectory).getChildFile("pluginScanState.tmp");
                    juce::PluginDirectoryScanner scanner(pluginList, pluginFormat, searchPaths, true, deadMansPedal, false);
                    juce::String pluginNames = "";
                    bool finished = false;
                    while (!finished)
                        finished = !scanner.scanNextFile(true, pluginNames);
                    juce::PopupMenu menu;
                    auto pluginTypes = pluginList.getTypes();
                    for (int j = 0; j < pluginTypes.size(); ++j)
                        menu.addItem(j + 1, pluginTypes[j].name);
                    menu.showMenuAsync(juce::PopupMenu::Options(),
                        [this, pluginTypes, i](int result)
                        {
                            if (result > 0)
                            {
                                auto selectedPlugin = pluginTypes[result - 1];
                                juce::String errorMessage;
                                auto instance = formatManager.createPluginInstance(
                                    selectedPlugin,
                                    audioProcessor.getSampleRate(),
                                    audioProcessor.getBlockSize(),
                                    errorMessage
                                );
                                if (instance == nullptr)
                                {
                                    DBG("Failed to load plugin: " << errorMessage);
                                }
                                else
                                {
                                    instance->setPlayConfigDetails(2, 2, audioProcessor.getSampleRate(), audioProcessor.getBlockSize());
                                    instance->prepareToPlay(audioProcessor.getSampleRate(), audioProcessor.getBlockSize());
                                    audioProcessor.pluginInstances.add(instance.release());
                                    selectedPluginNames.add(selectedPlugin.name);
                                    repaint();
                                }
                            }
                        });
                }
            }
            break;
        }
    }
}

void PluginDropZone::parameterValueChanged(int parameterIndex, float newValue)
{
    if (parameterIndex < parameters.size())
    {
        auto* param = parameters[parameterIndex];
        changedParameters[parameterIndex] = { param->getName(100), param->getText(newValue, 100)}; // Save Parameters Index
        
        hostEditor.emptyDetParams = false;
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


