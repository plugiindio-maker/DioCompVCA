#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <cmath>

using FParam = juce::AudioParameterFloat;
using NRange = juce::NormalisableRange<float>;
using FAttr  = juce::AudioParameterFloatAttributes;

namespace
{
    constexpr int kNumProfiles = 7, kNumProfileValues = 12;
    const char* const kProfileIds[kNumProfileValues] = {
        "thr", "ratio", "attack", "release", "knee", "hpf", "vcamode", "analogon", "drive", "color", "amix", "mix" };
    const char* const kProfileNames[kNumProfiles] = {
        "VOCAL", "KICK", "SNARE", "DRUM BUS", "808 / BASS", "INSTRUMENTAL", "MASTER BUS" };
    const char* const kTopoNames[3] = { "VCA", "OPTICO", "VARI-MU" };
    const char* const kModeNames[3] = { "SMOOTH", "PUNCH", "AGGRESSIVE" };

    // Sugerencias por perfil y topologia: [perfil][VCA, OPTICO, VARI-MU]
    // (vcamode: 0 SMOOTH, 1 PUNCH, 2 AGGRESSIVE; en OPTICO/VARI-MU no se usa)
    //                         thr    ratio atk    rel    knee  hpf    mode an   drv   col   amix  mix
    const float kProfiles[kNumProfiles][3][kNumProfileValues] = {
        { // VOCAL
            { -22.f, 3.5f, 12.f, 140.f,  9.f, 100.f, 0.f, 1.f, 15.f, 40.f, 40.f, 100.f },
            { -24.f, 4.f,  10.f, 300.f, 12.f,  80.f, 0.f, 1.f, 12.f, 55.f, 35.f, 100.f },
            { -22.f, 5.f,  20.f, 250.f, 10.f,  90.f, 0.f, 1.f, 18.f, 60.f, 40.f, 100.f } },
        { // KICK
            { -14.f, 4.f,  25.f,  80.f,  3.f,  30.f, 1.f, 1.f, 20.f, 30.f, 35.f, 100.f },
            { -18.f, 4.f,  10.f, 200.f, 12.f,  40.f, 0.f, 0.f, 20.f, 30.f, 35.f, 100.f },
            { -18.f, 4.f,  30.f, 150.f, 10.f,  35.f, 0.f, 1.f, 20.f, 40.f, 35.f, 100.f } },
        { // SNARE
            { -15.f, 5.f,  20.f,  90.f,  3.f, 120.f, 1.f, 1.f, 25.f, 35.f, 40.f, 100.f },
            { -18.f, 3.5f, 10.f, 250.f, 12.f, 100.f, 0.f, 0.f, 20.f, 35.f, 35.f, 100.f },
            { -17.f, 5.f,  25.f, 160.f, 10.f, 110.f, 0.f, 1.f, 22.f, 40.f, 40.f, 100.f } },
        { // DRUM BUS
            { -16.f, 4.f,  30.f, 100.f,  4.f,  80.f, 2.f, 1.f, 30.f, 45.f, 45.f, 100.f },
            { -18.f, 3.f,  10.f, 300.f, 12.f,  80.f, 0.f, 0.f, 20.f, 40.f, 35.f, 100.f },
            { -16.f, 4.f,  30.f, 200.f, 10.f,  80.f, 0.f, 1.f, 25.f, 45.f, 40.f, 100.f } },
        { // 808 / BASS
            { -20.f, 3.f,  25.f, 250.f,  8.f,  25.f, 0.f, 1.f, 25.f, 50.f, 40.f, 100.f },
            { -22.f, 3.f,  10.f, 400.f, 12.f,  25.f, 0.f, 1.f, 20.f, 50.f, 35.f, 100.f },
            { -22.f, 3.f,  30.f, 300.f, 10.f,  25.f, 0.f, 1.f, 22.f, 55.f, 35.f, 100.f } },
        { // INSTRUMENTAL
            { -18.f, 3.f,  15.f, 150.f,  8.f,  60.f, 0.f, 1.f, 20.f, 45.f, 35.f, 100.f },
            { -20.f, 3.f,  10.f, 300.f, 12.f,  60.f, 0.f, 0.f, 20.f, 45.f, 35.f, 100.f },
            { -18.f, 3.f,  20.f, 220.f, 10.f,  60.f, 0.f, 1.f, 20.f, 50.f, 35.f, 100.f } },
        { // MASTER BUS
            { -12.f, 2.f,  30.f, 150.f, 10.f,  80.f, 0.f, 0.f, 15.f, 40.f, 30.f, 100.f },
            { -14.f, 2.f,  10.f, 400.f, 12.f,  80.f, 0.f, 0.f, 15.f, 40.f, 30.f, 100.f },
            { -12.f, 2.5f, 30.f, 250.f, 10.f,  80.f, 0.f, 1.f, 10.f, 40.f, 30.f, 100.f } },
    };
    constexpr int kNumPresets = 1 + kNumProfiles * 3;

