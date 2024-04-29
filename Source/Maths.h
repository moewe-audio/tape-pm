/*
  ==============================================================================

    Maths.h
    Created: 15 Apr 2024 12:21:35pm
    Author:  Levin

  ==============================================================================
*/

#pragma once

static float interpolate (float x1, float x2, float dist)
{
    return (1.f - dist) * x1 + dist * x2;
}

static float mod(float a, float b) {
    double result = std::fmod(a, b);
    if (result < 0) {
        result += b;
    }
    return result;
}
