#include "../../PluginEditor.h"
#include "Display.h"
#include "../../Metrics/Metrics.h"
#include <cmath>

// ====================================== MACROS =======================================
// CLICK ANIMATION CONSTANTS
#define CLICK_ANIM_MULT 2.2f
#define PLUGIN_BOX_WIDTH 100.f
#define PLUGIN_BOX_X_DISP 40.f
#define ORIGINAL_SHRINKAGE 15.f
#define TARGET_SHRINKAGE 15.f - PLUGIN_BOX_X_DISP

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

void PluginDropZone::updateSlideAnimation(bool& needsRepaint)
{
    switch (slideState)
    {
        case SlideState::Hidden:
            if (hoveredPluginIndex >= 0) {
                slideState = SlideState::AnimatingIn;
            }
            slideAnim = 0.0f;
            break;

        case SlideState::AnimatingIn:
            if (hoveredPluginIndex < 0) {
                slideState = SlideState::AnimatingOut;
            } else {
                slideAnim += (1.0f - slideAnim) * 0.12f;
                needsRepaint = true;
                if (slideAnim >= 0.99f) {
                    slideAnim = 1.0f;
                    slideState = SlideState::Shown;
                }
                previouslyHoveredPluginIndex = hoveredPluginIndex;
            }
            break;

        case SlideState::Shown:
            if (hoveredPluginIndex < 0) {
                slideState = SlideState::AnimatingOut;
            }
            slideAnim = 1.0f;
            break;

        case SlideState::AnimatingOut:
            if (hoveredPluginIndex >= 0) {
                slideState = SlideState::AnimatingIn;
            } else {
                slideAnim += (0.0f - slideAnim) * 0.20f;
                needsRepaint = true;
                if (slideAnim <= 0.01f) {
                    slideAnim = 0.0f;
                    slideState = SlideState::Hidden;
                    previouslyHoveredPluginIndex = -1;
                }
            }
            break;

        case SlideState::AnimatingAway:
            slideAnim += (1.0f - slideAnim) * 0.12f;
            needsRepaint = true;
            
            // Logic for when the plugin "yeets" off the screen
            if (slideAnim >= 0.99f) {
                handlePluginRemoval(); // Helper for the cleanup logic
            }
            break;
    }
}

void PluginDropZone::handlePluginRemoval()
{
    // Ensure we have a valid index to remove
    if (previouslyHoveredPluginIndex >= 0 && previouslyHoveredPluginIndex < selectedPluginNames.size())
    {
        selectedPluginNames.remove(previouslyHoveredPluginIndex);
        audioProcessor.pluginInstances.remove(previouslyHoveredPluginIndex);
        
        // Reset UI states
        slideState = SlideState::Hidden;
        slideAnim = 0.0f;
        hoveredPluginIndex = -1;
        previouslyHoveredPluginIndex = -1;
        
        repaint();
    }
}
void PluginDropZone::timerCallback()
{
    dashPhase = std::fmod(dashPhase + 0.5f, 6.0f);
    bool needsRepaint = false;

    // 1. Hover Animation Progress
    float hoverTarget = (hoveredPluginIndex >= 0 || addHoveredIndex >= 0 || emptyPluginBoxHover) ? 1.0f : 0.0f;
    if (std::abs(hoverAnim - hoverTarget) > 0.001f) {
        hoverAnim += (hoverTarget - hoverAnim) * ((hoverTarget > hoverAnim) ? 0.07f : 0.03f);
        needsRepaint = true;
    }

    // 2. Click/Routing Animation Progress (The Sigmoid Drive)
    float clickTarget = (triggerClickAnim) ? 1.0f : 0.0f;
    if (std::abs(clickAnim - clickTarget) > 0.001f) {
        clickAnim += (clickTarget - clickAnim) * ((clickTarget > clickAnim) ? 0.07f : 0.03f);
        needsRepaint = true;
    } else {
        clickAnim = clickTarget;
        // Handle state completion
        if (triggerClickAnim && clickAnim >= 1.0f) {
            if (addState == AddState::SlidingOut) addState = AddState::Shown;
            else if (addState == AddState::SlidingIn) { addState = AddState::Default; addClicked = -1; }
            triggerClickAnim = false;
        }
    }
    float x = clickAnim;
    float y = 1.f - clickAnim;
    visualClickAnim = x * x * x * (x * (x * 6.0f - 15.0f) + 10.0f);
    visualClickAnimInv = y * y * y * (y * (y * 6.0f - 15.0f) + 10.0f);

    // 3. Slide State Machine (Simplified)
    updateSlideAnimation(needsRepaint);

    if (needsRepaint) repaint();
    
    // Stop timer only if everything is static
    if (!needsRepaint && hoverAnim <= 0.0f && slideAnim <= 0.0f && clickAnim <= 0.0f)
        stopTimer();
}

