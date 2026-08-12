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
    bypassSmoothed.reset(sampleRate, 0.02);

    inputGainSmoothed.setCurrentAndTargetValue(parameters.getRawParameterValue("inputGain")->load());
    widthSmoothed.setCurrentAndTargetValue(parameters.getRawParameterValue("width")->load());
    gainLeftSmoothed.setCurrentAndTargetValue(parameters.getRawParameterValue("gainLeft")->load());
    gainRightSmoothed.setCurrentAndTargetValue(parameters.getRawParameterValue("gainRight")->load());
    bypassSmoothed.setCurrentAndTargetValue(parameters.getRawParameterValue("bypass")->load() > 0.5f ? 1.0f : 0.0f);

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

    updateBandFilters();
    for (auto& s : bandCorrState)
    {
        s.powerL = 0.0f;
        s.powerR = 0.0f;
        s.corrAccum = 0.0f;
    }
    for (auto& c : bandCorrelation)
        c.store(0.0f);
}

void EPStereoFixerAudioProcessor::updateBandFilters()
{
    for (int i = 0; i < numCorrBands; ++i)
    {
        const float freq = bandFrequencies[i];
        const float Q = 1.4f;
        const float w0 = juce::MathConstants<float>::twoPi * freq / static_cast<float>(sampleRate);
        const float sinW0 = std::sin(w0);
        const float cosW0 = std::cos(w0);
        const float alpha = sinW0 / (2.0f * Q);

        const float a0 = 1.0f + alpha;
        bandFilters[i].b0 = (sinW0 * 0.5f) / a0;
        bandFilters[i].b1 = 0.0f;
        bandFilters[i].b2 = -(sinW0 * 0.5f) / a0;
        bandFilters[i].a1 = (-2.0f * cosW0) / a0;
        bandFilters[i].a2 = (1.0f - alpha) / a0;

        bandFilters[i].x1L = bandFilters[i].x2L = 0.0f;
        bandFilters[i].y1L = bandFilters[i].y2L = 0.0f;
        bandFilters[i].x1R = bandFilters[i].x2R = 0.0f;
        bandFilters[i].y1R = bandFilters[i].y2R = 0.0f;
    }
}

void EPStereoFixerAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    processBlockInternal(buffer);
}

void EPStereoFixerAudioProcessor::processBlock(juce::AudioBuffer<double>& buffer, juce::MidiBuffer&)
{
    processBlockInternal(buffer);
}

