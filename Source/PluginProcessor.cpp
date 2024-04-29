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
    tapeMachine.prepareToPlay(sampleRate, getTotalNumOutputChannels(), samplesPerBlock);
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
    
    tapeMachine.processBlock(block);
    
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
    auto headGapPar = apvts.getRawParameterValue("HEAD_GAP");
    float headGap = headGapPar->load();
    auto headSpacingPar = apvts.getRawParameterValue("HEAD_TAPE_SPACING");
    float headSpacing = headSpacingPar->load();
    auto tapeThicknessPar = apvts.getRawParameterValue("TAPE_THICKNESS");
    float tapeThickness = tapeThicknessPar->load();
    auto tapeSpeedPar = apvts.getRawParameterValue("TAPE_SPEED");
    float tapeSpeed = tapeSpeedPar->load();
    auto inputGainPar = apvts.getRawParameterValue("INPUT_GAIN");
    float inputGain = inputGainPar->load();
    auto outputGainPar = apvts.getRawParameterValue("OUTPUT_GAIN");
    float outputGain = outputGainPar->load();
    auto drivePar = apvts.getRawParameterValue("DRIVE");
    float drive = drivePar->load();
    
    auto flutterRatePar = apvts.getRawParameterValue("FLUTTER_RATE");
    float flutterRate = flutterRatePar->load();
    auto flutterDepthPar = apvts.getRawParameterValue("FLUTTER_DEPTH");
    float flutterDepth = flutterDepthPar->load();
    
    UserParameters& params = tapeMachine.getUserParams();
    params.drive = drive;
    params.gapWidth = headGap;
    params.spacingTapeHead = headSpacing;
    params.tapeSpeed = tapeSpeed;
    params.tapeThickness = tapeThickness;
    params.inputGain = inputGain;
    params.flutterRate = flutterRate;
    params.flutterDepth = flutterDepth;
    params.outputGain = outputGain;
    auto biasGainPar = apvts.getRawParameterValue("BIAS_GAIN");
    float biasGain = biasGainPar->load();
    BiasSignal& biasSignal = tapeMachine.getBiasSignal();
    biasSignal.setGain(biasGain);
}

juce::AudioProcessorValueTreeState::ParameterLayout TapepmAudioProcessor::createParameters()
{
    std::vector<std::unique_ptr<juce::AudioProcessorParameterGroup>> params;

    auto headGroup = std::make_unique<juce::AudioProcessorParameterGroup>("HEAD", "HEAD_GROUP", "|");
    headGroup->addChild(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID { "INPUT_GAIN",  1 }, "Input Gain", 0.00, 1.5, 1.00f));
    headGroup->addChild(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID { "HEAD_GAP",  1 }, "Head gap", 2.5, 12.f, 6.f));
    headGroup->addChild(std::make_unique<juce::AudioParameterInt>(juce::ParameterID { "WIRE_TURNS",  1 }, "Turns of wire", 50, 150, 100));
    headGroup->addChild(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID { "HEAD_EFFICIENCY",  1 }, "Head Efficiency", 0.05, 0.15, 0.1));
    headGroup->addChild(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID { "HEAD_TAPE_SPACING",  1 }, "Head to tape spacing", 0.01, 50, 20));
    headGroup->addChild(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID { "TAPE_THICKNESS",  1 }, "Tape thickness", 1.f, 50.f, 35.f));
    headGroup->addChild(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID { "TAPE_SPEED",  1 }, "Tape Speed", 5, 30, 15));
    headGroup->addChild(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID { "OUTPUT_GAIN",  1 }, "Output Gain", 0.00, 2, 1.00f));
    headGroup->addChild(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID { "DRIVE",  1 }, "Drive", 0.00f, 1.0f, 0.50f));
    params.push_back(std::move(headGroup));
    
    auto biasGroup = std::make_unique<juce::AudioProcessorParameterGroup>("BIAS", "BIAS_GROUP", "|");
    biasGroup->addChild(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID { "BIAS_GAIN",  1 }, "Bias Gain", 0, 3.f, 1.f));
    params.push_back(std::move(biasGroup));
    
    auto flutterGroup = std::make_unique<juce::AudioProcessorParameterGroup>("FLUTTER", "FLUTTER_GROUP", "|");
    flutterGroup->addChild(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID { "FLUTTER_RATE",  1 }, "Flutter Rate", 0.0, 20.0, 0.f));
    flutterGroup->addChild(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID { "FLUTTER_DEPTH",  1 }, "Flutter DEPTH", 0.0, 0.4, 0.f));
    params.push_back(std::move(flutterGroup));
    
    return { params.begin(), params.end() };
}
