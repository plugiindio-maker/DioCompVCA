#include "PluginEditor.h"
using namespace DioStyle;

// ---------------------------------------------------------------- Look & Feel
DioLookAndFeel::DioLookAndFeel()
{
    setColour (juce::ComboBox::backgroundColourId, juce::Colour (0xff111827));
    setColour (juce::ComboBox::outlineColourId, neon.withAlpha (0.7f));
    setColour (juce::ComboBox::textColourId, ice);
    setColour (juce::ComboBox::arrowColourId, neonHi);
    setColour (juce::PopupMenu::backgroundColourId, juce::Colour (0xff0d1424));
    setColour (juce::PopupMenu::textColourId, ice);
    setColour (juce::PopupMenu::headerTextColourId, neonHi);
    setColour (juce::PopupMenu::highlightedBackgroundColourId, neon);
    setColour (juce::PopupMenu::highlightedTextColourId, juce::Colours::white);
    setColour (juce::TextButton::buttonColourId, juce::Colour (0xff111827));
    setColour (juce::TextButton::textColourOffId, ice);
    setColour (juce::TextButton::textColourOnId, ice);
}

void DioLookAndFeel::drawRotarySlider (juce::Graphics& g, int x, int y, int w, int h,
                                       float pos, float startAngle, float endAngle, juce::Slider& slider)
{
    juce::Colour acc = neon;
    if (slider.getProperties().contains ("accent"))
        acc = juce::Colour ((juce::uint32) (int) slider.getProperties()["accent"]);
    const juce::Colour accHi = acc.interpolatedWith (juce::Colours::white, 0.4f);

    auto b = juce::Rectangle<float> ((float) x, (float) y, (float) w, (float) h).reduced (8.f);
    const float r = juce::jmin (b.getWidth(), b.getHeight()) * 0.5f;
    const auto c = b.getCentre();
    const float angle = startAngle + pos * (endAngle - startAngle);
    const float arcR = r * 0.84f, capR = r * 0.62f;

    auto pt = [&] (float a, float rad) { return juce::Point<float> (c.x + std::sin (a) * rad, c.y - std::cos (a) * rad); };

    // marcas alrededor
    for (int i = 0; i <= 10; ++i)
    {
        const float a = startAngle + (float) i / 10.f * (endAngle - startAngle);
        g.setColour (i % 5 == 0 ? accHi.withAlpha (0.85f) : juce::Colour (0xff35466b));
        g.drawLine (juce::Line<float> (pt (a, r * 0.98f), pt (a, r * 1.08f)), i % 5 == 0 ? 2.f : 1.f);
    }

    // pista y arco de valor con brillo
    juce::Path track;
    track.addCentredArc (c.x, c.y, arcR, arcR, 0.f, startAngle, endAngle, true);
    g.setColour (juce::Colour (0xff1a2438));
    g.strokePath (track, juce::PathStrokeType (4.f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    if (pos > 0.001f)
    {
        juce::Path val;
        val.addCentredArc (c.x, c.y, arcR, arcR, 0.f, startAngle, angle, true);
        g.setColour (acc.withAlpha (0.22f));
        g.strokePath (val, juce::PathStrokeType (11.f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        g.setColour (acc.withAlpha (0.5f));
        g.strokePath (val, juce::PathStrokeType (6.f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        g.setColour (accHi);
        g.strokePath (val, juce::PathStrokeType (3.f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    }

    // sombra + tapa metalica plateada
    g.setColour (juce::Colour (0x77000000));
    g.fillEllipse (c.x - capR - 1.f, c.y - capR + 3.f, (capR + 1.f) * 2.f, (capR + 1.f) * 2.f);
    g.setGradientFill (juce::ColourGradient (juce::Colour (0xfff7f9ff), c.x - capR * 0.6f, c.y - capR * 0.8f,
                                             juce::Colour (0xff4a5368), c.x + capR * 0.7f, c.y + capR * 0.9f, false));
    g.fillEllipse (c.x - capR, c.y - capR, capR * 2.f, capR * 2.f);

    // estrias
    g.setColour (juce::Colour (0x44000000));
    for (int i = 0; i < 28; ++i)
    {
        const float a = (float) i * juce::MathConstants<float>::twoPi / 28.f;
        g.drawLine (juce::Line<float> (pt (a, capR * 0.9f), pt (a, capR)), 1.f);
    }

    // plato interior
    const float innerR = capR * 0.72f;
    g.setGradientFill (juce::ColourGradient (juce::Colour (0xff98a2b9), c.x, c.y - innerR,
                                             juce::Colour (0xffe9eeff), c.x, c.y + innerR, false));
    g.fillEllipse (c.x - innerR, c.y - innerR, innerR * 2.f, innerR * 2.f);
    g.setColour (juce::Colour (0x88101624));
    g.drawEllipse (c.x - innerR, c.y - innerR, innerR * 2.f, innerR * 2.f, 1.2f);
    g.setColour (juce::Colour (0xff0b0f1a));
    g.drawEllipse (c.x - capR, c.y - capR, capR * 2.f, capR * 2.f, 1.6f);

    // puntero con brillo del color de la seccion
    const auto p1 = pt (angle, capR * 0.20f), p2 = pt (angle, capR * 0.94f);
    g.setColour (acc.withAlpha (0.35f));
    g.drawLine (juce::Line<float> (p1, p2), 7.f);
    g.setColour (accHi);
    g.drawLine (juce::Line<float> (p1, p2), 3.f);
}

void DioLookAndFeel::drawToggleButton (juce::Graphics& g, juce::ToggleButton& bt, bool highlighted, bool)
{
    auto r = bt.getLocalBounds().toFloat().reduced (3.f);
    const bool on = bt.getToggleState();
    if (on)
    {
        g.setColour (cyan.withAlpha (0.25f));
        g.fillRoundedRectangle (r.expanded (2.f), 9.f);
        g.setGradientFill (juce::ColourGradient (cyan.interpolatedWith (neon, 0.3f), r.getX(), r.getY(),
                                                 neon.darker (0.4f), r.getX(), r.getBottom(), false));
    }
    else
        g.setColour (juce::Colour (highlighted ? 0xff1f2840 : 0xff131a2b));
    g.fillRoundedRectangle (r, 7.f);
    g.setColour (on ? cyan : juce::Colour (0xff2b3856));
    g.drawRoundedRectangle (r, 7.f, 1.3f);
    g.setColour (on ? juce::Colours::white : juce::Colour (0xff9fb0d0));
    g.setFont (juce::Font (juce::FontOptions (12.5f, juce::Font::bold)));
    g.drawText (bt.getButtonText(), bt.getLocalBounds(), juce::Justification::centred);
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

// ---------------------------------------------------------------- Panel
DioPanel::DioPanel (VCACompAudioProcessor& p)
    : proc (p)
{
    // fila A: compresor (azul)
    addKnob (0, "THRESHOLD", "thr", " dB", neon);
    addKnob (1, "RATIO", "ratio", ":1", neon);
    addKnob (2, "ATTACK", "attack", " ms", neon);
    addKnob (3, "RELEASE", "release", " ms", neon);
    addKnob (4, "KNEE", "knee", " dB", neon);
    addKnob (5, "SC HPF", "hpf", " Hz", neon);
    // fila B: nivel (cian)
    addKnob (6, "MAKEUP", "makeup", " dB", cyan);
    addKnob (7, "MIX", "mix", " %", cyan);
    addKnob (8, "OUTPUT", "out", " dB", cyan);
    addKnob (9, "CEILING", "ceil", " dB", cyan);
    // fila C: caracter (colores calidos)
    addKnob (10, "BODY", "body", " %", orange);
    addKnob (11, "WEIGHT", "weight", " %", orange);
    addKnob (12, "BODY FREQ", "freq", " Hz", orange);
    addKnob (13, "DRIVE", "drive", " %", hot);
    addKnob (14, "PUNCH", "punch", " %", gold);
    addKnob (15, "PRESENCE", "presence", " %", purple);

    addToggle (0, "AUTO REL", "autorel");
    addToggle (1, "AUTO MAKEUP", "automk");
    addToggle (2, "RMS", "rms");

    safetyBox.setLookAndFeel (&laf);
    safetyBox.addItemList (juce::StringArray { "OFF", "LIMIT", "CLIP" }, 1);
    addAndMakeVisible (safetyBox);
    safetyAtt = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (proc.apvts, "safety", safetyBox);

    presetBox.setLookAndFeel (&laf);
    for (int i = 0; i < proc.getNumPrograms(); ++i)
        presetBox.addItem (proc.getProgramName (i), i + 1);
    presetBox.setSelectedId (proc.getCurrentProgram() + 1, juce::dontSendNotification);
    presetBox.onChange = [this] { proc.setCurrentProgram (presetBox.getSelectedId() - 1); };
    addAndMakeVisible (presetBox);

    settingsBtn.setLookAndFeel (&laf);
    settingsBtn.setButtonText ("SETTINGS");
    settingsBtn.onClick = [this] { showSettingsMenu(); };
    addAndMakeVisible (settingsBtn);

    setSize (kW, kH);
    startTimerHz (30);
}

DioPanel::~DioPanel()
{
    stopTimer();
    safetyBox.setLookAndFeel (nullptr);
    presetBox.setLookAndFeel (nullptr);
    settingsBtn.setLookAndFeel (nullptr);
    for (auto& k : knobs)   k.s.setLookAndFeel (nullptr);
    for (auto& t : toggles) t.b.setLookAndFeel (nullptr);
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
    k.l.setColour (juce::Label::textColourId, accent.interpolatedWith (juce::Colours::white, 0.35f));
    k.l.setFont (juce::Font (juce::FontOptions (12.f, juce::Font::bold)));
    addAndMakeVisible (k.s);
    addAndMakeVisible (k.l);
    k.a = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (proc.apvts, id, k.s);
}

void DioPanel::addToggle (int i, const juce::String& name, const juce::String& id)
{
    auto& t = toggles[(size_t) i];
    t.b.setLookAndFeel (&laf);
    t.b.setButtonText (name);
    addAndMakeVisible (t.b);
    t.a = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (proc.apvts, id, t.b);
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

void DioPanel::mouseDown (const juce::MouseEvent& e)
{
    if (vuHit.contains (e.getPosition()))        // clic en el medidor: alterna GR / OUTPUT (como el CLA-2A)
    {
        vuMode = 1 - vuMode;
        repaint (vuHit);
    }
}

void DioPanel::timerCallback()
{
    meterDisp = juce::jmax (proc.grMeter.load(), meterDisp * 0.88f);
    outDisp   = juce::jmax (proc.outMeter.load(), outDisp - 1.5f);

    const float posT = (vuMode == 0) ? 1.f - juce::jlimit (0.f, 1.f, meterDisp / 20.f)
                                     : juce::jlimit (0.f, 1.f, (outDisp + 20.f) / 23.f);
    needlePos += (posT - needlePos) * 0.30f;

    phase += 0.10f;
    if (phase > juce::MathConstants<float>::twoPi) phase -= juce::MathConstants<float>::twoPi;

    repaint (vuHit);
    if ((++tick & 1) == 0)
    {
        repaint (0, 0, kW, 142);       // coronas y titulo palpitan
        repaint (plateArea);
    }
}

void DioPanel::rebuildBackground()
{
    const int w = getWidth(), h = getHeight();
    if (w <= 0 || h <= 0) return;
    bg = juce::Image (juce::Image::ARGB, w, h, true);
    juce::Graphics g (bg);
    g.setGradientFill (juce::ColourGradient::vertical (juce::Colour (0xff05070e), 0.f, juce::Colour (0xff0a1126), (float) h));
    g.fillAll();

    juce::Random rnd (1337);   // humo azul (fijo, siempre igual)
    for (int i = 0; i < 46; ++i)
    {
        const float cx = rnd.nextFloat() * (float) w, cy = rnd.nextFloat() * (float) h;
        const float rad = 80.f + rnd.nextFloat() * 200.f;
        const auto c = juce::Colour::fromHSV (0.60f + rnd.nextFloat() * 0.06f, 0.85f, 0.5f + 0.5f * rnd.nextFloat(),
                                              0.05f + 0.10f * rnd.nextFloat());
        g.setGradientFill (juce::ColourGradient (c, cx, cy, c.withAlpha (0.f), cx + rad, cy, true));
        g.fillEllipse (cx - rad, cy - rad, rad * 2.f, rad * 2.f);
    }
    // destellos calidos (fuego/amber) en las esquinas inferiores: contraste con el azul
    auto glow = [&] (float cx, float cy, float rad, juce::Colour c)
    {
        g.setGradientFill (juce::ColourGradient (c, cx, cy, c.withAlpha (0.f), cx + rad, cy, true));
        g.fillEllipse (cx - rad, cy - rad, rad * 2.f, rad * 2.f);
    };
    glow (60.f, (float) h - 40.f, 240.f, orange.withAlpha (0.10f));
    glow ((float) w - 60.f, (float) h - 20.f, 260.f, purple.withAlpha (0.10f));

    // textura metalica fina en la zona del panel
    for (int yy = 184; yy < 674; yy += 2)
    {
        g.setColour (juce::Colours::white.withAlpha (0.012f + 0.02f * rnd.nextFloat()));
        g.drawHorizontalLine (yy, 22.f, 658.f);
    }
    // vineta
    g.setGradientFill (juce::ColourGradient (juce::Colours::transparentBlack, (float) w * 0.5f, (float) h * 0.5f,
                                             juce::Colour (0xe6000000), 0.f, 0.f, true));
    g.fillAll();
}

void DioPanel::rebuildTitle()
{
    juce::GlyphArrangement ga;
    ga.addLineOfText (juce::Font (juce::FontOptions (90.f, juce::Font::bold)), "DioCompVCA", 0.f, 0.f);
    titlePath.clear();
    ga.createPath (titlePath);
    titlePath.applyTransform (juce::AffineTransform::shear (-0.22f, 0.f));   // inclinado tipo graffiti

    const auto b = titlePath.getBounds();
    if (b.getWidth() <= 0.f) return;
    const juce::Rectangle<float> area (60.f, 42.f, (float) getWidth() - 120.f, 76.f);
    const float s = juce::jmin (area.getWidth() / b.getWidth(), area.getHeight() / b.getHeight());
    const float tx = area.getCentreX() - b.getWidth() * s * 0.5f;
    const float ty = area.getCentreY() - b.getHeight() * s * 0.5f;
    titlePath.applyTransform (juce::AffineTransform::translation (-b.getX(), -b.getY()).scaled (s).translated (tx, ty));
}

// Medidor de aguja retroiluminado (estilo CLA-2A)
void DioPanel::drawVU (juce::Graphics& g, juce::Rectangle<float> r)
{
    const bool grMode = (vuMode == 0);

    // marco metalico
    g.setGradientFill (juce::ColourGradient (juce::Colour (0xffe9edf7), r.getX(), r.getY(),
                                             juce::Colour (0xff5d6579), r.getX(), r.getBottom(), false));
    g.fillRoundedRectangle (r, 10.f);
    g.setColour (juce::Colour (0xff0b0e16));
    g.drawRoundedRectangle (r, 10.f, 1.5f);

    auto face = r.reduced (9.f);
    g.setColour (juce::Colour (0xff1a1205));
    g.fillRoundedRectangle (face.expanded (2.f), 6.f);
    // cara retroiluminada
    g.setGradientFill (juce::ColourGradient (juce::Colour (0xfffff4d2), face.getCentreX(), face.getBottom() - 8.f,
                                             juce::Colour (0xffc48a35), face.getX(), face.getY(), true));
    g.fillRoundedRectangle (face, 5.f);

    {
        juce::Graphics::ScopedSaveState ss (g);
        g.reduceClipRegion (face.toNearestInt());

        const juce::Point<float> P (face.getCentreX(), face.getBottom() + face.getHeight() * 0.18f);
        const float R = face.getHeight() * 0.98f;
        const float half = juce::degreesToRadians (46.f);
        auto polar = [&] (float ang, float rad) { return juce::Point<float> (P.x + std::sin (ang) * rad, P.y - std::cos (ang) * rad); };
        auto posOf = [&] (float v) { return grMode ? 1.f - juce::jlimit (0.f, 1.f, v / 20.f)
                                                   : juce::jlimit (0.f, 1.f, (v + 20.f) / 23.f); };
        auto angOf = [&] (float pos) { return -half + pos * 2.f * half; };
        const juce::Colour ink (0xff2b1a05), red (0xffc0301f);

        // zona roja
        juce::Path redArc;
        if (grMode) redArc.addCentredArc (P.x, P.y, R * 0.80f, R * 0.80f, 0.f, angOf (0.f), angOf (0.5f), true);
        else        redArc.addCentredArc (P.x, P.y, R * 0.80f, R * 0.80f, 0.f, angOf (posOf (0.f)), angOf (1.f), true);
        g.setColour (red.withAlpha (0.85f));
        g.strokePath (redArc, juce::PathStrokeType (4.f));

        // arco de la escala
        juce::Path arc;
        arc.addCentredArc (P.x, P.y, R * 0.80f, R * 0.80f, 0.f, -half, half, true);
        g.setColour (ink);
        g.strokePath (arc, juce::PathStrokeType (1.2f));

        // marcas menores y mayores + numeros
        const float lo = grMode ? 0.f : -20.f, hi = grMode ? 20.f : 3.f;
        for (float v = lo; v <= hi + 0.01f; v += 1.f)
        {
            const float a = angOf (posOf (v));
            g.setColour (ink);
            g.drawLine (juce::Line<float> (polar (a, R * 0.80f), polar (a, R * 0.74f)), 1.f);
        }
        const std::vector<float> majors = grMode ? std::vector<float> { 0.f, 2.f, 4.f, 6.f, 10.f, 15.f, 20.f }
                                                 : std::vector<float> { -20.f, -10.f, -7.f, -5.f, -3.f, 0.f, 3.f };
        g.setFont (juce::Font (juce::FontOptions (10.f, juce::Font::bold)));
        for (float v : majors)
        {
            const float a = angOf (posOf (v));
            const bool isRed = grMode ? (v >= 10.f) : (v >= 0.f);
            g.setColour (isRed ? red : ink);
            g.drawLine (juce::Line<float> (polar (a, R * 0.80f), polar (a, R * 0.70f)), 2.f);
            const auto tp = polar (a, R * 0.62f);
            juce::String txt = (! grMode && v > 0.f) ? "+" + juce::String ((int) v) : juce::String ((int) v);
            g.drawText (txt, juce::Rectangle<float> (tp.x - 14.f, tp.y - 7.f, 28.f, 14.f), juce::Justification::centred);
        }

        // texto de la cara
        g.setColour (ink.withAlpha (0.85f));
        g.setFont (juce::Font (juce::FontOptions (10.f, juce::Font::bold | juce::Font::italic)));
        g.drawText (grMode ? "GAIN REDUCTION" : "OUTPUT LEVEL", face.withTrimmedTop (face.getHeight() * 0.62f).withHeight (14.f),
                    juce::Justification::centred);
        g.setFont (juce::Font (juce::FontOptions (13.f, juce::Font::bold | juce::Font::italic)));
        g.drawText ("VU", face.withTrimmedTop (face.getHeight() * 0.76f).withHeight (16.f), juce::Justification::centred);

        // aguja con sombra
        const float na = angOf (juce::jlimit (0.f, 1.f, needlePos));
        const auto tip = polar (na, R * 0.96f);
        g.setColour (juce::Colour (0x44000000));
        g.drawLine (juce::Line<float> (P.translated (3.f, 3.f), tip.translated (3.f, 3.f)), 2.4f);
        g.setColour (juce::Colour (0xff150d05));
        g.drawLine (juce::Line<float> (P, tip), 1.8f);

        // reflejo del vidrio
        g.setGradientFill (juce::ColourGradient (juce::Colours::white.withAlpha (0.32f), face.getCentreX(), face.getY(),
                                                 juce::Colours::white.withAlpha (0.f), face.getCentreX(), face.getCentreY() + 8.f, false));
        g.fillRect (face.withHeight (face.getHeight() * 0.62f));
    }

    drawScrew (g, r.getX() + 5.f, r.getY() + 5.f, 2.6f);
    drawScrew (g, r.getRight() - 5.f, r.getY() + 5.f, 2.6f);
    drawScrew (g, r.getX() + 5.f, r.getBottom() - 5.f, 2.6f);
    drawScrew (g, r.getRight() - 5.f, r.getBottom() - 5.f, 2.6f);
}

void DioPanel::paint (juce::Graphics& g)
{
    g.drawImageAt (bg, 0, 0);
    const float W = (float) kW;
    const float pulse = 0.82f + 0.18f * std::sin (phase);

    // coronas: neon palpitante (arriba derecha) y plata (sobre el titulo)
    drawCrown (g, { W - 130.f, 10.f, 84.f, 54.f }, neonHi, neon, 3.f, pulse);
    drawCrown (g, { 62.f, 8.f, 58.f, 40.f }, ice, juce::Colour (0xff9fb6e8), 2.2f, 1.f);

    // titulo estilo graffiti: brillo azul + contorno + relleno plateado
    g.setColour (neon.withAlpha (0.28f * pulse));
    g.strokePath (titlePath, juce::PathStrokeType (12.f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    g.setColour (neon.withAlpha (0.85f));
    g.strokePath (titlePath, juce::PathStrokeType (5.f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    g.setColour (juce::Colour (0xff070b18));
    g.strokePath (titlePath, juce::PathStrokeType (2.5f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    const auto tb = titlePath.getBounds();
    g.setGradientFill (juce::ColourGradient::vertical (juce::Colour (0xfffbfcff), tb.getY(), juce::Colour (0xff8f99b3), tb.getBottom()));
    g.fillPath (titlePath);

    // subtitulo
    g.setColour (neonHi);
    g.setFont (juce::Font (juce::FontOptions (15.f, juce::Font::bold | juce::Font::italic)));
    g.drawText ("V C A    C O M P R E S S O R", 0, 118, kW, 20, juce::Justification::centred);
    g.setColour (ice.withAlpha (0.35f));
    g.drawLine (120.f, 129.f, W * 0.5f - 130.f, 129.f, 1.f);
    g.drawLine (W * 0.5f + 130.f, 129.f, W - 120.f, 129.f, 1.f);

    // etiqueta de presets
    g.setColour (neonHi);
    g.setFont (juce::Font (juce::FontOptions (12.f, juce::Font::bold)));
    g.drawText ("PRESET", kW / 2 - 190, 146, 70, 26, juce::Justification::centredRight);

    // panel principal (vidrio oscuro con brillo superior)
    const juce::Rectangle<float> panel (20.f, 182.f, 640.f, 494.f);
    g.setGradientFill (juce::ColourGradient (juce::Colour (0xd01a2133), 0.f, panel.getY(),
                                             juce::Colour (0xd0090c15), 0.f, panel.getBottom(), false));
    g.fillRoundedRectangle (panel, 12.f);
    g.setColour (juce::Colour (0x26ffffff));
    g.drawLine (34.f, panel.getY() + 1.5f, panel.getRight() - 34.f, panel.getY() + 1.5f, 1.f);
    g.setColour (neon.withAlpha (0.4f));
    g.drawRoundedRectangle (panel, 12.f, 1.3f);
    drawScrew (g, 30.f, 192.f, 3.f);   drawScrew (g, 650.f, 192.f, 3.f);
    drawScrew (g, 30.f, 666.f, 3.f);   drawScrew (g, 650.f, 666.f, 3.f);

    // titulos de seccion con barra de color
    auto section = [&] (const juce::String& text, int y, juce::Colour c)
    {
        g.setColour (c);
        g.fillRoundedRectangle (42.f, (float) y + 3.f, 4.f, 11.f, 2.f);
        g.setColour (c.interpolatedWith (juce::Colours::white, 0.4f));
        g.setFont (juce::Font (juce::FontOptions (12.f, juce::Font::bold)));
        g.drawText (text, 52, y, 520, 16, juce::Justification::centredLeft);
    };
    section ("C O M P R E S S O R", 186, neon);
    section ("L E V E L   &   O P T I O N S", 346, cyan);
    section ("C H A R A C T E R   :   B O D Y   /   D R I V E   /   P U N C H   /   P R E S E N C E", 506, orange);

    g.setColour (ice.withAlpha (0.7f));
    g.setFont (juce::Font (juce::FontOptions (10.f, juce::Font::bold)));
    g.drawText ("SAFETY", 556, 424, 100, 14, juce::Justification::centred);

    // ---- medidor de aguja
    drawVU (g, vuArea.toFloat());

    // LEDs de modo + lecturas digitales
    const bool grMode = (vuMode == 0);
    auto led = [&] (float cx, bool on, const juce::String& t)
    {
        if (on) { g.setColour (amber.withAlpha (0.35f)); g.fillEllipse (cx - 8.f, 358.f, 16.f, 16.f); }
        g.setColour (on ? amber : juce::Colour (0xff2a2418));
        g.fillEllipse (cx - 4.5f, 361.5f, 9.f, 9.f);
        g.setColour (on ? ice : ice.withAlpha (0.45f));
        g.setFont (juce::Font (juce::FontOptions (11.f, juce::Font::bold)));
        g.drawText (t, (int) cx + 8, 359, 80, 14, juce::Justification::centredLeft);
    };
    led (700.f, grMode, "GAIN RED.");
    led (800.f, ! grMode, "OUTPUT");

    auto readout = [&] (juce::Rectangle<float> rc, const juce::String& cap, const juce::String& val)
    {
        g.setColour (juce::Colour (0xff06080e));
        g.fillRoundedRectangle (rc, 6.f);
        g.setColour (amber.withAlpha (0.35f));
        g.drawRoundedRectangle (rc, 6.f, 1.f);
        g.setColour (ice.withAlpha (0.55f));
        g.setFont (juce::Font (juce::FontOptions (9.f, juce::Font::bold)));
        g.drawText (cap, rc.withHeight (11.f).translated (0.f, 2.f), juce::Justification::centred);
        g.setColour (amber);
        g.setFont (juce::Font (juce::FontOptions (16.f, juce::Font::bold)));
        g.drawText (val, rc.withTrimmedTop (11.f), juce::Justification::centred);
    };
    readout ({ 684.f, 380.f, 104.f, 34.f }, "GR", juce::String (meterDisp > 0.05f ? -meterDisp : 0.f, 1) + " dB");
    readout ({ 796.f, 380.f, 104.f, 34.f }, "OUT", (outDisp <= -59.f ? juce::String ("-inf") : juce::String (outDisp, 1)) + " dB");

    // ---- placa de marca
    const auto pl = plateArea.toFloat();
    g.setGradientFill (juce::ColourGradient (juce::Colour (0xff1c2438), pl.getX(), pl.getY(),
                                             juce::Colour (0xff080b13), pl.getX(), pl.getBottom(), false));
    g.fillRoundedRectangle (pl, 12.f);
    g.setColour (neon.withAlpha (0.35f + 0.25f * (pulse - 0.64f) / 0.36f));
    g.drawRoundedRectangle (pl, 12.f, 1.6f);
    drawCrown (g, { pl.getCentreX() - 46.f, pl.getY() + 22.f, 92.f, 60.f }, neonHi, neon, 3.5f, pulse);
    g.setGradientFill (juce::ColourGradient::vertical (juce::Colour (0xfffbfcff), pl.getY() + 98.f, juce::Colour (0xff8f99b3), pl.getY() + 130.f));
    g.setFont (juce::Font (juce::FontOptions (27.f, juce::Font::bold | juce::Font::italic)));
    g.drawText ("DioOnTheBeat", pl.withTrimmedTop (96.f).withHeight (36.f), juce::Justification::centred);
    g.setColour (neonHi);
    g.setFont (juce::Font (juce::FontOptions (13.f, juce::Font::bold | juce::Font::italic)));
    g.drawText ("B E A T M A K E R", pl.withTrimmedTop (136.f).withHeight (18.f), juce::Justification::centred);
    g.setColour (ice.withAlpha (0.3f));
    g.drawLine (pl.getX() + 30.f, pl.getY() + 164.f, pl.getRight() - 30.f, pl.getY() + 164.f, 1.f);
    g.setColour (orange);
    g.setFont (juce::Font (juce::FontOptions (12.f, juce::Font::bold)));
    g.drawText ("808  READY", pl.withTrimmedTop (172.f).withHeight (16.f), juce::Justification::centred);
    g.setColour (ice.withAlpha (0.55f));
    g.setFont (juce::Font (juce::FontOptions (10.f, juce::Font::bold)));
    g.drawText ("PUNCH  -  BODY  -  PRESENCE", pl.withTrimmedTop (192.f).withHeight (14.f), juce::Justification::centred);
    g.setGradientFill (juce::ColourGradient (juce::Colours::white.withAlpha (0.10f), pl.getX(), pl.getY(),
                                             juce::Colours::white.withAlpha (0.f), pl.getX(), pl.getCentreY(), false));
    g.fillRoundedRectangle (pl.reduced (2.f).withHeight (pl.getHeight() * 0.5f), 10.f);

    // pie
    g.setColour (ice.withAlpha (0.75f));
    g.setFont (juce::Font (juce::FontOptions (12.f, juce::Font::bold)));
    g.drawText (juce::String::fromUTF8 ("BEATS  \xe2\x80\xa2  MIX  \xe2\x80\xa2  MASTER  \xe2\x80\xa2  CREATIVE SOUNDS"),
                0, 684, kW, 16, juce::Justification::centred);
    g.setColour (neonHi);
    g.setFont (juce::Font (juce::FontOptions (13.f, juce::Font::bold)));
    g.drawText ("@DioOnTheBeat", 0, 702, kW, 18, juce::Justification::centred);
}

void DioPanel::resized()
{
    rebuildBackground();
    rebuildTitle();

    presetBox.setBounds (kW / 2 - 110, 146, 220, 26);
    settingsBtn.setBounds (kW - 140, 146, 120, 26);

    const int colW = 106, x0 = 24;
    const int rowY[3] = { 204, 364, 524 };
    for (int i = 0; i < 16; ++i)
    {
        const int row = (i < 6) ? 0 : (i < 10) ? 1 : 2;
        const int col = (i < 6) ? i : (i < 10) ? i - 6 : i - 10;
        knobs[(size_t) i].l.setBounds (x0 + col * colW, rowY[row], colW, 16);
        knobs[(size_t) i].s.setBounds (x0 + col * colW, rowY[row] + 16, colW, 124);
    }

    // botones y selector de seguridad junto a los knobs de nivel
    const int tx = x0 + 4 * colW + 2;
    toggles[0].b.setBounds (tx, 386, 102, 38);
    toggles[1].b.setBounds (tx + 104, 386, 102, 38);
    toggles[2].b.setBounds (tx, 436, 102, 38);
    safetyBox.setBounds (tx + 106, 440, 98, 30);
}

// ---------------------------------------------------------------- Ventana escalable
VCACompAudioProcessorEditor::VCACompAudioProcessorEditor (VCACompAudioProcessor& p)
    : AudioProcessorEditor (&p), proc (p), panel (p)
{
    addAndMakeVisible (panel);
    setResizeLimits (368, 290, 1380, 1086);                      // 40% a 150%
    getConstrainer()->setFixedAspectRatio ((double) DioPanel::kW / (double) DioPanel::kH);
    setResizable (true, true);

    panel.onSizePercent = [this] (int pct)
    {
        setSize (juce::roundToInt ((float) DioPanel::kW * (float) pct / 100.f),
                 juce::roundToInt ((float) DioPanel::kH * (float) pct / 100.f));
    };
    panel.getSizePercent = [this] { return juce::roundToInt (100.f * (float) getWidth() / (float) DioPanel::kW); };

    const int w = juce::jlimit (368, 1380, p.savedEditorWidth);
    setSize (w, juce::roundToInt ((float) w * (float) DioPanel::kH / (float) DioPanel::kW));
}

void VCACompAudioProcessorEditor::resized()
{
    const float s = (float) getWidth() / (float) DioPanel::kW;
    panel.setBounds (0, 0, DioPanel::kW, DioPanel::kH);
    panel.setTransform (juce::AffineTransform::scale (s));
    proc.savedEditorWidth = getWidth();
}
