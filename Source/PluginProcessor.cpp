#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <cmath>

using Param = juce::AudioParameterFloat;
using Range = juce::NormalisableRange<float>;
using Attr  = juce::AudioParameterFloatAttributes;

// ------------------------------------------------------------------ presets
namespace
{
    constexpr int kNumPresetValues = 17;
    const char* const kPresetIds[kNumPresetValues] = {
        "thr", "ratio", "attack", "release", "knee", "makeup", "mix", "hpf", "out", "ceil",
        "autorel", "automk", "rms", "guard", "body", "weight", "freq" };

    struct Preset { const char* name; float v[kNumPresetValues]; };

    //                      thr    ratio atk  rel  knee mk  mix  hpf  out ceil   aRel aMk rms grd body wgt freq
    const Preset kPresets[] = {
        { "Default",        { -18.f, 4.f,  10.f, 120.f, 6.f, 0.f, 100.f, 20.f,  0.f, -0.3f, 0.f, 1.f, 0.f, 1.f,  0.f,  0.f, 100.f } },
        { "Vocal",          { -22.f, 3.5f, 12.f, 140.f, 9.f, 0.f, 100.f, 100.f, 0.f, -0.3f, 1.f, 1.f, 0.f, 1.f, 12.f,  0.f, 120.f } },
        { "Drums",          { -16.f, 4.f,  25.f,  80.f, 4.f, 0.f,  75.f, 80.f,  0.f, -0.3f, 0.f, 1.f, 0.f, 1.f, 20.f, 10.f,  90.f } },
        { "Mix Bus Glue",   { -14.f, 2.f,  30.f, 150.f,10.f, 0.f, 100.f, 80.f,  0.f, -0.3f, 1.f, 1.f, 1.f, 1.f,  0.f,  0.f, 100.f } },
        { "Instruments",    { -18.f, 3.f,  15.f, 150.f, 8.f, 0.f,  85.f, 60.f,  0.f, -0.3f, 0.f, 1.f, 0.f, 1.f, 15.f,  0.f, 120.f } },
        { "808 Punch",      { -20.f, 5.f,  25.f, 220.f, 4.f, 0.f, 100.f, 30.f,  0.f, -0.5f, 0.f, 1.f, 1.f, 1.f, 40.f, 25.f,  90.f } },
        { "808 Heavy",      { -22.f, 6.f,  20.f, 280.f, 3.f, 0.f, 100.f, 30.f,  0.f, -0.5f, 0.f, 1.f, 1.f, 1.f, 70.f, 55.f,  80.f } },
    };
    constexpr int kNumPresets = (int) (sizeof (kPresets) / sizeof (kPresets[0]));
}

int VCACompAudioProcessor::getNumPrograms() { return kNumPresets; }

const juce::String VCACompAudioProcessor::getProgramName (int index)
{
    return (index >= 0 && index < kNumPresets) ? juce::String (kPresets[index].name) : juce::String();
}

void VCACompAudioProcessor::setCurrentProgram (int index)
{
    index = juce::jlimit (0, kNumPresets - 1, index);
    currentPreset = index;
    for (int i = 0; i < kNumPresetValues; ++i)
        if (auto* p = apvts.getParameter (kPresetIds[i]))
            p->setValueNotifyingHost (p->convertTo0to1 (kPresets[index].v[i]));
}

