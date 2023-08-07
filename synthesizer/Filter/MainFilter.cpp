/*
  ==============================================================================

    MainFilter.cpp
    Created: 5 Aug 2023 7:53:24pm
    Author:  mana

  ==============================================================================
*/

#include "MainFilter.h"
#include "FilterImplBase.h"
#include "AllFilterParameters.h"

#include "Filters/LowPass.h"
#include "Filters/HighPass.h"
#include "Filters/Analog/MoogLadderFilter.h"

static const juce::String kFilterNameAttribute = "filterName";

static juce::String _stToHzString(float st, int /*maxStringLen*/) {
    return juce::String(rpSynth::semitoneToHertz(st), 1);
}

namespace rpSynth::audio {
MainFilter::MainFilter(const juce::String& ID)
    :AudioProcessorBase(ID) {
    m_allFilterParameters = std::make_unique<filters::AllFilterParameters>();
    // Add all type of impl filter into hash map
    using namespace filters;
    m_allFilters.set(LowPass::kName, std::make_shared<LowPass>(*m_allFilterParameters));
    m_allFilters.set(HighPass::kName, std::make_shared<HighPass>(*m_allFilterParameters));
    m_allFilters.set(analog::MoogLadderFilter::kName, std::make_shared<analog::MoogLadderFilter>(*m_allFilterParameters));

    // Set the default Filter impl
    m_newFilterName = LowPass::kName;
    m_currentFilterImpl = m_allFilters.getReference(LowPass::kName);
}

MainFilter::~MainFilter() {
    m_allFilterParameters = nullptr;
}

//=============================================================================
// Notice: Please add parameter operator here when adding parameter
void MainFilter::addParameterToLayout(juce::AudioProcessorValueTreeState::ParameterLayout& layout) {
    // common control parameters
    layout.add(std::make_unique<MyHostedAudioProcessorParameter>(&m_allFilterParameters->cutoff,
                                                                 combineWithID("cutoff"),
                                                                 "cutoff",
                                                                 juce::NormalisableRange<float>(kStOf20hz, kStOf20000hz, 0.01f),
                                                                 hertzToSemitone(440.f),
                                                                 juce::AudioParameterFloatAttributes().withStringFromValueFunction(_stToHzString)
    ),
               std::make_unique<MyHostedAudioProcessorParameter>(&m_allFilterParameters->resonance,
                                                                 combineWithID("resonance"),
                                                                 "resonance",
                                                                 juce::NormalisableRange <float>(0.f, 1.f, 0.01f),
                                                                 0.f),
               std::make_unique<MyHostedAudioProcessorParameter>(&m_allFilterParameters->limitVolume,
                                                                 combineWithID("LimitVolume"),
                                                                 "LimitVolume",
                                                                 juce::NormalisableRange<float>(0.f, 1.f, 0.01f),
                                                                 1.f),
               std::make_unique<MyHostedAudioProcessorParameter>(&m_allFilterParameters->limitK,
                                                                 combineWithID("LimitK"),
                                                                 "LimitK",
                                                                 juce::NormalisableRange<float>(0.f, 0.5f, 0.001f),
                                                                 0.125f)
    );

    // router parameters
    for (auto& set : m_inputRouter) {
        auto pp = std::make_unique<juce::AudioParameterBool>(
            combineWithID(set.pProcessor->combineWithID("input")),
            combineWithID(set.pProcessor->combineWithID("input")),
            true);
        set.pJuceParameter = pp.get();
        layout.add(std::move(pp));
    }
}

void MainFilter::updateParameters(size_t numSamples) {
    m_allFilterParameters->cutoff.updateParameter(numSamples);
    m_allFilterParameters->resonance.updateParameter(numSamples);
    m_allFilterParameters->limitVolume.updateParameter(numSamples);
    m_allFilterParameters->limitK.updateParameter(numSamples);
}

void MainFilter::prepareParameters(FType sampleRate, size_t numSamples) {
    m_allFilterParameters->cutoff.prepare(sampleRate, numSamples);
    m_allFilterParameters->resonance.prepare(sampleRate, numSamples);
    m_allFilterParameters->limitVolume.prepare(sampleRate, numSamples);
    m_allFilterParameters->limitK.prepare(sampleRate, numSamples);
}
//=============================================================================

void MainFilter::prepare(FType sampleRate, size_t numSamlpes) {
    // buffer init
    m_filterInputBuffer.resize(numSamlpes);
    m_filterOutputBuffer.resize(numSamlpes);
    m_processorOutputBuffer.resize(numSamlpes);

    // init for all filters
    for (auto f : m_allFilters) {
        f->prepare(sampleRate, numSamlpes);
    }
}

void MainFilter::process(size_t beginSamplePos, size_t endSamplePos) {
    // process change filter event
    if (m_isFilterChanged) {
        m_currentFilterImpl = m_allFilters.getReference(m_newFilterName);
        m_currentFilterImpl->reset();
        m_isFilterChanged = false;
    }

    // Clear outputs and input
    m_processorOutputBuffer.clear();
    m_filterInputBuffer.clear();
    m_filterOutputBuffer.clear();

    // Mix input source
    size_t numSample = endSamplePos - beginSamplePos;
    for (auto& i : m_inputRouter) {
        // Atomic,may be Thread-safe?
        bool bypass = !i.pJuceParameter->get();
        if (bypass) {
            juce::FloatVectorOperations::add(m_processorOutputBuffer.left.data() + beginSamplePos,
                                             i.pProcessOutputBuffer->left.data() + beginSamplePos,
                                             numSample);
            juce::FloatVectorOperations::add(m_processorOutputBuffer.right.data() + beginSamplePos,
                                             i.pProcessOutputBuffer->right.data() + beginSamplePos,
                                             numSample);
        } else {
            juce::FloatVectorOperations::add(m_filterInputBuffer.left.data() + beginSamplePos,
                                             i.pProcessOutputBuffer->left.data() + beginSamplePos,
                                             numSample);
            juce::FloatVectorOperations::add(m_filterInputBuffer.right.data() + beginSamplePos,
                                             i.pProcessOutputBuffer->right.data() + beginSamplePos,
                                             numSample);
        }
    }

    // Filter process
    m_currentFilterImpl->process(m_filterInputBuffer, m_filterOutputBuffer, beginSamplePos, endSamplePos);

    // Mix final output
    juce::FloatVectorOperations::add(m_processorOutputBuffer.left.data() + beginSamplePos,
                                     m_filterOutputBuffer.left.data() + beginSamplePos,
                                     numSample);
    juce::FloatVectorOperations::add(m_processorOutputBuffer.right.data() + beginSamplePos,
                                     m_filterOutputBuffer.right.data() + beginSamplePos,
                                     numSample);
}

void MainFilter::saveExtraState(juce::XmlElement& xml) {
    auto* filterXML = xml.createNewChildElement(getProcessorID());
    filterXML->setAttribute(kFilterNameAttribute, m_newFilterName);
}

void MainFilter::loadExtraState(juce::XmlElement& xml, juce::AudioProcessorValueTreeState& /*apvts*/) {
    auto* filterXML = xml.getChildByName(getProcessorID());
    if (filterXML == nullptr) return;

    changeFilter(filterXML->getStringAttribute(kFilterNameAttribute));
}

void MainFilter::changeFilter(const juce::String& filterType) {
    // Just return if filter name is same
    if (filterType == m_currentFilterImpl->getFilterName()) {
        return;
    }

    // Do not contain this filter,also do nothing
    if (!m_allFilters.contains(filterType)) {
        return;
    }

    // set flag true,and changed it on audio thread
    m_newFilterName = filterType;
    m_isFilterChanged = true;

    // notify ui
    if (onFilterTypeChange) {
        onFilterTypeChange(filterType);
    }
}

void MainFilter::addAudioInput(AudioProcessorBase* pProcessor, StereoBuffer* pInput) {
    jassert(pInput != nullptr);
    jassert(pProcessor != nullptr);
    m_inputRouter.emplace_back(pProcessor, pInput, nullptr);
}

juce::StringArray MainFilter::getAllFilterNames() const {
    juce::StringArray s;
    for (auto f : m_allFilters) {
        s.add(f->getFilterName());
    }
    return s;
}

juce::StringRef MainFilter::getCurrentFilterName() const {
    return m_newFilterName;
}

void MainFilter::doLayout(ui::FilterKnobsPanel& p, const juce::String& name) {
    if (!m_allFilters.contains(name)) return;

    m_allFilters.getReference(name)->doLayout(p);
}
}