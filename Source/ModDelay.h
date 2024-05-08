/*
  ==============================================================================

    ModDelay.h
    Created: 21 Apr 2024 1:06:31pm
    Author:  Levin

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "Parameters.h"

class LFO
{
public:
    void setFrequency(double frequency);
    void setSampleRate(double sr);
    float getNextSample();

private:
    double phase = juce::MathConstants<double>::pi;
    double freq = 0.1;
    double samplerate;
    double phaseIncrement;

    void updateIncrement()
    {
        phaseIncrement = (juce::MathConstants<double>::twoPi * freq) / samplerate;
    }
};


class ModDelay : juce::Timer
{
public:
    ModDelay(UserParameters& userParams) : params(userParams) {};
    void prepareToPlay (double sampleRate, int samplesPerBlock);
    void processBlock (juce::dsp::AudioBlock<float>& audioBuffer);
    void pushSample(float sample);
    float getSample();
    void timerCallback();
private:
    juce::AudioBuffer<float> buffer;
    int writeIndex = 0;
    float fractionalReadIndex = 0.f;
    float readRate = 1.f;
    LFO lfo;
    UserParameters &params;
};

