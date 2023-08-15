/*
  ==============================================================================

    OrderableEffectsChain.h
    Created: 7 Aug 2023 3:38:22pm
    Author:  mana

  ==============================================================================
*/

#pragma once
#include "synthesizer/AudioProcessorBase.h"

namespace rpSynth::audio::effects {
class EffectProcessorBase;
}

namespace rpSynth::audio {
class OrderableEffectsChain : public AudioProcessorBase {
public:
    //================================================================================
    // Constructor
    OrderableEffectsChain(const juce::String& ID);
    ~OrderableEffectsChain() override;
    //================================================================================

    //================================================================================
    // implement for AudioProcessorBase
    void addParameterToLayout(juce::AudioProcessorValueTreeState::ParameterLayout& layout) override;
    //void updateParameters(size_t numSamples) override;
    void prepareParameters(FType sampleRate, size_t numSamples) override;
    void prepare(FType sampleRate, size_t numSamlpes) override;
    void process(size_t beginSamplePos, size_t endSamplePos) override;
    void saveExtraState(juce::XmlElement& xml) override;
    void loadExtraState(juce::XmlElement& xml, juce::AudioProcessorValueTreeState& apvts) override;
    void onCRClock(size_t) override;
    //================================================================================

    //================================================================================
    StereoBuffer& getChainBuffer() { return m_audioBuffer; }
    void reOrderProcessor(const juce::String& processorID, int newIndex);
    void reOrderProcessor(int oldIndex, int newIndex);
    decltype(auto) getAllEffectsProcessor() const { return m_effectsChain; }
    int getEffectOrder(const juce::String& name) const { return m_effectProcessorIndexes[name]; }
    std::function<void()> onOrderChanged;
    void clearBuffer();
    //================================================================================
private:
    //================================================================================
    // Effect chain ordering
    juce::CriticalSection m_orderLock;
    std::vector<std::shared_ptr<effects::EffectProcessorBase>> m_effectsChain;
    juce::HashMap<juce::String, int> m_effectProcessorIndexes;
    //================================================================================

    //================================================================================
    // Audio buffer
    StereoBuffer m_audioBuffer;
    //================================================================================
};
}