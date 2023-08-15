/*
  ==============================================================================

    EnvelopPanel.cpp
    Created: 27 Jul 2023 9:29:26pm
    Author:  mana

  ==============================================================================
*/

#include <JuceHeader.h>
#include "EnvelopPanel.h"
#include "synthesizer/modulation/Envelop.h"

namespace rpSynth::ui {
class EnvelopPanel::EnvelopDraw : public juce::Component , public juce::Timer {
public:
    ~EnvelopDraw() override = default;

    EnvelopDraw(rpSynth::audio::Envelop& e) :envelop(e) {
        startTimerHz(10);
    }

    void timerCallback() {
        if (envelop.hasNoModulationTargets()) {
            return;
        }

        repaint();
    }

    //=========================================================================

    void paint(juce::Graphics& g) {
        float width = static_cast<float>(getWidth());
        float height = static_cast<float>(getHeight());

        float lAttack = envelop.m_attackInMillSeconds.getNormalizedTargetValue();
        float lHold = envelop.m_holdInMillSeconds.getNormalizedTargetValue();
        float lDecay = envelop.m_decayInMillSeconds.getNormalizedTargetValue();
        float lSustain = 0.05f;
        float lRelease = envelop.m_releaseInMillSeconds.getNormalizedTargetValue();
        float totalLength = lAttack + lHold + lDecay + lSustain + lRelease;

        float xInit = 0.f;
        float xAttack = xInit + lAttack * width / totalLength;
        float xHold = xAttack + lHold * width / totalLength;
        float xDecay = xHold + lDecay * width / totalLength;
        float xSustain = xDecay + lSustain * width / totalLength;
        float xRelease = xSustain + lRelease * width / totalLength;
        float ySustain = height - envelop.m_sustainLevelInDecibels.getNormalizedTargetValue() * height;

        // 6 points
        juce::Point<float> initPoint{0.f,height};
        juce::Point<float> attackPoint{xAttack,0.f};
        juce::Point<float> holdPoint{xHold,0.f};
        juce::Point<float> decayPoint{xDecay,ySustain};
        juce::Point<float> sustainPoint{xSustain,ySustain};
        juce::Point<float> releasePoint{xRelease,height};

        // build curve
        juce::Path p;
        p.addLineSegment(juce::Line<float>(initPoint, attackPoint), 1.f);
        p.addLineSegment(juce::Line<float>(attackPoint, holdPoint), 1.f);
        p.addLineSegment(juce::Line<float>(holdPoint, decayPoint), 1.f);
        p.addLineSegment(juce::Line<float>(decayPoint, sustainPoint), 1.f);
        p.addLineSegment(juce::Line<float>(sustainPoint, releasePoint), 1.f);

        // background
        g.fillAll(juce::Colours::black);

        // draw curve
        g.setColour(juce::Colours::green);
        g.strokePath(p, juce::PathStrokeType(1.f));

        // draw circle
        juce::Rectangle<float> circleBound{6.f,6.f};
        g.setColour(juce::Colours::white);
        g.fillEllipse(circleBound.withCentre(initPoint));
        g.fillEllipse(circleBound.withCentre(attackPoint));
        g.fillEllipse(circleBound.withCentre(holdPoint));
        g.fillEllipse(circleBound.withCentre(decayPoint));
        g.fillEllipse(circleBound.withCentre(sustainPoint));
        g.fillEllipse(circleBound.withCentre(releasePoint));
    }
private:
    rpSynth::audio::Envelop& envelop;
};

class EnvelopPanel::EnvelopPointer : public juce::Component, public juce::Timer {
public:
    ~EnvelopPointer() = default;
    EnvelopPointer(audio::Envelop& e) : envelop(e) {
        startTimerHz(10);
        setInterceptsMouseClicks(false, false);
    }

    void timerCallback() {
        if (envelop.hasNoModulationTargets()) {
            return;
        }

        repaint();
    }