    inline float lnCosh (float u)
    {
        const float a = std::abs (u);
        return a + std::log1p (std::exp (-2.f * a)) - 0.69314718f;   // ln(cosh(u)) estable
    }

    // Saturacion con antiderivada (ADAA): casi sin aliasing, sin sobremuestreo ni latencia.
    // bias > 0 = asimetria (armonicos pares); bias = 0 = simetrica (armonicos impares)
    inline float adaaSat (float x, float k, float bias, float& pU, float& pLn, float& dcX, float& dcY, float dcR)
    {
        const float u = k * x + bias;
        const float ln = lnCosh (u);
        const float du = u - pU;
        float s = std::abs (du) > 1.0e-3f ? (ln - pLn) / (k * du) : std::tanh (0.5f * (u + pU)) / k;
        pU = u;  pLn = ln;
        s -= std::tanh (bias) / k;
        dcY = s - dcX + dcR * dcY;                 // bloqueador de DC
        dcX = s;
        return dcY;
    }

    inline float softClip (float x, float ceilLin)
    {
        const float t = 0.7f * ceilLin, ax = std::abs (x);
        if (ax <= t) return x;                                       // bajo el 70% no toca nada (transitorios intactos)
        const float r = ceilLin - t;
        const float y = t + r * std::tanh ((ax - t) / r);            // nunca pasa del ceiling
        return x < 0.f ? -y : y;
    }

    inline float gainCurve (float over, float r, float k)            // reduccion (dB, positiva) con soft knee
    {
        if (2.f * over > k) return over * (1.f - 1.f / r);
        if (2.f * over > -k)
        {
            const float t = over + k * 0.5f;
            return (1.f - 1.f / r) * t * t / (2.f * juce::jmax (k, 0.001f));
        }
        return 0.f;
    }

    inline void follow (float& s, float t, float ca, float cr)       // seguidor con ataque / release
    {
        const float c = t > s ? ca : cr;
        s = c * s + (1.f - c) * t;
    }
}

// ------------------------------------------------------------------ presets y perfiles
int DioCompressProcessor::getNumPrograms() { return kNumPresets; }

const juce::String DioCompressProcessor::getProgramName (int index)
{
    if (index == 0) return "Init";
    if (index < 0 || index >= kNumPresets) return {};
    const int p = (index - 1) / 3, t = (index - 1) % 3;
    juce::String topo = (t == 0) ? juce::String ("VCA ") + kModeNames[juce::jlimit (0, 2, juce::roundToInt (kProfiles[p][0][6]))]
                                 : juce::String (kTopoNames[t]);
    return juce::String (kProfileNames[p]) + " - " + topo;
}

void DioCompressProcessor::setParam (const char* id, float v)
{
    if (auto* p = apvts.getParameter (id))
        p->setValueNotifyingHost (p->convertTo0to1 (v));
}

void DioCompressProcessor::applyInit()
{
    setParam ("thr", -18.f);   setParam ("ratio", 4.f);   setParam ("attack", 10.f);  setParam ("release", 150.f);
    setParam ("knee", 6.f);    setParam ("hpf", 20.f);    setParam ("vcamode", 0.f);  setParam ("analogon", 0.f);
    setParam ("drive", 25.f);  setParam ("color", 50.f);  setParam ("amix", 50.f);    setParam ("mix", 100.f);
    setParam ("makeup", 0.f);  setParam ("input", 0.f);   setParam ("out", 0.f);      setParam ("automk", 1.f);
    setParam ("ceil", -0.3f);  setParam ("safety", 0.f);
}

