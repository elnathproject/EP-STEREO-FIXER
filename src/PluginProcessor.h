#pragma once

#include <array>
#include <atomic>
#include <juce_audio_utils/juce_audio_utils.h>

class EPStereoMatrixAudioProcessor : public juce::AudioProcessor
{
public:
    EPStereoMatrixAudioProcessor();
    ~EPStereoMatrixAudioProcessor() override = default;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    void processBlock(juce::AudioBuffer<double>&, juce::MidiBuffer&) override;
    bool supportsDoublePrecisionProcessing() const override { return true; }

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "EP STEREO MATRIX"; }
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
    float getBalance() const { return balanceMeter.load(); }
    float getMidLevel() const { return midMeter.load(); }
    float getSideLevel() const { return sideMeter.load(); }

    static constexpr int scopeSize = 256;
    const float* getScopeL() const { return scopeL.data(); }
    const float* getScopeR() const { return scopeR.data(); }
    int getScopeWriteIndex() const { return scopeWriteIndex.load(); }

    static constexpr int numCorrBands = 12;
    float getBandCorrelation(int band) const { return bandCorrelation[band].load(); }
    float getBandFrequency(int band) const { return bandFrequencies[band]; }

private:
    juce::AudioProcessorValueTreeState parameters;
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    juce::LinearSmoothedValue<float> inputGainSmoothed;
    juce::LinearSmoothedValue<float> widthSmoothed;
    juce::LinearSmoothedValue<float> gainLeftSmoothed;
    juce::LinearSmoothedValue<float> gainRightSmoothed;
    juce::LinearSmoothedValue<float> bypassSmoothed;

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
    std::atomic<float> balanceMeter { 0.0f };
    std::atomic<float> midMeter { 0.0f };
    std::atomic<float> sideMeter { 0.0f };

    float outputMeterMid = 0.0f;
    float outputMeterSide = 0.0f;
    float balanceSmoothed = 0.0f;

    std::array<float, scopeSize> scopeL {};
    std::array<float, scopeSize> scopeR {};
    std::atomic<int> scopeWriteIndex { 0 };
    int scopeCounter = 0;
    int scopeSkip = 1;

    static constexpr float bandFrequencies[numCorrBands] = {
        40.0f, 80.0f, 160.0f, 315.0f, 630.0f, 1250.0f,
        2500.0f, 4000.0f, 6300.0f, 8000.0f, 12500.0f, 16000.0f
    };

    struct BandFilter {
        float b0 = 0, b1 = 0, b2 = 0, a1 = 0, a2 = 0;
        float x1L = 0, x2L = 0, y1L = 0, y2L = 0;
        float x1R = 0, x2R = 0, y1R = 0, y2R = 0;
        float processL(float in) {
            float out = b0 * in + b1 * x1L + b2 * x2L - a1 * y1L - a2 * y2L;
            x2L = x1L; x1L = in; y2L = y1L; y1L = out;
            return out;
        }
        float processR(float in) {
            float out = b0 * in + b1 * x1R + b2 * x2R - a1 * y1R - a2 * y2R;
            x2R = x1R; x1R = in; y2R = y1R; y1R = out;
            return out;
        }
    };

    std::array<BandFilter, numCorrBands> bandFilters;
    struct BandCorrelationState {
        float powerL = 0, powerR = 0, corrAccum = 0;
    };
    std::array<BandCorrelationState, numCorrBands> bandCorrState {};
    std::array<std::atomic<float>, numCorrBands> bandCorrelation {};

    void updateBandFilters();

    template <typename FloatType>
    void processBlockInternal(juce::AudioBuffer<FloatType>& buffer);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EPStereoMatrixAudioProcessor)
};
