/*
  ==============================================================================

    LowPass.cpp
    Created: 5 Aug 2023 9:36:40pm
    Author:  mana

  ==============================================================================
*/

#include "LowPass.h"
#include "concepts.h"
#include "synthesizer/WrapParameter.h"
#include "synthesizer/Filter/AllFilterParameters.h"
#include "ui/filter/FilterPanel.h"

namespace rpSynth::audio::filters {
void LowPass::process(rpSynth::audio::StereoBuffer& input, rpSynth::audio::StereoBuffer& output, size_t begin, size_t end) {
    m_parameters.cutoff.applySemitoneToHertz(begin, end);
    for (size_t i = begin; i < end; i++) {
        FType cutoff = m_parameters.cutoff.getRaw(i) * m_oneDivNyquistRate;
        FType res = m_parameters.resonance.get(i);
        FType lv = m_parameters.limitVolume.get(i);
        FType lk = m_parameters.limitK.get(i);

        output.left[i] = LF.LPF2_ResoLimit_limit(input.left[i], cutoff, res, lv,lk);
        output.right[i] = RF.LPF2_ResoLimit_limit(input.right[i], cutoff, res, lv, lk);
    }
}

void LowPass::reset() {
    LF.reset();
    RF.reset();
}

void LowPass::prepare(rpSynth::audio::FType sampleRate, size_t /*numSamples*/) {
    m_oneDivNyquistRate = 2 / sampleRate;
}

void LowPass::doLayout(ui::FilterKnobsPanel& p) {
    p.m_cutoff.setVisible(true);
    p.m_resonance.setVisible(true);
    p.m_limitVolume.setVisible(true);
    p.m_limitK.setVisible(true);

    p.m_cutoff.setBounds(0, 0, 70, 70);
    p.m_resonance.setBounds(80, 0, 70, 70);
    p.m_limitVolume.setBounds(160, 0, 70, 70);
    p.m_limitK.setBounds(0, 80, 70, 70);
}
}