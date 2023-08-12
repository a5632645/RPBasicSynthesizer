/*
  ==============================================================================

    LowPass.h
    Created: 5 Aug 2023 9:36:40pm
    Author:  mana

  ==============================================================================
*/

#pragma once
#include "synthesizer/Filter/FilterImplBase.h"
#include "Filter.h"

namespace rpSynth::audio::filters {
class LowPass : public FilterImplBase {
public:
    inline static const juce::String kName = "LowPass";

    LowPass(AllFilterParameters& p)
        :FilterImplBase(kName)
        , m_parameters(p) {
    }

    //=========================================================================
    // implement for FilterImplBase
    void process(rpSynth::audio::StereoBuffer& input,
                 rpSynth::audio::StereoBuffer& output,
                 size_t begin, size_t end) override;
    void reset() override;
    void prepare(rpSynth::audio::FType sampleRate, size_t numSamples) override;
    void doLayout(ui::FilterKnobsPanel&) override;
    //=========================================================================
private:
    FType m_oneDivNyquistRate;
    Filter LF;
    Filter RF;
private:
    AllFilterParameters& m_parameters;
};
};