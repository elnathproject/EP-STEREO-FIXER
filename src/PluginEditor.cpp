#include "PluginEditor.h"
#include <cmath>

static void drawArrow(juce::Graphics& g, juce::Point<float> start, juce::Point<float> end, float headSize, float lineWidth)
{
    g.drawLine(juce::Line<float>(start, end), lineWidth);

    const float angle = std::atan2(end.y - start.y, end.x - start.x);
    juce::Path head;
    head.addTriangle(0.0f, -headSize * 0.5f, 0.0f, headSize * 0.5f, headSize, 0.0f);
    g.fillPath(head, juce::AffineTransform::rotation(angle).translated(end.x, end.y));
}

static void drawCircleOutline(juce::Graphics& g, juce::Point<float> centre, float radius, float lineWidth)
{
    juce::Path p;
    p.addEllipse(centre.x - radius, centre.y - radius, radius * 2.0f, radius * 2.0f);
    g.strokePath(p, juce::PathStrokeType(lineWidth, juce::PathStrokeType::mitered, juce::PathStrokeType::rounded));
}

IconButton::IconButton(Type type, const juce::String& label, const juce::String& tooltip)
    : Button(label), buttonType(type), labelText(label)
{
    setTooltip(tooltip);
    setClickingTogglesState(false);
}

void IconButton::paintButton(juce::Graphics& g, bool isMouseOver, bool)
{
    auto bounds = getLocalBounds().toFloat();
    const bool active = getToggleState();

    const juce::Colour bg = active ? juce::Colour(0xff5a5a6e) : (isMouseOver ? juce::Colour(0xff3a3a4e) : juce::Colour(0xff262632));
    g.setColour(bg);
    g.fillRoundedRectangle(bounds, 6.0f);

    g.setColour(juce::Colour(0x40ffffff));
    g.drawRoundedRectangle(bounds, 6.0f, 1.0f);

    auto iconArea = bounds.removeFromTop(bounds.getHeight() * 0.55f).reduced(8.0f);
    g.setColour(juce::Colours::white.withAlpha(active ? 1.0f : 0.75f));

    switch (buttonType)
    {
        case Type::Stereo:    drawStereoIcon(g, iconArea, active);    break;
        case Type::Flip:      drawFlipIcon(g, iconArea, active);      break;
        case Type::Sum:       drawSumIcon(g, iconArea, active);       break;
        case Type::Left:      drawLeftIcon(g, iconArea, active);      break;
        case Type::Right:     drawRightIcon(g, iconArea, active);     break;
        case Type::MidSide:   drawMidSideIcon(g, iconArea, active);   break;
        case Type::SoloMid:   drawSoloMidIcon(g, iconArea, active);   break;
        case Type::SoloSide:  drawSoloSideIcon(g, iconArea, active);  break;
        case Type::Link:      drawLinkIcon(g, iconArea, active);      break;
    }

    g.setFont(juce::Font(juce::FontOptions(10.0f, juce::Font::bold)));
    g.setColour(juce::Colours::white.withAlpha(active ? 1.0f : 0.8f));
    g.drawText(labelText, bounds, juce::Justification::centred);
}

void IconButton::drawStereoIcon(juce::Graphics& g, juce::Rectangle<float> area, bool)
{
    const float radius = std::min(area.getWidth(), area.getHeight()) * 0.22f;
    drawCircleOutline(g, { area.getX() + area.getWidth() * 0.25f, area.getCentreY() }, radius, 2.0f);
    drawCircleOutline(g, { area.getX() + area.getWidth() * 0.75f, area.getCentreY() }, radius, 2.0f);
}

void IconButton::drawFlipIcon(juce::Graphics& g, juce::Rectangle<float> area, bool)
{
    const float x0 = area.getX() + area.getWidth() * 0.15f;
    const float x1 = area.getX() + area.getWidth() * 0.85f;
    const float y0 = area.getY() + area.getHeight() * 0.25f;
    const float y1 = area.getY() + area.getHeight() * 0.75f;

    drawArrow(g, { x0, y0 }, { x1, y1 }, 6.0f, 2.0f);
    drawArrow(g, { x1, y0 }, { x0, y1 }, 6.0f, 2.0f);
}

