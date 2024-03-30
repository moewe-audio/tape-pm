/*
  ==============================================================================

    RecordHead.cpp
    Created: 30 Mar 2024 10:35:31am
    Author:  Levin

  ==============================================================================
*/

#include "TapeSim.h"

void TapeMachine::processBlock (juce::AudioBuffer<float>& audioBuffer)
{
    recHead.processBlock(audioBuffer);
}

void RecordHead::processBlock (juce::AudioBuffer<float>& audioBuffer)
{
    auto read = audioBuffer.getReadPointer(0);
    auto write = audioBuffer.getWritePointer(0);
    for (auto i = 0; i < audioBuffer.getNumSamples(); ++i)
    {
        write[i] = (turnsWire * headEfficiency * read[i]) / gapWidth;
    }
}
