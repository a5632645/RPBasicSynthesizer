/*
  ==============================================================================

    ContainModulableComponent.h
    Created: 4 Aug 2023 9:47:01am
    Author:  mana

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

namespace rpSynth::audio {
class ModulatorBase;
}

namespace rpSynth::ui {
class ContainModulableComponent : public juce::Component {
public:
    virtual void showModulationFrom(audio::ModulatorBase* m) = 0;
};
};