void IconButton::drawSumIcon(juce::Graphics& g, juce::Rectangle<float> area, bool)
{
    const float radius = std::min(area.getWidth(), area.getHeight()) * 0.18f;
    drawCircleOutline(g, area.getCentre(), radius, 2.0f);

    const float cy = area.getCentreY();
    const float cx = area.getCentreX();
    const float xL = area.getX() + area.getWidth() * 0.12f;
    const float xR = area.getX() + area.getWidth() * 0.88f;

    drawArrow(g, { xL, cy }, { cx - radius, cy }, 5.0f, 2.0f);
    drawArrow(g, { xR, cy }, { cx + radius, cy }, 5.0f, 2.0f);
}

void IconButton::drawLeftIcon(juce::Graphics& g, juce::Rectangle<float> area, bool)
{
    const float radius = std::min(area.getWidth(), area.getHeight()) * 0.25f;
    const float cx = area.getX() + area.getWidth() * 0.35f;
    const float cy = area.getCentreY();
    drawCircleOutline(g, { cx, cy }, radius, 2.0f);

    g.drawLine(cx + radius, cy, area.getX() + area.getWidth() * 0.85f, cy, 2.0f);
}

void IconButton::drawRightIcon(juce::Graphics& g, juce::Rectangle<float> area, bool)
{
    const float radius = std::min(area.getWidth(), area.getHeight()) * 0.25f;
    const float cx = area.getX() + area.getWidth() * 0.65f;
    const float cy = area.getCentreY();
    drawCircleOutline(g, { cx, cy }, radius, 2.0f);

    g.drawLine(cx - radius, cy, area.getX() + area.getWidth() * 0.15f, cy, 2.0f);
}

void IconButton::drawMidSideIcon(juce::Graphics& g, juce::Rectangle<float> area, bool)
{
    const float midRadius = std::min(area.getWidth(), area.getHeight()) * 0.18f;
    const float sideRadius = std::min(area.getWidth(), area.getHeight()) * 0.12f;

    drawCircleOutline(g, area.getCentre(), midRadius, 2.0f);
    drawCircleOutline(g, { area.getX() + area.getWidth() * 0.2f, area.getCentreY() }, sideRadius, 2.0f);
    drawCircleOutline(g, { area.getX() + area.getWidth() * 0.8f, area.getCentreY() }, sideRadius, 2.0f);
}

void IconButton::drawSoloMidIcon(juce::Graphics& g, juce::Rectangle<float> area, bool)
{
    const float radius = std::min(area.getWidth(), area.getHeight()) * 0.22f;
    drawCircleOutline(g, area.getCentre(), radius, 2.0f);
}

void IconButton::drawSoloSideIcon(juce::Graphics& g, juce::Rectangle<float> area, bool)
{
    const float radius = std::min(area.getWidth(), area.getHeight()) * 0.18f;
    drawCircleOutline(g, { area.getX() + area.getWidth() * 0.25f, area.getCentreY() }, radius, 2.0f);
    drawCircleOutline(g, { area.getX() + area.getWidth() * 0.75f, area.getCentreY() }, radius, 2.0f);
}

void IconButton::drawLinkIcon(juce::Graphics& g, juce::Rectangle<float> area, bool)
{
    juce::Path link1, link2;
    const float w = area.getWidth() * 0.55f;
    const float h = area.getHeight() * 0.22f;
    link1.addRoundedRectangle(-w * 0.5f, -h * 0.5f, w, h, h * 0.5f);
    link2.addRoundedRectangle(-w * 0.5f, -h * 0.5f, w, h, h * 0.5f);

    const auto t1 = juce::AffineTransform::rotation(juce::MathConstants<float>::pi * 0.25f)
                        .translated(area.getCentreX() - area.getWidth() * 0.04f, area.getCentreY());
    const auto t2 = juce::AffineTransform::rotation(-juce::MathConstants<float>::pi * 0.25f)
                        .translated(area.getCentreX() + area.getWidth() * 0.04f, area.getCentreY());

    g.strokePath(link1, juce::PathStrokeType(2.0f, juce::PathStrokeType::mitered, juce::PathStrokeType::rounded), t1);
    g.strokePath(link2, juce::PathStrokeType(2.0f, juce::PathStrokeType::mitered, juce::PathStrokeType::rounded), t2);
}

