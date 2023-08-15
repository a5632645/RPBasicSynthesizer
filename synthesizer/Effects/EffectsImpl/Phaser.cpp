/*
  ==============================================================================

    Phaser.cpp
    Created: 8 Aug 2023 12:43:38pm
    Author:  mana

  ==============================================================================
*/

#include "Phaser.h"
#include "synthesizer/NewWrapParameter.h"
#include "synthesizer/dsps/AllPassFilter.h"
#include "synthesizer/dsps/IIRHilbertTransform.h"


#include "ui/controller/FloatKnob.h"

//================================================================================
// 全部镶边器的参数
//================================================================================
namespace rpSynth::audio::effects {
struct PhaserParameters {
    MyAudioParameter beginSemitone;                // 最小频率[20,20000]hz
    MyAudioParameter endSemitone;                  // 最大频率[20,20000]hz
    MyAudioParameter spread;                       // 陷波间隔[0,0.5]
    MyAudioParameter feedback;                     // 反馈[-1,1]
    MyAudioParameter fbLowCut;                     // 反馈低切[20,20000]hz
    MyAudioParameter fbHighCut;                    // 反馈高切[20,20000]hz
    MyAudioParameter mix;                          // 混合[-1,1]
    MyAudioParameter rate;                         // LFO速率[-2,2]hz
    MyAudioParameter barberpoleRate;               // 另一个LFO速率[-10,10]hz
    MyAudioParameter barberpolePhase;              // barberpole的LFO相位[0,1]
    MyAudioParameter lfoPhase;                     // LFO相差相位[0,1]
    MyAudioParameter lfoShape;                     // LFO形状从三角波到正弦波[0,1]
    MyAudioParameter phaserState;                  // 相位器阶数
    juce::AudioParameterBool* disableBarberpole = nullptr;// 禁用希尔伯特导致的相位问题

    // Merge all
    void prepareAll(FType sr, size_t /*num*/) {
        beginSemitone.prepare(sr);
        endSemitone.prepare(sr);
        feedback.prepare(sr);
        fbLowCut.prepare(sr);
        fbHighCut.prepare(sr);
        mix.prepare(sr);
        rate.prepare(sr);
        barberpoleRate.prepare(sr);
        barberpolePhase.prepare(sr);
        lfoPhase.prepare(sr);
        lfoShape.prepare(sr);
        phaserState.prepare(sr);
        spread.prepare(sr);
    }

    void onCRClock() {
        beginSemitone.onCRClock();
        endSemitone.onCRClock();
        feedback.onCRClock();
        fbLowCut.onCRClock();
        fbHighCut.onCRClock();
        mix.onCRClock();
        rate.onCRClock();
        barberpoleRate.onCRClock();
        barberpolePhase.onCRClock();
        lfoPhase.onCRClock();
        lfoShape.onCRClock();
        phaserState.onCRClock();
        spread.onCRClock();
    }
};

//================================================================================
// Flanger Panel
//================================================================================
class PhaserPanel : public ui::ContainModulableComponent {
public:
    //================================================================================
    // implement for juce::Component
    //================================================================================
    ~PhaserPanel() override = default;

    PhaserPanel(PhaserParameters& f)
        :m_tzfDelay(&f.beginSemitone)
        , m_delay(&f.endSemitone)
        , m_fb(&f.feedback)
        , m_fbLowcut(&f.fbLowCut)
        , m_fbHighCut(&f.fbHighCut)
        , m_mix(&f.mix)
        , m_rate(&f.rate)
        , m_barberpoleRate(&f.barberpoleRate)
        , m_barberpolePhase(&f.barberpolePhase)
        , m_lfoPhase(&f.lfoPhase)
        , m_attach(*f.disableBarberpole, m_disableBarber)
        , m_lfoShape(&f.lfoShape)
    ,m_phaserState(&f.phaserState)
    ,m_spread(&f.spread){
        addAndMakeVisible(m_tzfDelay);
        addAndMakeVisible(m_delay);
        addAndMakeVisible(m_fb);
        addAndMakeVisible(m_fbLowcut);
        addAndMakeVisible(m_fbHighCut);
        addAndMakeVisible(m_mix);
        addAndMakeVisible(m_rate);
        addAndMakeVisible(m_barberpoleRate);
        addAndMakeVisible(m_barberpolePhase);
        addAndMakeVisible(m_lfoPhase);
        m_disableBarber.setButtonText(f.disableBarberpole->name);
        addAndMakeVisible(m_disableBarber);
        addAndMakeVisible(m_lfoShape);
        addAndMakeVisible(m_phaserState);
        addAndMakeVisible(m_spread);
    }

