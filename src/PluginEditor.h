#pragma once

#include "PluginProcessor.h"

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

class EPStereoFixerAudioProcessorEditor : public juce::AudioProcessorEditor,
                                                   private juce::Timer
{
public:
    EPStereoFixerAudioProcessorEditor(EPStereoFixerAudioProcessor&);
    ~EPStereoFixerAudioProcessorEditor() override = default;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    EPStereoFixerAudioProcessor& audioProcessor;

    juce::Label formatLabel;
    juce::Label outputGainLabel;
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
    Scope scope;

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

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EPStereoFixerAudioProcessorEditor)
};