PeakMeter::PeakMeter()
{
    startTimerHz(30);
}

void PeakMeter::setLevel(float linearAmplitude)
{
    currentLevel = linearAmplitude;
    if (currentLevel > peakHold)
        peakHold = currentLevel;
    repaint();
}

void PeakMeter::paint(juce::Graphics& g)
{
    auto fullArea = getLocalBounds().toFloat();

    g.setColour(juce::Colour(0xff2a2a36));
    g.fillRect(fullArea);

    const float dB = juce::Decibels::gainToDecibels(currentLevel, -60.0f);
    const float norm = juce::jlimit(0.0f, 1.0f, (dB + 60.0f) / 66.0f);
    const float barHeight = fullArea.getHeight() * norm;

    const juce::ColourGradient grad(juce::Colour(0xff66bb6a), 0.0f, fullArea.getBottom(),
                                      juce::Colour(0xffff5722), 0.0f, fullArea.getY(), false);
    g.setGradientFill(grad);
    g.fillRect(fullArea.removeFromBottom(barHeight));

    const float holddB = juce::Decibels::gainToDecibels(peakHold, -60.0f);
    const float holdNorm = juce::jlimit(0.0f, 1.0f, (holddB + 60.0f) / 66.0f);
    const float holdY = fullArea.getBottom() - fullArea.getHeight() * holdNorm;

    g.setColour(juce::Colours::white);
    g.drawLine(fullArea.getX(), holdY, fullArea.getRight(), holdY, 2.0f);

    g.setColour(juce::Colour(0x40ffffff));
    g.drawRect(fullArea, 1.0f);
}

void PeakMeter::timerCallback()
{
    peakHold *= 0.95f;
    if (peakHold < currentLevel)
        peakHold = currentLevel;
    repaint();
}

PhaseMeter::PhaseMeter()
{
}

void PhaseMeter::setCorrelation(float value)
{
    correlation = juce::jlimit(-1.0f, 1.0f, value);
    repaint();
}

void PhaseMeter::paint(juce::Graphics& g)
{
    auto fullArea = getLocalBounds().toFloat();

    g.setColour(juce::Colour(0xff2a2a36));
    g.fillRect(fullArea);

    const float size = std::min(fullArea.getWidth(), fullArea.getHeight());
    const auto dial = fullArea.withSizeKeepingCentre(size, size);

    const float cx = dial.getCentreX();
    const float cy = dial.getCentreY();
    const float radius = size * 0.42f;

    juce::Path arc;
    arc.addCentredArc(cx, cy, radius, radius, 0.0f, 0.0f, juce::MathConstants<float>::pi, true);
    g.setColour(juce::Colour(0x40ffffff));
    g.strokePath(arc, juce::PathStrokeType(1.0f, juce::PathStrokeType::mitered, juce::PathStrokeType::rounded));

    const float norm = (1.0f - correlation) * 0.5f;
    const float angle = norm * juce::MathConstants<float>::pi;
    const float x = cx + radius * std::cos(angle);
    const float y = cy + radius * std::sin(angle);

    const juce::Colour c = correlation > 0.0f ? juce::Colour(0xff66bb6a) : juce::Colour(0xffff5722);
    g.setColour(c);
    g.drawLine(cx, cy, x, y, 2.5f);

    g.setColour(juce::Colours::white);
    g.fillEllipse(cx - 3.0f, cy - 3.0f, 6.0f, 6.0f);

    g.setColour(juce::Colour(0x40ffffff));
    g.drawRect(fullArea, 1.0f);
}

Scope::Scope(EPStereoFixerAudioProcessor& p)
    : processor(p)
{
    setTooltip("Stereo goniometer: X = left, Y = right");
}

