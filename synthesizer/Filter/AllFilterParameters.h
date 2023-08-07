/*
  ==============================================================================

    AllFilterParameters.h
    Created: 6 Aug 2023 8:57:46pm
    Author:  mana

  ==============================================================================
*/

#pragma once
#include "synthesizer/WrapParameter.h"

namespace rpSynth::audio::filters {
/**
 * @brief ���������˲����Ĳ�������
 *        ��Ӧ�ñ������˲���ʵ�ֹ���
*/
struct AllFilterParameters {
    MyAudioProcessParameter cutoff;
    MyAudioProcessParameter resonance;
    MyAudioProcessParameter limitVolume;
    MyAudioProcessParameter limitK;
};
};