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

//void ChainBuilderAudioProcessorEditor::timerCallback()
//{
//
//}

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
    // Animate the dash phase for any dashed outlines (if used)
    dashPhase += 0.5f;
    if (dashPhase > 6.0f) // reset after one dash length
        dashPhase = 0.0f;

    // Determine the target hover animation value (1.0 if hovered, 0.0 if not)
    float target = (hoveredPluginIndex >= 0 || addHoveredIndex >= 0 || emptyPluginBoxHover) ? 1.0f : 0.0f;
    // Animation speeds for color fade
    float speedIn = 0.07f;   // Speed when fading in
    float speedOut = 0.03f;  // Speed when fading out
    float speed = (target > hoverAnim) ? speedIn : speedOut;
    bool needsRepaint = false;
    // Animate hover color transition
    if (std::abs(hoverAnim - target) > 0.01f) {
        hoverAnim += (target - hoverAnim) * speed;
        needsRepaint = true;
    } else {
        hoverAnim = target;
    }
    
    // --- SIMPLIFIED SLIDE LOGIC ---
    // State machine for slide button animation
    switch (slideState) {
        case SlideState::Hidden: // Button is fully hidden
            
            if (hoveredPluginIndex >= 0) {  // transition to AnimatingIn if hovered
                slideState = SlideState::AnimatingIn;
            }
            slideAnim = 0.0f;
            break;
            
        case SlideState::AnimatingIn: // Animate the button sliding in
            
            if (hoveredPluginIndex < 0) {
                // If no longer hovered, start animating out
                slideState = SlideState::AnimatingOut;
            } else {
                float slideTarget = 1.0f; // fully shown
                float slideSpeed = 0.12f; // slide-in speed
                slideAnim += (slideTarget - slideAnim) * slideSpeed; // animate
                
                needsRepaint = true; // repaint needed
                
                // If animation is close enough to target, snap to shown state
                if (std::abs(slideAnim - slideTarget) <= 0.01f) {
                    slideAnim = slideTarget;
                    slideState = SlideState::Shown;
                }
                previouslyHoveredPluginIndex = hoveredPluginIndex;
            }
            break;
        case SlideState::Shown:
            // Button is fully shown; transition to AnimatingOut if not hovered
            if (hoveredPluginIndex < 0) {
                slideState = SlideState::AnimatingOut;
            }
            slideAnim = 1.0f;
            break;
        case SlideState::AnimatingOut:
            // Animate the button sliding out
            if (hoveredPluginIndex >= 0) {
                // If hovered again, start animating in
                slideState = SlideState::AnimatingIn;
            } else {
                float slideTarget = 0.0f; // fully hidden
                float slideSpeed = 0.20f; // slide-out speed
                slideAnim += (slideTarget - slideAnim) * slideSpeed; // animate
                
                needsRepaint = true; // repaint needed
                
                // If animation is close enough to target, snap to hidden state
                if (std::abs(slideAnim - slideTarget) <= 0.01f) {
                    slideAnim = slideTarget;
                    slideState = SlideState::Hidden;
                    previouslyHoveredPluginIndex = -1;
                }
            }
            break;
        case SlideState::AnimatingAway:
            
            float slideTarget = 1.0f; // fully shown
            float slideSpeed = 0.12f; // slide-in speed
            slideAnim += (slideTarget - slideAnim) * slideSpeed; // animate
            needsRepaint = true; // repaint needed
            
            break;
    }

    // If any animation is in progress, repaint
    if (needsRepaint) {
        repaint();
    } else if (hoverAnim == 0.0f && slideAnim == 0.0f) {
        // If no animation is active, stop the timer and do a final repaint
        stopTimer();
        repaint();
    }
}

