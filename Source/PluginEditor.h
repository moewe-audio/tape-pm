/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
/**
*/
class TapepmAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    TapepmAudioProcessorEditor (TapepmAudioProcessor&);
    ~TapepmAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    TapepmAudioProcessor& audioProcessor;
    
    juce::Slider headGapSlider;
    juce::Slider wireTurnsSlider;
    juce::Slider tapeEfficiencySlider;
    juce::Slider biasGainSlider;
    juce::Slider biasFreqSlider;
    
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> headGapAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> wireTurnAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> tapeEfficiencyAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> biasGainAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> biasFreqAttachment;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TapepmAudioProcessorEditor)
};
