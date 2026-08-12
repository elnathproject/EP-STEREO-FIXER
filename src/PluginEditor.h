#pragma once

#include "PluginProcessor.h"

namespace Colours
{
    static const juce::Colour bg           { 0xff16161c };
    static const juce::Colour panelBg      { 0xff1e1e28 };
    static const juce::Colour panelBorder  { 0x30ffffff };
    static const juce::Colour accent       { 0xff00c8b4 };
    static const juce::Colour accentDim    { 0xff007a6e };
    static const juce::Colour btnOff       { 0xff242430 };
    static const juce::Colour btnHover     { 0xff32324a };
    static const juce::Colour btnOn        { 0xff00c8b4 };
    static const juce::Colour textBright   { 0xfff0f0f0 };
    static const juce::Colour textDim      { 0xffaaaaaa };
    static const juce::Colour meterGreen   { 0xff00e676 };
    static const juce::Colour meterYellow  { 0xffffc107 };
    static const juce::Colour meterRed     { 0xffff5252 };
    static const juce::Colour scopeTrace   { 0xff00e5ff };
}

class EPLookAndFeel : public juce::LookAndFeel_V4
{
public:
    EPLookAndFeel();

    void drawRotarySlider(juce::Graphics&, int x, int y, int width, int height,
                          float sliderPos, float startAngle, float endAngle,
                          juce::Slider&) override;

    void drawButtonBackground(juce::Graphics&, juce::Button&,
                              const juce::Colour&, bool isMouseOver, bool isButtonDown) override;

    void drawButtonText(juce::Graphics&, juce::TextButton&,
                        bool isMouseOver, bool isButtonDown) override;
};

class IconButton : public juce::Button
{
public:
    enum class Type { Stereo, Flip, Sum, Left, Right, MidSide, SoloMid, SoloSide, Link };

    IconButton(Type type, const juce::String& label, const juce::String& tooltip);
    void paintButton(juce::Graphics& g, bool isMouseOverButton, bool isButtonDown) override;

private:
    Type buttonType;
    juce::String labelText;

    void drawStereoIcon(juce::Graphics& g, juce::Rectangle<float> area, bool active);
    void drawFlipIcon(juce::Graphics& g, juce::Rectangle<float> area, bool active);
    void drawSumIcon(juce::Graphics& g, juce::Rectangle<float> area, bool active);
    void drawLeftIcon(juce::Graphics& g, juce::Rectangle<float> area, bool active);
    void drawRightIcon(juce::Graphics& g, juce::Rectangle<float> area, bool active);
    void drawMidSideIcon(juce::Graphics& g, juce::Rectangle<float> area, bool active);
    void drawSoloMidIcon(juce::Graphics& g, juce::Rectangle<float> area, bool active);
    void drawSoloSideIcon(juce::Graphics& g, juce::Rectangle<float> area, bool active);
    void drawLinkIcon(juce::Graphics& g, juce::Rectangle<float> area, bool active);
};

class PeakMeter : public juce::Component,
                  public juce::SettableTooltipClient,
                  private juce::Timer
{
public:
    PeakMeter();
    void setLevel(float linearAmplitude);
    void paint(juce::Graphics& g) override;

private:
    float currentLevel = 0.0f;
    float peakHold = 0.0f;
    void timerCallback() override;
};

class PhaseMeter : public juce::Component,
                   public juce::SettableTooltipClient
{
public:
    PhaseMeter();
    void setCorrelation(float value);
    void paint(juce::Graphics& g) override;

private:
    float correlation = 0.0f;
};

class Scope : public juce::Component,
              public juce::SettableTooltipClient
{
public:
    Scope(EPStereoFixerAudioProcessor& p);
    void paint(juce::Graphics& g) override;

private:
    EPStereoFixerAudioProcessor& processor;
};

class Correlometer : public juce::Component,
                     public juce::SettableTooltipClient
{
public:
    Correlometer(EPStereoFixerAudioProcessor& p);
    void update();
    void paint(juce::Graphics& g) override;

private:
    EPStereoFixerAudioProcessor& processor;
    std::array<float, EPStereoFixerAudioProcessor::numCorrBands> values {};
};