    void resized() override {
        auto bound = juce::Rectangle(0, 0, 70, 70);
        m_tzfDelay.setBounds(bound); bound.translate(80, 0);
        m_delay.setBounds(bound); bound.translate(80, 0);
        m_fb.setBounds(bound); bound.translate(80, 0);
        m_fbLowcut.setBounds(bound); bound.translate(80, 0);
        m_fbHighCut.setBounds(bound);

        bound = juce::Rectangle(0, 80, 70, 70);
        m_mix.setBounds(bound); bound.translate(80, 0);
        m_rate.setBounds(bound); bound.translate(80, 0);
        m_barberpoleRate.setBounds(bound); bound.translate(80, 0);
        m_barberpolePhase.setBounds(bound);

        bound = juce::Rectangle(0, 160, 70, 70);
        m_lfoPhase.setBounds(bound); bound.translate(80, 0);
        m_lfoShape.setBounds(bound); bound.translate(80, 0);
        m_spread.setBounds(bound); bound.translate(80, 0);
        m_phaserState.setBounds(bound);

        m_disableBarber.setBounds(0, 215, 200, 40);
    }

    //================================================================================
    // implement for ContainModulableComponent
    //================================================================================
    void showModulationFrom(audio::ModulatorBase* p) override {
        m_tzfDelay.showModulationFrom(p);
        m_delay.showModulationFrom(p);
        m_fb.showModulationFrom(p);
        m_fbLowcut.showModulationFrom(p);
        m_fbHighCut.showModulationFrom(p);
        m_mix.showModulationFrom(p);
        m_rate.showModulationFrom(p);
        m_barberpoleRate.showModulationFrom(p);
        m_barberpolePhase.showModulationFrom(p);
        m_lfoPhase.showModulationFrom(p);
        m_lfoShape.showModulationFrom(p);
        m_spread.showModulationFrom(p);
    }
private:
    //================================================================================
    // GUI controllers
    //================================================================================
    using Knob = ui::FloatKnob;
    Knob m_tzfDelay;
    Knob m_delay;
    Knob m_fb;
    Knob m_fbLowcut;
    Knob m_fbHighCut;
    Knob m_mix;
    Knob m_rate;
    Knob m_barberpoleRate;
    Knob m_barberpolePhase;
    Knob m_lfoPhase;
    Knob m_lfoShape;
    Knob m_phaserState;
    Knob m_spread;
    juce::ToggleButton m_disableBarber;
    juce::ButtonParameterAttachment m_attach;
};

//================================================================================
// Flanger Impl
//================================================================================
class PhaserImpl {
public:
    // 镶边器延迟和ZDF延迟平滑时间
    static constexpr double kDelaySmoothTime = 0.1;
    static constexpr size_t kMaxPhaserState=16;

    //================================================================================
    // 正弦三角LFO
    //================================================================================
    class TriSineShapeLFO {
    public:
        void prepare(FType sr) {
            m_phase = FType{};
            m_oneDivSampleRate = 1 / sr;
        }

