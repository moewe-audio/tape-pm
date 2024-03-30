/*
  ==============================================================================

    RecordHead.cpp
    Created: 30 Mar 2024 10:35:31am
    Author:  Levin

  ==============================================================================
*/

#include "TapeSim.h"

void TapeMachine::prepareToPlay (double sampleRate, int oversampling, int samplesPerBlock)
{
    bias.prepareToPlay(sampleRate, oversampling, samplesPerBlock);
}

void TapeMachine::processBlock (juce::dsp::AudioBlock<float>& audioBuffer)
{
    bias.processBlock(audioBuffer);
    recHead.processBlock(audioBuffer);
}

void RecordHead::processBlock (juce::dsp::AudioBlock<float>& audioBuffer)
{
    auto data = audioBuffer.getChannelPointer(0);
    for (auto i = 0; i < audioBuffer.getNumSamples(); ++i)
    {
        data[i] = (turnsWire * headEfficiency * data[i]) / gapWidth;
    }
}

void BiasSignal::prepareToPlay (double sampleRate, int oversampling, int samplesPerBlock)
{
    this->samplerate = sampleRate * oversampling;
    createWavetable();
}

void BiasSignal::processBlock (juce::dsp::AudioBlock<float>& audioBuffer)
{
    auto data = audioBuffer.getChannelPointer(0);
    
    for (auto i = 0; i < audioBuffer.getNumSamples(); ++i)
    {
        data[i] = data[i] + gain * getNextSample();
    }
}

float BiasSignal::getNextSample()
{
    const float indexIncrement = freq * wavetable.getNumSamples() / samplerate;
    currentIndex += indexIncrement;

    while (currentIndex >= wavetable.getNumSamples())
        currentIndex -= wavetable.getNumSamples();

    auto index0 = static_cast<int>(currentIndex);
    auto index1 = index0 + 1;
    if (index1 >= wavetable.getNumSamples()) index1 -= wavetable.getNumSamples();
    
    auto frac = currentIndex - index0;
    auto read = wavetable.getReadPointer(0);
    auto sample0 = read[index0];
    auto sample1 = read[index1];
    return sample0 + frac * (sample1 - sample0);
}

void BiasSignal::createWavetable()
{
    wavetable.setSize(1, tableSize);
    auto write = wavetable.getWritePointer(0);
    for (int i = 0; i < tableSize; ++i) {
        write[i] = std::cos(2.0 * juce::MathConstants<double>::pi * i / tableSize);
    }
}

void BiasSignal::setBiasFreq(float freq)
{
    auto tableSizeOverSampleRate = (float) tableSize / samplerate;
    tableDelta = freq * tableSizeOverSampleRate;
    this->freq = freq;
}
