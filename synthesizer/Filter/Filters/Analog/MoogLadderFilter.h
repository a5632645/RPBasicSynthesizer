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
        float c_az1;
        float c_az2;
        float c_az3;
        float c_az4;
        float c_az5;
        float c_ay1;
        float c_ay2;
        float c_ay3;
        float c_ay4;
        float c_amf;
    };
    Coeffects c_left;
    Coeffects c_right;
    //================================================================================
    forcedinline void processLeft(FType* input, FType* output, size_t begin, size_t end);
    forcedinline void processRight(FType* input, FType* output, size_t begin, size_t end);
private:
    AllFilterParameters& m_parameters;
};
};