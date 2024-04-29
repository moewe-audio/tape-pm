/*
  ==============================================================================

    RecordHead.cpp
    Created: 30 Mar 2024 10:35:31am
    Author:  Levin

  ==============================================================================
*/

#include "TapeSim.h"
#include "Maths.h"


TapeMachine::TapeMachine() : recHead(userParams), hysteresis(userParams), lossEffects(userParams), playHead(userParams), hpf(juce::dsp::IIR::Coefficients<float>::makeHighPass(44100, 35.f)), flutter(userParams) { }

void TapeMachine::prepareToPlay (double sampleRate, int totalNumOutputChannels, int samplesPerBlock)
{
    oversampling = std::make_unique<juce::dsp::Oversampling<float>>(totalNumOutputChannels,
                                                                    4,
                                                                    juce::dsp::Oversampling<float>::filterHalfBandFIREquiripple,
                                                                    false);
    oversampling->reset();
    oversampling->initProcessing(samplesPerBlock);
    bias.prepareToPlay(sampleRate, 1 << 4, samplesPerBlock);
    hysteresis.prepareToPlay(sampleRate, 1 << 4, samplesPerBlock);
    lossEffects.prepareToPlay(sampleRate, samplesPerBlock);
    flutter.prepareToPlay(sampleRate, samplesPerBlock);
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = samplesPerBlock;
    spec.numChannels = 1;
    auto filterCoefficients = juce::dsp::IIR::Coefficients<float>::makeHighPass(sampleRate, 35.f);
    hpf.reset();
    hpf.prepare(spec);
    *hpf.state = *filterCoefficients;
    juce::dsp::ProcessSpec spec2;
    spec2.maximumBlockSize = samplesPerBlock * (1 << 4);
    spec2.numChannels = 1;
    spec2.sampleRate = sampleRate * (1 << 4);
    lpf.prepare(spec2);
    lpf.coefficients = juce::dsp::IIR::Coefficients<float>::makeLowPass(sampleRate * (1 << 4), 20000, 4);
}

void TapeMachine::processBlock (juce::dsp::AudioBlock<float>& audioBuffer)
{
    juce::dsp::AudioBlock<float> oversampledBlock = oversampling->processSamplesUp(audioBuffer);
    juce::dsp::AudioBlock<float> blockToEdit = oversampledBlock.getSingleChannelBlock(0);
    bias.processBlock(blockToEdit);
    recHead.processBlock(blockToEdit);
    hysteresis.processBlock(blockToEdit);
    juce::dsp::ProcessContextReplacing<float> oversampledContext(blockToEdit);
    lpf.process(oversampledContext);
    oversampling->processSamplesDown(audioBuffer);
    juce::dsp::AudioBlock<float> normalBlock = audioBuffer.getSingleChannelBlock(0);
    juce::dsp::ProcessContextReplacing<float> context(normalBlock);
    hpf.process(context);
    playHead.processBlock(normalBlock);
    lossEffects.processBlock(normalBlock);
    flutter.processBlock(normalBlock);
    auto ch1 = audioBuffer.getChannelPointer(0); // TODO Fix stereo behavior
    auto ch2 = audioBuffer.getChannelPointer(1);
    std::copy(ch1, ch1 + audioBuffer.getNumSamples(), ch2);

}

void RecordHead::processBlock (juce::dsp::AudioBlock<float>& audioBuffer)
{
    auto data = audioBuffer.getChannelPointer(0);
    for (auto i = 0; i < audioBuffer.getNumSamples(); ++i)
    {
        float gwM = userParams.gapWidth * 1.0e-6;
        float in = userParams.inputGain * data[i];
        float out = (float)(in * turnsWire * headEfficiency) / gwM;
        data[i] =  out;
    }
}

void BiasSignal::prepareToPlay (double sampleRate, int oversampling, int samplesPerBlock)
{
    this->samplerate = sampleRate * oversampling;
    freq = 55000;
    phaseIncrement = juce::MathConstants<double>::twoPi * (float) freq / (float) this->samplerate;
    phase = 0.0;
}

void BiasSignal::processBlock (juce::dsp::AudioBlock<float>& audioBuffer)
{
    auto numSamples = audioBuffer.getNumSamples();
    auto channelData = audioBuffer.getChannelPointer(0);
    float g = gain * 0.5;
    for (int sample = 0; sample < numSamples; ++sample)
    {
        const double value = std::sin(phase);
        channelData[sample] = channelData[sample] + g * value;
        phase += phaseIncrement;
        if (phase >= 2.0 * M_PI)
            phase -= 2.0 * M_PI;
    }
}

//////////////////////////////////////////////////////
//////// Hysteresis

void Hysteresis::prepareToPlay (double sampleRate, int oversampling, int samplesPerBlock)
{
    T = (double) 1.0 / (sampleRate * oversampling);
    Ms = 3.5e5;
    k = 27.0e3;
    c = 1.7e-1;
    a = 22.0e3;
 }

