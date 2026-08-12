#include "PluginProcessor.h"
#include "PluginEditor.h"

EPStereoFixerAudioProcessor::EPStereoFixerAudioProcessor()
    : AudioProcessor(BusesProperties()
        .withInput("Input", juce::AudioChannelSet::stereo(), true)
        .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      parameters(*this, nullptr, "PARAMS", createParameterLayout())
{
}

juce::AudioProcessorValueTreeState::ParameterLayout EPStereoFixerAudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    auto choiceAttributes = juce::AudioParameterChoiceAttributes().withLabel("mode");

    layout.add(std::make_unique<juce::AudioParameterChoice>(
        "format", "Format",
        juce::StringArray("Stereo", "Flip Channels", "Sum L+R", "Left", "Right", "Mid/Side", "Solo Mid", "Solo Side"),
        0, choiceAttributes));

    auto gainAttributes = juce::AudioParameterFloatAttributes()
        .withStringFromValueFunction([](float value, int) { return juce::String(value, 1) + " dB"; })
        .withLabel("dB");

    auto widthAttributes = juce::AudioParameterFloatAttributes()
        .withStringFromValueFunction([](float value, int) { return juce::String(value, 1) + " %"; })
        .withLabel("%");

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "inputGain", "Input Gain",
        juce::NormalisableRange<float>(-18.0f, 18.0f, 0.1f), 0.0f, gainAttributes));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "width", "Width",
        juce::NormalisableRange<float>(0.0f, 200.0f, 0.1f), 100.0f, widthAttributes));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "gainLeft", "Gain Left",
        juce::NormalisableRange<float>(-18.0f, 18.0f, 0.1f), 0.0f, gainAttributes));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "gainRight", "Gain Right",
        juce::NormalisableRange<float>(-18.0f, 18.0f, 0.1f), 0.0f, gainAttributes));

    layout.add(std::make_unique<juce::AudioParameterBool>("invertLeft", "Invert Left", false));
    layout.add(std::make_unique<juce::AudioParameterBool>("invertRight", "Invert Right", false));
    layout.add(std::make_unique<juce::AudioParameterBool>("gainLink", "Link", false));
    layout.add(std::make_unique<juce::AudioParameterBool>("autoGain", "Auto Gain", false));
    layout.add(std::make_unique<juce::AudioParameterBool>("bypass", "Bypass", false));

    return layout;
}

void EPStereoFixerAudioProcessor::prepareToPlay(double newSampleRate, int)
{
    sampleRate = newSampleRate;

    inputGainSmoothed.reset(sampleRate, 0.05);
    widthSmoothed.reset(sampleRate, 0.05);
    gainLeftSmoothed.reset(sampleRate, 0.05);
    gainRightSmoothed.reset(sampleRate, 0.05);

    inputGainSmoothed.setCurrentAndTargetValue(parameters.getRawParameterValue("inputGain")->load());
    widthSmoothed.setCurrentAndTargetValue(parameters.getRawParameterValue("width")->load());
    gainLeftSmoothed.setCurrentAndTargetValue(parameters.getRawParameterValue("gainLeft")->load());
    gainRightSmoothed.setCurrentAndTargetValue(parameters.getRawParameterValue("gainRight")->load());

    const float peakRelease = 0.15f;
    const float corrRelease = 0.05f;
    const float agcRelease = 0.05f;

    peakDecay = std::exp(-1.0f / static_cast<float>(sampleRate * peakRelease));
    corrDecay = std::exp(-1.0f / static_cast<float>(sampleRate * corrRelease));
    agcDecay = std::exp(-1.0f / static_cast<float>(sampleRate * agcRelease));

    scopeSkip = static_cast<int>(sampleRate / 100.0);
    scopeSkip = std::max(scopeSkip, 1);
    scopeCounter = 0;
    scopeWriteIndex.store(0);
    scopeL.fill(0.0f);
    scopeR.fill(0.0f);

    inputPeak = 0.0f;
    outputPeak = 0.0f;
    autoGain = 1.0f;

    powerL = 0.0f;
    powerR = 0.0f;
    corrAccum = 0.0f;

    outputMeterL = 0.0f;
    outputMeterR = 0.0f;
    correlation = 0.0f;
}

void EPStereoFixerAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    if (buffer.getNumChannels() < 2)
        return;

    const auto numSamples = buffer.getNumSamples();

    auto* formatChoice = dynamic_cast<juce::AudioParameterChoice*>(parameters.getParameter("format"));
    const int format = formatChoice != nullptr ? formatChoice->getIndex() : 0;

    const bool invertL = parameters.getRawParameterValue("invertLeft")->load() > 0.5f;
    const bool invertR = parameters.getRawParameterValue("invertRight")->load() > 0.5f;
    const bool autoGainActive = parameters.getRawParameterValue("autoGain")->load() > 0.5f;
    const bool bypassActive = parameters.getRawParameterValue("bypass")->load() > 0.5f;

    inputGainSmoothed.setTargetValue(parameters.getRawParameterValue("inputGain")->load());
    widthSmoothed.setTargetValue(parameters.getRawParameterValue("width")->load());
    gainLeftSmoothed.setTargetValue(parameters.getRawParameterValue("gainLeft")->load());
    gainRightSmoothed.setTargetValue(parameters.getRawParameterValue("gainRight")->load());

    auto* left = buffer.getWritePointer(0);
    auto* right = buffer.getWritePointer(1);

    for (int i = 0; i < numSamples; ++i)
    {
        const float inGain = juce::Decibels::decibelsToGain(inputGainSmoothed.getNextValue());
        const float widthFactor = widthSmoothed.getNextValue() / 100.0f;
        const float linL = juce::Decibels::decibelsToGain(gainLeftSmoothed.getNextValue());
        const float linR = juce::Decibels::decibelsToGain(gainRightSmoothed.getNextValue());

        float l = left[i];
        float r = right[i];

        if (!bypassActive)
        {
            if (invertL)
                l = -l;
            if (invertR)
                r = -r;

            l *= inGain;
            r *= inGain;

            const float mid = (l + r) * 0.5f;
            const float side = (l - r) * 0.5f;
            l = mid + side * widthFactor;
            r = mid - side * widthFactor;
        }

        inputPeak = std::max(inputPeak * peakDecay, std::max(std::abs(l), std::abs(r)));

        float outL = l;
        float outR = r;

        if (!bypassActive)
        {
            switch (format)
            {
                case 0: // Stereo
                    break;
                case 1: // Flip Channels
                    std::swap(outL, outR);
                    break;
                case 2: // Sum L+R
                {
                    const float sum = (outL + outR) * 0.5f;
                    outL = sum;
                    outR = sum;
                    break;
                }
                case 3: // Left
                    outL = outL;
                    outR = outL;
                    break;
                case 4: // Right
                    outL = outR;
                    outR = outR;
                    break;
                case 5: // Mid/Side
                {
                    const float mid = (outL + outR) * 0.5f;
                    const float side = (outL - outR) * 0.5f;
                    outL = mid * linL + side * linR;
                    outR = mid * linL - side * linR;
                    break;
                }
                case 6: // Solo Mid
                {
                    const float mid = (outL + outR) * 0.5f;
                    outL = mid;
                    outR = mid;
                    break;
                }
                case 7: // Solo Side
                {
                    const float side = (outL - outR) * 0.5f;
                    outL = side;
                    outR = -side;
                    break;
                }
                default:
                    break;
            }

            if (format != 5)
            {
                outL *= linL;
                outR *= linR;
            }
        }

        outputPeak = std::max(outputPeak * peakDecay, std::max(std::abs(outL), std::abs(outR)));

        if (autoGainActive && !bypassActive)
        {
            const float targetGain = inputPeak / std::max(outputPeak, 1e-9f);
            autoGain = autoGain * agcDecay + juce::jlimit(0.01f, 100.0f, targetGain) * (1.0f - agcDecay);
            outL *= autoGain;
            outR *= autoGain;
        }
        else
        {
            autoGain = 1.0f;
        }

        outputMeterL = std::max(outputMeterL * peakDecay, std::abs(outL));
        outputMeterR = std::max(outputMeterR * peakDecay, std::abs(outR));

        powerL = powerL * corrDecay + outL * outL * (1.0f - corrDecay);
        powerR = powerR * corrDecay + outR * outR * (1.0f - corrDecay);
        corrAccum = corrAccum * corrDecay + outL * outR * (1.0f - corrDecay);

        const float denom = std::sqrt(powerL * powerR) + 1e-9f;
        correlation = std::clamp(corrAccum / denom, -1.0f, 1.0f);

        if (++scopeCounter >= scopeSkip)
        {
            scopeCounter = 0;
            const int idx = scopeWriteIndex.load();
            scopeL[idx] = outL;
            scopeR[idx] = outR;
            scopeWriteIndex.store((idx + 1) % scopeSize);
        }

        left[i] = outL;
        right[i] = outR;
    }

    leftMeter.store(outputMeterL);
    rightMeter.store(outputMeterR);
    phaseMeter.store(correlation);
}

juce::AudioProcessorEditor* EPStereoFixerAudioProcessor::createEditor()
{
    return new EPStereoFixerAudioProcessorEditor(*this);
}

void EPStereoFixerAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = parameters.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void EPStereoFixerAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xml(getXmlFromBinary(data, sizeInBytes));

    if (xml.get() != nullptr && xml->hasTagName(parameters.state.getType()))
        parameters.replaceState(juce::ValueTree::fromXml(*xml));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new EPStereoFixerAudioProcessor();
}