class BalanceMeter : public juce::Component,
                     public juce::SettableTooltipClient
{
public:
    BalanceMeter();
    void setBalance(float value);
    void paint(juce::Graphics& g) override;

private:
    float balance = 0.0f;
};

class MidSideMeter : public juce::Component,
                      public juce::SettableTooltipClient,
                      private juce::Timer
{
public:
    MidSideMeter();
    void setLevels(float midLevel, float sideLevel);
    void paint(juce::Graphics& g) override;

private:
    float midLevel = 0.0f;
    float sideLevel = 0.0f;
    float midPeakHold = 0.0f;
    float sidePeakHold = 0.0f;
    void timerCallback() override;
};

class EPStereoFixerAudioProcessorEditor : public juce::AudioProcessorEditor,
                                          private juce::Timer
{
public:
    EPStereoFixerAudioProcessorEditor(EPStereoFixerAudioProcessor&);
    ~EPStereoFixerAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    EPStereoFixerAudioProcessor& audioProcessor;
    EPLookAndFeel epLookAndFeel;

    juce::Label titleLabel;
    juce::Label formatLabel;
    juce::Label controlsLabel;
    juce::Label metersLabel;

    IconButton stereoButton { IconButton::Type::Stereo, "Stereo", "Standard stereo output" };
    IconButton flipButton { IconButton::Type::Flip, "Flip", "Swap left and right channels" };
    IconButton sumButton { IconButton::Type::Sum, "Sum", "Sum left and right to mono" };
    IconButton leftButton { IconButton::Type::Left, "Left", "Output left channel on both sides" };
    IconButton rightButton { IconButton::Type::Right, "Right", "Output right channel on both sides" };
    IconButton midSideButton { IconButton::Type::MidSide, "Mid/Side", "Gain Left = Mid, Gain Right = Side" };
    IconButton soloMidButton { IconButton::Type::SoloMid, "Solo M", "Output only the Mid component" };
    IconButton soloSideButton { IconButton::Type::SoloSide, "Solo S", "Output only the Side component" };

    IconButton linkButton { IconButton::Type::Link, "Link", "Link the two output gain knobs together" };

    juce::TextButton invertLeftButton { "Phase L" };
    juce::TextButton invertRightButton { "Phase R" };
    juce::TextButton autoGainButton { "AUTOGAIN" };
    juce::TextButton bypassButton { "BYPASS" };

    juce::Slider inputGainSlider;
    juce::Slider widthSlider;
    juce::Slider gainLeftSlider;
    juce::Slider gainRightSlider;

    juce::Label inputGainLabel;
    juce::Label widthLabel;
    juce::Label gainLeftLabel;
    juce::Label gainRightLabel;
    juce::Label leftDbLabel;
    juce::Label rightDbLabel;

    PeakMeter leftMeter;
    PeakMeter rightMeter;
    PhaseMeter phaseMeter;
    BalanceMeter balanceMeter;
    MidSideMeter midSideMeter;
    Scope scope;
    Correlometer correlometer;

    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> inputGainAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> widthAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> gainLeftAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> gainRightAttachment;

    bool updatingLink = false;
    int currentFormat = 0;
    juce::TooltipWindow tooltipWindow { this };

    void setFormat(int index);
    bool isMonoFormat(int index) const;
    void updateFormat(int index);
    void updateGainLabels();
    void setLinkState(bool active);
    void setAutoGainState(bool active);
    void setInvertLeftState(bool active);
    void setInvertRightState(bool active);
    void setBypassState(bool active);
    void syncLinkedGainFromLeft();
    void syncLinkedGainFromRight();
    void timerCallback() override;

    void setupGainSlider(juce::Slider& slider, juce::Label& label, const juce::String& name);
    void setupMeterDbLabel(juce::Label& label);
    void setupUtilityButton(juce::TextButton& button, const juce::String& tooltip);
    void setupSectionLabel(juce::Label& label, const juce::String& text);

    static void drawPanel(juce::Graphics& g, juce::Rectangle<int> bounds);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EPStereoFixerAudioProcessorEditor)
};
