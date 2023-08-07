#ifndef _Filter_  //v1.0
#define _Filter_



class Filter {  //滤波器
private:
  float tmp1 = 0.f, tmp2 = 0.f, out1 = 0.f, out2 = 0.f;
public:
  void reset() {
    tmp1 = 0, tmp2 = 0, out1 = 0, out2 = 0;
  }
  void AutoZero() {
    /*
    if (isnan(tmp1)) tmp1 = 0.0;
    if (isnan(tmp2)) tmp2 = 0.0;
    if (isnan(out1)) out1 = 0.0;
    if (isnan(out2)) out2 = 0.0;
    */
  }
  float LPF1(float vin, float ctof, float reso) {  //一阶低通带反馈
    float fb = reso + reso / (1.0f - ctof);
    tmp1 += ctof * (vin - tmp1 + fb * (tmp1 - out1));
    out1 += ctof * (tmp1 - out1);
    return out1;
  }
  float LPF1_limit(float vin, float ctof, float reso, float limVol = 52000.0f, float limK = 0.125f) {  //一阶低通带限制反馈
    float fb = reso + reso / (1.0f - ctof);
    tmp1 += ctof * (vin - tmp1 + fb * (tmp1 - out1));
    out1 += ctof * (tmp1 - out1);
    out1 = out1 > limVol ? ((out1 - limVol) * limK + limVol) : out1;
    out1 = out1 < -limVol ? ((out1 + limVol) * limK - limVol) : out1;
    tmp1 = tmp1 > limVol ? ((tmp1 - limVol) * limK + limVol) : tmp1;
    tmp1 = tmp1 < -limVol ? ((tmp1 + limVol) * limK - limVol) : tmp1;
    return out1;
  }
  float LPF1_Oversampling_limit(float vin, float ctof, float reso, float limVol = 52000.0, float limK = 0.5) {  //超采样一阶低通，可以和普通低通一样使用
    ctof *= 0.5f;
    float fb = reso + reso / (1.0f - ctof);
    tmp1 += ctof * (vin - tmp1 + fb * (tmp1 - out1));
    out1 += ctof * (tmp1 - out1);
    tmp1 += ctof * (vin - tmp1 + fb * (tmp1 - out1));
    out1 += ctof * (tmp1 - out1);
    out1 = out1 > limVol ? ((out1 - limVol) * limK + limVol) : out1;
    out1 = out1 < -limVol ? ((out1 + limVol) * limK - limVol) : out1;
    tmp1 = tmp1 > limVol ? ((tmp1 - limVol) * limK + limVol) : tmp1;
    tmp1 = tmp1 < -limVol ? ((tmp1 + limVol) * limK - limVol) : tmp1;
    return out1;
  }
  float LPF2(float vin, float ctof, float reso) {  //二阶的，同上
    float fb = reso + reso / (1.0f - ctof);
    tmp1 += ctof * (vin - tmp1 + fb * (tmp1 - out1));
    out1 += ctof * (tmp1 - out1);
    tmp2 += ctof * (out1 - tmp2 + fb * (tmp2 - out2));
    out2 += ctof * (tmp2 - out2);
    return out2;
  }
  float LPF2_limit(float vin, float ctof, float reso, float limVol = 52000.0, float limK = 0.125) {
    float fb = reso + reso / (1.0f - ctof);
    tmp1 += ctof * (vin - tmp1 + fb * (tmp1 - out1));
    out1 += ctof * (tmp1 - out1);
    tmp2 += ctof * (out1 - tmp2 + fb * (tmp2 - out2));
    out2 += ctof * (tmp2 - out2);
    out1 = out1 > limVol ? ((out1 - limVol) * limK + limVol) : out1;
    out1 = out1 < -limVol ? ((out1 + limVol) * limK - limVol) : out1;
    tmp1 = tmp1 > limVol ? ((tmp1 - limVol) * limK + limVol) : tmp1;
    tmp1 = tmp1 < -limVol ? ((tmp1 + limVol) * limK - limVol) : tmp1;
    out2 = out2 > limVol ? ((out2 - limVol) * limK + limVol) : out2;
    out2 = out2 < -limVol ? ((out2 + limVol) * limK - limVol) : out2;
    tmp2 = tmp2 > limVol ? ((tmp2 - limVol) * limK + limVol) : tmp2;
    tmp2 = tmp2 < -limVol ? ((tmp2 + limVol) * limK - limVol) : tmp2;
    return out2;
  }
  float LPF2_Oversampling_ResoLimit_limit(float vin, float ctof, float reso, float limVol = 52000.0, float limK = 0.125) {
    ctof *= 0.5f;
    float fb = reso + reso / (1.0f - ctof);
    tmp1 += ctof * (vin - tmp1);
    out1 += ctof * (tmp1 - out1);
    tmp2 += ctof * (out1 - tmp2 + fb * (tmp2 - out2));
    out2 += ctof * (tmp2 - out2);
    tmp1 += ctof * (vin - tmp1);
    out1 += ctof * (tmp1 - out1);
    tmp2 += ctof * (out1 - tmp2 + fb * (tmp2 - out2));
    out2 += ctof * (tmp2 - out2);
    out1 = out1 > limVol ? ((out1 - limVol) * limK + limVol) : out1;
    out1 = out1 < -limVol ? ((out1 + limVol) * limK - limVol) : out1;
    tmp1 = tmp1 > limVol ? ((tmp1 - limVol) * limK + limVol) : tmp1;
    tmp1 = tmp1 < -limVol ? ((tmp1 + limVol) * limK - limVol) : tmp1;
    out2 = out2 > limVol ? ((out2 - limVol) * limK + limVol) : out2;
    out2 = out2 < -limVol ? ((out2 + limVol) * limK - limVol) : out2;
    tmp2 = tmp2 > limVol ? ((tmp2 - limVol) * limK + limVol) : tmp2;
    tmp2 = tmp2 < -limVol ? ((tmp2 + limVol) * limK - limVol) : tmp2;
    return out2;
  }
  float LPF2_ResoLimit_limit(float vin, float ctof, float reso, float limVol = 52000.0, float limK = 0.125) {
    float fb = reso + reso / (1.0f - ctof);
    tmp1 += ctof * (vin - tmp1);
    out1 += ctof * (tmp1 - out1);
    tmp2 += ctof * (out1 - tmp2 + fb * (tmp2 - out2));
    out2 += ctof * (tmp2 - out2);
    out1 = out1 > limVol ? ((out1 - limVol) * limK + limVol) : out1;
    out1 = out1 < -limVol ? ((out1 + limVol) * limK - limVol) : out1;
    tmp1 = tmp1 > limVol ? ((tmp1 - limVol) * limK + limVol) : tmp1;
    tmp1 = tmp1 < -limVol ? ((tmp1 + limVol) * limK - limVol) : tmp1;
    out2 = out2 > limVol ? ((out2 - limVol) * limK + limVol) : out2;
    out2 = out2 < -limVol ? ((out2 + limVol) * limK - limVol) : out2;
    tmp2 = tmp2 > limVol ? ((tmp2 - limVol) * limK + limVol) : tmp2;
    tmp2 = tmp2 < -limVol ? ((tmp2 + limVol) * limK - limVol) : tmp2;
    return out2;
  }
  float LPF1_NoReso(float vin, float ctof) {  //低通
    tmp1 += ctof * (vin - tmp1);
    return tmp1;
  }
  float HPF1_NoReso(float vin, float ctof) {  //高通
    tmp1 += ctof * (vin - tmp1);
    return vin - tmp1;
  }
};

#endif  //code&test:Hiirofox