// ------------------------------------------------------------------ parametros
juce::AudioProcessorValueTreeState::ParameterLayout VCACompAudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout l;
    auto d0 = [] (float v, int) { return juce::String (juce::roundToInt (v)); };
    auto d1 = [] (float v, int) { return juce::String (v, 1); };

    l.add (std::make_unique<Param> (juce::ParameterID { "thr", 1 },    "Threshold", Range (-60.f, 0.f, 0.1f), -18.f, Attr().withLabel ("dB").withStringFromValueFunction (d1)));
    l.add (std::make_unique<Param> (juce::ParameterID { "ratio", 1 },  "Ratio",     Range (1.f, 20.f, 0.1f, 0.5f), 4.f, Attr().withStringFromValueFunction (d1)));
    l.add (std::make_unique<Param> (juce::ParameterID { "attack", 1 }, "Attack",    Range (0.1f, 100.f, 0.01f, 0.4f), 10.f, Attr().withLabel ("ms").withStringFromValueFunction (d1)));
    l.add (std::make_unique<Param> (juce::ParameterID { "release", 1 },"Release",   Range (10.f, 1500.f, 0.1f, 0.4f), 120.f, Attr().withLabel ("ms").withStringFromValueFunction (d0)));
    l.add (std::make_unique<Param> (juce::ParameterID { "knee", 1 },   "Knee",      Range (0.f, 18.f, 0.1f), 6.f, Attr().withLabel ("dB").withStringFromValueFunction (d1)));
    l.add (std::make_unique<Param> (juce::ParameterID { "makeup", 1 }, "Makeup",    Range (-12.f, 24.f, 0.1f), 0.f, Attr().withLabel ("dB").withStringFromValueFunction (d1)));
    l.add (std::make_unique<Param> (juce::ParameterID { "mix", 1 },    "Mix",       Range (0.f, 100.f, 1.f), 100.f, Attr().withLabel ("%").withStringFromValueFunction (d0)));
    l.add (std::make_unique<Param> (juce::ParameterID { "hpf", 1 },    "SC HPF",    Range (20.f, 500.f, 1.f, 0.4f), 20.f, Attr().withLabel ("Hz").withStringFromValueFunction (d0)));
    l.add (std::make_unique<Param> (juce::ParameterID { "out", 1 },    "Output",    Range (-12.f, 12.f, 0.1f), 0.f, Attr().withLabel ("dB").withStringFromValueFunction (d1)));
    l.add (std::make_unique<Param> (juce::ParameterID { "ceil", 1 },   "Ceiling",   Range (-12.f, 0.f, 0.1f), -0.3f, Attr().withLabel ("dB").withStringFromValueFunction (d1)));

    l.add (std::make_unique<juce::AudioParameterBool> (juce::ParameterID { "autorel", 1 }, "Auto Release", false));
    l.add (std::make_unique<juce::AudioParameterBool> (juce::ParameterID { "automk", 1 },  "Auto Makeup",  true));
    l.add (std::make_unique<juce::AudioParameterBool> (juce::ParameterID { "rms", 1 },     "RMS Detect",   false));
    l.add (std::make_unique<juce::AudioParameterBool> (juce::ParameterID { "guard", 1 },   "Peak Guard",   true));

    l.add (std::make_unique<Param> (juce::ParameterID { "body", 1 },   "Body",      Range (0.f, 100.f, 1.f), 0.f, Attr().withLabel ("%").withStringFromValueFunction (d0)));
    l.add (std::make_unique<Param> (juce::ParameterID { "weight", 1 }, "Weight",    Range (0.f, 100.f, 1.f), 0.f, Attr().withLabel ("%").withStringFromValueFunction (d0)));
    l.add (std::make_unique<Param> (juce::ParameterID { "freq", 1 },   "Body Freq", Range (40.f, 250.f, 1.f, 0.5f), 100.f, Attr().withLabel ("Hz").withStringFromValueFunction (d0)));
    return l;
}

VCACompAudioProcessor::VCACompAudioProcessor()
    : AudioProcessor (BusesProperties()
                        .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                        .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "PARAMS", createParameterLayout())
{
    pThr = apvts.getRawParameterValue ("thr");     pRatio = apvts.getRawParameterValue ("ratio");
    pAtk = apvts.getRawParameterValue ("attack");  pRel   = apvts.getRawParameterValue ("release");
    pKnee = apvts.getRawParameterValue ("knee");   pMakeup = apvts.getRawParameterValue ("makeup");
    pMix = apvts.getRawParameterValue ("mix");     pOut   = apvts.getRawParameterValue ("out");
    pHpf = apvts.getRawParameterValue ("hpf");     pCeil  = apvts.getRawParameterValue ("ceil");
    pAutoRel = apvts.getRawParameterValue ("autorel"); pAutoMk = apvts.getRawParameterValue ("automk");
    pRms = apvts.getRawParameterValue ("rms");     pGuard = apvts.getRawParameterValue ("guard");
    pBody = apvts.getRawParameterValue ("body");   pWeight = apvts.getRawParameterValue ("weight");
    pFreq = apvts.getRawParameterValue ("freq");
}

float VCACompAudioProcessor::currentMakeupDb() const
{
    float mk = pMakeup->load();
    if (pAutoMk->load() > 0.5f)
        mk += -pThr->load() * (1.f - 1.f / pRatio->load()) * 0.5f;   // compensacion aproximada
    return mk;
}

void VCACompAudioProcessor::clearState()
{
    hpState[0] = hpState[1] = 0.f;
    msState = grState = grFast = grSlow = 0.f;
    lp1[0] = lp1[1] = lp2[0] = lp2[1] = 0.f;
    chMsIn = chMsOut = 0.f;
    tgBuf.assign ((size_t) limLen, 1.f);
    minBuf.assign ((size_t) limLen, 1.f);
    limPos = 0;
    tgActive = minActive = 0;
    minSum = (double) limLen;
    limGain = 1.f;
    for (auto& d : delayBuf) d.assign ((size_t) delaySize, 0.f);
    dWrite = 0;
    grMeter.store (0.f);
}

void VCACompAudioProcessor::reset() { clearState(); }

