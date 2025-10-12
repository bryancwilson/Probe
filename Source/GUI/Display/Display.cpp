
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

void ChainBuilderAudioProcessorEditor::showText()
{

    main_text.setText(b_text, juce::dontSendNotification);

    // Set the font for the label
    std::string font = "Arial";
    main_text.setFont(juce::Font(font, 15.0f, 0));

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

        auto bounds = boundsToUse.reduced(10).toFloat();  // slight padding
        int columns = 1;  // vertical stack
        int rows = std::min<int>(9, parameterDisplays.size());  // limit to 9

        float rowSpacing = 8.0f;
        float rowHeight = (bounds.getHeight() - (rows - 1) * rowSpacing) / rows;
        float colWidth = bounds.getWidth();

        for (int i = 0; i < rows; ++i)
        {
            float x = bounds.getX();
            float y = bounds.getY() + i * (rowHeight + rowSpacing);

            parameterDisplays[i]->setBounds(x, y, colWidth, rowHeight);
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
    metrics_text.setFont(juce::Font(font, 15.0f, 0));
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
    if (auto* floatParam = dynamic_cast<juce::AudioParameterFloat*>(parameter))
    {
        float current = floatParam->get();
        float target = std::clamp(current + delta, floatParam->range.start, floatParam->range.end);
        setTargetValue(target);
    }
}

void ParameterDisplay::timerCallback()
{
    if (parameter != nullptr)
    {
        float val = parameter->getValue();
        juce::String units;

        if (auto* floatParam = dynamic_cast<juce::AudioParameterFloat*>(parameter))
        {
            val = floatParam->get();
            units = floatParam->label;
        }

        // Show current value
        valueLabel.setText(juce::String(val, 2) + " " + units, juce::dontSendNotification);

        // Show offset if target exists
        if (targetValue.has_value())
        {
            float diff = val - targetValue.value();
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
        if (auto* instance = audioProcessor.hostedPlugin.get())
        {
            if (auto* ed = instance->createEditorIfNeeded())
            {
                editor.reset(ed);

                // Plugin editor should not eat keyboard focus
                ed->setWantsKeyboardFocus(false);
                ed->setInterceptsMouseClicks(true, false);

                addAndMakeVisible(editor.get());

                // Animate appearance
                static juce::ComponentAnimator animator;

                // Target bounds: left side of Probe window (minus sidebar)
                int sidebarWidth = 300;
                auto fullArea = getLocalBounds();
                auto pluginArea = fullArea.removeFromRight(sidebarWidth);

                // Animate from collapsed to full height
                ed->setBounds(pluginArea.withHeight(1));
                animator.animateComponent(editor.get(),
                    pluginArea,
                    1.0f,   // final alpha
                    300,    // ms duration
                    true,   // use proxy
                    0.0f,
                    0.0f);

                hostEditor.extend_panel = true;
                hostEditor.togglePromptSidebar(hostEditor.extend_panel);
            }
        }

        auto* processor = audioProcessor.hostedPlugin.get();

        DBG("Name: " << processor->getName());
        DBG("Inputs: " << processor->getTotalNumInputChannels());
        DBG("Outputs: " << processor->getTotalNumOutputChannels());

        DBG("========================== Plugin Parameters =============================");
        parameters.clear();

        int index = 0;
        for (auto* param : audioProcessor.hostedPlugin->getParameters())
        {

            // Optional: filter out non-automatable or uninteresting params
            auto c = param->getCategory();
            if (param->isBoolean() || param->isMetaParameter())
                continue;

            // Get parameter data
            auto name = param->getName(100);
            auto label = param->getLabel();

            // Store and Listen to Paramters
            param->addListener(this);
            parameters.add(param);

        }


        params_loaded = true;
    }

    juce::FileSearchPath searchPaths;

    #if JUCE_WINDOWS
        searchPaths.add(juce::File("C:\\Program Files\\Common Files\\VST3"));
        searchPaths.add(juce::File("C:\\Program Files\\Steinberg\\VST3"));

    #elif JUCE_MAC
        // Typical system-wide VST3 location on macOS
        searchPaths.add(juce::File("/Library/Audio/Plug-Ins/VST3"));
        // User-specific location (optional)
        searchPaths.add(juce::File("~/Library/Audio/Plug-Ins/VST3"));

    #elif JUCE_LINUX
        // Common Linux locations
        searchPaths.add(juce::File("/usr/lib/vst3"));
        searchPaths.add(juce::File("/usr/local/lib/vst3"));
        // Add more if your distro uses different paths
    #endif

    
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

    menu.showMenuAsync(juce::PopupMenu::Options(),
        [this, pluginTypes](int result)
        {
            if (result > 0)
            {
                auto selectedPlugin = pluginTypes[result - 1];
                DBG("User chose plugin: " << selectedPlugin.name);

                // Example: create plugin instance
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
                    DBG("Plugin loaded successfully!");

                    audioProcessor.hostedPlugin = std::move(instance);

                    // Set stereo input/output channel layout
                    audioProcessor.hostedPlugin->setPlayConfigDetails(2, 2,
                        audioProcessor.getSampleRate(),
                        audioProcessor.getBlockSize());

                    audioProcessor.hostedPlugin->prepareToPlay(audioProcessor.getSampleRate(), audioProcessor.getBlockSize());
                    

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
        changedParameters[parameterIndex] = { param->getName(100), param->getText(newValue, 100)}; // Save Parameters Index
       
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