void Scope::paint(juce::Graphics& g)
{
    auto fullArea = getLocalBounds().toFloat();

    g.setColour(juce::Colour(0xff2a2a36));
    g.fillRect(fullArea);

    const float cx = fullArea.getCentreX();
    const float cy = fullArea.getCentreY();
    const float scale = std::min(fullArea.getWidth(), fullArea.getHeight()) * 0.45f;

    g.setColour(juce::Colour(0x40ffffff));
    g.drawLine(fullArea.getX(), cy, fullArea.getRight(), cy, 1.0f);
    g.drawLine(cx, fullArea.getY(), cx, fullArea.getBottom(), 1.0f);
    g.drawEllipse(cx - scale, cy - scale, scale * 2.0f, scale * 2.0f, 1.0f);

    const int writeIdx = processor.getScopeWriteIndex();
    const float* l = processor.getScopeL();
    const float* r = processor.getScopeR();

    juce::Path trace;
    bool first = true;

    for (int i = 0; i < processor.scopeSize; ++i)
    {
        const int idx = (writeIdx + i) % processor.scopeSize;
        const float x = cx + juce::jlimit(-scale, scale, l[idx] * scale);
        const float y = cy - juce::jlimit(-scale, scale, r[idx] * scale);

        if (first)
        {
            trace.startNewSubPath(x, y);
            first = false;
        }
        else
        {
            trace.lineTo(x, y);
        }
    }

    g.setColour(juce::Colour(0xff66bb6a));
    g.strokePath(trace, juce::PathStrokeType(1.5f, juce::PathStrokeType::mitered, juce::PathStrokeType::rounded));

    g.setColour(juce::Colour(0x40ffffff));
    g.drawRect(fullArea, 1.0f);
}

