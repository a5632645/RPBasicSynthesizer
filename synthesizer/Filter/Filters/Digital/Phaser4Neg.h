/*
  ==============================================================================

    Phaser4Neg.h
    Created: 15 Aug 2023 2:24:23pm
    Author:  mana

  ==============================================================================
*/

#pragma once
#include "synthesizer/Filter/FilterImplBase.h"
#include "synthesizer/dsps/PhaseShifter.h"
#include "synthesizer/dsps/AllPassFilter.h"

namespace rpSynth::audio::filters::digital {
class Phaser4Neg : public FilterImplBase {
public:
    inline static const juce::String kName = "Phaser-";
    inline static constexpr size_t kNotches = 4;
    Phaser4Neg(AllFilterParameters& p)
        : FilterImplBase(kName)
        , m_parameters(p) {
    }

    void process(StereoBuffer& input,
                 StereoBuffer& output,
                 size_t begin, size_t end) override;
    void reset() override;
    void prepare(FType sampleRate, size_t numSamples) override;
    void doLayout(ui::FilterKnobsPanel&) override;
private:
    using APF = SecondOrderAllPassFilter2<FType, PolyFType>;

    AllFilterParameters& m_parameters;
    FType m_sampleRate{};
    PolyFType m_fbValue{};
    APF::Coeffects m_sharedCoeffects;
    std::array<APF, kNotches> m_apfs;
    dsps::PhaseShifter<FType, FType, 2> m_phaseShifter;
};
}