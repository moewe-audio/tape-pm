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
    void processBlock (juce::dsp::AudioBlock<float>& audioBuffer);
    void setGapWidth(float gapWidth) { this->gapWidth = gapWidth; };
    void setTurnsWire(float turnsWire) { this->turnsWire = turnsWire; };
    void setHeadEfficiency(float headEfficiency) { this->headEfficiency = headEfficiency; };
private:
    float gapWidth = 6.f;
    float turnsWire = 100.f;
    float headEfficiency = 0.1;
};

class BiasSignal
{
public:
    void prepareToPlay (double sampleRate, int oversampling, int samplesPerBlock);
    void processBlock (juce::dsp::AudioBlock<float>& audioBuffer);
    void setGain(float gain) { this->gain = gain; };
    void setBiasFreq(float freq);
private:
    void createWavetable();
    float getNextSample();
    
    const unsigned int tableSize = 1 << 11; // 2048
    juce::AudioSampleBuffer wavetable;
    float samplerate;
    float gain;
    float freq;
    float currentIndex = 0.0f;
    float tableDelta = 0.0f;
};

class TapeMachine
{
public:
    void prepareToPlay (double sampleRate, int oversampling, int samplesPerBlock);
    void processBlock (juce::dsp::AudioBlock<float>& audioBuffer);

    RecordHead& getRecordHead () { return recHead; };
    BiasSignal& getBiasSignal () { return bias; };
private:
    RecordHead recHead;
    BiasSignal bias;
};
