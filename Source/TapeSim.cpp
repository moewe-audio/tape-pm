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
    hysteresis.prepareToPlay(sampleRate, oversampling, samplesPerBlock);
}

void TapeMachine::processBlock (juce::dsp::AudioBlock<float>& audioBuffer)
{
    bias.processBlock(audioBuffer);
    recHead.processBlock(audioBuffer);
    hysteresis.processBlock(audioBuffer);
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

//////////////////////////////////////////////////////
//////// Hysteresis

void Hysteresis::prepareToPlay (double sampleRate, int oversampling, int samplesPerBlock)
{
    period = 1 / (sampleRate * oversampling);
}

void Hysteresis::processBlock (juce::dsp::AudioBlock<float>& audioBuffer)
{
    float m = 0;
    auto data = audioBuffer.getChannelPointer(0);
    
    for (int i = 0; i < audioBuffer.getNumSamples(); i++)
    {
        float h = data[i];
        float dH = 2.f * (h - h_1) / period - dH_1;
        
        float dHHalf = (dH - dH_1) / 2.f;
        
        float k1 = period * derivM(m_1, h, h_1, dH_1);
        float k2 = period * derivM(m_1 + (k1 / 2), h, h_1, dHHalf);
        float k3 = period * derivM(m_1 + (k2 / 2), h, h_1, dHHalf);
        float k4 = period * derivM(m_1 + k3, h, h_1, dH);
        
        m = m_1 + (k1 / 6) + (k2 / 3) + (k3 / 3) + (k4 / 6);
        
        data[i] = m;
        
        dH_1 = dH;
        h_1 = h;
        m_1 = m;
    }
    
};

float Hysteresis::derivM(float m, float h, float h_1, float dH)
{
    float dM = 0;
    float deltaS = calculateDeltaS(h, h_1);
    float deltaM = calculateDeltaM(m, h, deltaS);
    float mAn = calculateMAn(h, m);
    float lPrimeQ = langevinPrime((h + meanField * m) / anhystericMag);
    dM = (1 - susceptibilityRatio) * deltaM * (mAn - m);
    dM /= (1 - susceptibilityRatio) * deltaS * coercity - meanField * (mAn - m);
    dM = dM * dH + susceptibilityRatio * (saturation / anhystericMag) * dH * lPrimeQ;
    dM /= 1 - susceptibilityRatio * meanField * (saturation / anhystericMag) * lPrimeQ;
    return dM;
}

int Hysteresis::calculateDeltaS(float currentH, float previousH)
{
    return (currentH > previousH) ? 1 : -1;
}

int Hysteresis::calculateDeltaM(float m, float h, int deltaS)
{
    if ((calculateMAn(h, m) - h) * deltaS > 0) {
        return 1;
    } else {
        return 0;
    }
}

float Hysteresis::langevin(float x) {
    if (std::abs(x) > 1e-4) {
        return (1 / tanh(x)) - 1 / x;
    } else {
        return x / 3;
    }
}

float Hysteresis::calculateMAn (float h, float m)
{
    return saturation * langevin((h + meanField * m) / anhystericMag);
}

float Hysteresis::langevinPrime(float x) {
    if (std::abs(x) > 1e-4) {
        double cothx = 1 / tanh(x);
        return 1 / (x * x) - (cothx * cothx) + 1;
    } else {
        return 1 / 3.0;
    }
}

float Hysteresis::tanh (float x)
{
    float result = 0.0;
    float xSquared = x * x;
    float denom = 4 * 2 - 1;
    for (int i = 4; i > 0; --i) {
        result = xSquared / (denom + result);
        denom -= 2.0;
    }
    result = x / (1 + result);

    return result;
}