void VCACompAudioProcessor::prepareToPlay (double sampleRate, int)
{
    fs = sampleRate;
    limLen = juce::jlimit (8, 500, juce::roundToInt (fs * 0.0015));
    dDelay = limLen - 1;
    clearState();
    setLatencySamples (dDelay);

    smMix.reset (fs, 0.03);    smMix.setCurrentAndTargetValue (pMix->load() * 0.01f);
    smOut.reset (fs, 0.03);    smOut.setCurrentAndTargetValue (juce::Decibels::decibelsToGain (pOut->load()));
    smMakeup.reset (fs, 0.03); smMakeup.setCurrentAndTargetValue (currentMakeupDb());
    smBody.reset (fs, 0.03);   smBody.setCurrentAndTargetValue (pBody->load() * 0.01f);
    smWeight.reset (fs, 0.03); smWeight.setCurrentAndTargetValue (pWeight->load() * 0.01f);
}

bool VCACompAudioProcessor::isBusesLayoutSupported (const BusesLayout& l) const
{
    auto in = l.getMainInputChannelSet();
    if (in != juce::AudioChannelSet::mono() && in != juce::AudioChannelSet::stereo())
        return false;
    return in == l.getMainOutputChannelSet();
}

void VCACompAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;
    const int numCh = juce::jmin (buffer.getNumChannels(), 2);
    const int n = buffer.getNumSamples();
    for (int c = getTotalNumInputChannels(); c < getTotalNumOutputChannels(); ++c)
        buffer.clear (c, 0, n);
    if (numCh == 0 || n == 0) return;

    // --- parametros ---
    const float thr = pThr->load(), ratio = pRatio->load(), knee = pKnee->load();
    const bool autoRel = pAutoRel->load() > 0.5f;
    const bool useRms = pRms->load() > 0.5f, guard = pGuard->load() > 0.5f;
    const float ceilLin = juce::Decibels::decibelsToGain (pCeil->load());

    smMix.setTargetValue (pMix->load() * 0.01f);
    smOut.setTargetValue (juce::Decibels::decibelsToGain (pOut->load()));
    smMakeup.setTargetValue (currentMakeupDb());
    smBody.setTargetValue (pBody->load() * 0.01f);
    smWeight.setTargetValue (pWeight->load() * 0.01f);

    auto coef = [this] (float ms) { return std::exp (-1.f / (juce::jmax (0.01f, ms) * 0.001f * (float) fs)); };
    const float atkC = coef (pAtk->load()), relC = coef (pRel->load());
    const float relFastC = coef (50.f), relSlowC = coef (juce::jmax (400.f, pRel->load() * 3.f));
    const float limRelC = coef (100.f);
    const float rmsC = 1.f - coef (10.f);
    const float hpA = 1.f - std::exp (-juce::MathConstants<float>::twoPi * pHpf->load() / (float) fs);
    // dos filtros de 1 polo en cascada: x1.554 deja el corte (-3 dB) en la frecuencia elegida
    const float bandFreq = juce::jmin (pFreq->load() * 1.554f, 0.45f * (float) fs);
    const float bandA = 1.f - std::exp (-juce::MathConstants<float>::twoPi * bandFreq / (float) fs);
    const float chC = 1.f - coef (30.f);

    float maxGr = 0.f;
    float* data[2] { buffer.getWritePointer (0), numCh > 1 ? buffer.getWritePointer (1) : nullptr };

    for (int i = 0; i < n; ++i)
    {
        // entrada segura (evita NaN/Inf que envenenarian el estado)
        float in[2] { 0.f, 0.f };
        for (int c = 0; c < numCh; ++c)
        {
            const float x = data[c][i];
            in[c] = std::isfinite (x) ? x : 0.f;
        }

        // --- detector (sidechain con HPF, enlazado estereo) ---
        float sc = 0.f;
        for (int c = 0; c < numCh; ++c)
        {
            hpState[c] += hpA * (in[c] - hpState[c]);
            sc = juce::jmax (sc, std::abs (in[c] - hpState[c]));
        }
        float lvl = sc;
        if (useRms) { msState += rmsC * (sc * sc - msState); lvl = std::sqrt (juce::jmax (0.f, msState)); }
        const float lvlDb = juce::Decibels::gainToDecibels (lvl, -100.f);

        // --- gain computer con soft knee ---
        const float over = lvlDb - thr;
        float target = 0.f;                              // reduccion positiva en dB
        if (2.f * over > knee)
            target = over * (1.f - 1.f / ratio);
        else if (2.f * over > -knee)
        {
            const float t = over + knee * 0.5f;
            target = (1.f - 1.f / ratio) * t * t / (2.f * juce::jmax (knee, 0.001f));
        }

        // --- ataque / release en dominio dB (estilo VCA) ---
        float gr;
        if (autoRel)
        {
            const float cf = target > grFast ? atkC : relFastC;
            const float cs = target > grSlow ? atkC : relSlowC;
            grFast = cf * grFast + (1.f - cf) * target;
            grSlow = cs * grSlow + (1.f - cs) * target;
            gr = 0.5f * (grFast + grSlow);
        }
        else
        {
            const float cc = target > grState ? atkC : relC;
            grState = cc * grState + (1.f - cc) * target;
            gr = grState;
        }
        maxGr = juce::jmax (maxGr, gr);

        const float makeupDb = smMakeup.getNextValue();
        const float g = juce::Decibels::decibelsToGain (-gr + makeupDb);
        const float mix = smMix.getNextValue();
        const float outG = smOut.getNextValue();
        const float body = smBody.getNextValue();
        const float gW = 1.f + smWeight.getNextValue();

        // --- mezcla paralela ---
        float y[2] { 0.f, 0.f };
        for (int c = 0; c < numCh; ++c)
            y[c] = in[c] * (1.f - mix) + in[c] * g * mix;

        // --- caracter: WEIGHT (refuerzo de graves) + BODY (saturacion de graves con nivel compensado) ---
        {
            const float d = 1.f + 3.f * body;
            float rest[2] { 0.f, 0.f }, bnd[2] { 0.f, 0.f }, sat[2] { 0.f, 0.f };
            float eIn = 0.f, eOut = 0.f;
            for (int c = 0; c < numCh; ++c)
            {
                lp1[c] += bandA * (y[c] - lp1[c]);
                lp2[c] += bandA * (lp1[c] - lp2[c]);
                rest[c] = y[c] - lp2[c];                  // resto = señal - graves (suma perfecta)
                bnd[c]  = lp2[c] * gW;
                sat[c]  = std::tanh (d * bnd[c]) / d;
                eIn  += bnd[c] * bnd[c];
                eOut += sat[c] * sat[c];
            }
            chMsIn  += chC * (eIn  / (float) numCh - chMsIn);
            chMsOut += chC * (eOut / (float) numCh - chMsOut);
            const float comp = juce::jlimit (1.f, 4.f, std::sqrt ((chMsIn + 1.0e-12f) / (chMsOut + 1.0e-12f)));
            for (int c = 0; c < numCh; ++c)
                y[c] = rest[c] + bnd[c] + body * (sat[c] * comp - bnd[c]);
        }

        // --- salida ---
        float peak = 0.f;
        for (int c = 0; c < numCh; ++c)
        {
            y[c] *= outG;
            peak = juce::jmax (peak, std::abs (y[c]));
        }

        // --- Peak Guard: limitador con lookahead (min + promedio) ---
        const float tg = (guard && peak > ceilLin) ? ceilLin / peak : 1.f;
        {
            const size_t p = (size_t) limPos;
            if (tgBuf[p] < 1.f) --tgActive;
            tgBuf[p] = tg;
            if (tg < 1.f) ++tgActive;

            float minG = 1.f;
            if (tgActive > 0)                       // solo escanea cuando hay un pico en la ventana
                for (int k = 0; k < limLen; ++k) minG = juce::jmin (minG, tgBuf[(size_t) k]);

            if (minBuf[p] < 1.f) --minActive;
            minSum += (double) minG - (double) minBuf[p];
            minBuf[p] = minG;
            if (minG < 1.f) ++minActive;
        }
        float avg = 1.f;
        if (minActive > 0) avg = juce::jlimit (0.f, 1.f, (float) (minSum / (double) limLen));
        else               minSum = (double) limLen;  // elimina deriva numerica
        limPos = (limPos + 1) % limLen;
        limGain = avg < limGain ? avg : limRelC * limGain + (1.f - limRelC) * avg;

        // --- retardo (compensa el lookahead) y salida ---
        for (int c = 0; c < numCh; ++c)
        {
            delayBuf[c][(size_t) dWrite] = y[c];
            float o = delayBuf[c][(size_t) ((dWrite - dDelay + delaySize) & delayMask)];
            if (guard) o = juce::jlimit (-ceilLin, ceilLin, o * limGain);
            data[c][i] = o;
        }
        dWrite = (dWrite + 1) & delayMask;
    }
    grMeter.store (maxGr);
}

juce::AudioProcessorEditor* VCACompAudioProcessor::createEditor() { return new VCACompAudioProcessorEditor (*this); }

void VCACompAudioProcessor::getStateInformation (juce::MemoryBlock& dest)
{
    if (auto xml = apvts.copyState().createXml()) copyXmlToBinary (*xml, dest);
}

void VCACompAudioProcessor::setStateInformation (const void* data, int size)
{
    if (auto xml = getXmlFromBinary (data, size))
        if (xml->hasTagName (apvts.state.getType())) apvts.replaceState (juce::ValueTree::fromXml (*xml));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new VCACompAudioProcessor(); }
