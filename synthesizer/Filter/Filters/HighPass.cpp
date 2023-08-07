/*
  ==============================================================================

    HighPass.cpp
    Created: 5 Aug 2023 10:54:54pm
    Author:  mana

  ==============================================================================
*/

#include "HighPass.h"
#include "ui/filter/FilterPanel.h"

namespace rpSynth::audio::filters {
void HighPass::process(rpSynth::audio::StereoBuffer& input, rpSynth::audio::StereoBuffer& output, size_t begin, size_t end) {
    lowpass.process(input, output, begin, end);

    for (size_t i = begin; i < end; i++) {
        output.left[i] = input.left[i] - output.left[i];
        output.right[i] = input.right[i] - output.right[i];
    }
}

void HighPass::reset() {
    lowpass.reset();
}

void HighPass::prepare(rpSynth::audio::FType sampleRate, size_t numSamples) {
    lowpass.prepare(sampleRate, numSamples);
}

void HighPass::doLayout(ui::FilterKnobsPanel& p) {
    lowpass.doLayout(p);
}
}