EPStereoFixerAudioProcessorEditor::EPStereoFixerAudioProcessorEditor(EPStereoFixerAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p), scope(audioProcessor)
{
    formatLabel.setText("Format", juce::dontSendNotification);
    formatLabel.setJustificationType(juce::Justification::centred);
    formatLabel.setFont(juce::Font(juce::FontOptions(16.0f, juce::Font::bold)));
    formatLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible(formatLabel);

    addAndMakeVisible(stereoButton);
    addAndMakeVisible(flipButton);
    addAndMakeVisible(sumButton);
    addAndMakeVisible(leftButton);
    addAndMakeVisible(rightButton);
    addAndMakeVisible(midSideButton);
    addAndMakeVisible(soloMidButton);
    addAndMakeVisible(soloSideButton);

    stereoButton.onClick = [this] { setFormat(0); };
    flipButton.onClick = [this] { setFormat(1); };
    sumButton.onClick = [this] { setFormat(2); };
    leftButton.onClick = [this] { setFormat(3); };
    rightButton.onClick = [this] { setFormat(4); };
    midSideButton.onClick = [this] { setFormat(5); };
    soloMidButton.onClick = [this] { setFormat(6); };
    soloSideButton.onClick = [this] { setFormat(7); };

    setupUtilityButton(invertLeftButton, "Invert the phase of the left channel");
    setupUtilityButton(invertRightButton, "Invert the phase of the right channel");
    setupUtilityButton(autoGainButton, "Auto Gain: keep output level equal to input level");
    setupUtilityButton(bypassButton, "Bypass all processing");

    invertLeftButton.onClick = [this]  { setInvertLeftState(invertLeftButton.getToggleState()); };
    invertRightButton.onClick = [this] { setInvertRightState(invertRightButton.getToggleState()); };
    autoGainButton.onClick = [this]    { setAutoGainState(autoGainButton.getToggleState()); };
    bypassButton.onClick = [this]     { setBypassState(bypassButton.getToggleState()); };

    setupGainSlider(inputGainSlider, inputGainLabel, "Input Gain");
    setupGainSlider(widthSlider, widthLabel, "Width");
    setupGainSlider(gainLeftSlider, gainLeftLabel, "Gain Left");
    setupGainSlider(gainRightSlider, gainRightLabel, "Gain Right");

    inputGainSlider.setTooltip("Adjust the input level before format processing");
    widthSlider.setTooltip("Adjust stereo width using Mid/Side encoding");
    gainLeftSlider.setTooltip("Adjust left output gain; becomes Mid gain in Mid/Side mode");
    gainRightSlider.setTooltip("Adjust right output gain; becomes Side gain in Mid/Side mode");

    gainLeftSlider.onValueChange = [this]
    {
        if (linkButton.getToggleState() && !updatingLink)
            syncLinkedGainFromLeft();
    };

    gainRightSlider.onValueChange = [this]
    {
        if (linkButton.getToggleState() && !updatingLink)
            syncLinkedGainFromRight();
    };

    inputGainAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getAPVTS(), "inputGain", inputGainSlider);
    widthAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getAPVTS(), "width", widthSlider);
    gainLeftAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getAPVTS(), "gainLeft", gainLeftSlider);
    gainRightAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getAPVTS(), "gainRight", gainRightSlider);

    linkButton.setClickingTogglesState(true);
    linkButton.onClick = [this] { setLinkState(linkButton.getToggleState()); };
    addAndMakeVisible(linkButton);

    addAndMakeVisible(leftMeter);
    addAndMakeVisible(rightMeter);
    addAndMakeVisible(phaseMeter);
    addAndMakeVisible(scope);

    leftMeter.setTooltip("Output peak level for the left channel");
    rightMeter.setTooltip("Output peak level for the right channel");
    phaseMeter.setTooltip("Phase correlation: left = mono, right = out-of-phase");

    setupMeterDbLabel(leftDbLabel);
    setupMeterDbLabel(rightDbLabel);

    metersLabel.setText("Meters", juce::dontSendNotification);
    metersLabel.setJustificationType(juce::Justification::centred);
    metersLabel.setFont(juce::Font(juce::FontOptions(16.0f, juce::Font::bold)));
    metersLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible(metersLabel);

    outputGainLabel.setText("Output Gain", juce::dontSendNotification);
    outputGainLabel.setJustificationType(juce::Justification::centred);
    outputGainLabel.setFont(juce::Font(juce::FontOptions(16.0f, juce::Font::bold)));
    outputGainLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible(outputGainLabel);

    auto* choice = dynamic_cast<juce::AudioParameterChoice*>(audioProcessor.getAPVTS().getParameter("format"));
    updateFormat(choice != nullptr ? choice->getIndex() : 0);

    auto* gainLinkParam = dynamic_cast<juce::AudioParameterBool*>(audioProcessor.getAPVTS().getParameter("gainLink"));
    linkButton.setToggleState(gainLinkParam != nullptr && *gainLinkParam, juce::dontSendNotification);

    auto* autoGainParam = dynamic_cast<juce::AudioParameterBool*>(audioProcessor.getAPVTS().getParameter("autoGain"));
    autoGainButton.setToggleState(autoGainParam != nullptr && *autoGainParam, juce::dontSendNotification);

    auto* invertLeftParam = dynamic_cast<juce::AudioParameterBool*>(audioProcessor.getAPVTS().getParameter("invertLeft"));
    invertLeftButton.setToggleState(invertLeftParam != nullptr && *invertLeftParam, juce::dontSendNotification);

    auto* invertRightParam = dynamic_cast<juce::AudioParameterBool*>(audioProcessor.getAPVTS().getParameter("invertRight"));
    invertRightButton.setToggleState(invertRightParam != nullptr && *invertRightParam, juce::dontSendNotification);

    auto* bypassParam = dynamic_cast<juce::AudioParameterBool*>(audioProcessor.getAPVTS().getParameter("bypass"));
    bypassButton.setToggleState(bypassParam != nullptr && *bypassParam, juce::dontSendNotification);

    startTimerHz(30);
    setSize(680, 620);

    setResizable(true, true);
    setResizeLimits(680, 620, 1360, 1240);
}

void EPStereoFixerAudioProcessorEditor::setupGainSlider(juce::Slider& slider, juce::Label& label, const juce::String& name)
{
    slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    slider.setTextValueSuffix(name == "Width" ? " %" : " dB");
    slider.setNumDecimalPlacesToDisplay(1);
    addAndMakeVisible(slider);

    label.setText(name, juce::dontSendNotification);
    label.attachToComponent(&slider, false);
    label.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(label);
}

void EPStereoFixerAudioProcessorEditor::setupUtilityButton(juce::TextButton& button, const juce::String& tooltip)
{
    button.setClickingTogglesState(true);
    button.setTooltip(tooltip);
    button.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff262632));
    button.setColour(juce::TextButton::buttonOnColourId, juce::Colour(0xff5a5a6e));
    button.setColour(juce::TextButton::textColourOnId, juce::Colours::white);
    button.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
    addAndMakeVisible(button);
}

