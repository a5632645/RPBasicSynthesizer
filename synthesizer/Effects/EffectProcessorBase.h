/*
  ==============================================================================

    EffectProcessorBase.h
    Created: 7 Aug 2023 3:39:24pm
    Author:  mana

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

#include "synthesizer/AudioProcessorBase.h"
#include "ui/ContainModulableComponent.h"

namespace rpSynth::audio {
class OrderableEffectsChain;
}

namespace rpSynth::audio::effects {
class EffectProcessorBase : public AudioProcessorBase {
public:
    EffectProcessorBase(OrderableEffectsChain& c, const juce::String& ID);
    virtual ~EffectProcessorBase() = default;
    virtual void processBlock(StereoBuffer& block, size_t begin, size_t end) = 0;
    virtual std::unique_ptr<ui::ContainModulableComponent> createEffectPanel() = 0;

    void process(size_t beginSamplePos, size_t endSamplePos) override;
    virtual void addParameterToLayout(juce::AudioProcessorValueTreeState::ParameterLayout& layout) override;
    const juce::String& getEffectName() const { return m_effectName; }
public:
    juce::AudioParameterBool* notBypass;
private:
    OrderableEffectsChain& chain;
    juce::String m_effectName;
};

inline void EffectProcessorBase::addParameterToLayout(juce::AudioProcessorValueTreeState::ParameterLayout& layout) {
    auto b = std::make_unique<juce::AudioParameterBool>(combineWithID("enable"),
                                                        combineWithID("enable"),
                                                        false);
    notBypass = b.get();
    layout.add(std::move(b));
}
}

#define EFFECTS_NAMESPACE rpSynth::audio::effects