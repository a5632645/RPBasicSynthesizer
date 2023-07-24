/*
  ==============================================================================

    WrapParameter.h
    Created: 24 Jul 2023 3:19:23pm
    Author:  mana

  ==============================================================================
*/

#pragma once
#ifndef RPSYNTH_ULTIL_WRAPPARAMTER
#define RPSYNTH_ULTIL_WRAPPARAMTER

#include <JuceHeader.h>

namespace {
template<typename Type>
class WrapParameter {
public:
private:
    juce::SmoothedValue<Type> m_smoothedValue;
    
};
}

#endif // !RPSYNTH_ULTIL_WRAPPARAMTER