        /**
         * @brief 让LFO在CR模式下运行一次
         * @param fre 频率
         * @param step CR两个时钟之间的时间差在SR下的采样数差
         * @param phase 额外的相位增加
         * @param shape 形状,[0,1]从三角线性插值到正弦
         * @return 左:无额外相位增加;右:有额外相位增加;均为[-1,1]
        */
        PolyFType CRTick(FType fre, size_t step, FType phase, FType shape) {
            auto phaseAdd = step * fre * m_oneDivSampleRate;
            m_phase += phaseAdd;
            m_phase = std::fmod(m_phase, static_cast<FType>(1));
            auto ppp = std::fmod(m_phase + phase, static_cast<FType>(1));

            PolyFType LR{};
            LR.left = triToSin(m_phase, shape);
            LR.right = triToSin(ppp, shape);
            return LR;
        }
    private:
        FType triToSin(FType phase, FType shape) {
            auto sinVal = std::cos(juce::MathConstants<FType>::twoPi * phase);
            auto triVal = static_cast<FType>(2)
                * std::abs(static_cast<FType>(2) * phase - static_cast<FType>(1))
                - static_cast<FType>(1);
            return shape * triVal + (static_cast<FType>(1) - shape) * sinVal;
        }

        FType m_oneDivSampleRate{};
        FType m_phase{};
    };
    //================================================================================
    // Dual Oscillor
    //================================================================================
    class SinCosLFO {
    public:
        void prepare(FType sr) {
            m_oneDivSampleRate = 1 / sr;
        }

        PolyFType SRTick(FType fre, FType additionalPhase) {
            auto phaseAdd = fre * m_oneDivSampleRate;
            m_phase += phaseAdd;
            m_phase = std::fmod(m_phase, static_cast<FType>(1));
            auto ppp = std::fmod(m_phase + additionalPhase, static_cast<FType>(1));

            PolyFType pp{};
            pp.left = juce::dsp::FastMathApproximations::cos(juce::MathConstants<FType>::twoPi * ppp);
            pp.right = juce::dsp::FastMathApproximations::sin(juce::MathConstants<FType>::twoPi * ppp);
            return pp;
        }
    private:
        FType m_oneDivSampleRate{};
        FType m_phase{};
    };

    PhaserImpl(PhaserParameters& e) :p(e) {};
    void prepare(FType sr, size_t num) {
        m_sampleRate = sr;
        m_oneDivNyquistRate = 2 / sr;
        juce::dsp::ProcessSpec spec{};
        spec.sampleRate = sr;
        spec.numChannels = 2;
        spec.maximumBlockSize = static_cast<juce::uint32>(num);

        fbLF.prepare(spec);
        fbHF.prepare(spec);

        m_leftHertzSmoother.reset(sr, kDelaySmoothTime);
        m_rightHertzSmoother.reset(sr, kDelaySmoothTime);
        m_leftBarberpoleLFO.prepare(sr);
        m_rightBarberpoleLFO.prepare(sr);
        m_mainDelayLFO.prepare(sr);
    }

    void onCRClock(size_t n) {
        // Feedback filter
        fbLF.setCutoffFrequency(p.fbLowCut.getTargetValue());
        fbHF.setCutoffFrequency(p.fbHighCut.getTargetValue());

        // apf cutoff and quality
        FType beginHertz = semitoneToHertz(p.beginSemitone.getTargetValue());
        FType endHertz = semitoneToHertz(p.endSemitone.getTargetValue());
        PolyFType LRPhase = m_mainDelayLFO.CRTick(p.rate.getTargetValue(), n, p.lfoPhase.getTargetValue(), p.lfoShape.getTargetValue());
        FType lHertz = juce::jmap(LRPhase.left, FType{-1}, FType{1}, beginHertz, endHertz);
        FType rHertz = juce::jmap(LRPhase.right, FType{-1}, FType{1}, beginHertz, endHertz);
        FType spread = p.spread.getTargetValue();
        m_APFCoeffects[0].setCenterFrequency(lHertz, m_sampleRate);
        m_APFCoeffects[0].setBandWidth(spread, m_sampleRate);
        m_APFCoeffects[1].setCenterFrequency(rHertz, m_sampleRate);
        m_APFCoeffects[1].setBandWidth(spread, m_sampleRate);
    }

