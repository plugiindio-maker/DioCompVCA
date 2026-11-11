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
    setColour (juce::PopupMenu::highlightedBackgroundColourId, neon);
    setColour (juce::PopupMenu::highlightedTextColourId, juce::Colours::white);
}

void DioLookAndFeel::drawRotarySlider (juce::Graphics& g, int x, int y, int w, int h,
                                       float pos, float startAngle, float endAngle, juce::Slider&)
{
    auto b = juce::Rectangle<float> ((float) x, (float) y, (float) w, (float) h).reduced (8.f);
    const float r = juce::jmin (b.getWidth(), b.getHeight()) * 0.5f;
    const auto c = b.getCentre();
    const float angle = startAngle + pos * (endAngle - startAngle);
    const float arcR = r * 0.84f, capR = r * 0.62f;

    auto pt = [&] (float a, float rad) { return juce::Point<float> (c.x + std::sin (a) * rad, c.y - std::cos (a) * rad); };

    // marcas alrededor (cada 5 mas largas)
    for (int i = 0; i <= 10; ++i)
    {
        const float a = startAngle + (float) i / 10.f * (endAngle - startAngle);
        g.setColour (i % 5 == 0 ? neonHi.withAlpha (0.8f) : juce::Colour (0xff35466b));
        g.drawLine (juce::Line<float> (pt (a, r * 0.98f), pt (a, r * 1.08f)), i % 5 == 0 ? 2.f : 1.f);
    }

    // pista y arco de valor con brillo neon
    juce::Path track;
    track.addCentredArc (c.x, c.y, arcR, arcR, 0.f, startAngle, endAngle, true);
    g.setColour (juce::Colour (0xff1a2438));
    g.strokePath (track, juce::PathStrokeType (4.f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    if (pos > 0.001f)
    {
        juce::Path val;
        val.addCentredArc (c.x, c.y, arcR, arcR, 0.f, startAngle, angle, true);
        g.setColour (neon.withAlpha (0.22f));
        g.strokePath (val, juce::PathStrokeType (11.f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        g.setColour (neon.withAlpha (0.5f));
        g.strokePath (val, juce::PathStrokeType (6.f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        g.setColour (neonHi);
        g.strokePath (val, juce::PathStrokeType (3.f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    }

    // sombra + tapa metalica plateada (a juego con el titulo)
    g.setColour (juce::Colour (0x77000000));
    g.fillEllipse (c.x - capR - 1.f, c.y - capR + 3.f, (capR + 1.f) * 2.f, (capR + 1.f) * 2.f);
    g.setGradientFill (juce::ColourGradient (juce::Colour (0xfff7f9ff), c.x - capR * 0.6f, c.y - capR * 0.8f,
                                             juce::Colour (0xff4a5368), c.x + capR * 0.7f, c.y + capR * 0.9f, false));
    g.fillEllipse (c.x - capR, c.y - capR, capR * 2.f, capR * 2.f);

    // estrias en el borde
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

    // puntero neon
    const auto p1 = pt (angle, capR * 0.20f), p2 = pt (angle, capR * 0.94f);
    g.setColour (neon.withAlpha (0.35f));
    g.drawLine (juce::Line<float> (p1, p2), 7.f);
    g.setColour (neonHi);
    g.drawLine (juce::Line<float> (p1, p2), 3.f);
}

void DioLookAndFeel::drawToggleButton (juce::Graphics& g, juce::ToggleButton& bt, bool highlighted, bool)
{
    auto r = bt.getLocalBounds().toFloat().reduced (3.f);
    const bool on = bt.getToggleState();
    if (on)
    {
        g.setColour (neon.withAlpha (0.30f));
        g.fillRoundedRectangle (r.expanded (2.f), 9.f);
        g.setGradientFill (juce::ColourGradient (neonHi, r.getX(), r.getY(), neon.darker (0.4f), r.getX(), r.getBottom(), false));
    }
    else
        g.setColour (juce::Colour (highlighted ? 0xff1f2840 : 0xff131a2b));
    g.fillRoundedRectangle (r, 7.f);
    g.setColour (on ? neonHi : juce::Colour (0xff2b3856));
    g.drawRoundedRectangle (r, 7.f, 1.3f);
    g.setColour (on ? juce::Colours::white : juce::Colour (0xff9fb0d0));
    g.setFont (juce::Font (juce::FontOptions (12.5f, juce::Font::bold)));
    g.drawText (bt.getButtonText(), bt.getLocalBounds(), juce::Justification::centred);
}

// ---------------------------------------------------------------- helpers
static void drawCrown (juce::Graphics& g, juce::Rectangle<float> r, juce::Colour core, juce::Colour glow, float thick)
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
        g.setColour (glow.withAlpha (k == 4.f ? 0.12f : k == 2.5f ? 0.25f : 0.5f));
        g.strokePath (p, juce::PathStrokeType (thick * k, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    }
    g.setColour (core);
    g.strokePath (p, juce::PathStrokeType (thick, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
}

// ---------------------------------------------------------------- Editor
VCACompAudioProcessorEditor::VCACompAudioProcessorEditor (VCACompAudioProcessor& p)
    : AudioProcessorEditor (&p), proc (p)
{
    addKnob (0, "THRESHOLD", "thr", " dB");
    addKnob (1, "RATIO", "ratio", ":1");
    addKnob (2, "ATTACK", "attack", " ms");
    addKnob (3, "RELEASE", "release", " ms");
    addKnob (4, "KNEE", "knee", " dB");
    addKnob (5, "MAKEUP", "makeup", " dB");
    addKnob (6, "MIX", "mix", " %");
    addKnob (7, "SC HPF", "hpf", " Hz");
    addKnob (8, "OUTPUT", "out", " dB");
    addKnob (9, "CEILING", "ceil", " dB");
    addKnob (10, "BODY", "body", " %");
    addKnob (11, "WEIGHT", "weight", " %");
    addKnob (12, "BODY FREQ", "freq", " Hz");

    addToggle (0, "AUTO REL", "autorel");
    addToggle (1, "AUTO MAKEUP", "automk");
    addToggle (2, "RMS", "rms");
    addToggle (3, "PEAK GUARD", "guard");

    presetBox.setLookAndFeel (&laf);
    for (int i = 0; i < proc.getNumPrograms(); ++i)
        presetBox.addItem (proc.getProgramName (i), i + 1);
    presetBox.setSelectedId (proc.getCurrentProgram() + 1, juce::dontSendNotification);
    presetBox.onChange = [this] { proc.setCurrentProgram (presetBox.getSelectedId() - 1); };
    addAndMakeVisible (presetBox);

    setSize (760, 696);
    startTimerHz (30);
}

VCACompAudioProcessorEditor::~VCACompAudioProcessorEditor()
{
    stopTimer();
    presetBox.setLookAndFeel (nullptr);
    for (auto& k : knobs)   k.s.setLookAndFeel (nullptr);
    for (auto& t : toggles) t.b.setLookAndFeel (nullptr);
}

void VCACompAudioProcessorEditor::addKnob (int i, const juce::String& name, const juce::String& id, const juce::String& suffix)
{
    auto& k = knobs[(size_t) i];
    k.s.setLookAndFeel (&laf);
    k.s.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    k.s.setRotaryParameters (juce::MathConstants<float>::pi * 1.2f, juce::MathConstants<float>::pi * 2.8f, true);
    k.s.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 90, 18);
    k.s.setTextValueSuffix (suffix);
    k.s.setColour (juce::Slider::textBoxTextColourId, ice);
    k.s.setColour (juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    k.l.setText (name, juce::dontSendNotification);
    k.l.setJustificationType (juce::Justification::centred);
    k.l.setColour (juce::Label::textColourId, neonHi);
    k.l.setFont (juce::Font (juce::FontOptions (12.f, juce::Font::bold)));
    addAndMakeVisible (k.s);
    addAndMakeVisible (k.l);
    k.a = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (proc.apvts, id, k.s);
}

void VCACompAudioProcessorEditor::addToggle (int i, const juce::String& name, const juce::String& id)
{
    auto& t = toggles[(size_t) i];
    t.b.setLookAndFeel (&laf);
    t.b.setButtonText (name);
    addAndMakeVisible (t.b);
    t.a = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (proc.apvts, id, t.b);
}

void VCACompAudioProcessorEditor::timerCallback()
{
    meterDisp = juce::jmax (proc.grMeter.load(), meterDisp * 0.88f);
    repaint (meterArea.expanded (30, 30));
}

void VCACompAudioProcessorEditor::rebuildBackground()
{
    const int w = getWidth(), h = getHeight();
    if (w <= 0 || h <= 0) return;
    bg = juce::Image (juce::Image::ARGB, w, h, true);
    juce::Graphics g (bg);
    g.setGradientFill (juce::ColourGradient::vertical (juce::Colour (0xff060810), 0.f, juce::Colour (0xff0a1126), (float) h));
    g.fillAll();

    juce::Random rnd (1337);   // humo azul (fijo, siempre igual)
    for (int i = 0; i < 44; ++i)
    {
        const float cx = rnd.nextFloat() * (float) w, cy = rnd.nextFloat() * (float) h;
        const float rad = 70.f + rnd.nextFloat() * 190.f;
        const auto c = juce::Colour::fromHSV (0.61f + rnd.nextFloat() * 0.04f, 0.85f, 0.5f + 0.5f * rnd.nextFloat(),
                                              0.05f + 0.09f * rnd.nextFloat());
        g.setGradientFill (juce::ColourGradient (c, cx, cy, c.withAlpha (0.f), cx + rad, cy, true));
        g.fillEllipse (cx - rad, cy - rad, rad * 2.f, rad * 2.f);
    }
    // vineta
    g.setGradientFill (juce::ColourGradient (juce::Colours::transparentBlack, (float) w * 0.5f, (float) h * 0.5f,
                                             juce::Colour (0xe6000000), 0.f, 0.f, true));
    g.fillAll();
}

void VCACompAudioProcessorEditor::rebuildTitle()
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

void VCACompAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.drawImageAt (bg, 0, 0);
    const float W = (float) getWidth();

    // coronas: neon (arriba derecha) y plata (sobre el titulo)
    drawCrown (g, { W - 120.f, 10.f, 80.f, 52.f }, neonHi, neon, 3.f);
    drawCrown (g, { 62.f, 8.f, 58.f, 40.f }, ice, juce::Colour (0xff9fb6e8), 2.2f);

    // titulo estilo graffiti: brillo azul + contorno + relleno plateado
    g.setColour (neon.withAlpha (0.28f));
    g.strokePath (titlePath, juce::PathStrokeType (11.f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
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
    g.drawText ("V C A    C O M P R E S S O R", 0, 118, getWidth(), 20, juce::Justification::centred);
    g.setColour (ice.withAlpha (0.35f));
    g.drawLine (120.f, 129.f, W * 0.5f - 130.f, 129.f, 1.f);
    g.drawLine (W * 0.5f + 130.f, 129.f, W - 120.f, 129.f, 1.f);

    // etiqueta de presets
    g.setColour (neonHi);
    g.setFont (juce::Font (juce::FontOptions (12.f, juce::Font::bold)));
    g.drawText ("PRESET", (int) (W * 0.5f) - 180, 146, 70, 26, juce::Justification::centredRight);

    // panel
    g.setColour (juce::Colour (0x66000000));
    g.fillRoundedRectangle (20.f, 182.f, 632.f, 456.f, 12.f);
    g.setColour (neon.withAlpha (0.35f));
    g.drawRoundedRectangle (20.f, 182.f, 632.f, 456.f, 12.f, 1.2f);

    // divisor + titulos de seccion
    g.setColour (ice.withAlpha (0.18f));
    g.drawLine (40.f, 466.f, 632.f, 466.f, 1.f);
    g.setColour (neonHi);
    g.setFont (juce::Font (juce::FontOptions (12.f, juce::Font::bold)));
    g.drawText ("C H A R A C T E R   (808 / BASS)", 26, 468, 360, 16, juce::Justification::centred);
    g.drawText ("O P T I O N S", 386, 468, 240, 16, juce::Justification::centred);

    // medidor GR segmentado
    const int segs = 20;
    const float segH = (float) meterArea.getHeight() / (float) segs;
    const int lit = juce::roundToInt (juce::jlimit (0.f, 1.f, meterDisp / 24.f) * (float) segs);
    for (int i = 0; i < segs; ++i)
    {
        auto r = juce::Rectangle<float> ((float) meterArea.getX(), (float) meterArea.getY() + (float) i * segH + 1.f,
                                         (float) meterArea.getWidth(), segH - 2.f);
        const float t = (float) i / (float) segs;
        g.setColour (i < lit ? neon.interpolatedWith (juce::Colours::white, t) : juce::Colour (0xff141b2c));
        g.fillRoundedRectangle (r, 2.f);
    }
    g.setColour (neonHi);
    g.setFont (juce::Font (juce::FontOptions (12.f, juce::Font::bold)));
    g.drawText ("GAIN REDUCTION", meterArea.getX() - 30, meterArea.getY() - 24, meterArea.getWidth() + 60, 16, juce::Justification::centred);
    g.setColour (ice);
    g.drawText (juce::String (meterDisp > 0.05f ? -meterDisp : 0.f, 1) + " dB", meterArea.getX() - 30, meterArea.getBottom() + 6,
                meterArea.getWidth() + 60, 16, juce::Justification::centred);

    // pie
    g.setColour (ice.withAlpha (0.75f));
    g.setFont (juce::Font (juce::FontOptions (12.f, juce::Font::bold)));
    g.drawText (juce::String::fromUTF8 ("BEATS  \xe2\x80\xa2  MIX  \xe2\x80\xa2  MASTER  \xe2\x80\xa2  CREATIVE SOUNDS"),
                0, 650, getWidth(), 16, juce::Justification::centred);
    g.setColour (neonHi);
    g.setFont (juce::Font (juce::FontOptions (13.f, juce::Font::bold)));
    g.drawText ("@DioOnTheBeat", 0, 668, getWidth(), 18, juce::Justification::centred);
}

void VCACompAudioProcessorEditor::resized()
{
    rebuildBackground();
    rebuildTitle();

    presetBox.setBounds (getWidth() / 2 - 100, 146, 220, 26);

    const int colW = 120, rowH = 140, x0 = 26, y0 = 188;
    for (int i = 0; i < 13; ++i)
    {
        const int cx = x0 + (i % 5) * colW;
        const int cy = (i < 10) ? y0 + (i / 5) * rowH : y0 + 2 * rowH + 18;
        knobs[(size_t) i].l.setBounds (cx, cy, colW, 16);
        knobs[(size_t) i].s.setBounds (cx, cy + 16, colW, 124);
    }
    // botones 2x2 junto a los knobs de caracter
    for (int i = 0; i < 4; ++i)
        toggles[(size_t) i].b.setBounds (390 + (i % 2) * 124, 510 + (i / 2) * 50, 122, 40);

    meterArea = { 690, 214, 34, 300 };
}
