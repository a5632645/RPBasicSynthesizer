/*
  ==============================================================================

    Phaser.h
    Created: 8 Aug 2023 12:43:38pm
    Author:  mana

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include "synthesizer/Effects/EffectProcessorBase.h"
#include "synthesizer/WrapParameter.h"

namespace rpSynth::audio::effects {
class PhaserPanel;
struct PhaserParameters;
class PhaserImpl;

class Phaser : public EffectProcessorBase {
public:
    Phaser(OrderableEffectsChain&);

    ~Phaser() override;

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
    friend class PhaserPanel;
    std::unique_ptr<PhaserParameters> m_allFlangerParameters;
    std::unique_ptr<PhaserImpl> m_flangerImpl;
};
}