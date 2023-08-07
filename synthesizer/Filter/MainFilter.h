/*
  ==============================================================================

    MainFilter.h
    Created: 5 Aug 2023 7:53:24pm
    Author:  mana

  ==============================================================================
*/

#pragma once
#include "synthesizer/AudioProcessorBase.h"

namespace rpSynth::audio::filters {
class FilterImplBase;
struct AllFilterParameters;
};

namespace rpSynth::ui {
class FilterKnobsPanel;
}

namespace rpSynth::audio {
class MainFilter : public AudioProcessorBase {
public:
    struct InputRouterSet {
        AudioProcessorBase* pProcessor = nullptr;
        StereoBuffer* pProcessOutputBuffer = nullptr;
        juce::AudioParameterBool* pJuceParameter = nullptr;
    };

    //=========================================================================
    // Constructor
    MainFilter(const juce::String& ID);
    ~MainFilter() override;
    //=========================================================================

    //=========================================================================
    // implement for AudioProcessorBase
    void addParameterToLayout(juce::AudioProcessorValueTreeState::ParameterLayout& layout) override;
    void updateParameters(size_t numSamples) override;
    void prepareParameters(FType sampleRate, size_t numSamples) override;
    void prepare(FType sampleRate, size_t numSamlpes) override;
    void process(size_t beginSamplePos, size_t endSamplePos) override;
    void saveExtraState(juce::XmlElement& xml) override;
    void loadExtraState(juce::XmlElement& xml, juce::AudioProcessorValueTreeState& apvts) override;
    //=========================================================================

    //=========================================================================
    void changeFilter(const juce::String& filterType);
    void addAudioInput(AudioProcessorBase* pProcessor, StereoBuffer* pInput);
    StereoBuffer& getFilterOutput() { return m_processorOutputBuffer; }
    juce::StringArray getAllFilterNames() const;
    juce::StringRef getCurrentFilterName() const;
    const InputRouterSet& getInputSet(size_t index) const { return m_inputRouter[index]; }
    size_t getNumInputs() const { return m_inputRouter.size(); }
    void doLayout(ui::FilterKnobsPanel&, const juce::String& name);
    //=========================================================================

    //=========================================================================
    std::function<void(const juce::String&)> onFilterTypeChange;
    //=========================================================================
private:
    //=========================================================================
    // filter changing
    std::shared_ptr<filters::FilterImplBase> m_currentFilterImpl;
    std::atomic<bool> m_isFilterChanged = false;
    juce::String m_newFilterName;
    //=========================================================================

    //=========================================================================
    // Audio router
    StereoBuffer m_filterInputBuffer;
    StereoBuffer m_filterOutputBuffer;
    StereoBuffer m_processorOutputBuffer;
    std::vector<InputRouterSet> m_inputRouter;
    //=========================================================================

    //=========================================================================
    // All types of filters
    juce::HashMap<juce::String, std::shared_ptr<filters::FilterImplBase>> m_allFilters;
    //=========================================================================

public:
    //=========================================================================
    // all types of filter impl parameters
    std::unique_ptr<filters::AllFilterParameters> m_allFilterParameters;
    //=========================================================================
};
}