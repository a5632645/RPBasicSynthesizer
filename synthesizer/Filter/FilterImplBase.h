/*
  ==============================================================================

    FilterImplBase.h
    Created: 5 Aug 2023 7:53:46pm
    Author:  mana

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include "synthesizer/types.h"

namespace rpSynth::ui {
class FilterKnobsPanel;
}

namespace rpSynth::audio {
class MyAudioProcessParameter;
}

namespace rpSynth::audio::filters {
struct AllFilterParameters;
}

namespace rpSynth::audio::filters {
class FilterImplBase {
public:
    virtual ~FilterImplBase() = default;
    //=========================================================================
    // Filter name
    FilterImplBase(const juce::String& name = "_InvalidFilter_") : m_filterName(name) {}
    const juce::String& getFilterName() { return m_filterName; }
    //=========================================================================

    //=========================================================================
    // interfaces
    virtual void process(StereoBuffer& /*input*/,
                         StereoBuffer& /*output*/,
                         size_t /*begin*/, size_t /*end*/) {
    }
    virtual void reset() {}
    virtual void prepare(FType /*sampleRate*/, size_t /*numSamples*/) {}
    virtual void doLayout(ui::FilterKnobsPanel&) {}
    virtual void onCRClock(size_t) {}
    //=========================================================================
private:
    juce::String m_filterName;
};
};