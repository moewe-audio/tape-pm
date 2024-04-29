/*
  ==============================================================================

    RecordHead.h
    Created: 30 Mar 2024 10:35:31am
    Author:  Levin

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "Parameters.h"
#include "ModDelay.h"

class RecordHead
{
public:
    RecordHead(UserParameters& userParams) : userParams(userParams) { };
    void processBlock (juce::dsp::AudioBlock<float>& audioBuffer);
private:
    float turnsWire = 100.f;
    float headEfficiency = 0.1;
    UserParameters& userParams;
};

class BiasSignal
{
public:
    void prepareToPlay (double sampleRate, int oversampling, int samplesPerBlock);
    void processBlock (juce::dsp::AudioBlock<float>& audioBuffer);
    void setGain(float gain) { this->gain = gain; };
private:
    float samplerate;
    float gain;
    float freq;
    double phase;
    double phaseIncrement;

};

class Hysteresis
{
public:
    Hysteresis(UserParameters& params) : userParams(params) {};
    void prepareToPlay (double sampleRate, int oversampling, int samplesPerBlock);
    void processBlock (juce::dsp::AudioBlock<float>& audioBuffer);
private:
    float derivM(float M, float H, float dH);
    
    double Ms = 1.0;
    double a = Ms / 4.0;
    double c = 1.7e-1;
    double k = 0.47875;
    double alpha = 1.6e-3;
    float H_1 = 0;
    float dH_1 = 0;
    float M_1 = 0;
    double T;
    UserParameters& userParams;
};

class PlayHead
{
public:
    PlayHead(UserParameters& params) : userParams(params) { };
    void processBlock (juce::dsp::AudioBlock<float>& audioBuffer);
private:
    UserParameters& userParams;
    float headWidth = 0.125;
    float turnsWire = 100.f;
    float headEfficiency = 0.1;
};

class LossEffectFilter
{
public:
    LossEffectFilter(UserParameters& userParams) : params(userParams) {
        fft = std::make_unique<juce::dsp::FFT>(7);
        filter = juce::dsp::FIR::Filter<float>();
    };
    void prepareToPlay(double sampleRate, int samplesPerBlock);
    void processBlock(juce::dsp::AudioBlock<float>& audioBuffer);
    void calculateCoefficients(int samplesPerBlock);
private:
    float samplerate;
    int filterOrder = 1 << 7;
    float binWidth;
    juce::Array<std::complex<float>> H;
    juce::dsp::FIR::Coefficients<float>::Ptr coef;
    UserParameters &params;
    juce::dsp::FIR::Filter<float> filter;
    juce::Array<float> coefficients;
    std::unique_ptr<juce::dsp::FFT> fft;
    std::vector<std::complex<float>> timeDomainData;
    int samplesPerBlock;
};

class TapeMachine
{
public:
    TapeMachine();
    void prepareToPlay (double sampleRate, int totalNumOutputChannels, int samplesPerBlock);
    void processBlock (juce::dsp::AudioBlock<float>& audioBuffer);

    RecordHead& getRecordHead () { return recHead; };
    BiasSignal& getBiasSignal () { return bias; };
    UserParameters& getUserParams() { return userParams; };
    
    void setUserParams(UserParameters &userParams) { this->userParams = userParams; };
private:
    std::unique_ptr<juce::dsp::Oversampling<float>> oversampling;
    RecordHead recHead;
    BiasSignal bias;
    Hysteresis hysteresis;
    LossEffectFilter lossEffects;
    PlayHead playHead;
    juce::dsp::ProcessorDuplicator <juce::dsp::IIR::Filter<float>, juce::dsp::IIR::Coefficients <float>> hpf;
    juce::dsp::IIR::Filter<float> lpf;
    UserParameters userParams;
    ModDelay flutter;
};
