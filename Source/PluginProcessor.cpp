/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
TapepmAudioProcessor::TapepmAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ), apvts(*this, nullptr, "PARAMETERS", createParameters())
#endif
{
}

TapepmAudioProcessor::~TapepmAudioProcessor()
{
}

//==============================================================================
const juce::String TapepmAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool TapepmAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool TapepmAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool TapepmAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double TapepmAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int TapepmAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int TapepmAudioProcessor::getCurrentProgram()
{
    return 0;
}

void TapepmAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String TapepmAudioProcessor::getProgramName (int index)
{
    return {};
}

void TapepmAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void TapepmAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    oversampling = std::make_unique<juce::dsp::Oversampling<float>>(getTotalNumOutputChannels(),
                                                                    4,
                                                                    juce::dsp::Oversampling<float>::filterHalfBandFIREquiripple,
                                                                    false);
    oversampling->reset();
    oversampling->initProcessing(samplesPerBlock);
    tapeMachine.prepareToPlay(sampleRate, 1 << 4, samplesPerBlock);
    startTimerHz(10);
}

void TapepmAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool TapepmAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void TapepmAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());
    
    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::AudioBlock<float> oversampledBlock = oversampling->processSamplesUp(block);

    
    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        tapeMachine.processBlock(oversampledBlock);
    }
    oversampling->processSamplesDown(block);
}

//==============================================================================
bool TapepmAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* TapepmAudioProcessor::createEditor()
{
    return new TapepmAudioProcessorEditor (*this);
}

//==============================================================================
void TapepmAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void TapepmAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new TapepmAudioProcessor();
}

void TapepmAudioProcessor::timerCallback()
{
    auto recHeadGapPar = apvts.getRawParameterValue("RECORD_HEAD_GAP");
    float recHeadGap = recHeadGapPar->load();
    auto wireTurnsPar = apvts.getRawParameterValue("WIRE_TURNS");
    float wireTurns = wireTurnsPar->load();
    auto headEfficiencyPar = apvts.getRawParameterValue("HEAD_EFFICIENCY");
    float headEfficiency = headEfficiencyPar->load();
    RecordHead& recordHead = tapeMachine.getRecordHead();
    recordHead.setGapWidth(recHeadGap);
    recordHead.setTurnsWire(wireTurns);
    recordHead.setHeadEfficiency(headEfficiency);
    
    auto biasGainPar = apvts.getRawParameterValue("BIAS_GAIN");
    float biasGain = biasGainPar->load();
    auto biasFreqPar = apvts.getRawParameterValue("BIAS_FREQ");
    float biasFreq = biasFreqPar->load();
    BiasSignal& biasSignal = tapeMachine.getBiasSignal();
    biasSignal.setGain(biasGain);
    biasSignal.setBiasFreq(biasFreq);
}

juce::AudioProcessorValueTreeState::ParameterLayout TapepmAudioProcessor::createParameters()
{
    std::vector<std::unique_ptr<juce::AudioProcessorParameterGroup>> params;

    auto recordGroup = std::make_unique<juce::AudioProcessorParameterGroup>("RECORD_HEAD", "REC_GROUP", "|");
    recordGroup->addChild(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID { "RECORD_HEAD_GAP",  1 }, "Record head gap", 2.5, 12.f, 6.f));
    recordGroup->addChild(std::make_unique<juce::AudioParameterInt>(juce::ParameterID { "WIRE_TURNS",  1 }, "Turns of wire", 50, 150, 100));
    recordGroup->addChild(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID { "HEAD_EFFICIENCY",  1 }, "Head Efficiency", 0.05, 0.15, 0.1));
    params.push_back(std::move(recordGroup));
    
    auto biasGroup = std::make_unique<juce::AudioProcessorParameterGroup>("BIAS", "BIAS_GROUP", "|");
    biasGroup->addChild(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID { "BIAS_GAIN",  1 }, "Bias Gain", 0, 8, 5.f));
    biasGroup->addChild(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID { "BIAS_FREQ",  1 }, "Bias Freq", 2000.f, 60000.f, 55000.f));
    params.push_back(std::move(biasGroup));
    
    return { params.begin(), params.end() };
}
