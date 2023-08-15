/*
  ==============================================================================

    MoogLadderFilter.cpp
    Created: 6 Aug 2023 10:39:09pm
    Author:  mana

  ==============================================================================
*/

#include "MoogLadderFilter.h"
#include "synthesizer/Filter/AllFilterParameters.h"
#include "synthesizer/WrapParameter.h"
#include "ui/filter/FilterPanel.h"

namespace rpSynth::audio::filters::analog {
void MoogLadderFilter::process(rpSynth::audio::StereoBuffer& input,
                               rpSynth::audio::StereoBuffer& output,
                               size_t begin,
                               size_t end) {
    for (size_t i = begin; i < end; i++) {
        static constexpr FType v2 = static_cast<FType>(40000);   // twice the 'thermal voltage of a transistor'
        static constexpr FType invV2 = 1 / v2;

        FType cutoff_hz = semitoneToHertz(m_parameters.cutoff.getNextValue());
        FType kfc = cutoff_hz * m_oneDivNyquistRate;
        FType kf = kfc * static_cast<FType>(0.5f);

        // frequency & amplitude correction
        FType kfcr = static_cast<FType>(1.8730 * (kfc * kfc * kfc) + 0.4955 * (kfc * kfc) - 0.6490 * kfc + 0.9988);
        FType kacr = static_cast<FType>(-3.9364 * (kfc * kfc) + 1.8409 * kfc + 0.9968);

        float x = -juce::MathConstants<FType>::twoPi * kfcr * kf;
        float exp_out = juce::dsp::FastMathApproximations::exp(x);
        float k2vg = v2 * (static_cast<FType>(1) - exp_out); // filter tuning

        // cascade of 4 1st order sections
        float resonance = m_parameters.resonance.getNextValue();
        PolyFType temp = m_coeffect.c_amf * kacr * static_cast<FType>(4) * resonance;

        PolyFType x1 = (input[i] - temp) * invV2;
        PolyFType tanh1 = juce::dsp::FastMathApproximations::tanh(x1);
        PolyFType x2 = m_coeffect.c_az1 * invV2;
        PolyFType tanh2 = juce::dsp::FastMathApproximations::tanh(x2);
        m_coeffect.c_ay1 = m_coeffect.c_az1 + (tanh1 - tanh2) * k2vg;

        m_coeffect.c_ay1 = m_coeffect.c_az1 +(juce::dsp::FastMathApproximations::tanh((input[i] - temp) * invV2) - juce::dsp::FastMathApproximations::tanh(m_coeffect.c_az1 * invV2)) * k2vg;
        m_coeffect.c_az1 = m_coeffect.c_ay1;

        m_coeffect.c_ay2 = m_coeffect.c_az2 + (juce::dsp::FastMathApproximations::tanh(m_coeffect.c_ay1 * invV2) - juce::dsp::FastMathApproximations::tanh(m_coeffect.c_az2 * invV2)) * k2vg;
        m_coeffect.c_az2 = m_coeffect.c_ay2;

        m_coeffect.c_ay3 = m_coeffect.c_az3 +  (juce::dsp::FastMathApproximations::tanh(m_coeffect.c_ay2 * invV2) - juce::dsp::FastMathApproximations::tanh(m_coeffect.c_az3 * invV2)) * k2vg;
        m_coeffect.c_az3 = m_coeffect.c_ay3;

        m_coeffect.c_ay4 = m_coeffect.c_az4 +  (juce::dsp::FastMathApproximations::tanh(m_coeffect.c_ay3 * invV2) - juce::dsp::FastMathApproximations::tanh(m_coeffect.c_az4 * invV2)) * k2vg;
        m_coeffect.c_az4 = m_coeffect.c_ay4;

        // 1/2-sample delay for phase compensation
        m_coeffect.c_amf = (m_coeffect.c_ay4 + m_coeffect.c_az5) * static_cast<FType>(0.5);
        m_coeffect.c_az5 = m_coeffect.c_ay4;

        // end of sm code
        output[i] = m_coeffect.c_amf;
    }
}

void MoogLadderFilter::reset() {
    m_coeffect = Coeffects{};
}

void MoogLadderFilter::prepare(rpSynth::audio::FType sampleRate, size_t /*numSamples*/) {
    m_oneDivSampleRate = 1 / sampleRate;
    m_oneDivNyquistRate = m_oneDivSampleRate * static_cast<FType>(2);
}

void MoogLadderFilter::doLayout(ui::FilterKnobsPanel& p) {
    p.m_cutoff.setVisible(true);
    p.m_resonance.setVisible(true);

    p.m_cutoff.setBounds(0, 0, 70, 70);
    p.m_resonance.setBounds(80, 0, 70, 70);
}

//void MoogLadderFilter::processLeft(FType* input, FType* output, size_t begin, size_t end) {
//    for (size_t i = begin; i < end; i++) {
//        static constexpr FType v2 = static_cast<FType>(40000);   // twice the 'thermal voltage of a transistor'
//        static constexpr FType invV2 = 1 / v2;
//
//        FType cutoff_hz = m_parameters.cutoff.getRaw(i);
//        FType kfc = cutoff_hz * m_oneDivNyquistRate;
//        FType kf = kfc * static_cast<FType>(0.5f);
//
//        // frequency & amplitude correction
//        FType kfcr = static_cast<FType>(1.8730 * (kfc * kfc * kfc) + 0.4955 * (kfc * kfc) - 0.6490 * kfc + 0.9988);
//        FType kacr = static_cast<FType>(-3.9364 * (kfc * kfc) + 1.8409 * kfc + 0.9968);
//
//        float x = -juce::MathConstants<FType>::twoPi * kfcr * kf;
//        float exp_out = juce::dsp::FastMathApproximations::exp(x);
//        float k2vg = v2 * (static_cast<FType>(1) - exp_out); // filter tuning
//
//        // cascade of 4 1st order sections
//        float resonance = m_parameters.resonance.get(i);
//        float temp = static_cast<FType>(4) * resonance * c_left.c_amf * kacr;
//
//        float x1 = (input[i] - temp) * invV2;
//        float tanh1 = juce::dsp::FastMathApproximations::tanh(x1);
//        float x2 = c_left.c_az1 * invV2;
//        float tanh2 = juce::dsp::FastMathApproximations::tanh(x2);
//        c_left.c_ay1 = c_left.c_az1 + k2vg * (tanh1 - tanh2);
//
//        c_left.c_ay1 = c_left.c_az1 + k2vg * (juce::dsp::FastMathApproximations::tanh((input[i] - temp) * invV2) - juce::dsp::FastMathApproximations::tanh(c_left.c_az1 * invV2));
//        c_left.c_az1 = c_left.c_ay1;
//
//        c_left.c_ay2 = c_left.c_az2 + k2vg * (juce::dsp::FastMathApproximations::tanh(c_left.c_ay1 * invV2) - juce::dsp::FastMathApproximations::tanh(c_left.c_az2 * invV2));
//        c_left.c_az2 = c_left.c_ay2;
//
//        c_left.c_ay3 = c_left.c_az3 + k2vg * (juce::dsp::FastMathApproximations::tanh(c_left.c_ay2 * invV2) - juce::dsp::FastMathApproximations::tanh(c_left.c_az3 * invV2));
//        c_left.c_az3 = c_left.c_ay3;
//
//        c_left.c_ay4 = c_left.c_az4 + k2vg * (juce::dsp::FastMathApproximations::tanh(c_left.c_ay3 * invV2) - juce::dsp::FastMathApproximations::tanh(c_left.c_az4 * invV2));
//        c_left.c_az4 = c_left.c_ay4;
//
//        // 1/2-sample delay for phase compensation
//        c_left.c_amf = (c_left.c_ay4 + c_left.c_az5) * static_cast<FType>(0.5);
//        c_left.c_az5 = c_left.c_ay4;
//
//        // end of sm code
//        output[i] = c_left.c_amf;
//    }
//}
//
//void MoogLadderFilter::processRight(FType* input, FType* output, size_t begin, size_t end) {
//    for (size_t i = begin; i < end; i++) {
//        static constexpr FType v2 = static_cast<FType>(40000);   // twice the 'thermal voltage of a transistor'
//        static constexpr FType invV2 = 1 / v2;
//
//        FType cutoff_hz = m_parameters.cutoff.getRaw(i);
//        FType kfc = cutoff_hz * m_oneDivNyquistRate;
//        FType kf = kfc * static_cast<FType>(0.5f);
//
//        // frequency & amplitude correction
//        FType kfcr = static_cast<FType>(1.8730 * (kfc * kfc * kfc) + 0.4955 * (kfc * kfc) - 0.6490 * kfc + 0.9988);
//        FType kacr = static_cast<FType>(-3.9364 * (kfc * kfc) + 1.8409 * kfc + 0.9968);
//
//        float x = -juce::MathConstants<FType>::twoPi * kfcr * kf;
//        float exp_out = juce::dsp::FastMathApproximations::exp(x);
//        float k2vg = v2 * (static_cast<FType>(1) - exp_out); // filter tuning
//
//        // cascade of 4 1st order sections
//        float resonance = m_parameters.resonance.get(i);
//        float temp = static_cast<FType>(4) * resonance * c_right.c_amf * kacr;
//
//        float x1 = (input[i] - temp) * invV2;
//        float tanh1 = juce::dsp::FastMathApproximations::tanh(x1);
//        float x2 = c_right.c_az1 * invV2;
//        float tanh2 = juce::dsp::FastMathApproximations::tanh(x2);
//        c_right.c_ay1 = c_right.c_az1 + k2vg * (tanh1 - tanh2);
//
//        c_right.c_ay1 = c_right.c_az1 + k2vg * (juce::dsp::FastMathApproximations::tanh((input[i] - temp) * invV2) - juce::dsp::FastMathApproximations::tanh(c_right.c_az1 * invV2));
//        c_right.c_az1 = c_right.c_ay1;
//
//        c_right.c_ay2 = c_right.c_az2 + k2vg * (juce::dsp::FastMathApproximations::tanh(c_right.c_ay1 * invV2) - juce::dsp::FastMathApproximations::tanh(c_right.c_az2 * invV2));
//        c_right.c_az2 = c_right.c_ay2;
//
//        c_right.c_ay3 = c_right.c_az3 + k2vg * (juce::dsp::FastMathApproximations::tanh(c_right.c_ay2 * invV2) - juce::dsp::FastMathApproximations::tanh(c_right.c_az3 * invV2));
//        c_right.c_az3 = c_right.c_ay3;
//
//        c_right.c_ay4 = c_right.c_az4 + k2vg * (juce::dsp::FastMathApproximations::tanh(c_right.c_ay3 * invV2) - juce::dsp::FastMathApproximations::tanh(c_right.c_az4 * invV2));
//        c_right.c_az4 = c_right.c_ay4;
//
//        // 1/2-sample delay for phase compensation
//        c_right.c_amf = (c_right.c_ay4 + c_right.c_az5) * static_cast<FType>(0.5);
//        c_right.c_az5 = c_right.c_ay4;
//
//        // end of sm code
//        output[i] = c_right.c_amf;
//    }
//}
}