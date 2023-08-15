/*
  ==============================================================================

    MoogLadderFilter.h
    Created: 6 Aug 2023 10:39:09pm
    Author:  mana

  ==============================================================================
*/

#pragma once
#include "synthesizer/Filter/FilterImplBase.h"

namespace rpSynth::audio::filters::analog {
class MoogLadderFilter : public FilterImplBase {
public:
    inline static const juce::String kName = "MoogLadder";

    MoogLadderFilter(AllFilterParameters& p)
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
    FType m_oneDivSampleRate;
    FType m_oneDivNyquistRate;

    //================================================================================
    // Filter coeffects
    struct Coeffects {
        PolyFType c_az1;
        PolyFType c_az2;
        PolyFType c_az3;
        PolyFType c_az4;
        PolyFType c_az5;
        PolyFType c_ay1;
        PolyFType c_ay2;
        PolyFType c_ay3;
        PolyFType c_ay4;
        PolyFType c_amf;
    }m_coeffect;
private:
    AllFilterParameters& m_parameters;
};
};