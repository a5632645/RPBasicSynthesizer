/*
  ==============================================================================

    HighPass.h
    Created: 5 Aug 2023 10:54:54pm
    Author:  mana

  ==============================================================================
*/

#pragma once
#include "synthesizer/Filter/FilterImplBase.h"
#include "LowPass.h"

namespace rpSynth::audio {
class MyAudioProcessParameter;
}

namespace rpSynth::audio::filters {
class HighPass : public FilterImplBase {
public:
    inline static const juce::String kName = "HighPass";

    HighPass(AllFilterParameters& p)
        :FilterImplBase(kName)
        , lowpass(p) {
    }

    //=========================================================================
    // implement for FilterImplBase
    virtual void process(rpSynth::audio::StereoBuffer& input,
                         rpSynth::audio::StereoBuffer& output,
                         size_t begin, size_t end) override;
    virtual void reset() override;
    virtual void prepare(rpSynth::audio::FType sampleRate, size_t numSamples) override;
    void doLayout(ui::FilterKnobsPanel&) override;
    //=========================================================================
private:
    LowPass lowpass;
private:
};
};