    void process(StereoBuffer& buffer, size_t begin, size_t end) {
        int numState = static_cast<int>(p.phaserState.getTargetValue());
        if (p.disableBarberpole->get()) {
            processChannelWithoutHilbert(buffer, begin, end, numState);
        } else {
            processChannel(buffer, begin, end, numState);
        }
    }

    //================================================================================
    // Barberpole Phaser                 +-> out
    //                                   |
    // in------------------------------>Mix
    // ↓                                 ↑ 
    // Add--->APFS---->SSBFreShift-------+ 
    // ↑       ↑          ↑              | 
    // |      LFO        LFO          Feedback
    // +---------------------------------+
    //================================================================================
    void processChannel(StereoBuffer& data, size_t begin, size_t end,int state) {
        for (size_t i = begin; i < end; i++) {
            PolyFType sample = data[i];
            PolyFType fbVal = juce::jlimit(FType{-0.9}, FType{0.9}, p.feedback.getNextValue())
                * m_fbValue;
            PolyFType input = sample + fbVal;
            for (int j = 0; j < state; j++) {
                input.left = m_APFArray[j].processSingle(m_APFCoeffects[0], input.left, 0);
                input.right = m_APFArray[j].processSingle(m_APFCoeffects[1], input.right, 1);
            }

            //FPolyType hilbertMid = hilbert<channel>(input);
            //auto sincos = getBarberLFO<channel>(p.barberpoleRate.get(i), p.barberpolePhase.get(i));
            //auto hilbertOut = sincos.right * hilbertMid.left + sincos.left * hilbertMid.right;
            //auto mixout = sample + p.mix.get(i) * hilbertOut + fbVal;

            //fbUpdate<channel>(hilbertOut);
            PolyFType hilbertLout{};
            PolyFType hilbertRout{};
            hilbertL.process(input.left, &hilbertLout.left, &hilbertLout.right);
            hilbertR.process(input.right, &hilbertRout.left, &hilbertRout.right);

            FType barberRate = p.barberpoleRate.getNextValue();
            FType barberPhase = p.barberpolePhase.getNextValue();
            PolyFType sincosL = m_leftBarberpoleLFO.SRTick(barberRate, FType{});
            PolyFType sincosR = m_rightBarberpoleLFO.SRTick(barberRate, barberPhase);

            PolyFType hilbertOut{};
            hilbertOut.left = sincosL.right * hilbertLout.left + sincosL.left * hilbertLout.right;
            hilbertOut.right = sincosR.right * hilbertRout.left + sincosR.left * hilbertRout.right;
            auto mixout = sample + p.mix.getNextValue() * hilbertOut + fbVal;

            m_fbValue.left = fbLF.processSample(0, fbHF.processSample(0, hilbertOut.left));
            m_fbValue.right = fbLF.processSample(1, fbHF.processSample(1, hilbertOut.right));
            fbLF.snapToZero();
            fbHF.snapToZero();
            data[i] = mixout;
        }
    }