void EPStereoFixerAudioProcessorEditor::setupMeterDbLabel(juce::Label& label)
{
    label.setText("-inf", juce::dontSendNotification);
    label.setJustificationType(juce::Justification::centred);
    label.setFont(juce::Font(juce::FontOptions(10.0f, juce::Font::plain)));
    label.setColour(juce::Label::textColourId, juce::Colours::white.withAlpha(0.7f));
    addAndMakeVisible(label);
}

void EPStereoFixerAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff1e1e24));
}

void EPStereoFixerAudioProcessorEditor::resized()
{
    auto area = getLocalBounds().reduced(16);

    formatLabel.setBounds(area.removeFromTop(24));

    auto formatRow = area.removeFromTop(64);
    const int formatButtonWidth = formatRow.getWidth() / 8;
    stereoButton.setBounds(formatRow.removeFromLeft(formatButtonWidth).reduced(2));
    flipButton.setBounds(formatRow.removeFromLeft(formatButtonWidth).reduced(2));
    sumButton.setBounds(formatRow.removeFromLeft(formatButtonWidth).reduced(2));
    leftButton.setBounds(formatRow.removeFromLeft(formatButtonWidth).reduced(2));
    rightButton.setBounds(formatRow.removeFromLeft(formatButtonWidth).reduced(2));
    midSideButton.setBounds(formatRow.removeFromLeft(formatButtonWidth).reduced(2));
    soloMidButton.setBounds(formatRow.removeFromLeft(formatButtonWidth).reduced(2));
    soloSideButton.setBounds(formatRow.removeFromLeft(formatButtonWidth).reduced(2));

    area.removeFromTop(8);

    auto utilityRow = area.removeFromTop(36);
    const int utilityWidth = utilityRow.getWidth() / 4;
    invertLeftButton.setBounds(utilityRow.removeFromLeft(utilityWidth).reduced(4));
    invertRightButton.setBounds(utilityRow.removeFromLeft(utilityWidth).reduced(4));
    autoGainButton.setBounds(utilityRow.removeFromLeft(utilityWidth).reduced(4));
    bypassButton.setBounds(utilityRow.removeFromLeft(utilityWidth).reduced(4));

    area.removeFromTop(8);

    auto ioRow = area.removeFromTop(120);
    inputGainSlider.setBounds(ioRow.removeFromLeft(ioRow.getWidth() / 2).reduced(8));
    widthSlider.setBounds(ioRow.reduced(8));

    area.removeFromTop(8);
    outputGainLabel.setBounds(area.removeFromTop(24));

    auto gainRow = area.removeFromTop(120);
    const int totalW = gainRow.getWidth();
    const int leftW = static_cast<int>(totalW * 0.40f);
    const int linkW = static_cast<int>(totalW * 0.10f);
    const int rightW = static_cast<int>(totalW * 0.40f);
    const int autoW = totalW - leftW - linkW - rightW;

    gainLeftSlider.setBounds(gainRow.removeFromLeft(leftW).reduced(8));
    linkButton.setBounds(gainRow.removeFromLeft(linkW).reduced(4));
    gainRightSlider.setBounds(gainRow.removeFromLeft(rightW).reduced(8));
    autoGainButton.setBounds(gainRow.reduced(4));

    area.removeFromTop(8);
    metersLabel.setBounds(area.removeFromTop(24));

    auto meterArea = area.removeFromTop(120);
    auto dbLabelRow = meterArea.removeFromTop(20);
    auto meterRow = meterArea;

    leftDbLabel.setBounds(dbLabelRow.removeFromLeft(60));
    rightDbLabel.setBounds(dbLabelRow.removeFromLeft(60));

    leftMeter.setBounds(meterRow.removeFromLeft(60).reduced(0, 2));
    rightMeter.setBounds(meterRow.removeFromLeft(60).reduced(0, 2));

    const int phaseSize = meterRow.getHeight();
    phaseMeter.setBounds(meterRow.removeFromLeft(phaseSize).reduced(4));
    scope.setBounds(meterRow.reduced(4));
}