void PluginDropZone::drawCurvedArrow (juce::Graphics& g,
                      juce::Point<float> from,
                      juce::Point<float> to,
                      float curvature = 30.0f,
                      float shaftThickness = 3.0f,
                      float headLength = 12.0f,
                      float headWidth = 14.0f,
                      juce::Colour colour = juce::Colours::white)
{
    const float len = from.getDistanceFrom (to);
    if (len <= 1.0f) return;

    // 1. Calculate Control Point for the curve
    auto midPoint = (from + to) * 0.5f;
    auto dir = (to - from) / len;
    auto perp = juce::Point<float> (-dir.y, dir.x);
    auto controlPoint = midPoint + (perp * curvature);

    // 2. Determine Arrow Head Angle (Corrected Normalization)
    auto tangentVec = to - controlPoint;
    auto tangentLen = tangentVec.getDistanceFromOrigin();
    auto tangentDir = (tangentLen > 0.0f) ? tangentVec / tangentLen : dir;
    auto tangentPerp = juce::Point<float> (-tangentDir.y, tangentDir.x);

    // 3. Define Arrow Head Points
    auto headBase = to - (tangentDir * headLength);
    auto left  = headBase + (tangentPerp * (headWidth * 0.5f));
    auto right = headBase - (tangentPerp * (headWidth * 0.5f));

    // 4. Draw the Curved Shaft
    juce::Path shaft;
    shaft.startNewSubPath (from);
    // Draw to headBase so the line doesn't overlap the tip
    shaft.quadraticTo (controlPoint, headBase);
    
    g.setColour (colour);
    g.strokePath (shaft, juce::PathStrokeType (shaftThickness,
                                               juce::PathStrokeType::curved,
                                               juce::PathStrokeType::rounded));

    // 5. Draw the Head
    juce::Path head;
    head.addTriangle (to, left, right);
    g.fillPath (head);
}

