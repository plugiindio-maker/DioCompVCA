#include "PluginEditor.h"
using namespace DioStyle;

// ---------------------------------------------------------------- Look & Feel
DioLookAndFeel::DioLookAndFeel()
{
    setColour (juce::ComboBox::backgroundColourId, juce::Colour (0xff120a1f));
    setColour (juce::ComboBox::outlineColourId, violet.withAlpha (0.6f));
    setColour (juce::ComboBox::textColourId, ice);
    setColour (juce::ComboBox::arrowColourId, violetHi);
    setColour (juce::PopupMenu::backgroundColourId, juce::Colour (0xff120a1f));
    setColour (juce::PopupMenu::textColourId, ice);
    setColour (juce::PopupMenu::headerTextColourId, violetHi);
    setColour (juce::PopupMenu::highlightedBackgroundColourId, violet);
    setColour (juce::PopupMenu::highlightedTextColourId, juce::Colours::white);
    setColour (juce::TextButton::buttonColourId, juce::Colour (0xff120a1f));
    setColour (juce::TextButton::textColourOffId, juce::Colour (0xffb9a3d6));
    setColour (juce::TextButton::textColourOnId, juce::Colours::white);
}

void DioLookAndFeel::drawRotarySlider (juce::Graphics& g, int x, int y, int w, int h,
                                       float pos, float startAngle, float endAngle, juce::Slider& slider)
{
    juce::Colour acc = violet;
    if (slider.getProperties().contains ("accent"))
        acc = juce::Colour ((juce::uint32) (int) slider.getProperties()["accent"]);
    const juce::Colour accHi = acc.interpolatedWith (juce::Colours::white, 0.4f);

    auto b = juce::Rectangle<float> ((float) x, (float) y, (float) w, (float) h).reduced (8.f);
    const float r = juce::jmin (b.getWidth(), b.getHeight()) * 0.5f;
    const auto c = b.getCentre();
    const float angle = startAngle + pos * (endAngle - startAngle);
    const float arcR = r * 0.84f, capR = r * 0.64f;

    auto pt = [&] (float a, float rad) { return juce::Point<float> (c.x + std::sin (a) * rad, c.y - std::cos (a) * rad); };

    // marcas alrededor
    for (int i = 0; i <= 10; ++i)
    {
        const float a = startAngle + (float) i / 10.f * (endAngle - startAngle);
        g.setColour (i % 5 == 0 ? accHi.withAlpha (0.9f) : juce::Colour (0xff4a3470));
        g.drawLine (juce::Line<float> (pt (a, r * 0.98f), pt (a, r * 1.08f)), i % 5 == 0 ? 2.f : 1.f);
    }

    // pista y arco de valor con brillo neon
    juce::Path track;
    track.addCentredArc (c.x, c.y, arcR, arcR, 0.f, startAngle, endAngle, true);
    g.setColour (juce::Colour (0xff1a1030));
    g.strokePath (track, juce::PathStrokeType (4.f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    if (pos > 0.001f)
    {
        juce::Path val;
        val.addCentredArc (c.x, c.y, arcR, arcR, 0.f, startAngle, angle, true);
        g.setColour (acc.withAlpha (0.22f));
        g.strokePath (val, juce::PathStrokeType (12.f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        g.setColour (acc.withAlpha (0.5f));
        g.strokePath (val, juce::PathStrokeType (6.f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        g.setColour (accHi);
        g.strokePath (val, juce::PathStrokeType (3.f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    }

    // sombra + tapa de metal oscuro cepillado
    g.setColour (juce::Colour (0x99000000));
    g.fillEllipse (c.x - capR - 1.f, c.y - capR + 4.f, (capR + 1.f) * 2.f, (capR + 1.f) * 2.f);
    g.setGradientFill (juce::ColourGradient (juce::Colour (0xff6e6882), c.x - capR * 0.35f, c.y - capR * 0.45f,
                                             juce::Colour (0xff0d0b14), c.x + capR * 0.9f, c.y + capR * 0.9f, true));
    g.fillEllipse (c.x - capR, c.y - capR, capR * 2.f, capR * 2.f);

    // cepillado radial
    g.setColour (juce::Colours::white.withAlpha (0.05f));
    for (int i = 0; i < 40; ++i)
    {
        const float a = (float) i * juce::MathConstants<float>::twoPi / 40.f;
        g.drawLine (juce::Line<float> (pt (a, capR * 0.10f), pt (a, capR * 0.97f)), 1.f);
    }

    // borde con luz violeta
    g.setColour (accHi.withAlpha (0.45f));
    g.drawEllipse (c.x - capR, c.y - capR, capR * 2.f, capR * 2.f, 1.6f);
    g.setColour (juce::Colour (0xff04020a));
    g.drawEllipse (c.x - capR * 0.97f, c.y - capR * 0.97f, capR * 1.94f, capR * 1.94f, 1.f);

    // puntero blanco
    const auto p1 = pt (angle, capR * 0.30f), p2 = pt (angle, capR * 0.92f);
    g.setColour (juce::Colours::white.withAlpha (0.25f));
    g.drawLine (juce::Line<float> (p1, p2), 6.f);
    g.setColour (juce::Colours::white);
    g.drawLine (juce::Line<float> (p1, p2), 3.f);
}

void DioLookAndFeel::drawToggleButton (juce::Graphics& g, juce::ToggleButton& bt, bool highlighted, bool)
{
    const bool on = bt.getToggleState();

    if (bt.getProperties().contains ("power"))          // boton de encendido redondo
    {
        auto r = bt.getLocalBounds().toFloat().reduced (4.f);
        const auto c = r.getCentre();
        const float rad = juce::jmin (r.getWidth(), r.getHeight()) * 0.5f;
        g.setGradientFill (juce::ColourGradient (juce::Colour (0xff2a1745), c.x, r.getY(), juce::Colour (0xff0d0716), c.x, r.getBottom(), false));
        g.fillEllipse (r);
        if (on) { g.setColour (violet.withAlpha (0.3f)); g.drawEllipse (r.expanded (3.f), 5.f); }
        g.setColour (on ? violetHi : juce::Colour (0xff4a3470));
        g.drawEllipse (r, 2.f);
        juce::Path p;
        p.addCentredArc (c.x, c.y, rad * 0.5f, rad * 0.5f, 0.f, 0.6f, juce::MathConstants<float>::twoPi - 0.6f, true);
        p.startNewSubPath (c.x, c.y - rad * 0.62f);
        p.lineTo (c.x, c.y - rad * 0.08f);
        if (on) { g.setColour (violet.withAlpha (0.4f)); g.strokePath (p, juce::PathStrokeType (7.f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded)); }
        g.setColour (on ? juce::Colours::white : juce::Colour (0xff7d6aa0));
        g.strokePath (p, juce::PathStrokeType (3.f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        return;
    }

    auto r = bt.getLocalBounds().toFloat().reduced (3.f);
    if (on)
    {
        g.setColour (magenta.withAlpha (0.25f));
        g.fillRoundedRectangle (r.expanded (2.f), 9.f);
        g.setGradientFill (juce::ColourGradient (magenta.interpolatedWith (violet, 0.4f), r.getX(), r.getY(),
                                                 violet.darker (0.5f), r.getX(), r.getBottom(), false));
    }
    else
        g.setColour (juce::Colour (highlighted ? 0xff22143a : 0xff140c24));
    g.fillRoundedRectangle (r, 7.f);
    g.setColour (on ? magenta : juce::Colour (0xff3a2560));
    g.drawRoundedRectangle (r, 7.f, 1.3f);
    g.setColour (on ? juce::Colours::white : juce::Colour (0xffb9a3d6));
    g.setFont (juce::Font (juce::FontOptions (12.5f, juce::Font::bold)));
    g.drawText (bt.getButtonText(), bt.getLocalBounds(), juce::Justification::centred);
}

void DioLookAndFeel::drawButtonBackground (juce::Graphics& g, juce::Button& b, const juce::Colour&, bool highlighted, bool down)
{
    auto r = b.getLocalBounds().toFloat().reduced (1.5f);
    const bool on = b.getToggleState();
    if (on)
    {
        g.setColour (violet.withAlpha (0.28f));
        g.fillRoundedRectangle (r.expanded (2.f), 9.f);
        g.setGradientFill (juce::ColourGradient (violet, r.getX(), r.getY(), violet.darker (0.6f), r.getX(), r.getBottom(), false));
    }
    else
        g.setColour (juce::Colour (down ? 0xff2a1745 : highlighted ? 0xff1d1030 : 0xff120a1f));
    g.fillRoundedRectangle (r, 7.f);
    g.setColour (on ? violetHi : juce::Colour (0xff3a2560));
    g.drawRoundedRectangle (r, 7.f, 1.3f);
}

// ---------------------------------------------------------------- helpers
static void drawCrown (juce::Graphics& g, juce::Rectangle<float> r, juce::Colour core, juce::Colour glow,
                       float thick, float intensity)
{
    juce::Path p;
    p.startNewSubPath (0.f, 0.78f);
    p.lineTo (0.05f, 0.15f);
    p.lineTo (0.28f, 0.50f);
    p.lineTo (0.50f, 0.f);
    p.lineTo (0.72f, 0.50f);
    p.lineTo (0.95f, 0.15f);
    p.lineTo (1.f, 0.78f);
    p.closeSubPath();
    p.startNewSubPath (0.03f, 0.97f);
    p.lineTo (0.97f, 0.97f);
    p.applyTransform (juce::AffineTransform::scale (r.getWidth(), r.getHeight()).translated (r.getX(), r.getY()));

    for (float k : { 4.f, 2.5f, 1.4f })
    {
        g.setColour (glow.withAlpha ((k == 4.f ? 0.12f : k == 2.5f ? 0.25f : 0.5f) * intensity));
        g.strokePath (p, juce::PathStrokeType (thick * k, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    }
    g.setColour (core);
    g.strokePath (p, juce::PathStrokeType (thick, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
}

static void drawScrew (juce::Graphics& g, float cx, float cy, float r)
{
    g.setGradientFill (juce::ColourGradient (juce::Colour (0xffe8ecf6), cx - r * 0.5f, cy - r * 0.6f,
                                             juce::Colour (0xff3d4558), cx + r * 0.7f, cy + r * 0.8f, false));
    g.fillEllipse (cx - r, cy - r, r * 2.f, r * 2.f);
    g.setColour (juce::Colour (0xff0a0d14));
    g.drawEllipse (cx - r, cy - r, r * 2.f, r * 2.f, 1.f);
    g.drawLine (cx - r * 0.6f, cy + r * 0.3f, cx + r * 0.6f, cy - r * 0.3f, 1.4f);
}

static void drawLedBar (juce::Graphics& g, juce::Rectangle<float> r, float db, const juce::String& label)
{
    g.setColour (violetHi);
    g.setFont (juce::Font (juce::FontOptions (12.f, juce::Font::bold)));
    g.drawText (label, r.withHeight (16.f).translated (0.f, -20.f), juce::Justification::centredLeft);
    g.setColour (ice);
    g.drawText (db <= -79.f ? juce::String ("-inf dB") : juce::String (db, 1) + " dB",
                r.withHeight (16.f).translated (0.f, -20.f), juce::Justification::centredRight);

    g.setColour (juce::Colour (0xff0a0614));
    g.fillRoundedRectangle (r.expanded (3.f), 5.f);
    const int segs = 40;
    const float sw = r.getWidth() / (float) segs;
    const int lit = juce::roundToInt (juce::jlimit (0.f, 1.f, (db + 48.f) / 48.f) * (float) segs);
    for (int i = 0; i < segs; ++i)
    {
        const float t = (float) i / (float) segs;
        const juce::Colour on = t < 0.70f ? violet : t < 0.90f ? magenta : juce::Colour (0xffff7aa8);
        g.setColour (i < lit ? on : juce::Colour (0xff1b1130));
        g.fillRoundedRectangle (r.getX() + (float) i * sw + 1.f, r.getY(), sw - 2.f, r.getHeight(), 1.5f);
    }
}

// ---------------------------------------------------------------- Panel
DioPanel::DioPanel (DioCompressProcessor& p)
    : proc (p)
{
    addKnob (0,  "THRESHOLD", "thr",     " dB", violet);
    addKnob (1,  "RATIO",     "ratio",   ":1",  violet);
    addKnob (2,  "ATTACK",    "attack",  " ms", violet);
    addKnob (3,  "RELEASE",   "release", " ms", violet);
    addKnob (4,  "KNEE",      "knee",    " dB", violet);
    addKnob (5,  "SC HPF",    "hpf",     " Hz", violet);
    addKnob (6,  "CEILING",   "ceil",    " dB", violet);
    addKnob (7,  "INPUT",     "input",   " dB", violetHi);
    addKnob (8,  "MAKEUP",    "makeup",  " dB", violetHi);
    addKnob (9,  "OUTPUT",    "out",     " dB", violetHi);
    addKnob (10, "DRY / WET", "mix",     " %",  violetHi);
    addKnob (11, "DRIVE",     "drive",   " %",  magenta);
    addKnob (12, "COLOR",     "color",   " %",  magenta);
    addKnob (13, "MIX",       "amix",    " %",  magenta);

    // selector de topologia y personalidad VCA
    const char* topoNames[3] = { "VCA", "OPTICO", "VARI-MU" };
    const char* modeNames[3] = { "SMOOTH", "PUNCH", "AGGRESSIVE" };
    for (int i = 0; i < 3; ++i)
    {
        topoBtn[(size_t) i].setLookAndFeel (&laf);
        topoBtn[(size_t) i].setButtonText (topoNames[i]);
        topoBtn[(size_t) i].setRadioGroupId (101);
        topoBtn[(size_t) i].setClickingTogglesState (true);
        topoBtn[(size_t) i].onClick = [this, i] { setChoice ("topology", i); };
        addAndMakeVisible (topoBtn[(size_t) i]);

        modeBtn[(size_t) i].setLookAndFeel (&laf);
        modeBtn[(size_t) i].setButtonText (modeNames[i]);
        modeBtn[(size_t) i].setRadioGroupId (102);
        modeBtn[(size_t) i].setClickingTogglesState (true);
        modeBtn[(size_t) i].onClick = [this, i] { setChoice ("vcamode", i); };
        addAndMakeVisible (modeBtn[(size_t) i]);
    }

    powerBtn.getProperties().set ("power", true);
    powerBtn.setLookAndFeel (&laf);
    powerBtn.onClick = [this] { setBool ("bypass", ! powerBtn.getToggleState()); };
    addAndMakeVisible (powerBtn);

    autoMkBtn.setLookAndFeel (&laf);
    autoMkBtn.setButtonText ("AUTO");
    addAndMakeVisible (autoMkBtn);
    autoMkAtt = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (proc.apvts, "automk", autoMkBtn);

    analogBtn.setLookAndFeel (&laf);
    analogBtn.setButtonText ("ANALOG ON");
    addAndMakeVisible (analogBtn);
    analogAtt = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (proc.apvts, "analogon", analogBtn);

    safetyBox.setLookAndFeel (&laf);
    safetyBox.addItemList (juce::StringArray { "OFF", "LIMIT", "CLIP" }, 1);
    addAndMakeVisible (safetyBox);
    safetyAtt = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (proc.apvts, "safety", safetyBox);

    // presets con flechas
    presetBox.setLookAndFeel (&laf);
    for (int i = 0; i < proc.getNumPrograms(); ++i)
        presetBox.addItem (proc.getProgramName (i), i + 1);
    presetBox.setSelectedId (proc.getCurrentProgram() + 1, juce::dontSendNotification);
    presetBox.onChange = [this] { proc.setCurrentProgram (presetBox.getSelectedId() - 1); };
    addAndMakeVisible (presetBox);

    prevBtn.setLookAndFeel (&laf);  prevBtn.setButtonText ("<");
    nextBtn.setLookAndFeel (&laf);  nextBtn.setButtonText (">");
    prevBtn.onClick = [this]
    {
        const int n = presetBox.getNumItems();
        const int id = presetBox.getSelectedId();
        presetBox.setSelectedId (id <= 1 ? n : id - 1);
    };
    nextBtn.onClick = [this]
    {
        const int n = presetBox.getNumItems();
        const int id = presetBox.getSelectedId();
        presetBox.setSelectedId (id >= n ? 1 : id + 1);
    };
    addAndMakeVisible (prevBtn);
    addAndMakeVisible (nextBtn);

    // perfiles de fuente
    profileBox.setLookAndFeel (&laf);
    profileBox.addItemList (juce::StringArray { "VOCAL", "KICK", "SNARE", "DRUM BUS", "808 / BASS", "INSTRUMENTAL", "MASTER BUS" }, 1);
    profileBox.setTextWhenNothingSelected ("SOURCE PROFILE");
    if (proc.getCurrentProfile() >= 0)
        profileBox.setSelectedId (proc.getCurrentProfile() + 1, juce::dontSendNotification);
    profileBox.onChange = [this] { proc.applyProfile (profileBox.getSelectedId() - 1); };
    addAndMakeVisible (profileBox);

    settingsBtn.setLookAndFeel (&laf);
    settingsBtn.setButtonText ("SETTINGS");
    settingsBtn.onClick = [this] { showSettingsMenu(); };
    addAndMakeVisible (settingsBtn);

    uiTopo = getChoice ("topology");
    applyTopologyUi();

    setSize (kW, kH);
    startTimerHz (30);
}

DioPanel::~DioPanel()
{
    stopTimer();
    for (auto& b : topoBtn) b.setLookAndFeel (nullptr);
    for (auto& b : modeBtn) b.setLookAndFeel (nullptr);
    powerBtn.setLookAndFeel (nullptr);
    autoMkBtn.setLookAndFeel (nullptr);
    analogBtn.setLookAndFeel (nullptr);
    safetyBox.setLookAndFeel (nullptr);
    presetBox.setLookAndFeel (nullptr);
    profileBox.setLookAndFeel (nullptr);
    prevBtn.setLookAndFeel (nullptr);
    nextBtn.setLookAndFeel (nullptr);
    settingsBtn.setLookAndFeel (nullptr);
    for (auto& k : knobs) k.s.setLookAndFeel (nullptr);
}

void DioPanel::addKnob (int i, const juce::String& name, const juce::String& id, const juce::String& suffix, juce::Colour accent)
{
    auto& k = knobs[(size_t) i];
    k.s.setLookAndFeel (&laf);
    k.s.getProperties().set ("accent", (int) accent.getARGB());
    k.s.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    k.s.setRotaryParameters (juce::MathConstants<float>::pi * 1.2f, juce::MathConstants<float>::pi * 2.8f, true);
    k.s.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 90, 18);
    k.s.setTextValueSuffix (suffix);
    k.s.setColour (juce::Slider::textBoxTextColourId, ice);
    k.s.setColour (juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    k.l.setText (name, juce::dontSendNotification);
    k.l.setJustificationType (juce::Justification::centred);
    k.l.setColour (juce::Label::textColourId, accent.interpolatedWith (juce::Colours::white, 0.45f));
    k.l.setFont (juce::Font (juce::FontOptions (13.f, juce::Font::bold)));
    addAndMakeVisible (k.s);
    addAndMakeVisible (k.l);
    k.a = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (proc.apvts, id, k.s);
}

void DioPanel::setChoice (const char* id, int index)
{
    if (auto* p = proc.apvts.getParameter (id))
    {
        p->beginChangeGesture();
        p->setValueNotifyingHost (p->convertTo0to1 ((float) index));
        p->endChangeGesture();
    }
}

void DioPanel::setBool (const char* id, bool v) { setChoice (id, v ? 1 : 0); }

int DioPanel::getChoice (const char* id) const
{
    return juce::roundToInt (proc.apvts.getRawParameterValue (id)->load());
}

// muestra solo los controles que tienen sentido en la topologia elegida
void DioPanel::applyTopologyUi()
{
    const bool vca = (uiTopo == 0), opto = (uiTopo == 1);
    knobs[2].s.setVisible (! opto);  knobs[2].l.setVisible (! opto);     // ATTACK: la celda optica es automatica
    knobs[4].s.setVisible (vca);     knobs[4].l.setVisible (vca);        // KNEE: solo VCA (las otras tienen curva propia)
    for (auto& b : modeBtn) b.setVisible (vca);                          // SMOOTH / PUNCH / AGGRESSIVE: solo VCA
    knobs[1].l.setText (uiTopo == 2 ? "MAX RATIO" : opto ? "COMPRESSION" : "RATIO", juce::dontSendNotification);
    knobs[3].l.setText (opto ? "RECOVERY" : "RELEASE", juce::dontSendNotification);
    repaint();
}

void DioPanel::showSettingsMenu()
{
    juce::PopupMenu m;
    m.setLookAndFeel (&laf);
    m.addSectionHeader ("WINDOW SIZE");
    const int cur = getSizePercent ? getSizePercent() : 0;
    for (int pct : { 40, 50, 60, 70, 80, 90, 100, 125 })
        m.addItem (pct, juce::String (pct) + "%", true, std::abs (cur - pct) < 3);

    juce::Component::SafePointer<DioPanel> safe (this);
    m.showMenuAsync (juce::PopupMenu::Options().withTargetComponent (settingsBtn),
                     [safe] (int result)
                     {
                         if (safe != nullptr && result > 0 && safe->onSizePercent)
                             safe->onSizePercent (result);
                     });
}

void DioPanel::timerCallback()
{
    grDisp  = juce::jmax (proc.grMeter.load(), grDisp * 0.88f);
    inDisp  = juce::jmax (proc.inMeter.load(),  inDisp - 1.2f);
    outDisp = juce::jmax (proc.outMeter.load(), outDisp - 1.2f);

    const float posT = 1.f - std::sqrt (juce::jlimit (0.f, 1.f, grDisp / 20.f));
    needlePos += (posT - needlePos) * 0.30f;

    phase += 0.10f;
    if (phase > juce::MathConstants<float>::twoPi) phase -= juce::MathConstants<float>::twoPi;

    // sincroniza la interfaz con los parametros (presets, automatizacion del DAW)
    const int topo = getChoice ("topology"), mode = getChoice ("vcamode");
    if (topo != uiTopo) { uiTopo = topo; applyTopologyUi(); }
    for (int i = 0; i < 3; ++i)
    {
        topoBtn[(size_t) i].setToggleState (i == topo, juce::dontSendNotification);
        modeBtn[(size_t) i].setToggleState (i == mode, juce::dontSendNotification);
    }
    powerBtn.setToggleState (getChoice ("bypass") == 0, juce::dontSendNotification);

    const bool an = analogBtn.getToggleState();
    if (an != analogUiOn)
    {
        analogUiOn = an;
        for (int i = 11; i < 14; ++i)
        {
            knobs[(size_t) i].s.setAlpha (an ? 1.f : 0.4f);
            knobs[(size_t) i].l.setAlpha (an ? 1.f : 0.4f);
        }
    }

    repaint (meterArea.expanded (10));
    repaint (660, 400, 420, 160);                               // medidores de entrada / salida
    if ((++tick & 1) == 0) repaint (0, 0, kW, 120);             // logo y power palpitan
}

void DioPanel::rebuildBackground()
{
    const int w = getWidth(), h = getHeight();
    if (w <= 0 || h <= 0) return;
    bg = juce::Image (juce::Image::ARGB, w, h, true);
    juce::Graphics g (bg);
    g.setGradientFill (juce::ColourGradient::vertical (juce::Colour (0xff040208), 0.f, juce::Colour (0xff0d0620), (float) h));
    g.fillAll();

    juce::Random rnd (4242);   // humo violeta (fijo, siempre igual)
    for (int i = 0; i < 46; ++i)
    {
        const float cx = rnd.nextFloat() * (float) w, cy = rnd.nextFloat() * (float) h;
        const float rad = 90.f + rnd.nextFloat() * 220.f;
        const bool mg = rnd.nextFloat() > 0.7f;
        const auto c = juce::Colour::fromHSV (mg ? 0.85f : 0.74f + rnd.nextFloat() * 0.04f, 0.85f,
                                              0.5f + 0.5f * rnd.nextFloat(), 0.05f + 0.09f * rnd.nextFloat());
        g.setGradientFill (juce::ColourGradient (c, cx, cy, c.withAlpha (0.f), cx + rad, cy, true));
        g.fillEllipse (cx - rad, cy - rad, rad * 2.f, rad * 2.f);
    }
    // textura metalica fina en el panel
    for (int yy = 124; yy < 712; yy += 2)
    {
        g.setColour (juce::Colours::white.withAlpha (0.01f + 0.018f * rnd.nextFloat()));
        g.drawHorizontalLine (yy, 20.f, 1080.f);
    }
    g.setGradientFill (juce::ColourGradient (juce::Colours::transparentBlack, (float) w * 0.5f, (float) h * 0.5f,
                                             juce::Colour (0xe6000000), 0.f, 0.f, true));
    g.fillAll();
}

void DioPanel::rebuildTitle()
{
    juce::GlyphArrangement ga;
    ga.addLineOfText (juce::Font (juce::FontOptions (90.f, juce::Font::bold | juce::Font::italic)), "DioCompress", 0.f, 0.f);
    titlePath.clear();
    ga.createPath (titlePath);
    titlePath.applyTransform (juce::AffineTransform::shear (-0.12f, 0.f));

    const auto b = titlePath.getBounds();
    if (b.getWidth() <= 0.f) return;
    const juce::Rectangle<float> area (66.f, 24.f, 210.f, 42.f);
    const float s = juce::jmin (area.getWidth() / b.getWidth(), area.getHeight() / b.getHeight());
    const float tx = area.getX();
    const float ty = area.getCentreY() - b.getHeight() * s * 0.5f;
    titlePath.applyTransform (juce::AffineTransform::translation (-b.getX(), -b.getY()).scaled (s).translated (tx, ty));
}

// Medidor de aguja de reduccion de ganancia (cara retroiluminada rosa/violeta)
void DioPanel::drawMeter (juce::Graphics& g, juce::Rectangle<float> r)
{
    g.setGradientFill (juce::ColourGradient (juce::Colour (0xff2a2438), r.getX(), r.getY(), juce::Colour (0xff0a0812), r.getX(), r.getBottom(), false));
    g.fillRoundedRectangle (r, 12.f);
    g.setColour (juce::Colour (0xff05030a));
    g.drawRoundedRectangle (r, 12.f, 2.f);

    auto face = r.reduced (14.f);
    g.setColour (juce::Colour (0xff12081e));
    g.fillRoundedRectangle (face.expanded (3.f), 8.f);
    g.setGradientFill (juce::ColourGradient (juce::Colour (0xffffb4c8), face.getCentreX(), face.getBottom() - 4.f,
                                             juce::Colour (0xff7a38d8), face.getX(), face.getY(), true));
    g.fillRoundedRectangle (face, 7.f);

    {
        juce::Graphics::ScopedSaveState ss (g);
        g.reduceClipRegion (face.toNearestInt());

        const juce::Point<float> P (face.getCentreX(), face.getBottom() + face.getHeight() * 0.22f);
        const float R = face.getHeight() * 1.02f;
        const float half = juce::degreesToRadians (44.f);
        auto polar = [&] (float ang, float rad) { return juce::Point<float> (P.x + std::sin (ang) * rad, P.y - std::cos (ang) * rad); };
        auto posOf = [] (float gr) { return 1.f - std::sqrt (juce::jlimit (0.f, 1.f, gr / 20.f)); };   // escala no lineal
        auto angOf = [&] (float pos) { return -half + pos * 2.f * half; };
        const juce::Colour ink (0xff1c0a2e), hotc (0xffb0125e);

        // zona caliente (mas de 10 dB de reduccion)
        juce::Path hotArc;
        hotArc.addCentredArc (P.x, P.y, R * 0.80f, R * 0.80f, 0.f, angOf (posOf (20.f)), angOf (posOf (10.f)), true);
        g.setColour (hotc.withAlpha (0.8f));
        g.strokePath (hotArc, juce::PathStrokeType (4.5f));

        juce::Path arc;
        arc.addCentredArc (P.x, P.y, R * 0.80f, R * 0.80f, 0.f, -half, half, true);
        g.setColour (ink);
        g.strokePath (arc, juce::PathStrokeType (1.4f));

        for (float v = 0.f; v <= 20.01f; v += 1.f)
        {
            const float a = angOf (posOf (v));
            g.drawLine (juce::Line<float> (polar (a, R * 0.80f), polar (a, R * 0.75f)), 1.f);
        }
        g.setFont (juce::Font (juce::FontOptions (13.f, juce::Font::bold)));
        for (float v : { 0.f, 1.f, 3.f, 5.f, 7.f, 10.f, 20.f })
        {
            const float a = angOf (posOf (v));
            g.setColour (v >= 10.f ? hotc : ink);
            g.drawLine (juce::Line<float> (polar (a, R * 0.80f), polar (a, R * 0.70f)), 2.f);
            const auto tp = polar (a, R * 0.62f);
            g.drawText (v == 0.f ? juce::String ("0") : "-" + juce::String ((int) v),
                        juce::Rectangle<float> (tp.x - 18.f, tp.y - 8.f, 36.f, 16.f), juce::Justification::centred);
        }

        g.setColour (ink.withAlpha (0.9f));
        g.setFont (juce::Font (juce::FontOptions (16.f, juce::Font::bold)));
        g.drawText ("dB", face.withTrimmedTop (face.getHeight() * 0.58f).withHeight (18.f), juce::Justification::centred);
        g.setFont (juce::Font (juce::FontOptions (13.f, juce::Font::bold)));
        g.drawText ("GAIN REDUCTION", face.withTrimmedTop (face.getHeight() * 0.72f).withHeight (16.f), juce::Justification::centred);

        // aguja con sombra
        const float na = angOf (juce::jlimit (0.f, 1.f, needlePos));
        const auto tip = polar (na, R * 0.98f);
        g.setColour (juce::Colour (0x55000000));
        g.drawLine (juce::Line<float> (P.translated (3.f, 3.f), tip.translated (3.f, 3.f)), 2.8f);
        g.setColour (juce::Colour (0xff10061a));
        g.drawLine (juce::Line<float> (P, tip), 2.2f);

        // reflejo del vidrio
        g.setGradientFill (juce::ColourGradient (juce::Colours::white.withAlpha (0.30f), face.getCentreX(), face.getY(),
                                                 juce::Colours::white.withAlpha (0.f), face.getCentreX(), face.getCentreY() + 10.f, false));
        g.fillRect (face.withHeight (face.getHeight() * 0.6f));
    }

    drawScrew (g, r.getX() + 8.f, r.getY() + 8.f, 3.f);
    drawScrew (g, r.getRight() - 8.f, r.getY() + 8.f, 3.f);
    drawScrew (g, r.getX() + 8.f, r.getBottom() - 8.f, 3.f);
    drawScrew (g, r.getRight() - 8.f, r.getBottom() - 8.f, 3.f);
}

void DioPanel::paint (juce::Graphics& g)
{
    g.drawImageAt (bg, 0, 0);
    const float W = (float) kW;
    const float pulse = 0.82f + 0.18f * std::sin (phase);

    // ---- barra superior
    const juce::Rectangle<float> head (16.f, 12.f, 1068.f, 66.f);
    g.setGradientFill (juce::ColourGradient (juce::Colour (0xf01c1130), 0.f, head.getY(), juce::Colour (0xf00a0614), 0.f, head.getBottom(), false));
    g.fillRoundedRectangle (head, 12.f);
    g.setColour (violet.withAlpha (0.35f + 0.2f * pulse));
    g.drawRoundedRectangle (head, 12.f, 1.6f);
    g.setColour (juce::Colour (0x26ffffff));
    g.drawLine (30.f, head.getY() + 1.5f, head.getRight() - 30.f, head.getY() + 1.5f, 1.f);

    // logo DioCompress
    drawCrown (g, { 26.f, 20.f, 34.f, 24.f }, ice, violetHi, 1.8f, pulse);
    g.setColour (magenta.withAlpha (0.25f * pulse));
    g.strokePath (titlePath, juce::PathStrokeType (7.f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    g.setColour (violet.withAlpha (0.8f));
    g.strokePath (titlePath, juce::PathStrokeType (3.f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    const auto tb = titlePath.getBounds();
    g.setGradientFill (juce::ColourGradient::vertical (juce::Colour (0xfffbf5ff), tb.getY(), juce::Colour (0xffa57be0), tb.getBottom()));
    g.fillPath (titlePath);
    g.setColour (violetHi.withAlpha (0.8f));
    g.setFont (juce::Font (juce::FontOptions (10.f, juce::Font::bold)));
    g.drawText ("3 - I N - 1   C O M P R E S S O R", 66, 62, 220, 13, juce::Justification::centredLeft);

    // ---- barra secundaria
    g.setColour (violetHi);
    g.setFont (juce::Font (juce::FontOptions (12.f, juce::Font::bold)));
    g.drawText ("SOURCE", 24, 86, 60, 28, juce::Justification::centredLeft);
    g.drawText ("SAFETY", 820, 86, 60, 28, juce::Justification::centredRight);
    if (uiTopo != 0)
    {
        g.setColour (violetHi.withAlpha (0.85f));
        g.setFont (juce::Font (juce::FontOptions (13.f, juce::Font::bold | juce::Font::italic)));
        g.drawText (uiTopo == 1 ? "OPTICAL CELL  -  PROGRAM-DEPENDENT RESPONSE"
                                : "VARI-MU TUBE  -  PROGRESSIVE RATIO",
                    350, 86, 400, 28, juce::Justification::centred);
    }

    // ---- panel principal
    const juce::Rectangle<float> panel (16.f, 122.f, 1068.f, 590.f);
    g.setGradientFill (juce::ColourGradient (juce::Colour (0xd01a1030), 0.f, panel.getY(), juce::Colour (0xd0080510), 0.f, panel.getBottom(), false));
    g.fillRoundedRectangle (panel, 14.f);
    g.setColour (juce::Colour (0x26ffffff));
    g.drawLine (34.f, panel.getY() + 1.5f, panel.getRight() - 34.f, panel.getY() + 1.5f, 1.f);
    g.setColour (violet.withAlpha (0.45f));
    g.drawRoundedRectangle (panel, 14.f, 1.4f);

    // franjas diagonales en las esquinas
    for (int i = 0; i < 4; ++i)
    {
        const float o = (float) i * 15.f;
        juce::Path s1, s2;
        s1.startNewSubPath (22.f + o, 128.f);  s1.lineTo (31.f + o, 128.f);  s1.lineTo (22.f, 137.f + o);  s1.lineTo (22.f, 128.f + o);  s1.closeSubPath();
        s2.startNewSubPath (1078.f - o, 706.f); s2.lineTo (1069.f - o, 706.f); s2.lineTo (1078.f, 697.f - o); s2.lineTo (1078.f, 706.f - o); s2.closeSubPath();
        g.setColour (violet.withAlpha (0.55f - 0.1f * (float) i));
        g.fillPath (s1);
        g.setColour (magenta.withAlpha (0.5f - 0.1f * (float) i));
        g.fillPath (s2);
    }

    // titulo sobre el medidor (segun topologia)
    g.setColour (violet);
    g.setFont (juce::Font (juce::FontOptions (19.f, juce::Font::bold)));
    g.drawText (uiTopo == 0 ? "V C A   C O M P R E S S O R"
              : uiTopo == 1 ? "O P T I C A L   C O M P R E S S O R"
                            : "V A R I - M U   C O M P R E S S O R",
                (int) meterArea.getX(), 126, meterArea.getWidth(), 24, juce::Justification::centred);

    drawMeter (g, meterArea.toFloat());

    // controles ocultos segun la topologia: se explica por que
    g.setColour (violetHi.withAlpha (0.55f));
    g.setFont (juce::Font (juce::FontOptions (12.f, juce::Font::bold)));
    if (uiTopo == 1)
    {
        g.drawText ("ATTACK", 34, 410, 118, 16, juce::Justification::centred);
        g.drawText ("AUTO (CELL)", 34, 470, 118, 16, juce::Justification::centred);
    }
    if (uiTopo != 0)
    {
        g.drawText ("KNEE", 34 + 4 * 118 - 118 * 0, 410, 0, 0, juce::Justification::centred);
        g.drawText ("KNEE", 270, 410, 118, 16, juce::Justification::centred);
        g.drawText (uiTopo == 1 ? "FIXED" : "CURVE", 270, 470, 118, 16, juce::Justification::centred);
    }

    // divisores
    g.setColour (ice.withAlpha (0.14f));
    g.drawLine (40.f, 396.f, 1060.f, 396.f, 1.f);
    g.drawLine (40.f, 562.f, 1060.f, 562.f, 1.f);

    // medidores de entrada y salida
    drawLedBar (g, { 668.f, 450.f, 380.f, 22.f }, inDisp, "INPUT");
    drawLedBar (g, { 668.f, 516.f, 380.f, 22.f }, outDisp, "OUTPUT");

    // seccion de nivel y panel ANALOG COLOR
    g.setColour (violetHi);
    g.setFont (juce::Font (juce::FontOptions (12.f, juce::Font::bold)));
    g.drawText ("L E V E L   &   M I X", 34, 568, 400, 18, juce::Justification::centredLeft);

    const juce::Rectangle<float> an (528.f, 566.f, 540.f, 142.f);
    g.setGradientFill (juce::ColourGradient (juce::Colour (0x66300a38), an.getX(), an.getY(), juce::Colour (0x66100618), an.getX(), an.getBottom(), false));
    g.fillRoundedRectangle (an, 12.f);
    g.setColour (magenta.withAlpha (analogUiOn ? 0.75f : 0.3f));
    g.drawRoundedRectangle (an, 12.f, 1.5f);
    g.setColour (magenta.interpolatedWith (juce::Colours::white, 0.35f));
    g.setFont (juce::Font (juce::FontOptions (12.f, juce::Font::bold)));
    g.drawText ("A N A L O G   C O L O R", 544, 568, 300, 18, juce::Justification::centredLeft);

    // pie
    g.setColour (ice.withAlpha (0.7f));
    g.setFont (juce::Font (juce::FontOptions (12.f, juce::Font::bold)));
    g.drawText (juce::String::fromUTF8 ("BEATS  \xe2\x80\xa2  MIX  \xe2\x80\xa2  MASTER  \xe2\x80\xa2  CREATIVE SOUNDS"),
                0, 716, kW, 14, juce::Justification::centred);
    g.setColour (violetHi);
    g.setFont (juce::Font (juce::FontOptions (13.f, juce::Font::bold)));
    g.drawText ("@DioOnTheBeat", 0, 730, kW, 14, juce::Justification::centred);
    (void) W;
}

void DioPanel::resized()
{
    rebuildBackground();
    rebuildTitle();

    // barra superior
    prevBtn.setBounds (294, 26, 32, 38);
    presetBox.setBounds (330, 26, 230, 38);
    nextBtn.setBounds (564, 26, 32, 38);
    for (int i = 0; i < 3; ++i) topoBtn[(size_t) i].setBounds (616 + i * 94, 26, 90, 38);
    settingsBtn.setBounds (904, 26, 86, 38);
    powerBtn.setBounds (1006, 18, 62, 58);

    // barra secundaria
    profileBox.setBounds (88, 86, 200, 28);
    for (int i = 0; i < 3; ++i) modeBtn[(size_t) i].setBounds (394 + i * 104, 86, 100, 28);
    safetyBox.setBounds (888, 86, 186, 28);

    auto place = [this] (int i, int x, int y, int w, int h)
    {
        knobs[(size_t) i].l.setBounds (x, y, w, 18);
        knobs[(size_t) i].s.setBounds (x, y + 18, w, h);
    };
    // fila A: threshold, (medidor), ratio
    place (0, 40, 128, 230, 246);
    place (1, 830, 128, 230, 246);
    // fila B
    const int bx = 34, bw = 118;
    place (2, bx + 0 * bw, 404, bw, 134);   // attack
    place (3, bx + 1 * bw, 404, bw, 134);   // release
    place (4, bx + 2 * bw, 404, bw, 134);   // knee
    place (5, bx + 3 * bw, 404, bw, 134);   // sc hpf
    place (6, bx + 4 * bw, 404, bw, 134);   // ceiling
    // fila C
    place (7, bx + 0 * bw, 590, bw, 118);   // input
    place (8, bx + 1 * bw, 590, bw, 118);   // makeup
    place (9, bx + 2 * bw, 590, bw, 118);   // output
    place (10, bx + 3 * bw, 590, bw, 118);  // dry/wet
    place (11, 548, 590, bw, 118);          // drive
    place (12, 548 + bw, 590, bw, 118);     // color
    place (13, 548 + 2 * bw, 590, bw, 118); // mix analogico
    knobs[8].l.setBounds (bx + 1 * bw, 590, 66, 18);                 // etiqueta MAKEUP mas corta
    autoMkBtn.setBounds (bx + 1 * bw + 64, 588, 56, 22);
    analogBtn.setBounds (930, 600, 124, 40);
}

// ---------------------------------------------------------------- Ventana escalable
DioCompressEditor::DioCompressEditor (DioCompressProcessor& p)
    : AudioProcessorEditor (&p), proc (p), panel (p)
{
    addAndMakeVisible (panel);
    setResizeLimits (440, 298, 1650, 1116);                      // 40% a 150%
    getConstrainer()->setFixedAspectRatio ((double) DioPanel::kW / (double) DioPanel::kH);
    setResizable (true, true);

    panel.onSizePercent = [this] (int pct)
    {
        setSize (juce::roundToInt ((float) DioPanel::kW * (float) pct / 100.f),
                 juce::roundToInt ((float) DioPanel::kH * (float) pct / 100.f));
    };
    panel.getSizePercent = [this] { return juce::roundToInt (100.f * (float) getWidth() / (float) DioPanel::kW); };

    const int w = juce::jlimit (440, 1650, p.savedEditorWidth);
    setSize (w, juce::roundToInt ((float) w * (float) DioPanel::kH / (float) DioPanel::kW));
}

void DioCompressEditor::resized()
{
    const float s = (float) getWidth() / (float) DioPanel::kW;
    panel.setBounds (0, 0, DioPanel::kW, DioPanel::kH);
    panel.setTransform (juce::AffineTransform::scale (s));
    proc.savedEditorWidth = getWidth();
}