    //================================================================================
    // Basic Phaser                       +-> out
    //                                   |
    // in-------------------------------Mix
    // ↓                                 ↑ 
    // Add--->APFS-----------------------+
    // ↑       ↑                         ↓       
    // |      LFO                    Feedback
    // +---------------------------------+
    //================================================================================
    void processChannelWithoutHilbert(StereoBuffer& data, size_t begin, size_t end,int state) {
        for (size_t i = begin; i < end; i++) {
            PolyFType sample = data[i];
            PolyFType withFb = sample + p.feedback.getNextValue() * m_fbValue;
            //auto withFb = sample + p.feedback.get(i) * getFeedback<channel>();
            for (int j = 0; j < state; j++) {
                withFb.left = m_APFArray[j].processSingle(m_APFCoeffects[0], withFb.left, 0);
                withFb.right = m_APFArray[j].processSingle(m_APFCoeffects[1], withFb.right, 1);
            }
            //auto mixout = sample + p.mix.get(i) * withFb;
            PolyFType mixout = sample + p.mix.getNextValue() * withFb;
            /*fbUpdate<channel>(withFb);*/
            m_fbValue.left = fbLF.processSample(0, fbHF.processSample(0, withFb.left));
            m_fbValue.right = fbLF.processSample(1, fbHF.processSample(1, withFb.right));
            fbLF.snapToZero();
            fbHF.snapToZero();
            data[i] = mixout;
        }
    }
private:
    // LFO
    TriSineShapeLFO m_mainDelayLFO;
    SinCosLFO m_leftBarberpoleLFO;
    SinCosLFO m_rightBarberpoleLFO;

    // 线性平滑器
    juce::SmoothedValue<FType> m_leftHertzSmoother;
    juce::SmoothedValue<FType> m_rightHertzSmoother;
    juce::SmoothedValue<FType> m_leftTZFDelaySmoother;
    juce::SmoothedValue<FType> m_rightTZFDelaySmoother;

    PhaserParameters& p;
    FType m_oneDivNyquistRate{};
    FType m_sampleRate{};
    PolyFType m_fbValue{};

    // 反馈滤波器
    juce::dsp::FirstOrderTPTFilter<FType> fbLF;
    juce::dsp::FirstOrderTPTFilter<FType> fbHF;

    // 全通滤波器组
    std::array<SecondOrderAllPassFilter2<FType, 2>, kMaxPhaserState> m_APFArray;
    std::array<SecondOrderAllPassFilter2<FType, 2>::Coeffects, 2> m_APFCoeffects;

    // 希尔伯特变换器
    IIRHilbertTransformer<FType, 4> hilbertL{IIRHilbertCoeffect::kCoeffects1<FType>};
    IIRHilbertTransformer<FType, 4> hilbertR{IIRHilbertCoeffect::kCoeffects1<FType>};
};
}