void EPStereoFixerAudioProcessorEditor::setFormat(int index)
{
    auto* choice = dynamic_cast<juce::AudioParameterChoice*>(audioProcessor.getAPVTS().getParameter("format"));
    if (choice != nullptr)
        *choice = index;

    updateFormat(index);
}

void EPStereoFixerAudioProcessorEditor::updateFormat(int index)
{
    currentFormat = index;

    stereoButton.setToggleState(index == 0, juce::dontSendNotification);
    flipButton.setToggleState(index == 1, juce::dontSendNotification);
    sumButton.setToggleState(index == 2, juce::dontSendNotification);
    leftButton.setToggleState(index == 3, juce::dontSendNotification);
    rightButton.setToggleState(index == 4, juce::dontSendNotification);
    midSideButton.setToggleState(index == 5, juce::dontSendNotification);
    soloMidButton.setToggleState(index == 6, juce::dontSendNotification);
    soloSideButton.setToggleState(index == 7, juce::dontSendNotification);

    if (isMonoFormat(index))
    {
        const bool any = invertLeftButton.getToggleState() || invertRightButton.getToggleState();

        auto* leftParam = dynamic_cast<juce::AudioParameterBool*>(audioProcessor.getAPVTS().getParameter("invertLeft"));
        if (leftParam != nullptr)
            *leftParam = any;

        auto* rightParam = dynamic_cast<juce::AudioParameterBool*>(audioProcessor.getAPVTS().getParameter("invertRight"));
        if (rightParam != nullptr)
            *rightParam = any;

        invertLeftButton.setToggleState(any, juce::dontSendNotification);
        invertRightButton.setToggleState(any, juce::dontSendNotification);
    }

    updateGainLabels();
}

void EPStereoFixerAudioProcessorEditor::updateGainLabels()
{
    if (midSideButton.getToggleState())
    {
        gainLeftLabel.setText("Mid", juce::dontSendNotification);
        gainRightLabel.setText("Side", juce::dontSendNotification);
    }
    else if (soloMidButton.getToggleState() || soloSideButton.getToggleState())
    {
        gainLeftLabel.setText("Gain", juce::dontSendNotification);
        gainRightLabel.setText("Gain", juce::dontSendNotification);
    }
    else
    {
        gainLeftLabel.setText("Gain Left", juce::dontSendNotification);
        gainRightLabel.setText("Gain Right", juce::dontSendNotification);
    }
}

void EPStereoFixerAudioProcessorEditor::setLinkState(bool active)
{
    auto* param = dynamic_cast<juce::AudioParameterBool*>(audioProcessor.getAPVTS().getParameter("gainLink"));
    if (param != nullptr)
        *param = active;

    if (active)
        syncLinkedGainFromLeft();
}

void EPStereoFixerAudioProcessorEditor::setAutoGainState(bool active)
{
    auto* param = dynamic_cast<juce::AudioParameterBool*>(audioProcessor.getAPVTS().getParameter("autoGain"));
    if (param != nullptr)
        *param = active;
}

void EPStereoFixerAudioProcessorEditor::setInvertLeftState(bool active)
{
    auto* leftParam = dynamic_cast<juce::AudioParameterBool*>(audioProcessor.getAPVTS().getParameter("invertLeft"));
    if (leftParam != nullptr)
        *leftParam = active;

    invertLeftButton.setToggleState(active, juce::dontSendNotification);

    if (isMonoFormat(currentFormat))
    {
        auto* rightParam = dynamic_cast<juce::AudioParameterBool*>(audioProcessor.getAPVTS().getParameter("invertRight"));
        if (rightParam != nullptr)
            *rightParam = active;
        invertRightButton.setToggleState(active, juce::dontSendNotification);
    }
}

void EPStereoFixerAudioProcessorEditor::setInvertRightState(bool active)
{
    auto* rightParam = dynamic_cast<juce::AudioParameterBool*>(audioProcessor.getAPVTS().getParameter("invertRight"));
    if (rightParam != nullptr)
        *rightParam = active;

    invertRightButton.setToggleState(active, juce::dontSendNotification);

    if (isMonoFormat(currentFormat))
    {
        auto* leftParam = dynamic_cast<juce::AudioParameterBool*>(audioProcessor.getAPVTS().getParameter("invertLeft"));
        if (leftParam != nullptr)
            *leftParam = active;
        invertLeftButton.setToggleState(active, juce::dontSendNotification);
    }
}

