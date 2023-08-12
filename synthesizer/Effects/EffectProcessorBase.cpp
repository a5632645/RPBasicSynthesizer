/*
  ==============================================================================

    EffectProcessorBase.cpp
    Created: 8 Aug 2023 10:37:39am
    Author:  mana

  ==============================================================================
*/

#include "EffectProcessorBase.h"
#include "synthesizer/Effects/OrderableEffectsChain.h"

rpSynth::audio::effects::EffectProcessorBase::EffectProcessorBase(OrderableEffectsChain& c, const juce::String& ID)
    : AudioProcessorBase(c.combineWithID(ID))
    , chain(c)
    , m_effectName(ID) {
}

void rpSynth::audio::effects::EffectProcessorBase::process(size_t beginSamplePos, size_t endSamplePos){ 
    if (notBypass->get()) {
        processBlock(*chain.getChainOutput(), beginSamplePos, endSamplePos);
    }
}
