/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
TapepmAudioProcessorEditor::TapepmAudioProcessorEditor (TapepmAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{

    setSize (400, 300);
    addAndMakeVisible(headGapSlider);
    addAndMakeVisible(wireTurnsSlider);
    addAndMakeVisible(tapeEfficiencySlider);
    addAndMakeVisible(biasGainSlider);
    addAndMakeVisible(biasFreqSlider);
    
    headGapSlider.setName("Record Head Gap");
    headGapSlider.setSliderStyle(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag);
    wireTurnsSlider.setName("Turns of wire");
    wireTurnsSlider.setSliderStyle(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag);
    tapeEfficiencySlider.setName("Tape efficiency");
    tapeEfficiencySlider.setSliderStyle(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag);
    biasGainSlider.setName("Bias Gain");
    biasGainSlider.setSliderStyle(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag);
    biasFreqSlider.setName("Bias Frequency");
    biasFreqSlider.setSliderStyle(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag);
    juce::AudioProcessorValueTreeState& apvts = audioProcessor.getApvts();
    headGapAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, "RECORD_HEAD_GAP", headGapSlider);
    wireTurnAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, "WIRE_TURNS", wireTurnsSlider);
    tapeEfficiencyAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, "HEAD_EFFICIENCY", tapeEfficiencySlider);
    biasGainAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, "BIAS_GAIN", biasGainSlider);
    biasFreqAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, "BIAS_FREQ", biasFreqSlider);
}

TapepmAudioProcessorEditor::~TapepmAudioProcessorEditor()
{
}

//==============================================================================
void TapepmAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

}

void TapepmAudioProcessorEditor::resized()
{
    auto area = getLocalBounds();
    headGapSlider.setBounds(area.removeFromTop(50));
    wireTurnsSlider.setBounds(area.removeFromTop(50));
    tapeEfficiencySlider.setBounds(area.removeFromTop(50));
    biasGainSlider.setBounds(area.removeFromTop(50));
    biasFreqSlider.setBounds(area.removeFromTop(50));
}