    void paint(juce::Graphics& g) {
        float width = static_cast<float>(getWidth());

        float lAttack = envelop.m_attackInMillSeconds.getNormalizedTargetValue();
        float lHold = envelop.m_holdInMillSeconds.getNormalizedTargetValue();
        float lDecay = envelop.m_decayInMillSeconds.getNormalizedTargetValue();
        float lSustain = 0.05f;
        float lRelease = envelop.m_releaseInMillSeconds.getNormalizedTargetValue();
        float totalLength = lAttack + lHold + lDecay + lSustain + lRelease;

        float xInit = 0.f;
        float xAttack = xInit + lAttack * width / totalLength;
        float xHold = xAttack + lHold * width / totalLength;
        float xDecay = xHold + lDecay * width / totalLength;
        float xSustain = xDecay + lSustain * width / totalLength;
        float xRelease = xSustain + lRelease * width / totalLength;

        auto currentEnvelopPosition = envelop.getCurrentEnvelopState();
        int newPosition{};
        switch (currentEnvelopPosition.first) {
            case audio::Envelop::EnvelopState::Init:
                break;
            case audio::Envelop::EnvelopState::Attack:
                newPosition = static_cast<int>(xInit + (xAttack - xInit) * currentEnvelopPosition.second);
                break;
            case audio::Envelop::EnvelopState::Hold:
                newPosition = static_cast<int>(xAttack + (xHold - xAttack) * currentEnvelopPosition.second);
                break;
            case audio::Envelop::EnvelopState::Decay:
                newPosition = static_cast<int>(xHold + (xDecay - xHold) * currentEnvelopPosition.second);
                break;
            case audio::Envelop::EnvelopState::Sustain:
                newPosition = static_cast<int>(xDecay + (xSustain - xDecay) * currentEnvelopPosition.second);
                break;
            case audio::Envelop::EnvelopState::Release:
                newPosition = static_cast<int>(xSustain + (xRelease - xSustain) * currentEnvelopPosition.second);
                break;
        }

        g.setColour(juce::Colours::white);
        g.drawVerticalLine(newPosition, 0.f, static_cast<float>(getHeight()));
    }
private:
    audio::Envelop& envelop;
};
};

namespace rpSynth::ui {
EnvelopPanel::EnvelopPanel(audio::Envelop& e)
    :m_bindEnvelopModulator(e)
    , m_attack(&e.m_attackInMillSeconds)
    , m_hold(&e.m_holdInMillSeconds)
    , m_decay(&e.m_decayInMillSeconds)
    , m_sustain(&e.m_sustainLevelInDecibels)
    , m_release(&e.m_releaseInMillSeconds) {
    m_envelopDrawer = std::make_unique<EnvelopDraw>(e);
    m_envelopPointer = std::make_unique<EnvelopPointer>(e);

    addAndMakeVisible(m_envelopDrawer.get());
    addAndMakeVisible(m_envelopPointer.get());
    addAndMakeVisible(m_attack);
    addAndMakeVisible(m_hold);
    addAndMakeVisible(m_decay);
    addAndMakeVisible(m_sustain);
    addAndMakeVisible(m_release);

    auto redraw = [this] {
        m_envelopDrawer->repaint();
    };

    m_attack.onValueChange = redraw;
    m_hold.onValueChange = redraw;
    m_decay.onValueChange = redraw;
    m_sustain.onValueChange = redraw;
    m_release.onValueChange = redraw;
}

EnvelopPanel::~EnvelopPanel() {
    m_envelopDrawer = nullptr;
}

void EnvelopPanel::paint(juce::Graphics& g) {
    g.fillAll(juce::Component::findColour(juce::DocumentWindow::backgroundColourId));
}

void EnvelopPanel::resized() {
    auto knobHeight = 70;
    auto knobWidth = 70;
    auto top = getHeight() - knobHeight;
    auto knobBound = juce::Rectangle<int>(0, top, knobWidth, knobHeight);

    m_envelopDrawer->setBounds(0, 0, getWidth(), top);
    m_envelopPointer->setBounds(m_envelopDrawer->getBounds());

    m_attack.setBounds(knobBound);
    knobBound.translate(knobWidth, 0);
    m_hold.setBounds(knobBound);
    knobBound.translate(knobWidth, 0);
    m_decay.setBounds(knobBound);
    knobBound.translate(knobWidth, 0);
    m_sustain.setBounds(knobBound);
    knobBound.translate(knobWidth, 0);
    m_release.setBounds(knobBound);
}

void rpSynth::ui::EnvelopPanel::showModulationFrom(audio::ModulatorBase* /*e*/) {
}
}