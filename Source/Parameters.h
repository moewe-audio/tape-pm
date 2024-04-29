/*
  ==============================================================================

    Parameters.h
    Created: 10 Apr 2024 2:24:44pm
    Author:  Levin

  ==============================================================================
*/

#pragma once

class UserParameters
{
public:
    float spacingTapeHead = 20; // Microns
    float gapWidth = 4; // Microns
    float tapeThickness = 35; // Microns
    float tapeSpeed = 15; //Inch per second
    float inputGain = 1.f;
    float outputGain = 1.f;
    float drive = 0.5f;
    // Flutter
    float flutterRate = 0.0;
    float flutterDepth = 0.0;
    
};
