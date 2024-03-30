/*
  ==============================================================================

    RecordHead.h
    Created: 30 Mar 2024 10:35:31am
    Author:  Levin

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>


class RecordHead
{
public:
    void processBlock (juce::AudioBuffer<float>& audioBuffer);
private:
    float gapWidth = 6.f;
    float turnsWire = 100.f;
    float headEfficiency = 0.1;
};

class BiasSignal
{
public:
    void processBlock (juce::AudioBuffer<float>& audioBuffer);
private:
    float gain;
    float fBias;
};

class TapeMachine
{
public:
    void processBlock (juce::AudioBuffer<float>& audioBuffer);
private:
    RecordHead recHead;
};