void Hysteresis::processBlock (juce::dsp::AudioBlock<float>& audioBuffer)
{
    auto data = audioBuffer.getChannelPointer(0);
    float drive = userParams.drive;
    for (int i = 0; i < audioBuffer.getNumSamples(); i++)
    {
        float H = data[i] * drive * 0.5;
        double dH = ((1.75 / T) * (H - H_1)) - 0.75 * dH_1;
        const double H_1_2 = (H + H_1) * 0.5;
        const double dH_1_2 = (dH + dH_1) * 0.5;
        
        double k1 = T * derivM(M_1, H_1, dH_1);
        double k2 = T * derivM(M_1 + (k1 / 2.f), H_1_2, dH_1_2);
        double k3 = T * derivM(M_1 + (k2 / 2.f), H_1_2, dH_1_2);
        double k4 = T * derivM(M_1 + k3, H, dH);
        float M = 0;
        if(k1 + k2 + k3 + k4 != 0)
        {
            M = M_1 + (k1 / 6.f) + (k2 / 3.f) + (k3 / 3.f) + (k4 / 6.f);
        }
        bool nan = std::isnan (M);
        M = nan ? 0.0 : M;
        dH = nan ? 0.0 : dH;
        data[i] = M;
        dH_1 = dH;
        H_1 = H;
        M_1 = M;
    }
};

float Hysteresis::derivM(float M, float H, float dH)
{
    double Q = (H + alpha * M) * (1.f / a);
    double cothQ = 1.0 / std::tanh(Q);
    double oneOverQ = (double) 1.0 / Q;
    double oneQSq = oneOverQ * oneOverQ;
    const auto deltaS = (double) ((dH >= 0.0) - (dH < 0.0));
    double ManMinM = cothQ - oneOverQ;
    const auto deltaM = (double) ((deltaS >= 0.f && ManMinM >= 0.f) || (deltaS < 0.f && ManMinM < 0.f));
    double LPrimeQ = (oneQSq) - (cothQ * cothQ) + 1.f;
    double cMsOverALPrime = c * Ms / a * LPrimeQ;
    double result = ((1.f - c) * deltaM) * ManMinM;
    result /= ((1.f - c) * deltaS) * k - alpha * ManMinM;
    result *= dH;
    result += cMsOverALPrime * dH;
    result /= (float) 1.f - cMsOverALPrime * alpha;
    return result;
}

///////////////////////////////////////////////////////////
///////////// PlayHead

void PlayHead::processBlock (juce::dsp::AudioBlock<float>& audioBuffer)
{
    float * data = audioBuffer.getChannelPointer(0);
    double mu0 = 4.f * M_PI * 1e-7;
    float hwM = headWidth * 0.0254;
    float tsM = userParams.tapeSpeed * 0.0254;
    float gwM = userParams.gapWidth * 1.0e-6;
    float gain = userParams.outputGain * 0.593586e7;
    for (int i = 0; i < audioBuffer.getNumSamples(); i++)
    {
        float out = turnsWire * headEfficiency * gwM * hwM * mu0 * tsM * data[i];
        out *= gain;
        data[i] = out;
    }
}

///////////////////////////////////////////////////////////
///////////// LossEffectFilter

void LossEffectFilter::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    this->samplerate = sampleRate;
    this->samplesPerBlock = samplesPerBlock;
    filter.prepare({sampleRate, (juce::uint32)samplesPerBlock, 1});
    calculateCoefficients(samplesPerBlock);
}

void LossEffectFilter::processBlock(juce::dsp::AudioBlock<float>& audioBuffer)
{
    calculateCoefficients(samplesPerBlock);
    auto data = audioBuffer.getChannelPointer(0);
    for (int i = 0; i < audioBuffer.getNumSamples(); i++)
    {
        data[i] = filter.processSample(data[i]);
    }
}

void LossEffectFilter::calculateCoefficients(int samplesPerBlock)
{
    binWidth = samplerate / (float) filterOrder;
    H.resize(filterOrder);
    float tapeSpeed = params.tapeSpeed * 0.0254; // * 0.0254 to convert from ips to meter per second
    float spacing = params.spacingTapeHead * 1.0e-6; // microns to meters
    float thickness = params.tapeThickness * 1.0e-6;
    float gap = params.gapWidth * 1.0e-6;
    for (int n = 0; n < filterOrder / 2; n++)
    {
        auto hData = H.data();
        float f = n == 0 ? 20.0 : binWidth * n;
        float k = (juce::MathConstants<float>::twoPi * f) / (tapeSpeed);
        float magnitude = 0;
        magnitude = exp(-k * spacing);
        float kThickness = k * thickness;
        magnitude *= (1 - exp(-kThickness)) / kThickness;
        float kGapHalf = k * gap * 0.5;
        magnitude *= sin(kGapHalf) / kGapHalf;
        hData[n] = {magnitude, 0};
        hData[filterOrder - n - 1] = {magnitude, 0};
    }
    timeDomainData.clear();
    timeDomainData = std::vector<std::complex<float>>();
    timeDomainData.resize(filterOrder);
    fft->perform(H.getRawDataPointer(), timeDomainData.data(), true);
    coefficients.clear();
    coefficients.ensureStorageAllocated(filterOrder);
    for (int i = 0; i < filterOrder; i++)
    {
        coefficients.add(timeDomainData[i].real());
    }
    coef = new juce::dsp::FIR::Coefficients<float>(coefficients.getRawDataPointer(), coefficients.size());
    filter.coefficients = coef.get();
}