//================================================================================
// Flanger
//================================================================================
namespace rpSynth::audio::effects {
Phaser::Phaser(OrderableEffectsChain& c)
    : EffectProcessorBase(c, "Phaser") {
    m_allFlangerParameters = std::make_unique<PhaserParameters>();
    m_flangerImpl = std::make_unique<PhaserImpl>(*m_allFlangerParameters);
}

Phaser::~Phaser() {
    m_allFlangerParameters = nullptr;
    m_flangerImpl = nullptr;
}

void Phaser::addParameterToLayout(juce::AudioProcessorValueTreeState::ParameterLayout& layout) {
    EffectProcessorBase::addParameterToLayout(layout);
    layout.add(
        std::make_unique<MyHostParameter>(m_allFlangerParameters->beginSemitone,
                                                          combineWithID("MinHertz"),
                                                          "MinHertz",
                                                          juce::NormalisableRange(kStOf10hz, kStOf20000hz, 0.01f),
                                                          hertzToSemitone(440.f),
                                                          g_PitchHertzFloatParameterAttribute),
        std::make_unique<MyHostParameter>(m_allFlangerParameters->endSemitone,
                                                          combineWithID("MaxHertz"),
                                                          "MaxHertz",
                                                          juce::NormalisableRange(kStOf10hz, kStOf20000hz, 0.01f),
                                                          hertzToSemitone(880.f),
                                                          g_PitchHertzFloatParameterAttribute),
        std::make_unique<MyHostParameter>(m_allFlangerParameters->feedback,
                                                          combineWithID("Feedback"),
                                                          "Feedback",
                                                          juce::NormalisableRange(-0.99f, 0.99f, 0.01f),
                                                          0.f),
        std::make_unique<MyHostParameter>(m_allFlangerParameters->fbLowCut,
                                                          combineWithID("FB_LowCut"),
                                                          "FB_LowCut",
                                                          juce::NormalisableRange(20.f, 20000.f, 1.f),
                                                          20000.f),
        std::make_unique<MyHostParameter>(m_allFlangerParameters->fbHighCut,
                                                          combineWithID("FB_HighCut"),
                                                          "FB_HighCut",
                                                          juce::NormalisableRange(20.f, 20000.f, 1.f),
                                                          20.f),
        std::make_unique<MyHostParameter>(m_allFlangerParameters->mix,
                                                          combineWithID("mix"),
                                                          "mix",
                                                          juce::NormalisableRange<float>(-1.f, 1.f, 0.01f),
                                                          1.f),
        std::make_unique<MyHostParameter>(m_allFlangerParameters->rate,
                                                          combineWithID("Rate"),
                                                          "Rate",
                                                          juce::NormalisableRange(-2.f, 2.f, 0.1f),
                                                          0.f),
        std::make_unique<MyHostParameter>(m_allFlangerParameters->barberpoleRate,
                                                          combineWithID("BarberRate"),
                                                          "BarberRate",
                                                          juce::NormalisableRange(-10.f, 10.f, 0.1f),
                                                          0.f),
        std::make_unique<MyHostParameter>(m_allFlangerParameters->barberpolePhase,
                                                          combineWithID("BarberPhase"),
                                                          "BarberPhase",
                                                          juce::NormalisableRange(0.f, 0.5f, 0.01f),
                                                          0.f),
        std::make_unique<MyHostParameter>(m_allFlangerParameters->lfoPhase,
                                                          combineWithID("LFOPhase"),
                                                          "Phase",
                                                          juce::NormalisableRange(0.f, 0.5f, 0.01f),
                                                          0.f),
        std::make_unique<MyHostParameter>(m_allFlangerParameters->lfoShape,
                                                          combineWithID("LFOShape"),
                                                          "LFOShape",
                                                          juce::NormalisableRange(0.f, 1.f, 0.01f),
                                                          0.f),
        std::make_unique<MyHostParameter>(m_allFlangerParameters->spread,
                                                          combineWithID("Spread"),
                                                          "Spread",
                                                          juce::NormalisableRange(0.01f,0.49f,0.01f),
                                                          0.25f),
        std::make_unique<MyHostParameter>(m_allFlangerParameters->phaserState,
                                                          combineWithID("State"),
                                                          "State",
                                                          juce::NormalisableRange<float>(0,PhaserImpl::kMaxPhaserState,1),
                                                          4.f)
    );

    auto pDisable = std::make_unique<juce::AudioParameterBool>(combineWithID("disBarber"),
                                                               "Disable Barberpole",
                                                               true);
    m_allFlangerParameters->disableBarberpole = pDisable.get();
    layout.add(std::move(pDisable));
}

//void Phaser::updateParameters(size_t numSamples) {
//    m_allFlangerParameters->updateAll(numSamples);
//}

void Phaser::prepareParameters(FType sampleRate, size_t numSamples) {
    m_allFlangerParameters->prepareAll(sampleRate, numSamples);
}

void Phaser::prepare(FType sampleRate, size_t numSamlpes) {
    m_flangerImpl->prepare(sampleRate, numSamlpes);
}

void Phaser::saveExtraState(juce::XmlElement& /*xml*/) {
}

void Phaser::loadExtraState(juce::XmlElement& /*xml*/, juce::AudioProcessorValueTreeState& /*apvts*/) {
}

void Phaser::processBlock(StereoBuffer& block, size_t begin, size_t end) {
    m_flangerImpl->process(block, begin, end);
}

std::unique_ptr<ui::ContainModulableComponent> Phaser::createEffectPanel() {
    return std::make_unique<PhaserPanel>(*m_allFlangerParameters);
}

void Phaser::onCRClock(size_t n) {
    m_allFlangerParameters->onCRClock();
    m_flangerImpl->onCRClock(n);
}
}