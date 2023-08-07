/*
  ==============================================================================

    PointBlock.h
    Created: 28 Jul 2023 7:47:54am
    Author:  mana

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
/*
*/
namespace rpSynth::ui {
class PointBlock : public juce::Component {
public:
    struct Listener {
        virtual ~Listener() = default;
        virtual void pointDeleted(PointBlock*) = 0;
        virtual void pointPressed(PointBlock*, const juce::MouseEvent& e) = 0;
        virtual void pointMoved(PointBlock*, const juce::MouseEvent& e) = 0;
        virtual void pointSet(PointBlock*) = 0;
    };

    PointBlock(size_t index)
        :m_blockIndex(index) {

    }

    void paint(juce::Graphics& g) override {
        g.fillAll(juce::Colours::grey);
        g.setColour(juce::Colours::black);
        g.drawRect(getLocalBounds());
        g.setColour(juce::Colours::white);
        g.setFont(10.f);
        g.drawSingleLineText(juce::String(m_blockIndex), 2, 8);
    }

    void mouseDown(const juce::MouseEvent& e) override {
        jassert(m_pListener != nullptr);
        m_pListener->pointPressed(this, e);
    }

    void mouseDrag(const juce::MouseEvent& e) override {
        jassert(m_pListener != nullptr);
        m_pListener->pointMoved(this,e);
    }

    void mouseUp(const juce::MouseEvent&) override {
        jassert(m_pListener != nullptr);
        m_pListener->pointSet(this);
    }

    void mouseDoubleClick(const juce::MouseEvent&) override {
        jassert(m_pListener != nullptr);
        m_pListener->pointDeleted(this);
    };

    void setListener(Listener* pL) {
        m_pListener = pL;
    }

    size_t getIndex() const {
        jassert(m_blockIndex != kInvalidBlockIndex);
        return m_blockIndex;
    }

    void increase() {
        m_blockIndex++;
    }

    void decrease() {
        jassert(m_blockIndex > 0);
        m_blockIndex--;
    }

private:
    static constexpr size_t kInvalidBlockIndex = static_cast<size_t>(-1);

    size_t m_blockIndex = kInvalidBlockIndex;
    Listener* m_pListener = nullptr;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PointBlock)
};
};