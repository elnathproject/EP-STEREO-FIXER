#pragma once

#include <array>
#include <atomic>
#include <juce_audio_utils/juce_audio_utils.h>

class EPStereoFixerAudioProcessor : public juce::AudioProcessor
{
public:
    EPStereoFixerAudioProcessor();
    ~EPStereoFixerAudioProcessor() override = default;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "EP STEREO FIXER"; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState& getAPVTS() { return parameters; }

    float getLeftMeter() const { return leftMeter.load(); }
    float getRightMeter() const { return rightMeter.load(); }
    float getPhaseMeter() const { return phaseMeter.load(); }

    static constexpr int scopeSize = 256;
    const float* getScopeL() const { return scopeL.data(); }
    const float* getScopeR() const { return scopeR.data(); }
    int getScopeWriteIndex() const { return scopeWriteIndex.load(); }

private:
    juce::AudioProcessorValueTreeState parameters;
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    juce::LinearSmoothedValue<float> inputGainSmoothed;
    juce::LinearSmoothedValue<float> widthSmoothed;
    juce::LinearSmoothedValue<float> gainLeftSmoothed;
    juce::LinearSmoothedValue<float> gainRightSmoothed;

    double sampleRate = 44100.0;
    float peakDecay = 0.999f;
    float corrDecay = 0.99f;
    float agcDecay = 0.99f;

    float inputPeak = 0.0f;
    float outputPeak = 0.0f;
    float autoGain = 1.0f;

    float powerL = 0.0f;
    float powerR = 0.0f;
    float corrAccum = 0.0f;

    float outputMeterL = 0.0f;
    float outputMeterR = 0.0f;
    float correlation = 0.0f;

    std::atomic<float> leftMeter { 0.0f };
    std::atomic<float> rightMeter { 0.0f };
    std::atomic<float> phaseMeter { 0.0f };

    std::array<float, scopeSize> scopeL {};
    std::array<float, scopeSize> scopeR {};
    std::atomic<int> scopeWriteIndex { 0 };
    int scopeCounter = 0;
    int scopeSkip = 1;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EPStereoFixerAudioProcessor)
};
