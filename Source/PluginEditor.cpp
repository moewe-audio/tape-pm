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

    setSize (300, 500);
    addAndMakeVisible(headGapSlider);
    addAndMakeVisible(headTapeSpacingSlider);
    addAndMakeVisible(tapeThicknessSlider);
    addAndMakeVisible(biasGainSlider);
    addAndMakeVisible(inputGainSlider);
    addAndMakeVisible(tapeSpeedSlider);
    addAndMakeVisible(outputGainSlider);
    addAndMakeVisible(driveSlider);
    addAndMakeVisible(flutterRateSlider);
    addAndMakeVisible(flutterDepthSlider);
    
    headGapSlider.setName("Head Gap");
    headGapSlider.setSliderStyle(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag);
    headTapeSpacingSlider.setName("Head to tape spacing");
    headTapeSpacingSlider.setSliderStyle(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag);
    tapeThicknessSlider.setName("Tape thickness");
    tapeThicknessSlider.setSliderStyle(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag);
    tapeSpeedSlider.setName("Tape speed");
    tapeSpeedSlider.setSliderStyle(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag);
    inputGainSlider.setName("Input Gain");
    inputGainSlider.setSliderStyle(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag);
    outputGainSlider.setName("Output Gain");
    outputGainSlider.setSliderStyle(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag);
    driveSlider.setName("Drive");
    driveSlider.setSliderStyle(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag);
    biasGainSlider.setName("Bias Gain");
    biasGainSlider.setSliderStyle(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag);
    
    flutterRateSlider.setName("Flutter Rate");
    flutterRateSlider.setSliderStyle(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag);
    flutterDepthSlider.setName("Flutter Depth");
    flutterDepthSlider.setSliderStyle(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag);
    
    for (auto* child : getChildren())
    {
        if (child != nullptr) {
            if (juce::Slider* slider = dynamic_cast<juce::Slider*>(child))
            {
                auto label = std::make_unique<juce::Label>();
                label->setText(slider->getName(), juce::dontSendNotification);
                label->attachToComponent(slider, true);
                addAndMakeVisible(label.get());
                sliderLabels.push_back(std::move(label));
            }
        }
    }
    
    juce::AudioProcessorValueTreeState& apvts = audioProcessor.getApvts();
    headGapAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, "HEAD_GAP", headGapSlider);
    headTapeSpaceAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, "HEAD_TAPE_SPACING", headTapeSpacingSlider);
    tapeThicknessAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, "TAPE_THICKNESS", tapeThicknessSlider);
    tapeSpeedAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, "TAPE_SPEED", tapeSpeedSlider);
    biasGainAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, "BIAS_GAIN", biasGainSlider);
    inputGainAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, "INPUT_GAIN", inputGainSlider);
    outputGainAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, "OUTPUT_GAIN", outputGainSlider);
    driveAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, "DRIVE", driveSlider);
    flutterRateAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, "FLUTTER_RATE", flutterRateSlider);
    flutterDepthAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, "FLUTTER_DEPTH", flutterDepthSlider);
    
}

TapepmAudioProcessorEditor::~TapepmAudioProcessorEditor()
{
    
}

//==============================================================================
void TapepmAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
    g.setColour(juce::Colours::white);
}

void TapepmAudioProcessorEditor::resized()
{
    auto area = getLocalBounds();
    auto labelArea = area.removeFromLeft(150);
    for (int i = 0; i < sliderLabels.size(); i++)
    {
        auto subArea = labelArea.removeFromTop(50);
        auto label = sliderLabels[i].get();
        label->setBounds(subArea);
    }
    
    inputGainSlider.setBounds(area.removeFromTop(50));
    headGapSlider.setBounds(area.removeFromTop(50));
    headTapeSpacingSlider.setBounds(area.removeFromTop(50));
    tapeThicknessSlider.setBounds(area.removeFromTop(50));
    tapeSpeedSlider.setBounds(area.removeFromTop(50));
    biasGainSlider.setBounds(area.removeFromTop(50));
    driveSlider.setBounds(area.removeFromTop(50));
    flutterRateSlider.setBounds(area.removeFromTop(50));
    flutterDepthSlider.setBounds(area.removeFromTop(50));
    outputGainSlider.setBounds(area.removeFromTop(50));
}
