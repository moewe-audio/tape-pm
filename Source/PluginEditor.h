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
    juce::Slider headTapeSpacingSlider;
    juce::Slider tapeThicknessSlider;
    juce::Slider tapeSpeedSlider;
    juce::Slider biasGainSlider;
    juce::Slider inputGainSlider;
    juce::Slider outputGainSlider;
    juce::Slider driveSlider;
    juce::Slider flutterRateSlider;
    juce::Slider flutterDepthSlider;

    std::vector<std::unique_ptr<juce::Label>> sliderLabels;
    
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> headGapAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> headTapeSpaceAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> tapeThicknessAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> tapeSpeedAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> biasGainAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> inputGainAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> outputGainAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> driveAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> flutterRateAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> flutterDepthAttachment;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TapepmAudioProcessorEditor)
};
