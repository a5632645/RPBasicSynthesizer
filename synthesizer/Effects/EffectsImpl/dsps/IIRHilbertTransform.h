/*
  ==============================================================================

    IIRHilbertTransform.h
    Created: 19 Jul 2023 11:03:34am
    Author:  mana

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

namespace IIRHilbertCoeffect {
template<typename SampleType, size_t size>
struct Coeffects {
    std::array<SampleType, size> reals;
    std::array<SampleType, size> images;
};

template<typename type>
static constexpr Coeffects<type, 4> kCoeffects1 = {
    {(type)0.4021921162426,(type)0.8561710882420,(type)0.9722909545651,(type)0.9952884791278},
    {(type)0.6923878,(type)0.9360654322959,(type)0.9882295226860,(type)0.9987488452737}
};

template<typename type>
static constexpr Coeffects<type, 8> kCoeffects2 = {
    {
        (type)0.0406273391966415,(type)0.2984386654059753
        ,(type)0.5938455547890998,(type)0.7953345677003365
        ,(type)0.9040699927853059,(type)0.9568366727621767
        ,(type)0.9815966237057977,(type)0.9938718801312583
    },
    {
        (type)0.1500685240941415,(type)0.4538477444783975
        ,(type)0.7081016258869689,(type)0.8589957406397113
        ,(type)0.9353623391637175,(type)0.9715130669899118
        ,(type)0.9886689766148302,(type)0.9980623781456869
    }
};
}

template<typename SampleType, size_t size>
    requires std::is_floating_point_v<SampleType>
struct IIRHilbertTransformer {
    IIRHilbertTransformer(IIRHilbertCoeffect::Coeffects<SampleType, size> const& initCoeffects) {
        for (size_t i = 0; i < size; i++) {
            m_realAPFs[i].setA(initCoeffects.reals[i]);
            m_imagAPFs[i].setA(initCoeffects.images[i]);
        }
    }

    void process(SampleType input, SampleType* pReal, SampleType* pImg) {
        SampleType copy = input;
        *pImg = m_unitDelay;

        for (PolyphaseAPF& apf : m_realAPFs) {
            input = apf.process(input);
        }
        *pReal = input;

        for (PolyphaseAPF& apf : m_imagAPFs) {
            copy = apf.process(copy);
        }
        m_unitDelay = copy;
    }
private:
    struct PolyphaseAPF {
        /*        +------g(a)---+
         *        ¡ý             |
         * in -> add -+-> z^-2 -+-> add -> out
         *            |              ¡ü
         *            +---g(-a)------+
        */
        SampleType process(SampleType input) {
            SampleType in = input + m_a * m_z1;
            SampleType out = -m_a * in + m_z1;
            m_z1 = m_z0;
            m_z0 = in;

            return out;
        }

        void setA(SampleType a) {
            m_a = a;
        }
    private:
        SampleType m_z0{};
        SampleType m_z1{};
        SampleType m_a;
    };

    SampleType m_unitDelay{};
    std::array<PolyphaseAPF, size> m_realAPFs;
    std::array<PolyphaseAPF, size> m_imagAPFs;
};