/*
  ==============================================================================

    Flanger.h
    Created: 8 Aug 2023 9:38:15am
    Author:  mana

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include "synthesizer/Effects/EffectProcessorBase.h"
#include "synthesizer/WrapParameter.h"

namespace rpSynth::audio::effects {
class FlangerPanel;
struct FlangerParameters;
class FlangerImpl;

class Flanger : public EffectProcessorBase {
public:
    Flanger(OrderableEffectsChain&);

    ~Flanger() override;

    //================================================================================
    // implement for EffectProcessorBase
    //================================================================================
    void addParameterToLayout(juce::AudioProcessorValueTreeState::ParameterLayout& layout) override;

    void updateParameters(size_t numSamples) override;

    void prepareParameters(FType sampleRate, size_t numSamples) override;

    void prepare(FType sampleRate, size_t numSamlpes) override;

    void saveExtraState(juce::XmlElement& xml) override;

    void loadExtraState(juce::XmlElement& xml, juce::AudioProcessorValueTreeState& apvts) override;

    void processBlock(StereoBuffer& block, size_t begin, size_t end) override;

    std::unique_ptr<ui::ContainModulableComponent> createEffectPanel() override;

private:
    //================================================================================
    // ²ÎÊý
    //================================================================================
    friend class FlangerPanel;
    std::unique_ptr<FlangerParameters> m_allFlangerParameters;
    std::unique_ptr<FlangerImpl> m_flangerImpl;
};
}