void PluginDropZone::paint(juce::Graphics& g)
{
    loadPluginBoxes.clear();
    xButtonRects.clear();
    bypassButtonRects.clear();
    int maxPlugins = 3;
    int numBoxes = std::min(maxPlugins, (int)selectedPluginNames.size() + 1);
    
    // Box Parameters
    float boxWidth = 100.0f;
    float boxHeight = 50.0f;
    float spacing = 80.0f;
    float totalHeight = numBoxes * boxHeight + (numBoxes - 1) * spacing;
    float startY = getHeight() / 2.0f - totalHeight / 2.0f;
    float centerX = getWidth() / 2.0f;
    
    // Slide Box Parameters
    float btnW = 36.0f; // Skinnier button
    float btnH = boxHeight; // Match plugin button height
    float gap = 8.0f; // Space between plugin box and x button
    float slideOffset = btnW * (1.0f - slideAnim) * 4.0f; // Slide offset based on animation progress;
    
    for (int i = 0; i < numBoxes; ++i)
    {
        // =================================== Load Plugin Box Logic ===========================================
        float boxX = centerX - boxWidth / 2.0f;
        if (i == hoveredPluginIndex || i == previouslyHoveredPluginIndex) // slightly slide over load plugin box if hovered
        {
            switch (slideState) {
                case SlideState::AnimatingIn:
                    boxX -= 30.0f * slideAnim;
                    break;
                case SlideState::Shown:
                    boxX -= 30.0f;
                    break;
                case SlideState::AnimatingOut:
                    boxX -= 30.0f * slideAnim;
                    break;
                case SlideState::AnimatingAway:
                    boxX -= 300.0f * slideAnim;
                    if (slideAnim == 1.f)
                    {
                        // Remove plugin at index i
                        selectedPluginNames.remove(i);
                        audioProcessor.pluginInstances.remove(i);
                        slideState = SlideState::Hidden;
                        repaint();
        
                    }
                default:
                    break;
            }
        }
        
        float boxY = startY + i * (boxHeight + spacing);
        juce::Rectangle<float> box(boxX, boxY, boxWidth, boxHeight);
        loadPluginBoxes.add(box);

        // ============================== Handle Hover Logic =======================================
        if (i == selectedPluginNames.size()) // This is the empty box
        {
            if (emptyPluginBoxHover && hoverAnim > 0.01f)
            {
                juce::Colour base = juce::Colours::grey;
                juce::Colour hover = juce::Colours::white;
                juce::Colour blended = base.interpolatedWith(hover, hoverAnim * 0.5f);
                g.setColour(blended);
            }
            else
            {
                g.setColour(juce::Colours::grey);
            }
            g.setFont(16.0f);
            g.drawText("Load Plugin", box.toNearestInt(), juce::Justification::centred);
            g.drawRoundedRectangle(box, 8.0f, 2.0f);
            continue; // Skip rest of loop for empty box
        }
        else{
            // Set Colour For Load Plugin Box
            if (i == hoveredPluginIndex && hoverAnim > 0.01f)
            {
                juce::Colour base = juce::Colours::white;
                juce::Colour hover = juce::Colours::orange;
                juce::Colour blended = base.interpolatedWith(hover, hoverAnim * 0.5f);
                g.setColour(blended);
            }
            else
            {
                g.setColour(juce::Colours::white);
            }
            g.setFont(16.0f);
            g.drawText(selectedPluginNames[i], box.toNearestInt(), juce::Justification::centred);
            g.drawRoundedRectangle(box, 8.0f, 2.0f); // Draw Load Plugin Box
        }
        
        // =================================== Handle Connection Logic ====================================
        if (i < selectedPluginNames.size())
        {
            int shrinkage = 15;
            if (hoveredPluginIndex == i)
            {
                juce::Colour base = juce::Colours::grey;
                juce::Colour hover = juce::Colours::black;
                juce::Colour blended = base.interpolatedWith(hover, hoverAnim);
                g.setColour(blended);
            }
            else if (addHoveredIndex == i)
            {
                juce::Colour base = juce::Colours::grey;
                juce::Colour hover = juce::Colours::white;
                juce::Colour blended = base.interpolatedWith(hover, hoverAnim);
                g.setColour(blended);
            }
            else
            {
                g.setColour(juce::Colours::grey);
            }
            
            juce::Rectangle<float> add(
                box.getX() - gap - btnW + shrinkage,
                box.getY(),
                btnW - shrinkage,
                btnH
            );
            
            g.setFont(24.0f);
            g.drawText("+", add, juce::Justification::centred);
            g.drawRoundedRectangle(add, 8.0f, 2.0f); // may look good without
            
            addButtonRects.add(add); // add buttons array

        }
        
        // =================================== Sliding Button Logic ===========================================
        if ((i == hoveredPluginIndex || i == previouslyHoveredPluginIndex) && i < selectedPluginNames.size())
        {
            
            juce::Rectangle<float> close(
                box.getRight() + gap + slideOffset,
                box.getY(),
                btnW,
                btnH
            );
            juce::Rectangle<float> bypass(
                close.getRight() + gap + slideOffset,
                close.getY(),
                btnW,
                btnH
            );
            
            switch (slideState) {
                case SlideState::AnimatingIn:
                    slideOffset -= (1.0f * slideAnim);
                    break;
                case SlideState::AnimatingOut:
                    slideOffset -= 1.0f * slideAnim;
                    break;
                case SlideState::AnimatingAway:
                    if (bypass.getRight() < 1.0f)
                    {
                        // Remove plugin at index i
                        selectedPluginNames.remove(i);
                        audioProcessor.pluginInstances.remove(i);
                        slideState = SlideState::Hidden;
                        
                        hoveredPluginIndex = -1;
                        // previouslyHoveredPluginIndex = -1;
                        
                        repaint();
                    }
                default:
                    break;
            }

            // Store for hit testing
            xButtonRects.add(close);
            bypassButtonRects.add(bypass);
            // Draw close button
            g.setColour(juce::Colours::white.withAlpha(0.12f));
            g.fillRoundedRectangle(close, 8.0f);
            g.setColour(juce::Colours::red);
            g.setFont(24.0f);
            g.drawText("x", close, juce::Justification::centred);
            g.drawRoundedRectangle(close, 8.0f, 2.0f);
            // Draw bypass button
            g.setColour(juce::Colours::white.withAlpha(0.12f));
            g.fillRoundedRectangle(bypass, 8.0f);
            g.setColour(juce::Colours::blue);
            g.setFont(24.0f);
            g.drawText("b", bypass, juce::Justification::centred);
            g.drawRoundedRectangle(bypass, 8.0f, 2.0f);

            // --- Conditional: Check if right side of bypass meets left side of plugin box ---
            if (std::abs(bypass.getRight() - box.getX()) < 1.0f) // Allow for float rounding
            {
                // They are touching (or nearly touching)
                DBG("Bypass right meets plugin left");
                // You can add any logic here that should happen when they meet
            }
        } else {
            // Keep arrays in sync
            xButtonRects.add(juce::Rectangle<float>());
            bypassButtonRects.add(juce::Rectangle<float>());
        }
        
            
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
    // Check for x or bypass button clicks first
    for (int i = 0; i < xButtonRects.size(); ++i)
    {
        if (xButtonRects[i].contains((float)event.x, (float)event.y))
        {
            slideState = SlideState::AnimatingAway;
            slideAnim = 0.0f; // Reset Slide Animation
            startTimerHz(60); // Start timer for hover/slide animation
            return;
        }
        if (bypassButtonRects[i].contains((float)event.x, (float)event.y))
        {
            // Toggle bypass for plugin at index i (example logic)
            // You may need to implement actual bypass logic in your processor
            // For now, just print
            DBG("Bypass button clicked for plugin " << i);
            hostEditor.SlideOverDropZone();
            
            // Optionally: set a bypass state array and repaint
            return;
        }
    }
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

void PluginDropZone::mouseMove(const juce::MouseEvent& event)
{

    // =========================== Add Buttin Hover Logic ====================================
    int addHovered = addHoveredIndex;
    
    for (int i = 0; i < addButtonRects.size(); ++i)
    {
        if (addButtonRects[i].contains((float)event.x, (float)event.y))
        {
            addHovered = i;
            break; // Stop searching after the first match
        }
        addHovered = -1;
    }

    
    // ========================== Load Plugin Box Hover Logic ==================================
    int hovered = hoveredPluginIndex;
    if (hovered != -1 and ~in_vertical_bounds)
    {
        float boxX = loadPluginBoxes[hovered].getX();
        float boxY = loadPluginBoxes[hovered].getY();
        float boxHeight = loadPluginBoxes[hovered].getHeight();
        if (event.x > boxX + 100.0f) // 100.0f is box width
        {
            if (event.y >= boxY && event.y <= boxY + boxHeight)
            {
                // Mouse has moved to the right while still in vertical bounds
                in_vertical_bounds = true;
                return; // Keep the hovered index unchanged
            }
        }
    }
    // Iterate through all plugin boxes
    for (int i = 0; i < loadPluginBoxes.size(); ++i)
    {
        // Only check boxes that correspond to loaded plugins (not the empty 'Load Plugin' box)
        if (i < selectedPluginNames.size() && loadPluginBoxes[i].contains((float)event.x, (float)event.y))
        {
            // If the mouse is inside this plugin box, mark it as hovered
            hovered = i;
            break; // Stop searching after the first match
        }
        
        if (i == selectedPluginNames.size() && loadPluginBoxes[i].contains((float)event.x, (float)event.y))
        {
            // If the mouse is inside the 'Load Plugin' box, mark it as hovered
            emptyPluginBoxHover = true;
            repaint();
            break; // Stop searching after the first match
        }
        else{
            emptyPluginBoxHover = false;
            repaint();
        }
        hovered = -1; // No box is hovered
    }
    // If the hovered box has changed, update the state and start the animation timer
    if (hovered != hoveredPluginIndex || emptyPluginBoxHover || addHovered != addHoveredIndex)
    {
        if (hovered != hoveredPluginIndex)
        {
            hoveredPluginIndex = hovered;
        }
        else if (addHovered != addHoveredIndex)
        {
            addHoveredIndex = addHovered;
        }
        startTimerHz(60); // Start timer for hover/slide animation
    }
}

void PluginDropZone::mouseEnter(const juce::MouseEvent& event)
{
    mouseMove(event);
}

void PluginDropZone::mouseExit(const juce::MouseEvent&)
{
    if (hoveredPluginIndex != -1)
    {
        hoveredPluginIndex = -1;
        startTimerHz(60);
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






