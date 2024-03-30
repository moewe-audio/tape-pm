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

class Hysteresis
{
public:
    void prepareToPlay (double sampleRate, int oversampling, int samplesPerBlock);
    void processBlock (juce::dsp::AudioBlock<float>& audioBuffer);
private:
    float derivM(float m, float h, float h_1, float dH);
    int calculateDeltaS(float currentH, float previousH);
    int calculateDeltaM(float m, float h, int deltaS);
    float calculateMAn (float h, float m);
    float langevin(float x);
    float langevinPrime(float x);
    
    float tanh (float x);
    
    float saturation = 3.5e5;
    float anhystericMag = 22;
    float susceptibilityRatio = 1.7e-1;
    float coercity = 27.f;
    float meanField = 1.6e-3;
    float h_1 = 0;
    float dH_1 = 0;
    float m_1 = 0;
    float period;
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
    Hysteresis hysteresis;
};