void DioCompressProcessor::applyProfileFor (int profile, int topology)
{
    profile = juce::jlimit (0, kNumProfiles - 1, profile);
    topology = juce::jlimit (0, 2, topology);
    currentProfile = profile;
    for (int i = 0; i < kNumProfileValues; ++i)
        setParam (kProfileIds[i], kProfiles[profile][topology][i]);
    setParam ("makeup", 0.f);
    setParam ("automk", 1.f);
}

void DioCompressProcessor::applyProfile (int profile)
{
    applyProfileFor (profile, juce::roundToInt (pTopo->load()));
}

void DioCompressProcessor::setCurrentProgram (int index)
{
    index = juce::jlimit (0, kNumPresets - 1, index);
    currentPreset = index;
    if (index == 0) { setParam ("topology", 0.f); applyInit(); currentProfile = -1; return; }
    const int p = (index - 1) / 3, t = (index - 1) % 3;
    setParam ("topology", (float) t);
    applyProfileFor (p, t);
}

// ------------------------------------------------------------------ parametros
juce::AudioProcessorValueTreeState::ParameterLayout DioCompressProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout l;
    auto d0 = [] (float v, int) { return juce::String (juce::roundToInt (v)); };
    auto d1 = [] (float v, int) { return juce::String (v, 1); };

    l.add (std::make_unique<juce::AudioParameterChoice> (juce::ParameterID { "topology", 1 }, "Topology",
                                                         juce::StringArray { "VCA", "OPTICO", "VARI-MU" }, 0));
    l.add (std::make_unique<juce::AudioParameterChoice> (juce::ParameterID { "vcamode", 1 }, "VCA Mode",
                                                         juce::StringArray { "SMOOTH", "PUNCH", "AGGRESSIVE" }, 0));
    l.add (std::make_unique<FParam> (juce::ParameterID { "input", 1 },   "Input",     NRange (-24.f, 24.f, 0.1f), 0.f, FAttr().withLabel ("dB").withStringFromValueFunction (d1)));
    l.add (std::make_unique<FParam> (juce::ParameterID { "thr", 1 },     "Threshold", NRange (-60.f, 0.f, 0.1f), -18.f, FAttr().withLabel ("dB").withStringFromValueFunction (d1)));
    l.add (std::make_unique<FParam> (juce::ParameterID { "ratio", 1 },   "Ratio",     NRange (1.f, 20.f, 0.1f, 0.5f), 4.f, FAttr().withStringFromValueFunction (d1)));
    l.add (std::make_unique<FParam> (juce::ParameterID { "attack", 1 },  "Attack",    NRange (0.1f, 100.f, 0.01f, 0.4f), 10.f, FAttr().withLabel ("ms").withStringFromValueFunction (d1)));
    l.add (std::make_unique<FParam> (juce::ParameterID { "release", 1 }, "Release",   NRange (20.f, 5000.f, 1.f, 0.35f), 150.f, FAttr().withLabel ("ms").withStringFromValueFunction (d0)));
    l.add (std::make_unique<FParam> (juce::ParameterID { "knee", 1 },    "Knee",      NRange (0.f, 18.f, 0.1f), 6.f, FAttr().withLabel ("dB").withStringFromValueFunction (d1)));
    l.add (std::make_unique<FParam> (juce::ParameterID { "makeup", 1 },  "Makeup",    NRange (-12.f, 24.f, 0.1f), 0.f, FAttr().withLabel ("dB").withStringFromValueFunction (d1)));
    l.add (std::make_unique<FParam> (juce::ParameterID { "out", 1 },     "Output",    NRange (-18.f, 18.f, 0.1f), 0.f, FAttr().withLabel ("dB").withStringFromValueFunction (d1)));
    l.add (std::make_unique<FParam> (juce::ParameterID { "mix", 1 },     "Dry/Wet",   NRange (0.f, 100.f, 1.f), 100.f, FAttr().withLabel ("%").withStringFromValueFunction (d0)));
    l.add (std::make_unique<FParam> (juce::ParameterID { "hpf", 1 },     "SC HPF",    NRange (20.f, 500.f, 1.f, 0.4f), 20.f, FAttr().withLabel ("Hz").withStringFromValueFunction (d0)));
    l.add (std::make_unique<juce::AudioParameterBool> (juce::ParameterID { "automk", 1 },  "Auto Makeup", true));
    l.add (std::make_unique<juce::AudioParameterBool> (juce::ParameterID { "bypass", 1 },  "Bypass", false));

    l.add (std::make_unique<juce::AudioParameterChoice> (juce::ParameterID { "safety", 1 }, "Safety",
                                                         juce::StringArray { "OFF", "LIMIT", "CLIP" }, 0));
    l.add (std::make_unique<FParam> (juce::ParameterID { "ceil", 1 },    "Ceiling",   NRange (-12.f, 0.f, 0.1f), -0.3f, FAttr().withLabel ("dB").withStringFromValueFunction (d1)));

    l.add (std::make_unique<juce::AudioParameterBool> (juce::ParameterID { "analogon", 1 }, "Analog On", false));
    l.add (std::make_unique<FParam> (juce::ParameterID { "drive", 1 },   "Analog Drive", NRange (0.f, 100.f, 1.f), 25.f, FAttr().withLabel ("%").withStringFromValueFunction (d0)));
    l.add (std::make_unique<FParam> (juce::ParameterID { "color", 1 },   "Analog Color", NRange (0.f, 100.f, 1.f), 50.f, FAttr().withLabel ("%").withStringFromValueFunction (d0)));
    l.add (std::make_unique<FParam> (juce::ParameterID { "amix", 1 },    "Analog Mix",   NRange (0.f, 100.f, 1.f), 50.f, FAttr().withLabel ("%").withStringFromValueFunction (d0)));
    return l;
}