void PluginDropZone::paint(juce::Graphics& g)
{
    loadPluginBoxes.clear();
    xButtonRects.clear();
    addButtonRects.clear();
    bypassButtonRects.clear();
    int maxPlugins = 3;
    int numBoxes = std::min(maxPlugins, (int)selectedPluginNames.size() + 1);
    
    // Box Parameters
    float boxHeight = 50.0f;
    float spacing = 80.0f;
    float totalHeight = numBoxes * boxHeight + (numBoxes - 1) * spacing;
    float startY = getHeight() / 2.0f - totalHeight / 2.0f;
    
    // Slide Box Parameters
    float btnW = 36.0f; // Skinnier button
    float btnH = boxHeight; // Match plugin button height
    float gap = 8.0f; // Space between plugin box and x button
    float slideOffset = btnW * (1.0f - slideAnim) * 4.0f; // Slide offset based on animation progress;
    
    for (int i = 0; i < numBoxes; ++i)
    {
        // =================================== Sliding Animation (Load Plugin Box Logic) ===========================================
        float boxX = (getWidth() / 2.0f) - 100 / 2.0f;
        float boxWidth = PLUGIN_BOX_WIDTH;
        float shrinkage = ORIGINAL_SHRINKAGE;
        
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
        
        // ================================== Click Button Animation (Load Plugin Box Logic and "+" Box) ===========================================
        // Common constants for all cases
        const float fullWidth = PLUGIN_BOX_WIDTH;
        const float targetWidth = PLUGIN_BOX_WIDTH - PLUGIN_BOX_X_DISP;
        const float originalBoxX = (getWidth() / 2.0f) - (100.0f / 2.0f);
        const float origShrinkage = ORIGINAL_SHRINKAGE; // how much the box shrinks during click animation
        const float targetShrinkage = TARGET_SHRINKAGE;

        switch (addState)
        {
            case AddState::Default:
                if (triggerClickAnim)
                {
                    addState = AddState::SlidingOut;
                    init_var_for_click_anim = true;
                }
                break;

            case AddState::SlidingOut:
                if (addClicked != i && i < selectedPluginNames.size())
                {
                    // Use the sigmoid to map from Start -> End
                    boxWidth  = fullWidth + (targetWidth - fullWidth) * visualClickAnim;
                    boxX      = originalBoxX + (PLUGIN_BOX_X_DISP * visualClickAnim);
                    shrinkage = origShrinkage + (targetShrinkage - origShrinkage) * visualClickAnim;
                    
                    juce::Colour base = juce::Colours::white;
                    juce::Colour hover = juce::Colours::grey;
                    juce::Colour blended = base.interpolatedWith(hover, clickAnim);
                    g.setColour(blended);
                    DBG("Blended: " + blended.toString());
                    
                    if (clickAnim >= .9f)
                    {
                        clickAnim = 1.0f;
                        triggerClickAnim = false;
                        addState = AddState::Shown;
                    }
                }
                break;

            case AddState::Shown:
                if (triggerClickAnim)
                {
                    addState = AddState::SlidingIn;
                }
                
                // While shown, keep the values locked at the "End" state
                if (addClicked != i && i < selectedPluginNames.size())
                {
                    boxWidth  = targetWidth;
                    boxX      = originalBoxX + PLUGIN_BOX_X_DISP;
                    shrinkage = targetShrinkage;
                    g.setColour(juce::Colours::grey);
                }

                break;

            case AddState::SlidingIn:

                if (addClicked != i && i < selectedPluginNames.size())
                {
                    // The same math applies! As clickAnim goes from 1.0 down to 0.0,
                    // the values will naturally slide back to their original positions.
                    boxWidth  = fullWidth + (targetWidth - fullWidth) * visualClickAnimInv;
                    boxX      = originalBoxX + (PLUGIN_BOX_X_DISP * visualClickAnimInv);
                    shrinkage = origShrinkage + (targetShrinkage - origShrinkage) * visualClickAnimInv;
                    
                    juce::Colour base = juce::Colours::grey;
                    juce::Colour hover = juce::Colours::white;
                    juce::Colour blended = base.interpolatedWith(hover, visualClickAnim);
                    g.setColour(blended);

                    if (clickAnim >= .9f)
                    {
                        clickAnim = 1.0f;
                        triggerClickAnim = false;
                        addState = AddState::Default;
                        addClicked = -1;
                    }
                }
                break;
        }

        // ============================== Handle Hover Logic (Load Plugin Box) =======================================
        if (i == selectedPluginNames.size()) // This is the empty box
        {

            if (emptyPluginBoxHover && hoverAnim > 0.01f)
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
            
        }
        else{
            if (i == hoveredPluginIndex && hoverAnim > 0.01f)
            {
                juce::Colour base = juce::Colours::white;
                juce::Colour hover = juce::Colours::orange;
                juce::Colour blended = base.interpolatedWith(hover, hoverAnim * 0.5f);
                g.setColour(blended);
                // DBG("Blended: " + blended.toString());
            }
            else
            {
                g.setColour(juce::Colours::white);
            }
            
        }
        
        // =========================== Draw Load Plugin Box ===========================================
        float boxY = startY + i * (boxHeight + spacing);
        juce::Rectangle<float> box(boxX, boxY, boxWidth, boxHeight);
        loadPluginBoxes.add(box);
        g.setFont(16.0f);
        if (i == selectedPluginNames.size())
        {
            g.drawText("Load Plugin", box.toNearestInt(), juce::Justification::centred);
            g.drawRoundedRectangle(box, 8.0f, 2.0f);
            continue; // Skip rest of loop for empty box
        }
        else
        {
            g.drawText(selectedPluginNames[i], box.toNearestInt(), juce::Justification::centred);
            g.drawRoundedRectangle(box, 8.0f, 2.0f); // Draw Load Plugin Box
        }
        // =================================== Handle Connection Logic ====================================
        if (i < selectedPluginNames.size())
        {
            if (hoveredPluginIndex == i) // Disappear effect when hovering over plugin box
            {
                juce::Colour base = juce::Colours::grey;
                juce::Colour hover = juce::Colours::black;
                juce::Colour blended = base.interpolatedWith(hover, hoverAnim);
                g.setColour(blended);
            }
            else if (addHoveredIndex == i) // Add button hover effect
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
    // compute box
    float boxW = 120.0f;
    float boxH = 10.0f;
    float x = getWidth() / 2.0f - boxW / 2.0f; // center horizontally
    float y = getHeight() - boxH; // bottom of component

    juce::Rectangle<float> hintBox (x, y, boxW, boxH);

    // background (semi-transparent)
    g.setColour(juce::Colours::black.withAlpha(0.6f));
    g.fillRoundedRectangle(hintBox, 4.0f);

    // text inside box
    g.setColour(juce::Colours::white);
    g.setFont(12.0f); // float font size
    // drawText takes an int rect so convert for crisp text:
    if (addState == AddState::SlidingOut || addState == AddState::Shown)
    {
        g.drawText("route plugin to other plugin by clicking on their respective '+' buttons", hintBox.toNearestInt(), juce::Justification::centred, true);
    }
    
    // =================================== Arrow Connection Logic ====================================
    if (connection_array.size() >= 2 && addState == AddState::SlidingIn) // need at least 2 connections to draw an arrow
    {
        arrow_beg_ends.clear(); // clear out arrow positions
        for (int i = 0; i < selectedPluginNames.size() - 1; i++)
        {
            // index plugin box id's
            int idx_1 = connection_array[i];
            int idx_2 = connection_array[i + 1];
            
            // get positions for arrow start and end
            juce::Point<float> pos_1 (loadPluginBoxes[idx_1].getCentreX(), loadPluginBoxes[idx_1].getBottom());
            juce::Point<float> pos_2 (loadPluginBoxes[idx_2].getCentreX(), loadPluginBoxes[idx_2].getY());
            
            // append arrow positions to array
            arrow_beg_ends.add(pos_1);
            arrow_beg_ends.add(pos_2);
            
        }
    }

    if (arrow_beg_ends.size() >= 2)
    {
        for (int i = 0; i < arrow_beg_ends.size(); i += 2)
        {
            juce::Point<float> from = arrow_beg_ends[i];
            juce::Point<float> to = arrow_beg_ends[i + 1];
            drawCurvedArrow(g, from, to, 35.0f, 5.0f, 5.0f, 5.0f, juce::Colours::yellow);
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
            DBG("Bypass button clicked for plugin " << i);
            hostEditor.SlideOverDropZone();
            
            // Optionally: set a bypass state array and repaint
            return;
        }
        if (addButtonRects[i].contains((float)event.x, (float)event.y))
        {
            // connection_array.add(i); // add connection to array
            if (addClicked == -1) // Only trigger if no other add button is currently clicked (prevents multiple rapid clicks)
            {
                addClicked = i; // Store which plugin's add button was clicked
                triggerClickAnim = true; // Start click animation
                startTimerHz(60); // Start timer for hover/slide animation
                return;
            }
            else if (addClicked == i) // If the same button is clicked again, reset the state (toggle behavior)
            {
                triggerClickAnim = true; // Start click animation
                startTimerHz(60); // Start timer for hover/slide animation
                connection_array.removeLast(1); // remove last connection from array
                return;
            }
            else if (addClicked != i) // If a different add button is clicked while one is already active, you route the connection
{
//                if (!connection_array.contains(i)) // prevent duplicate connections
//                {
//                    connection_array.add(i); // add second connection to array
//                }
                addState = AddState::SlidingIn;
                triggerClickAnim = true; // Start click animation
                startTimerHz(60); // Start timer for hover/slide animation
                return;
            }
            
            
            


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