bool EPStereoFixerAudioProcessorEditor::isMonoFormat(int index) const
{
    return index == 2 || index == 3 || index == 4 || index == 6;
}

void EPStereoFixerAudioProcessorEditor::setBypassState(bool active)
{
    auto* param = dynamic_cast<juce::AudioParameterBool*>(audioProcessor.getAPVTS().getParameter("bypass"));
    if (param != nullptr)
        *param = active;
}

void EPStereoFixerAudioProcessorEditor::syncLinkedGainFromLeft()
{
    updatingLink = true;
    const float value = gainLeftSlider.getValue();
    gainRightSlider.setValue(value, juce::dontSendNotification);
    auto* rightParam = audioProcessor.getAPVTS().getParameter("gainRight");
    if (rightParam != nullptr)
        rightParam->setValueNotifyingHost(rightParam->convertTo0to1(value));
    updatingLink = false;
}

void EPStereoFixerAudioProcessorEditor::syncLinkedGainFromRight()
{
    updatingLink = true;
    const float value = gainRightSlider.getValue();
    gainLeftSlider.setValue(value, juce::dontSendNotification);
    auto* leftParam = audioProcessor.getAPVTS().getParameter("gainLeft");
    if (leftParam != nullptr)
        leftParam->setValueNotifyingHost(leftParam->convertTo0to1(value));
    updatingLink = false;
}

void EPStereoFixerAudioProcessorEditor::timerCallback()
{
    auto* choice = dynamic_cast<juce::AudioParameterChoice*>(audioProcessor.getAPVTS().getParameter("format"));
    if (choice != nullptr)
        updateFormat(choice->getIndex());

    auto* gainLinkParam = dynamic_cast<juce::AudioParameterBool*>(audioProcessor.getAPVTS().getParameter("gainLink"));
    if (gainLinkParam != nullptr)
        linkButton.setToggleState(*gainLinkParam, juce::dontSendNotification);

    auto* autoGainParam = dynamic_cast<juce::AudioParameterBool*>(audioProcessor.getAPVTS().getParameter("autoGain"));
    if (autoGainParam != nullptr)
        autoGainButton.setToggleState(*autoGainParam, juce::dontSendNotification);

    auto* invertLeftParam = dynamic_cast<juce::AudioParameterBool*>(audioProcessor.getAPVTS().getParameter("invertLeft"));
    if (invertLeftParam != nullptr)
        invertLeftButton.setToggleState(*invertLeftParam, juce::dontSendNotification);

    auto* invertRightParam = dynamic_cast<juce::AudioParameterBool*>(audioProcessor.getAPVTS().getParameter("invertRight"));
    if (invertRightParam != nullptr)
        invertRightButton.setToggleState(*invertRightParam, juce::dontSendNotification);

    auto* bypassParam = dynamic_cast<juce::AudioParameterBool*>(audioProcessor.getAPVTS().getParameter("bypass"));
    if (bypassParam != nullptr)
        bypassButton.setToggleState(*bypassParam, juce::dontSendNotification);

    const float leftLevel = audioProcessor.getLeftMeter();
    const float rightLevel = audioProcessor.getRightMeter();

    leftMeter.setLevel(leftLevel);
    rightMeter.setLevel(rightLevel);
    phaseMeter.setCorrelation(audioProcessor.getPhaseMeter());
    scope.repaint();

    leftDbLabel.setText(juce::Decibels::gainToDecibels(leftLevel, -60.0f) > -59.0f
                          ? juce::String(juce::Decibels::gainToDecibels(leftLevel), 1) + " dB"
                          : "-inf",
                          juce::dontSendNotification);

    rightDbLabel.setText(juce::Decibels::gainToDecibels(rightLevel, -60.0f) > -59.0f
                           ? juce::String(juce::Decibels::gainToDecibels(rightLevel), 1) + " dB"
                           : "-inf",
                           juce::dontSendNotification);
}