DioCompressProcessor::DioCompressProcessor()
    : AudioProcessor (BusesProperties()
                        .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                        .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "PARAMS", createParameterLayout())
{
    pTopo = apvts.getRawParameterValue ("topology");   pVmode = apvts.getRawParameterValue ("vcamode");
    pIn = apvts.getRawParameterValue ("input");        pThr = apvts.getRawParameterValue ("thr");
    pRatio = apvts.getRawParameterValue ("ratio");     pAtk = apvts.getRawParameterValue ("attack");
    pRel = apvts.getRawParameterValue ("release");     pKnee = apvts.getRawParameterValue ("knee");
    pMakeup = apvts.getRawParameterValue ("makeup");   pMix = apvts.getRawParameterValue ("mix");
    pHpf = apvts.getRawParameterValue ("hpf");         pOut = apvts.getRawParameterValue ("out");
    pCeil = apvts.getRawParameterValue ("ceil");       pAutoMk = apvts.getRawParameterValue ("automk");
    pBypass = apvts.getRawParameterValue ("bypass");   pSafety = apvts.getRawParameterValue ("safety");
    pAnalog = apvts.getRawParameterValue ("analogon"); pDrive = apvts.getRawParameterValue ("drive");
    pColor = apvts.getRawParameterValue ("color");     pAmix = apvts.getRawParameterValue ("amix");
}

float DioCompressProcessor::currentMakeupDb() const
{
    float mk = pMakeup->load();
    if (pAutoMk->load() > 0.5f)
    {
        const int topo = juce::jlimit (0, 2, juce::roundToInt (pTopo->load()));
        const float r = pRatio->load();
        float re = r;                                                     // ratio efectivo de cada topologia
        if (topo == 0)      re = (juce::roundToInt (pVmode->load()) == 2) ? juce::jmin (30.f, r * 1.35f) : r;
        else if (topo == 1) re = 1.f + 0.55f * (r - 1.f);
        else                re = 1.f + 0.60f * (r - 1.f);
        mk += -pThr->load() * (1.f - 1.f / juce::jmax (1.f, re)) * 0.5f;  // compensacion aproximada
    }
    return mk;
}