template <typename FloatType>
void EPStereoFixerAudioProcessor::processBlockInternal(juce::AudioBuffer<FloatType>& buffer)
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
    bypassSmoothed.setTargetValue(bypassActive ? 1.0f : 0.0f);

    inputGainSmoothed.setTargetValue(parameters.getRawParameterValue("inputGain")->load());
    widthSmoothed.setTargetValue(parameters.getRawParameterValue("width")->load());
    gainLeftSmoothed.setTargetValue(parameters.getRawParameterValue("gainLeft")->load());
    gainRightSmoothed.setTargetValue(parameters.getRawParameterValue("gainRight")->load());

    auto* left = buffer.getWritePointer(0);
    auto* right = buffer.getWritePointer(1);

    for (int i = 0; i < numSamples; ++i)
    {
        const auto inGain = static_cast<FloatType>(juce::Decibels::decibelsToGain(inputGainSmoothed.getNextValue()));
        const auto widthFactor = static_cast<FloatType>(widthSmoothed.getNextValue() / 100.0f);
        const auto linL = static_cast<FloatType>(juce::Decibels::decibelsToGain(gainLeftSmoothed.getNextValue()));
        const auto linR = static_cast<FloatType>(juce::Decibels::decibelsToGain(gainRightSmoothed.getNextValue()));
        const auto bypassMix = static_cast<FloatType>(bypassSmoothed.getNextValue());

        const FloatType dryL = left[i];
        const FloatType dryR = right[i];

        FloatType l = dryL;
        FloatType r = dryR;

        if (invertL)
            l = -l;
        if (invertR)
            r = -r;

        l *= inGain;
        r *= inGain;

        {
            const FloatType half = static_cast<FloatType>(0.5);
            const FloatType mid = (l + r) * half;
            const FloatType side = (l - r) * half;
            l = mid + side * widthFactor;
            r = mid - side * widthFactor;
        }

        inputPeak = std::max(inputPeak * peakDecay, std::max(std::abs(static_cast<float>(l)), std::abs(static_cast<float>(r))));

        FloatType outL = l;
        FloatType outR = r;

        switch (format)
        {
            case 0: break;
            case 1: std::swap(outL, outR); break;
            case 2:
            {
                const FloatType sum = (outL + outR) * static_cast<FloatType>(0.5);
                outL = sum;
                outR = sum;
                break;
            }
            case 3: outR = outL; break;
            case 4: outL = outR; break;
            case 5:
            {
                const FloatType mid = (outL + outR) * static_cast<FloatType>(0.5);
                const FloatType side = (outL - outR) * static_cast<FloatType>(0.5);
                outL = mid * linL + side * linR;
                outR = mid * linL - side * linR;
                break;
            }
            case 6:
            {
                const FloatType mid = (outL + outR) * static_cast<FloatType>(0.5);
                outL = mid;
                outR = mid;
                break;
            }
            case 7:
            {
                const FloatType side = (outL - outR) * static_cast<FloatType>(0.5);
                outL = side;
                outR = -side;
                break;
            }
            default: break;
        }

        if (format != 5)
        {
            outL *= linL;
            outR *= linR;
        }

        const float fOutL = static_cast<float>(outL);
        const float fOutR = static_cast<float>(outR);

        outputPeak = std::max(outputPeak * peakDecay, std::max(std::abs(fOutL), std::abs(fOutR)));

        if (autoGainActive && static_cast<float>(bypassMix) < 0.999f)
        {
            const float targetGain = inputPeak / std::max(outputPeak, 1e-9f);
            autoGain = autoGain * agcDecay + juce::jlimit(0.01f, 100.0f, targetGain) * (1.0f - agcDecay);
            outL *= static_cast<FloatType>(autoGain);
            outR *= static_cast<FloatType>(autoGain);
        }
        else
        {
            autoGain = 1.0f;
        }

        outL = dryL * bypassMix + outL * (static_cast<FloatType>(1) - bypassMix);
        outR = dryR * bypassMix + outR * (static_cast<FloatType>(1) - bypassMix);

        const float mOutL = static_cast<float>(outL);
        const float mOutR = static_cast<float>(outR);

        outputMeterL = std::max(outputMeterL * peakDecay, std::abs(mOutL));
        outputMeterR = std::max(outputMeterR * peakDecay, std::abs(mOutR));

        const float outMid = (mOutL + mOutR) * 0.5f;
        const float outSide = (mOutL - mOutR) * 0.5f;
        outputMeterMid = std::max(outputMeterMid * peakDecay, std::abs(outMid));
        outputMeterSide = std::max(outputMeterSide * peakDecay, std::abs(outSide));

        const float sumLR = std::abs(mOutL) + std::abs(mOutR) + 1e-9f;
        const float bal = (std::abs(mOutR) - std::abs(mOutL)) / sumLR;
        balanceSmoothed = balanceSmoothed * corrDecay + bal * (1.0f - corrDecay);

        powerL = powerL * corrDecay + mOutL * mOutL * (1.0f - corrDecay);
        powerR = powerR * corrDecay + mOutR * mOutR * (1.0f - corrDecay);
        corrAccum = corrAccum * corrDecay + mOutL * mOutR * (1.0f - corrDecay);

        const float denom = std::sqrt(powerL * powerR) + 1e-9f;
        correlation = std::clamp(corrAccum / denom, -1.0f, 1.0f);

        if ((i & 3) == 0)
        {
            for (int b = 0; b < numCorrBands; ++b)
            {
                const float bL = bandFilters[b].processL(mOutL);
                const float bR = bandFilters[b].processR(mOutR);
                auto& bs = bandCorrState[b];
                bs.powerL = bs.powerL * corrDecay + bL * bL * (1.0f - corrDecay);
                bs.powerR = bs.powerR * corrDecay + bR * bR * (1.0f - corrDecay);
                bs.corrAccum = bs.corrAccum * corrDecay + bL * bR * (1.0f - corrDecay);
            }
        }

        if (++scopeCounter >= scopeSkip)
        {
            scopeCounter = 0;
            const int idx = scopeWriteIndex.load();
            scopeL[idx] = mOutL;
            scopeR[idx] = mOutR;
            scopeWriteIndex.store((idx + 1) % scopeSize);
        }

        left[i] = outL;
        right[i] = outR;
    }

    leftMeter.store(outputMeterL);
    rightMeter.store(outputMeterR);
    phaseMeter.store(correlation);
    balanceMeter.store(balanceSmoothed);
    midMeter.store(outputMeterMid);
    sideMeter.store(outputMeterSide);

    for (int b = 0; b < numCorrBands; ++b)
    {
        const auto& bs = bandCorrState[b];
        const float d = std::sqrt(bs.powerL * bs.powerR) + 1e-9f;
        bandCorrelation[b].store(std::clamp(bs.corrAccum / d, -1.0f, 1.0f));
    }
}

template void EPStereoFixerAudioProcessor::processBlockInternal<float>(juce::AudioBuffer<float>&);
template void EPStereoFixerAudioProcessor::processBlockInternal<double>(juce::AudioBuffer<double>&);

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
