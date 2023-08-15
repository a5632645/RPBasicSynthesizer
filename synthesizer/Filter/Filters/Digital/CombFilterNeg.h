/*
  ==============================================================================

    CombFilterNeg.h
    Created: 15 Aug 2023 12:17:57pm
    Author:  mana

  ==============================================================================
*/

#pragma once
#include "synthesizer/Filter/FilterImplBase.h"
#include "synthesizer/dsps/PhaseShifter.h"

namespace rpSynth::audio::filters::digital {
class CombNeg : public FilterImplBase {
public:
    inline static const juce::String kName = "Comb-";

    CombNeg(AllFilterParameters& p)
        :FilterImplBase(kName)
        , m_parameters(p) {
    }

    //=========================================================================
    // implement for FilterImplBase
    void process(StereoBuffer& input,
                 StereoBuffer& output,
                 size_t begin, size_t end) override;
    void reset() override;
    void prepare(FType sampleRate, size_t numSamples) override;
    void doLayout(ui::FilterKnobsPanel&) override;
    void onCRClock(size_t) override;
    //=========================================================================
private:
    FType m_sampleRate{};
    AllFilterParameters& m_parameters;
    juce::SmoothedValue<FType> m_smoothedDelaySamples;

    // comb filter
    std::vector<PolyFType> m_delayBuffer;
    size_t m_writePosition{};
    size_t m_delayBufferSize{};
    PolyFType m_lastOutput{};

    // constant phase shifter
    dsps::PhaseShifter<FType, FType, 2> m_phaseShifter;
};
};