void DioCompressProcessor::clearState()
{
    for (int c = 0; c < 2; ++c)
    {
        hpState[c] = aLp1[c] = aLp2[c] = 0.f;
        for (int b = 0; b < 2; ++b) aPU[c][b] = aPLn[c][b] = aDcX[c][b] = aDcY[c][b] = 0.f;
    }
    ms20 = ms40 = 0.f;
    vF = vS = oF = oS = oMem = mF = mS = 0.f;
    aMsIn = aMsOut = 0.f;
    tgBuf.assign ((size_t) limLen, 1.f);
    minBuf.assign ((size_t) limLen, 1.f);
    limPos = 0;
    tgActive = minActive = 0;
    minSum = (double) limLen;
    limGain = 1.f;
    for (auto& d : delayBuf) d.assign ((size_t) delaySize, 0.f);
    for (auto& d : inDelay)  d.assign ((size_t) delaySize, 0.f);
    dWrite = 0;
    grMeter.store (0.f);
    inMeter.store (-80.f);
    outMeter.store (-80.f);
}

void DioCompressProcessor::reset() { clearState(); }

void DioCompressProcessor::prepareToPlay (double sampleRate, int)
{
    fs = sampleRate;
    limLen = juce::jlimit (8, 500, juce::roundToInt (fs * 0.0015));
    dDelay = limLen - 1;
    clearState();
    setLatencySamples (dDelay);

    auto init = [this] (juce::SmoothedValue<float>& s, double sec, float v)
    {
        s.reset (fs, sec);
        s.setCurrentAndTargetValue (v);
    };
    const int topo = juce::jlimit (0, 2, juce::roundToInt (pTopo->load()));
    init (smIn, 0.03, juce::Decibels::decibelsToGain (pIn->load()));
    init (smMix, 0.03, pMix->load() * 0.01f);
    init (smOut, 0.03, juce::Decibels::decibelsToGain (pOut->load()));
    init (smMakeup, 0.03, currentMakeupDb());
    init (smDrive, 0.03, pDrive->load() * 0.01f);
    init (smColor, 0.03, pColor->load() * 0.01f);
    init (smAmix, 0.03, pAmix->load() * 0.01f);
    init (smAnalog, 0.03, pAnalog->load() > 0.5f ? 1.f : 0.f);
    init (smBypass, 0.02, pBypass->load() > 0.5f ? 1.f : 0.f);
    init (smWV, 0.04, topo == 0 ? 1.f : 0.f);
    init (smWO, 0.04, topo == 1 ? 1.f : 0.f);
    init (smWM, 0.04, topo == 2 ? 1.f : 0.f);
}

bool DioCompressProcessor::isBusesLayoutSupported (const BusesLayout& l) const
{
    auto in = l.getMainInputChannelSet();
    if (in != juce::AudioChannelSet::mono() && in != juce::AudioChannelSet::stereo())
        return false;
    return in == l.getMainOutputChannelSet();
}

void DioCompressProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;
    const int numCh = juce::jmin (buffer.getNumChannels(), 2);
    const int n = buffer.getNumSamples();
    for (int c = getTotalNumInputChannels(); c < getTotalNumOutputChannels(); ++c)
        buffer.clear (c, 0, n);
    if (numCh == 0 || n == 0) return;

    // --- parametros ---
    const int topo  = juce::jlimit (0, 2, juce::roundToInt (pTopo->load()));
    const int vmode = juce::jlimit (0, 2, juce::roundToInt (pVmode->load()));
    const float thr = pThr->load(), ratio = pRatio->load(), knee = pKnee->load();
    const float atkMs = pAtk->load(), relMs = pRel->load();
    const int safety = juce::roundToInt (pSafety->load());     // 0 OFF, 1 LIMIT, 2 CLIP
    const float ceilLin = juce::Decibels::decibelsToGain (pCeil->load());

    smIn.setTargetValue (juce::Decibels::decibelsToGain (pIn->load()));
    smMix.setTargetValue (pMix->load() * 0.01f);
    smOut.setTargetValue (juce::Decibels::decibelsToGain (pOut->load()));
    smMakeup.setTargetValue (currentMakeupDb());
    smDrive.setTargetValue (pDrive->load() * 0.01f);
    smColor.setTargetValue (pColor->load() * 0.01f);
    smAmix.setTargetValue (pAmix->load() * 0.01f);
    smAnalog.setTargetValue (pAnalog->load() > 0.5f ? 1.f : 0.f);
    smBypass.setTargetValue (pBypass->load() > 0.5f ? 1.f : 0.f);
    smWV.setTargetValue (topo == 0 ? 1.f : 0.f);
    smWO.setTargetValue (topo == 1 ? 1.f : 0.f);
    smWM.setTargetValue (topo == 2 ? 1.f : 0.f);

    auto coefMs = [this] (float ms) { return std::exp (-1.f / (juce::jmax (0.01f, ms) * 0.001f * (float) fs)); };
    const float hpA = 1.f - std::exp (-juce::MathConstants<float>::twoPi * pHpf->load() / (float) fs);
    const float c20 = 1.f - coefMs (20.f), c40 = 1.f - coefMs (40.f);

    // VCA: tiempos segun la personalidad
    const float vAtk0 = coefMs (juce::jmax (atkMs, 6.f));                        // SMOOTH: ataque minimo 6 ms (cuerpo)
    const float vRel0F = coefMs (relMs * 0.4f), vRel0S = coefMs (relMs * 1.6f);
    const float vRel1 = coefMs (relMs * 0.7f);                                   // PUNCH
    const float vAtk2 = coefMs (juce::jmax (0.1f, atkMs * 0.4f));                // AGGRESSIVE
    const float vRel2F = coefMs (relMs * 0.3f), vRel2S = coefMs (relMs * 1.2f);
    // OPTICO: celda con memoria
    const float ratioO = 1.f + 0.55f * (ratio - 1.f);
    const float oRelF = coefMs (60.f), oRelS = coefMs (juce::jlimit (400.f, 8000.f, relMs * 2.5f));
    const float memAtk = 1.f - coefMs (1500.f), memRel = 1.f - coefMs (6000.f);
    // VARI-MU: ratio progresivo
    const float aM = 1.f - 1.f / juce::jmax (1.f, ratio);

    // ANALOG COLOR
    const float subA = 1.f - std::exp (-juce::MathConstants<float>::twoPi * 70.f * 1.554f / (float) fs);
    const float subKeep = 0.3f;                                                  // el subgrave recibe solo 30% de la saturacion
    const float dcR = 1.f - juce::MathConstants<float>::twoPi * 8.f / (float) fs;
    const float chC = 1.f - coefMs (30.f);

    const float limRelC = coefMs (100.f);

    float maxGr = 0.f, inPk = 0.f, outPk = 0.f;
    float* data[2] { buffer.getWritePointer (0), numCh > 1 ? buffer.getWritePointer (1) : nullptr };

    for (int i = 0; i < n; ++i)
    {
        // entrada segura (evita NaN/Inf) y ganancia de entrada
        float raw[2] { 0.f, 0.f }, x[2] { 0.f, 0.f };
        const float inG = smIn.getNextValue();
        for (int c = 0; c < numCh; ++c)
        {
            const float v = data[c][i];
            raw[c] = std::isfinite (v) ? v : 0.f;
            x[c] = raw[c] * inG;
            inPk = juce::jmax (inPk, std::abs (x[c]));
        }

        // --- detector comun (sidechain con HPF, enlazado estereo) ---
        float sc = 0.f;
        for (int c = 0; c < numCh; ++c)
        {
            hpState[c] += hpA * (x[c] - hpState[c]);
            sc = juce::jmax (sc, std::abs (x[c] - hpState[c]));
        }
        ms20 += c20 * (sc * sc - ms20);
        ms40 += c40 * (sc * sc - ms40);
        const float pkDb    = juce::Decibels::gainToDecibels (sc, -100.f);
        const float rms20Db = juce::Decibels::gainToDecibels (std::sqrt (juce::jmax (0.f, ms20)), -100.f);
        const float rms40Db = juce::Decibels::gainToDecibels (std::sqrt (juce::jmax (0.f, ms40)), -100.f);

        // === VCA ===
        float lvlV, ratioV, kneeV;
        if (vmode == 0)      { lvlV = rms20Db; ratioV = ratio; kneeV = knee + 4.f; }                            // SMOOTH: RMS
        else if (vmode == 1) { lvlV = pkDb;    ratioV = ratio; kneeV = knee; }                                  // PUNCH: picos
        else                 { lvlV = juce::jmax (pkDb, rms20Db + 3.f); ratioV = juce::jmin (30.f, ratio * 1.35f); kneeV = knee * 0.5f; }
        const float tgtV = gainCurve (lvlV - thr, ratioV, kneeV);
        float grV;
        if (vmode == 0)
        {
            follow (vF, tgtV, vAtk0, vRel0F);  follow (vS, tgtV, vAtk0, vRel0S);
            grV = 0.5f * (vF + vS);
        }
        else if (vmode == 1)
        {
            // ataque adaptativo: deja pasar el transitorio si la reduccion es pequena, lo atrapa si es grande
            const float t = juce::jlimit (0.f, 1.f, (tgtV - 3.f) / 5.f);
            follow (vF, tgtV, coefMs (atkMs * (1.6f - 1.1f * t)), vRel1);
            vS = vF;
            grV = vF;
        }
        else
        {
            follow (vF, tgtV, vAtk2, vRel2F);  follow (vS, tgtV, vAtk2, vRel2S);
            grV = 0.6f * vF + 0.4f * vS;
        }

        // === OPTICO: tiempos que dependen del programa + memoria de la celda ===
        const float tgtO = gainCurve (rms20Db - thr, ratioO, 12.f);
        const float wSprev = 0.25f + 0.65f * oMem;
        const float prevO = (1.f - wSprev) * oF + wSprev * oS;
        const float errO = juce::jmax (0.f, tgtO - oF);
        const float caO = coefMs (juce::jmax (1.5f, 10.f / (1.f + 0.1f * errO)));   // mas rapido cuanto mas empuja
        follow (oF, tgtO, caO, oRelF);
        follow (oS, tgtO, caO, oRelS);
        const float mT = juce::jlimit (0.f, 1.f, prevO / 12.f);
        oMem += (mT - oMem) * (mT > oMem ? memAtk : memRel);
        const float wS = 0.25f + 0.65f * oMem;                                        // mas compresion sostenida = cola mas larga
        const float grO = (1.f - wS) * oF + wS * oS;

        // === VARI-MU: ratio progresivo segun el nivel ===
        const float oM = juce::jmax (0.f, rms40Db - thr);
        const float tgtM = aM * oM * oM / (oM + 10.f);                                // ratio sube con la reduccion
        const float grMprev = 0.55f * mF + 0.45f * mS;
        const float caM = coefMs (juce::jmax (0.3f, atkMs / (1.f + 0.05f * tgtM)));
        const float rsc = 1.f + grMprev / 15.f;                                       // release mas lento cuanto mas profundo
        follow (mF, tgtM, caM, coefMs (relMs * 0.35f * rsc));
        follow (mS, tgtM, caM, coefMs (relMs * 2.0f * rsc));
        const float grM = 0.55f * mF + 0.45f * mS;

        // mezcla suave entre topologias (sin clicks)
        const float wV = smWV.getNextValue(), wO = smWO.getNextValue(), wM = smWM.getNextValue();
        const float gr = wV * grV + wO * grO + wM * grM;
        maxGr = juce::jmax (maxGr, gr);

        const float g = juce::Decibels::decibelsToGain (-gr + smMakeup.getNextValue());
        const float mix = smMix.getNextValue();
        float y[2] { 0.f, 0.f };
        for (int c = 0; c < numCh; ++c)
            y[c] = x[c] * (1.f - mix) + x[c] * g * mix;                               // Dry/Wet (paralelo)

        // === ANALOG COLOR (modulo independiente, despues de la compresion) ===
        {
            const float aAmt = smAnalog.getNextValue() * smAmix.getNextValue();
            const float drive = smDrive.getNextValue(), color = smColor.getNextValue();
            const float k = 1.f + 7.f * drive;                   // DRIVE: cuanto se empuja la saturacion
            const float bias = 0.55f * color;                    // COLOR: asimetria = armonicos pares (calido) vs impares (agresivo)
            const float dAmt = juce::jlimit (0.f, 1.f, drive * 3.f);   // con DRIVE en 0 la senal queda limpia
            float wet[2] { 0.f, 0.f };
            float eIn = 0.f, eOut = 0.f;
            for (int c = 0; c < numCh; ++c)
            {
                aLp1[c] += subA * (y[c] - aLp1[c]);
                aLp2[c] += subA * (aLp1[c] - aLp2[c]);
                const float lo = aLp2[c], hi = y[c] - lo;
                const float sh = adaaSat (hi, k, bias, aPU[c][0], aPLn[c][0], aDcX[c][0], aDcY[c][0], dcR);
                const float sl = adaaSat (lo, k, bias, aPU[c][1], aPLn[c][1], aDcX[c][1], aDcY[c][1], dcR);
                wet[c] = (hi + dAmt * (sh - hi)) + (lo + dAmt * subKeep * (sl - lo));
                eIn += y[c] * y[c];
                eOut += wet[c] * wet[c];
            }
            const float invCh = 1.f / (float) numCh;
            aMsIn  += chC * (eIn  * invCh - aMsIn);
            aMsOut += chC * (eOut * invCh - aMsOut);
            const float comp = juce::jlimit (0.5f, 2.5f, std::sqrt ((aMsIn + 1.0e-12f) / (aMsOut + 1.0e-12f)));   // nivel controlado
            for (int c = 0; c < numCh; ++c)
                y[c] += aAmt * (wet[c] * comp - y[c]);
        }

        // --- volumen de salida ---
        const float outG = smOut.getNextValue();
        float peak = 0.f;
        for (int c = 0; c < numCh; ++c)
        {
            y[c] *= outG;
            peak = juce::jmax (peak, std::abs (y[c]));
        }

        // --- SAFETY modo LIMIT: limitador con lookahead (min + promedio) ---
        const float tg = (safety == 1 && peak > ceilLin) ? ceilLin / peak : 1.f;
        {
            const size_t p = (size_t) limPos;
            if (tgBuf[p] < 1.f) --tgActive;
            tgBuf[p] = tg;
            if (tg < 1.f) ++tgActive;

            float minG = 1.f;
            if (tgActive > 0)
                for (int k2 = 0; k2 < limLen; ++k2) minG = juce::jmin (minG, tgBuf[(size_t) k2]);

            if (minBuf[p] < 1.f) --minActive;
            minSum += (double) minG - (double) minBuf[p];
            minBuf[p] = minG;
            if (minG < 1.f) ++minActive;
        }
        float avg = 1.f;
        if (minActive > 0) avg = juce::jlimit (0.f, 1.f, (float) (minSum / (double) limLen));
        else               minSum = (double) limLen;
        limPos = (limPos + 1) % limLen;
        limGain = avg < limGain ? avg : limRelC * limGain + (1.f - limRelC) * avg;

        // --- retardo fijo (latencia constante), bypass con crossfade alineado y salida ---
        const float bp = smBypass.getNextValue();
        for (int c = 0; c < numCh; ++c)
        {
            delayBuf[c][(size_t) dWrite] = y[c];
            inDelay[c][(size_t) dWrite] = raw[c];
            const size_t rd = (size_t) ((dWrite - dDelay + delaySize) & delayMask);
            float o = delayBuf[c][rd];
            if (safety == 1)      o = juce::jlimit (-ceilLin, ceilLin, o * limGain);
            else if (safety == 2) o = softClip (o, ceilLin);
            o = o * (1.f - bp) + inDelay[c][rd] * bp;
            data[c][i] = o;
            outPk = juce::jmax (outPk, std::abs (o));
        }
        dWrite = (dWrite + 1) & delayMask;
    }
    grMeter.store (maxGr);
    inMeter.store (juce::Decibels::gainToDecibels (inPk, -80.f));
    outMeter.store (juce::Decibels::gainToDecibels (outPk, -80.f));
}

juce::AudioProcessorEditor* DioCompressProcessor::createEditor() { return new DioCompressEditor (*this); }

void DioCompressProcessor::getStateInformation (juce::MemoryBlock& dest)
{
    if (auto xml = apvts.copyState().createXml())
    {
        xml->setAttribute ("uiWidth", savedEditorWidth);
        xml->setAttribute ("profile", currentProfile);
        copyXmlToBinary (*xml, dest);
    }
}

void DioCompressProcessor::setStateInformation (const void* data, int size)
{
    if (auto xml = getXmlFromBinary (data, size))
        if (xml->hasTagName (apvts.state.getType()))
        {
            savedEditorWidth = juce::jlimit (440, 1650, xml->getIntAttribute ("uiWidth", 770));
            currentProfile = xml->getIntAttribute ("profile", -1);
            apvts.replaceState (juce::ValueTree::fromXml (*xml));
        }
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new DioCompressProcessor(); }
