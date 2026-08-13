#include "PluginEditor.h"
#include <cmath>
#include "BinaryData.h"

EPLookAndFeel::EPLookAndFeel()
{
    setColour(juce::Slider::textBoxTextColourId, Colours::textBright);
    setColour(juce::Slider::textBoxBackgroundColourId, juce::Colour(0x00000000));
    setColour(juce::Slider::textBoxOutlineColourId, juce::Colour(0x00000000));
    setColour(juce::Label::textColourId, Colours::textDim);
}

void EPLookAndFeel::drawRotarySlider(juce::Graphics& g, int x, int y, int w, int h,
                                      float sliderPos, float startAngle, float endAngle,
                                      juce::Slider&)
{
    const auto bounds = juce::Rectangle<float>(static_cast<float>(x), static_cast<float>(y),
                                                static_cast<float>(w), static_cast<float>(h));
    const float radius = std::min(bounds.getWidth(), bounds.getHeight()) * 0.38f;
    const float cx = bounds.getCentreX();
    const float cy = bounds.getCentreY();
    const float lineW = 3.0f;

    juce::Path bgArc;
    bgArc.addCentredArc(cx, cy, radius, radius, 0.0f, startAngle, endAngle, true);
    g.setColour(juce::Colour(0xff2a2a3a));
    g.strokePath(bgArc, juce::PathStrokeType(lineW + 2.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    const float angle = startAngle + sliderPos * (endAngle - startAngle);
    juce::Path valueArc;
    valueArc.addCentredArc(cx, cy, radius, radius, 0.0f, startAngle, angle, true);
    g.setColour(Colours::accent);
    g.strokePath(valueArc, juce::PathStrokeType(lineW, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    g.setColour(Colours::accent.withAlpha(0.15f));
    g.strokePath(valueArc, juce::PathStrokeType(lineW + 6.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    const float thumbR = 5.0f;
    const float tx = cx + radius * std::cos(angle - juce::MathConstants<float>::halfPi);
    const float ty = cy + radius * std::sin(angle - juce::MathConstants<float>::halfPi);
    g.setColour(Colours::textBright);
    g.fillEllipse(tx - thumbR, ty - thumbR, thumbR * 2.0f, thumbR * 2.0f);

    g.setColour(Colours::accent.withAlpha(0.4f));
    g.fillEllipse(tx - thumbR - 2.0f, ty - thumbR - 2.0f, (thumbR + 2.0f) * 2.0f, (thumbR + 2.0f) * 2.0f);
}

void EPLookAndFeel::drawButtonBackground(juce::Graphics& g, juce::Button& button,
                                          const juce::Colour&, bool isMouseOver, bool isButtonDown)
{
    auto bounds = button.getLocalBounds().toFloat().reduced(0.5f);
    const bool on = button.getToggleState();

    juce::Colour bg = on ? Colours::accent.withAlpha(0.25f)
                         : (isMouseOver ? Colours::btnHover : Colours::btnOff);

    if (isButtonDown)
        bg = bg.brighter(0.1f);

    g.setColour(bg);
    g.fillRoundedRectangle(bounds, 5.0f);

    g.setColour(on ? Colours::accent.withAlpha(0.6f) : Colours::panelBorder);
    g.drawRoundedRectangle(bounds, 5.0f, 1.0f);
}

void EPLookAndFeel::drawButtonText(juce::Graphics& g, juce::TextButton& button,
                                    bool, bool)
{
    g.setFont(juce::Font(juce::FontOptions(12.0f, juce::Font::bold)));
    g.setColour(button.getToggleState() ? Colours::accent : Colours::textBright);
    g.drawText(button.getButtonText(), button.getLocalBounds(), juce::Justification::centred);
}

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

    const juce::Colour bg = active ? Colours::accent.withAlpha(0.2f)
                                   : (isMouseOver ? Colours::btnHover : Colours::btnOff);
    g.setColour(bg);
    g.fillRoundedRectangle(bounds, 6.0f);

    g.setColour(active ? Colours::accent.withAlpha(0.5f) : Colours::panelBorder);
    g.drawRoundedRectangle(bounds, 6.0f, 1.0f);

    auto iconArea = bounds.removeFromTop(bounds.getHeight() * 0.55f).reduced(8.0f);
    g.setColour(active ? Colours::accent : Colours::textDim);

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
    g.setColour(active ? Colours::accent : Colours::textDim);
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
    drawArrow(g, { area.getX() + area.getWidth() * 0.12f, cy }, { cx - radius, cy }, 5.0f, 2.0f);
    drawArrow(g, { area.getX() + area.getWidth() * 0.88f, cy }, { cx + radius, cy }, 5.0f, 2.0f);
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
    drawCircleOutline(g, area.getCentre(), std::min(area.getWidth(), area.getHeight()) * 0.22f, 2.0f);
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

PeakMeter::PeakMeter() { startTimerHz(30); }

void PeakMeter::setLevel(float linearAmplitude)
{
    currentLevel = linearAmplitude;
    if (currentLevel > peakHold) peakHold = currentLevel;
    repaint();
}

void PeakMeter::paint(juce::Graphics& g)
{
    auto fullArea = getLocalBounds().toFloat();
    g.setColour(juce::Colour(0xff1a1a24));
    g.fillRoundedRectangle(fullArea, 3.0f);

    const float dB = juce::Decibels::gainToDecibels(currentLevel, -60.0f);
    const float norm = juce::jlimit(0.0f, 1.0f, (dB + 60.0f) / 66.0f);
    const float barH = fullArea.getHeight() * norm;

    auto barArea = fullArea.reduced(2.0f);
    auto filledArea = barArea.removeFromBottom(barArea.getHeight() * norm);

    juce::ColourGradient grad(Colours::meterGreen, 0.0f, barArea.getBottom(),
                               Colours::meterRed, 0.0f, barArea.getY(), false);
    grad.addColour(0.7, Colours::meterYellow);
    g.setGradientFill(grad);
    g.fillRoundedRectangle(filledArea, 2.0f);

    g.setColour(Colours::meterGreen.withAlpha(0.08f));
    g.fillRoundedRectangle(filledArea, 2.0f);

    const float holddB = juce::Decibels::gainToDecibels(peakHold, -60.0f);
    const float holdNorm = juce::jlimit(0.0f, 1.0f, (holddB + 60.0f) / 66.0f);
    const float holdY = fullArea.getBottom() - fullArea.getHeight() * holdNorm;
    g.setColour(Colours::textBright);
    g.drawLine(fullArea.getX() + 2.0f, holdY, fullArea.getRight() - 2.0f, holdY, 1.5f);

    g.setColour(Colours::panelBorder);
    g.drawRoundedRectangle(fullArea, 3.0f, 1.0f);
}

void PeakMeter::timerCallback()
{
    peakHold *= 0.95f;
    if (peakHold < currentLevel) peakHold = currentLevel;
    repaint();
}

PhaseMeter::PhaseMeter() {}

void PhaseMeter::setCorrelation(float value) { correlation = juce::jlimit(-1.0f, 1.0f, value); repaint(); }

void PhaseMeter::paint(juce::Graphics& g)
{
    auto fullArea = getLocalBounds().toFloat();
    g.setColour(juce::Colour(0xff1a1a24));
    g.fillRoundedRectangle(fullArea, 3.0f);

    const float cx = fullArea.getCentreX();
    g.setColour(juce::Colour(0x30ffffff));
    g.drawLine(cx, fullArea.getY() + 4.0f, cx, fullArea.getBottom() - 4.0f, 1.0f);

    const float norm = (1.0f - correlation) * 0.5f;
    const float x = fullArea.getX() + 4.0f + norm * (fullArea.getWidth() - 8.0f);

    const juce::Colour c = correlation > 0.0f ? Colours::meterGreen : Colours::meterRed;
    g.setColour(c);
    g.fillRoundedRectangle(x - 3.0f, fullArea.getY() + 4.0f, 6.0f, fullArea.getHeight() - 8.0f, 2.0f);

    g.setColour(c.withAlpha(0.15f));
    g.fillRoundedRectangle(x - 6.0f, fullArea.getY() + 4.0f, 12.0f, fullArea.getHeight() - 8.0f, 3.0f);

    g.setColour(Colours::panelBorder);
    g.drawRoundedRectangle(fullArea, 3.0f, 1.0f);
}

Correlometer::Correlometer(EPStereoFixerAudioProcessor& p) : processor(p)
{
    setTooltip("Frequency correlation: green = correlated, red = out-of-phase");
}

void Correlometer::update()
{
    for (int i = 0; i < EPStereoFixerAudioProcessor::numCorrBands; ++i)
        values[i] = processor.getBandCorrelation(i);
    repaint();
}

void Correlometer::paint(juce::Graphics& g)
{
    auto area = getLocalBounds().toFloat();
    g.setColour(juce::Colour(0xff1a1a24));
    g.fillRoundedRectangle(area, 3.0f);

    const float pad = 4.0f;
    const float plotL = area.getX() + pad;
    const float plotR = area.getRight() - pad;
    const float plotT = area.getY() + pad;
    const float plotB = area.getBottom() - 14.0f;
    const float plotW = plotR - plotL;
    const float plotH = plotB - plotT;
    const float midY = plotT + plotH * 0.5f;

    g.setColour(juce::Colour(0x18ffffff));
    g.drawLine(plotL, midY, plotR, midY, 0.5f);

    g.setFont(juce::Font(juce::FontOptions(8.0f, juce::Font::plain)));
    g.setColour(Colours::textDim.withAlpha(0.4f));
    g.drawText("+1", juce::Rectangle<float>(plotR + 2.0f, plotT - 4.0f, 16.0f, 10.0f), juce::Justification::centredLeft);
    g.drawText("-1", juce::Rectangle<float>(plotR + 2.0f, plotB - 6.0f, 16.0f, 10.0f), juce::Justification::centredLeft);

    const int n = EPStereoFixerAudioProcessor::numCorrBands;
    const float barW = plotW / static_cast<float>(n);

    const char* freqLabels[12] = { "40", "80", "160", "315", "630", "1.2k", "2.5k", "4k", "6.3k", "8k", "12k", "16k" };

    for (int i = 0; i < n; ++i)
    {
        const float x = plotL + static_cast<float>(i) * barW;
        const float corr = values[i];
        const float h = std::abs(corr) * plotH * 0.5f;
        const bool hovered = (i == hoveredBand);

        juce::Colour col;
        if (corr >= 0.0f)
            col = Colours::meterGreen.withAlpha(0.5f + corr * 0.5f);
        else
            col = Colours::meterRed.withAlpha(0.5f + std::abs(corr) * 0.5f);

        if (hovered)
            col = col.brighter(0.3f);

        if (corr >= 0.0f)
        {
            g.setColour(col);
            g.fillRoundedRectangle(x + 1.0f, midY - h, barW - 2.0f, h, 1.5f);
            g.setColour(col.withAlpha(0.15f));
            g.fillRoundedRectangle(x, midY - h - 1.0f, barW, h + 2.0f, 2.0f);
        }
        else
        {
            g.setColour(col);
            g.fillRoundedRectangle(x + 1.0f, midY, barW - 2.0f, h, 1.5f);
            g.setColour(col.withAlpha(0.15f));
            g.fillRoundedRectangle(x, midY - 1.0f, barW, h + 2.0f, 2.0f);
        }
    }

    g.setFont(juce::Font(juce::FontOptions(7.0f, juce::Font::plain)));
    g.setColour(Colours::textDim.withAlpha(0.5f));
    for (int i = 0; i < n; ++i)
    {
        const float x = plotL + static_cast<float>(i) * barW;
        g.drawText(freqLabels[i], juce::Rectangle<float>(x, plotB + 1.0f, barW, 12.0f), juce::Justification::centred);
    }

    if (hoveredBand >= 0 && hoveredBand < n)
    {
        const float corr = values[hoveredBand];
        juce::String info = juce::String(freqLabels[hoveredBand]) + " Hz: " + juce::String(corr, 2);
        g.setFont(juce::Font(juce::FontOptions(10.0f, juce::Font::bold)));

        const float tw = juce::GlyphArrangement::getStringWidth(g.getCurrentFont(), info) + 12.0f;
        const float th = 16.0f;
        float tx = plotL + (static_cast<float>(hoveredBand) + 0.5f) * barW - tw * 0.5f;
        tx = juce::jlimit(area.getX() + 2.0f, area.getRight() - tw - 2.0f, tx);

        g.setColour(juce::Colour(0xe0101018));
        g.fillRoundedRectangle(tx, plotT, tw, th, 3.0f);
        g.setColour(Colours::textBright);
        g.drawText(info, juce::Rectangle<float>(tx, plotT, tw, th), juce::Justification::centred);
    }

    g.setColour(Colours::panelBorder);
    g.drawRoundedRectangle(area, 3.0f, 1.0f);
}

void Correlometer::mouseMove(const juce::MouseEvent& event)
{
    auto area = getLocalBounds().toFloat();
    const float pad = 4.0f;
    const float plotL = area.getX() + pad;
    const float plotR = area.getRight() - pad;
    const float plotW = plotR - plotL;
    const int n = EPStereoFixerAudioProcessor::numCorrBands;
    const float barW = plotW / static_cast<float>(n);

    const float mx = static_cast<float>(event.x);
    int band = static_cast<int>((mx - plotL) / barW);
    if (band < 0 || band >= n)
        band = -1;

    if (band != hoveredBand)
    {
        hoveredBand = band;
        repaint();
    }
}

void Correlometer::mouseExit(const juce::MouseEvent&)
{
    hoveredBand = -1;
    repaint();
}

BalanceMeter::BalanceMeter() {}

void BalanceMeter::setBalance(float value) { balance = juce::jlimit(-1.0f, 1.0f, value); repaint(); }

void BalanceMeter::paint(juce::Graphics& g)
{
    auto fullArea = getLocalBounds().toFloat();
    g.setColour(juce::Colour(0xff1a1a24));
    g.fillRoundedRectangle(fullArea, 3.0f);

    const float cx = fullArea.getCentreX();
    const float cy = fullArea.getCentreY();

    g.setColour(juce::Colour(0x20ffffff));
    g.drawLine(cx, fullArea.getY() + 2.0f, cx, fullArea.getBottom() - 2.0f, 1.0f);

    g.setFont(juce::Font(juce::FontOptions(9.0f, juce::Font::plain)));
    g.setColour(Colours::textDim.withAlpha(0.5f));
    g.drawText("L", fullArea.withWidth(14.0f).withX(fullArea.getX() + 2.0f), juce::Justification::centredLeft);
    g.drawText("R", fullArea.withWidth(14.0f).withX(fullArea.getRight() - 16.0f), juce::Justification::centredRight);

    const float barW = (fullArea.getWidth() - 8.0f) * 0.5f;
    const float barH = 6.0f;
    const float barY = cy - barH * 0.5f;

    if (balance < 0.0f)
    {
        const float w = -balance * barW;
        g.setColour(Colours::accent);
        g.fillRoundedRectangle(cx - w, barY, w, barH, 2.0f);
        g.setColour(Colours::accent.withAlpha(0.15f));
        g.fillRoundedRectangle(cx - w - 2.0f, barY - 2.0f, w + 4.0f, barH + 4.0f, 3.0f);
    }
    else if (balance > 0.0f)
    {
        const float w = balance * barW;
        g.setColour(Colours::accent);
        g.fillRoundedRectangle(cx, barY, w, barH, 2.0f);
        g.setColour(Colours::accent.withAlpha(0.15f));
        g.fillRoundedRectangle(cx - 2.0f, barY - 2.0f, w + 4.0f, barH + 4.0f, 3.0f);
    }

    g.setColour(Colours::textBright);
    g.fillEllipse(cx + balance * barW - 3.0f, cy - 3.0f, 6.0f, 6.0f);

    g.setColour(Colours::panelBorder);
    g.drawRoundedRectangle(fullArea, 3.0f, 1.0f);
}

MidSideMeter::MidSideMeter() { startTimerHz(30); }

void MidSideMeter::setLevels(float mid, float side)
{
    midLevel = mid;
    sideLevel = side;
    if (midLevel > midPeakHold) midPeakHold = midLevel;
    if (sideLevel > sidePeakHold) sidePeakHold = sideLevel;
    repaint();
}

void MidSideMeter::paint(juce::Graphics& g)
{
    auto fullArea = getLocalBounds().toFloat();
    g.setColour(juce::Colour(0xff1a1a24));
    g.fillRoundedRectangle(fullArea, 3.0f);

    const float barW = (fullArea.getWidth() - 12.0f) * 0.5f;
    const float barX1 = fullArea.getX() + 4.0f;
    const float barX2 = barX1 + barW + 4.0f;
    const float barTop = fullArea.getY() + 16.0f;
    const float barBot = fullArea.getBottom() - 4.0f;
    const float barHeight = barBot - barTop;

    g.setFont(juce::Font(juce::FontOptions(9.0f, juce::Font::bold)));
    g.setColour(Colours::textDim);
    g.drawText("M", juce::Rectangle<float>(barX1, fullArea.getY() + 2.0f, barW, 12.0f), juce::Justification::centred);
    g.drawText("S", juce::Rectangle<float>(barX2, fullArea.getY() + 2.0f, barW, 12.0f), juce::Justification::centred);

    auto drawBar = [&](float x, float w, float level, float peakHold, juce::Colour col)
    {
        const float dB = juce::Decibels::gainToDecibels(level, -60.0f);
        const float norm = juce::jlimit(0.0f, 1.0f, (dB + 60.0f) / 66.0f);
        const float h = barHeight * norm;

        juce::ColourGradient grad(col, 0.0f, barBot, col.withMultipliedBrightness(0.5f), 0.0f, barTop, false);
        g.setGradientFill(grad);
        g.fillRoundedRectangle(x, barBot - h, w, h, 2.0f);

        const float holdDb = juce::Decibels::gainToDecibels(peakHold, -60.0f);
        const float holdNorm = juce::jlimit(0.0f, 1.0f, (holdDb + 60.0f) / 66.0f);
        const float holdY = barBot - barHeight * holdNorm;
        g.setColour(Colours::textBright);
        g.drawLine(x, holdY, x + w, holdY, 1.5f);
    };

    drawBar(barX1, barW, midLevel, midPeakHold, Colours::accent);
    drawBar(barX2, barW, sideLevel, sidePeakHold, Colours::scopeTrace);

    g.setColour(Colours::panelBorder);
    g.drawRoundedRectangle(fullArea, 3.0f, 1.0f);
}

void MidSideMeter::timerCallback()
{
    midPeakHold *= 0.95f;
    sidePeakHold *= 0.95f;
    if (midPeakHold < midLevel) midPeakHold = midLevel;
    if (sidePeakHold < sideLevel) sidePeakHold = sideLevel;
    repaint();
}

Scope::Scope(EPStereoFixerAudioProcessor& p) : processor(p)
{
    setTooltip("Stereo goniometer: X = left, Y = right");
}

void Scope::paint(juce::Graphics& g)
{
    auto fullArea = getLocalBounds().toFloat();
    g.setColour(juce::Colour(0xff1a1a24));
    g.fillRoundedRectangle(fullArea, 3.0f);

    const float cx = fullArea.getCentreX();
    const float cy = fullArea.getCentreY();
    const float scale = std::min(fullArea.getWidth(), fullArea.getHeight()) * 0.42f;

    g.setColour(juce::Colour(0x20ffffff));
    g.drawLine(fullArea.getX(), cy, fullArea.getRight(), cy, 0.5f);
    g.drawLine(cx, fullArea.getY(), cx, fullArea.getBottom(), 0.5f);

    juce::Path circle;
    circle.addEllipse(cx - scale, cy - scale, scale * 2.0f, scale * 2.0f);
    g.setColour(juce::Colour(0x18ffffff));
    g.strokePath(circle, juce::PathStrokeType(0.5f));

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
        if (first) { trace.startNewSubPath(x, y); first = false; }
        else       { trace.lineTo(x, y); }
    }

    g.setColour(Colours::scopeTrace.withAlpha(0.12f));
    g.strokePath(trace, juce::PathStrokeType(4.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    g.setColour(Colours::scopeTrace);
    g.strokePath(trace, juce::PathStrokeType(1.5f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    g.setColour(Colours::panelBorder);
    g.drawRoundedRectangle(fullArea, 3.0f, 1.0f);
}

void EPStereoFixerAudioProcessorEditor::drawPanel(juce::Graphics& g, juce::Rectangle<int> bounds)
{
    auto b = bounds.toFloat();
    g.setColour(Colours::panelBg);
    g.fillRoundedRectangle(b, 8.0f);
    g.setColour(Colours::panelBorder);
    g.drawRoundedRectangle(b, 8.0f, 1.0f);
}

void EPStereoFixerAudioProcessorEditor::setupSectionLabel(juce::Label& label, const juce::String& text)
{
    label.setText(text, juce::dontSendNotification);
    label.setJustificationType(juce::Justification::centredLeft);
    label.setFont(juce::Font(juce::FontOptions(11.0f, juce::Font::bold)));
    label.setColour(juce::Label::textColourId, Colours::accent);
    addAndMakeVisible(label);
}

EPStereoFixerAudioProcessorEditor::EPStereoFixerAudioProcessorEditor(EPStereoFixerAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p), scope(audioProcessor), correlometer(audioProcessor)
{
    setLookAndFeel(&epLookAndFeel);

    titleLabel.setText("EP STEREO FIXER", juce::dontSendNotification);
    titleLabel.setJustificationType(juce::Justification::centred);
    titleLabel.setFont(juce::Font(juce::FontOptions(22.0f, juce::Font::bold)));
    titleLabel.setColour(juce::Label::textColourId, Colours::accent);
    addAndMakeVisible(titleLabel);

    setupSectionLabel(formatLabel, "FORMAT");
    setupSectionLabel(controlsLabel, "CONTROLS");
    setupSectionLabel(metersLabel, "METERS");

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
    setupGainSlider(gainLeftSlider, gainLeftLabel, "Gain L");
    setupGainSlider(gainRightSlider, gainRightLabel, "Gain R");

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
    addAndMakeVisible(balanceMeter);
    addAndMakeVisible(midSideMeter);
    addAndMakeVisible(scope);
    addAndMakeVisible(correlometer);

    logoImage = juce::ImageFileFormat::loadFrom(BinaryData::logo_png, BinaryData::logo_pngSize);
    logoComponent.setImage(logoImage);
    logoComponent.setAlpha(0.6f);
    addAndMakeVisible(logoComponent);

    leftMeter.setTooltip("Output peak level for the left channel");
    rightMeter.setTooltip("Output peak level for the right channel");
    phaseMeter.setTooltip("Phase correlation: left = mono, right = out-of-phase");
    balanceMeter.setTooltip("Stereo balance: centre = balanced, L/R = panned");
    midSideMeter.setTooltip("Mid and Side signal levels");

    setupMeterDbLabel(leftDbLabel);
    setupMeterDbLabel(rightDbLabel);

    auto setupMeterLabel = [this](juce::Label& label, const juce::String& text) {
        label.setText(text, juce::dontSendNotification);
        label.setJustificationType(juce::Justification::centred);
        label.setFont(juce::Font(juce::FontOptions(10.0f, juce::Font::plain)));
        label.setColour(juce::Label::textColourId, Colours::textDim);
        addAndMakeVisible(label);
    };
    setupMeterLabel(phaseLabel, "Phase");
    setupMeterLabel(balanceLabel, "Balance");
    setupMeterLabel(msLabel, "M/S");
    setupMeterLabel(scopeLabel, "Scope");
    setupMeterLabel(corrLabel, "Correlation");

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
    setSize(700, 670);

    setResizable(true, true);
    setResizeLimits(700, 670, 1400, 1340);
}

EPStereoFixerAudioProcessorEditor::~EPStereoFixerAudioProcessorEditor()
{
    setLookAndFeel(nullptr);
}

void EPStereoFixerAudioProcessorEditor::setupGainSlider(juce::Slider& slider, juce::Label& label, const juce::String& name)
{
    slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 18);
    slider.setNumDecimalPlacesToDisplay(1);
    addAndMakeVisible(slider);

    label.setText(name, juce::dontSendNotification);
    label.attachToComponent(&slider, false);
    label.setJustificationType(juce::Justification::centred);
    label.setFont(juce::Font(juce::FontOptions(11.0f, juce::Font::plain)));
    label.setColour(juce::Label::textColourId, Colours::textDim);
    addAndMakeVisible(label);
}

void EPStereoFixerAudioProcessorEditor::setupUtilityButton(juce::TextButton& button, const juce::String& tooltip)
{
    button.setClickingTogglesState(true);
    button.setTooltip(tooltip);
    addAndMakeVisible(button);
}

void EPStereoFixerAudioProcessorEditor::setupMeterDbLabel(juce::Label& label)
{
    label.setText("-inf", juce::dontSendNotification);
    label.setJustificationType(juce::Justification::centred);
    label.setFont(juce::Font(juce::FontOptions(10.0f, juce::Font::plain)));
    label.setColour(juce::Label::textColourId, Colours::textDim);
    addAndMakeVisible(label);
}

void EPStereoFixerAudioProcessorEditor::paint(juce::Graphics& g)
{
    juce::ColourGradient bgGrad(Colours::bg, 0.0f, 0.0f,
                                 Colours::bg.brighter(0.04f), 0.0f, static_cast<float>(getHeight()), false);
    g.setGradientFill(bgGrad);
    g.fillAll();

    auto area = getLocalBounds().reduced(12);
    area.removeFromTop(34);

    auto formatPanel = area.removeFromTop(130);
    drawPanel(g, formatPanel);

    area.removeFromTop(6);
    auto controlsPanel = area.removeFromTop(140);
    drawPanel(g, controlsPanel);

    area.removeFromTop(6);
    drawPanel(g, area);
}

void EPStereoFixerAudioProcessorEditor::resized()
{
    auto area = getLocalBounds().reduced(12);

    titleLabel.setBounds(area.removeFromTop(32));
    area.removeFromTop(2);

    auto formatPanel = area.removeFromTop(130);
    auto formatInner = formatPanel.reduced(10);
    formatLabel.setBounds(formatInner.removeFromTop(18));
    formatInner.removeFromTop(4);

    auto formatRow = formatInner.removeFromTop(56);
    const int fbW = formatRow.getWidth() / 8;
    stereoButton.setBounds(formatRow.removeFromLeft(fbW).reduced(2));
    flipButton.setBounds(formatRow.removeFromLeft(fbW).reduced(2));
    sumButton.setBounds(formatRow.removeFromLeft(fbW).reduced(2));
    leftButton.setBounds(formatRow.removeFromLeft(fbW).reduced(2));
    rightButton.setBounds(formatRow.removeFromLeft(fbW).reduced(2));
    midSideButton.setBounds(formatRow.removeFromLeft(fbW).reduced(2));
    soloMidButton.setBounds(formatRow.removeFromLeft(fbW).reduced(2));
    soloSideButton.setBounds(formatRow.removeFromLeft(fbW).reduced(2));

    formatInner.removeFromTop(4);
    auto utilityRow = formatInner.removeFromTop(32);
    const int uW = utilityRow.getWidth() / 4;
    invertLeftButton.setBounds(utilityRow.removeFromLeft(uW).reduced(2));
    invertRightButton.setBounds(utilityRow.removeFromLeft(uW).reduced(2));
    autoGainButton.setBounds(utilityRow.removeFromLeft(uW).reduced(2));
    bypassButton.setBounds(utilityRow.removeFromLeft(uW).reduced(2));

    area.removeFromTop(6);
    auto controlsPanel = area.removeFromTop(140);
    auto controlsInner = controlsPanel.reduced(10);
    controlsLabel.setBounds(controlsInner.removeFromTop(18));
    controlsInner.removeFromTop(14);

    auto knobRow = controlsInner;
    const int linkBtnW = 36;
    const int knobW = (knobRow.getWidth() - linkBtnW) / 4;
    inputGainSlider.setBounds(knobRow.removeFromLeft(knobW).reduced(4));
    widthSlider.setBounds(knobRow.removeFromLeft(knobW).reduced(4));
    gainLeftSlider.setBounds(knobRow.removeFromLeft(knobW).reduced(4));
    linkButton.setBounds(knobRow.removeFromLeft(linkBtnW).reduced(2));
    gainRightSlider.setBounds(knobRow.removeFromLeft(knobW).reduced(4));

    area.removeFromTop(6);
    auto metersPanel = area;
    auto metersInner = metersPanel.reduced(10);
    metersLabel.setBounds(metersInner.removeFromTop(18));
    metersInner.removeFromTop(4);

    auto corrRow = metersInner.removeFromBottom(100);
    metersInner.removeFromBottom(6);

    auto dbRow = metersInner.removeFromTop(18);
    auto meterRow = metersInner;

    const int meterBarW = 44;
    leftDbLabel.setBounds(dbRow.removeFromLeft(meterBarW));
    rightDbLabel.setBounds(dbRow.removeFromLeft(meterBarW));
    dbRow.removeFromLeft(10);
    phaseLabel.setBounds(dbRow.removeFromLeft(100));
    dbRow.removeFromLeft(10);
    balanceLabel.setBounds(dbRow.removeFromLeft(100));
    dbRow.removeFromLeft(10);
    msLabel.setBounds(dbRow.removeFromLeft(60));
    dbRow.removeFromLeft(10);
    scopeLabel.setBounds(dbRow.removeFromLeft(meterRow.getHeight()));

    leftMeter.setBounds(meterRow.removeFromLeft(meterBarW).reduced(2, 2));
    rightMeter.setBounds(meterRow.removeFromLeft(meterBarW).reduced(2, 2));
    meterRow.removeFromLeft(10);
    phaseMeter.setBounds(meterRow.removeFromLeft(100).reduced(2));
    meterRow.removeFromLeft(10);
    balanceMeter.setBounds(meterRow.removeFromLeft(100).reduced(2));
    meterRow.removeFromLeft(10);
    midSideMeter.setBounds(meterRow.removeFromLeft(60).reduced(2));
    meterRow.removeFromLeft(10);
    const int scopeSz = meterRow.getHeight();
    scope.setBounds(meterRow.removeFromLeft(scopeSz).reduced(2));

    auto logoArea = meterRow.removeFromLeft(meterRow.getWidth()).reduced(4);
    if (logoImage.isValid())
    {
        const float aspect = (float)logoImage.getWidth() / (float)logoImage.getHeight();
        const int logoW = juce::jmin(logoArea.getWidth(), (int)(logoArea.getHeight() * aspect));
        const int logoH = (int)(logoW / aspect);
        const int logoX = logoArea.getX() + (logoArea.getWidth() - logoW) / 2;
        const int logoY = logoArea.getY() + (logoArea.getHeight() - logoH) / 2;
        logoComponent.setBounds(logoX, logoY, logoW, logoH);
    }

    auto corrLabelRow = corrRow.removeFromTop(14);
    corrLabel.setBounds(corrLabelRow);
    correlometer.setBounds(corrRow.reduced(2));
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
        auto* lp = dynamic_cast<juce::AudioParameterBool*>(audioProcessor.getAPVTS().getParameter("invertLeft"));
        if (lp != nullptr) *lp = any;
        auto* rp = dynamic_cast<juce::AudioParameterBool*>(audioProcessor.getAPVTS().getParameter("invertRight"));
        if (rp != nullptr) *rp = any;
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
        gainLeftLabel.setText("Gain L", juce::dontSendNotification);
        gainRightLabel.setText("Gain R", juce::dontSendNotification);
    }
}

void EPStereoFixerAudioProcessorEditor::setLinkState(bool active)
{
    auto* param = dynamic_cast<juce::AudioParameterBool*>(audioProcessor.getAPVTS().getParameter("gainLink"));
    if (param != nullptr) *param = active;
    if (active) syncLinkedGainFromLeft();
}

void EPStereoFixerAudioProcessorEditor::setAutoGainState(bool active)
{
    auto* param = dynamic_cast<juce::AudioParameterBool*>(audioProcessor.getAPVTS().getParameter("autoGain"));
    if (param != nullptr) *param = active;
}

void EPStereoFixerAudioProcessorEditor::setInvertLeftState(bool active)
{
    auto* leftParam = dynamic_cast<juce::AudioParameterBool*>(audioProcessor.getAPVTS().getParameter("invertLeft"));
    if (leftParam != nullptr) *leftParam = active;
    invertLeftButton.setToggleState(active, juce::dontSendNotification);
    if (isMonoFormat(currentFormat))
    {
        auto* rightParam = dynamic_cast<juce::AudioParameterBool*>(audioProcessor.getAPVTS().getParameter("invertRight"));
        if (rightParam != nullptr) *rightParam = active;
        invertRightButton.setToggleState(active, juce::dontSendNotification);
    }
}

void EPStereoFixerAudioProcessorEditor::setInvertRightState(bool active)
{
    auto* rightParam = dynamic_cast<juce::AudioParameterBool*>(audioProcessor.getAPVTS().getParameter("invertRight"));
    if (rightParam != nullptr) *rightParam = active;
    invertRightButton.setToggleState(active, juce::dontSendNotification);
    if (isMonoFormat(currentFormat))
    {
        auto* leftParam = dynamic_cast<juce::AudioParameterBool*>(audioProcessor.getAPVTS().getParameter("invertLeft"));
        if (leftParam != nullptr) *leftParam = active;
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
    if (param != nullptr) *param = active;
}

void EPStereoFixerAudioProcessorEditor::syncLinkedGainFromLeft()
{
    updatingLink = true;
    const float value = gainLeftSlider.getValue();
    gainRightSlider.setValue(value, juce::dontSendNotification);
    auto* rightParam = audioProcessor.getAPVTS().getParameter("gainRight");
    if (rightParam != nullptr) rightParam->setValueNotifyingHost(rightParam->convertTo0to1(value));
    updatingLink = false;
}

void EPStereoFixerAudioProcessorEditor::syncLinkedGainFromRight()
{
    updatingLink = true;
    const float value = gainRightSlider.getValue();
    gainLeftSlider.setValue(value, juce::dontSendNotification);
    auto* leftParam = audioProcessor.getAPVTS().getParameter("gainLeft");
    if (leftParam != nullptr) leftParam->setValueNotifyingHost(leftParam->convertTo0to1(value));
    updatingLink = false;
}

void EPStereoFixerAudioProcessorEditor::timerCallback()
{
    auto* choice = dynamic_cast<juce::AudioParameterChoice*>(audioProcessor.getAPVTS().getParameter("format"));
    if (choice != nullptr) updateFormat(choice->getIndex());

    auto* gainLinkParam = dynamic_cast<juce::AudioParameterBool*>(audioProcessor.getAPVTS().getParameter("gainLink"));
    if (gainLinkParam != nullptr) linkButton.setToggleState(*gainLinkParam, juce::dontSendNotification);

    auto* autoGainParam = dynamic_cast<juce::AudioParameterBool*>(audioProcessor.getAPVTS().getParameter("autoGain"));
    if (autoGainParam != nullptr) autoGainButton.setToggleState(*autoGainParam, juce::dontSendNotification);

    auto* invertLeftParam = dynamic_cast<juce::AudioParameterBool*>(audioProcessor.getAPVTS().getParameter("invertLeft"));
    if (invertLeftParam != nullptr) invertLeftButton.setToggleState(*invertLeftParam, juce::dontSendNotification);

    auto* invertRightParam = dynamic_cast<juce::AudioParameterBool*>(audioProcessor.getAPVTS().getParameter("invertRight"));
    if (invertRightParam != nullptr) invertRightButton.setToggleState(*invertRightParam, juce::dontSendNotification);

    auto* bypassParam = dynamic_cast<juce::AudioParameterBool*>(audioProcessor.getAPVTS().getParameter("bypass"));
    if (bypassParam != nullptr) bypassButton.setToggleState(*bypassParam, juce::dontSendNotification);

    const float leftLevel = audioProcessor.getLeftMeter();
    const float rightLevel = audioProcessor.getRightMeter();

    leftMeter.setLevel(leftLevel);
    rightMeter.setLevel(rightLevel);
    phaseMeter.setCorrelation(audioProcessor.getPhaseMeter());
    balanceMeter.setBalance(audioProcessor.getBalance());
    midSideMeter.setLevels(audioProcessor.getMidLevel(), audioProcessor.getSideLevel());
    correlometer.update();
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
