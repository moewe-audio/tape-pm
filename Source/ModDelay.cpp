/*
  ==============================================================================

    ModDelay.cpp
    Created: 21 Apr 2024 1:06:31pm
    Author:  Levin

  ==============================================================================
*/

#include <JuceHeader.h>
#include "ModDelay.h"

void ModDelay::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    lfo.setSampleRate(sampleRate);
    lfo.setFrequency(8);
    buffer.setSize(1, sampleRate*3);
    this->startTimerHz(5);
}

void ModDelay::processBlock (juce::dsp::AudioBlock<float>& audioBuffer)
{
    auto data = audioBuffer.getChannelPointer(0);
    float mod = 0;
    float enabled = (float)(params.flutterRate > 0);
    float depth = params.flutterDepth * enabled;
    for (int i = 0; i < audioBuffer.getNumSamples(); i++)
    {
        mod = depth * lfo.getNextSample();
        readRate = (1.f - depth) + mod;
        pushSample(data[i]);
        data[i] = getSample();
    }
}

void ModDelay::pushSample(float sample)
{
    auto data = buffer.getWritePointer(0);
    data[writeIndex] = sample;
    writeIndex++;
    if (writeIndex >= buffer.getNumSamples())
        writeIndex = 0;
}

float ModDelay::getSample()
{
    int index = floor(fractionalReadIndex);
    int nextIndex = index + 1;
    float frac = fractionalReadIndex - index;
    auto read = buffer.getReadPointer(0);
    if (nextIndex >= buffer.getNumSamples())
    {
        nextIndex -= buffer.getNumSamples();
    }
    float sample = (1.0f - frac) * read[index] + frac * read[nextIndex];
    fractionalReadIndex += readRate;
    if (fractionalReadIndex >= buffer.getNumSamples())
    {
        fractionalReadIndex -= buffer.getNumSamples();
    }

    return sample;
}

void ModDelay::timerCallback()
{
    lfo.setFrequency(params.flutterRate);
}

///////////
///// LFO
void LFO::setFrequency(double frequency)
{
    freq = frequency;
    updateIncrement();
}

void LFO::setSampleRate(double sr)
{
    samplerate = sr;
    updateIncrement();
}

float LFO::getNextSample()
{
    float value = std::sin(phase);
    phase += phaseIncrement;
    if (phase >= 2.0 * M_PI)
        phase -= 2.0 * M_PI;
    return value;
}
