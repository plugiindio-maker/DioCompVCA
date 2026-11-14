#include "PluginEditor.h"
using namespace DioStyle;

// ================================================================ Look & Feel
DioLookAndFeel::DioLookAndFeel()
{
    setColour (juce::ComboBox::backgroundColourId, juce::Colour (0xff0a1224));
    setColour (juce::ComboBox::outlineColourId, electric.withAlpha (0.65f));
    setColour (juce::ComboBox::textColourId, ice);
    setColour (juce::ComboBox::arrowColourId, cyan);
    setColour (juce::PopupMenu::backgroundColourId, juce::Colour (0xff0a1224));
    setColour (juce::PopupMenu::textColourId, ice);
    setColour (juce::PopupMenu::headerTextColourId, cyan);
    setColour (juce::PopupMenu::highlightedBackgroundColourId, electric);
    setColour (juce::PopupMenu::highlightedTextColourId, juce::Colours::white);
    setColour (juce::TextButton::buttonColourId, juce::Colour (0xff0a1224));
    setColour (juce::TextButton::textColourOffId, juce::Colour (0xffa9c4e6));
    setColour (juce::TextButton::textColourOnId, juce::Colours::white);
}

juce::Font DioLookAndFeel::getLabelFont (juce::Label& l)
{
    if (dynamic_cast<juce::Slider*> (l.getParentComponent()) != nullptr)      // cajita de valor de las perillas
        return juce::Font (juce::FontOptions (21.f, juce::Font::bold));
    return l.getFont();
}

juce::Font DioLookAndFeel::getComboBoxFont (juce::ComboBox&)  { return juce::Font (juce::FontOptions (22.f, juce::Font::bold)); }
juce::Font DioLookAndFeel::getPopupMenuFont()                 { return juce::Font (juce::FontOptions (17.f)); }

juce::Font DioLookAndFeel::getTextButtonFont (juce::TextButton&, int buttonHeight)
{
    return juce::Font (juce::FontOptions (juce::jmin (22.f, (float) buttonHeight * 0.46f), juce::Font::bold));
}

void DioLookAndFeel::drawRotarySlider (juce::Graphics& g, int x, int y, int w, int h,
                                       float pos, float startAngle, float endAngle, juce::Slider& slider)
{
    juce::Colour acc = electric;
    if (slider.getProperties().contains ("accent"))
        acc = juce::Colour ((juce::uint32) (int) slider.getProperties()["accent"]);
    const juce::Colour accHi = acc.interpolatedWith (juce::Colours::white, 0.4f);

    auto b = juce::Rectangle<float> ((float) x, (float) y, (float) w, (float) h).reduced (juce::jmax (6.f, (float) juce::jmin (w, h) * 0.06f));
    const float r = juce::jmin (b.getWidth(), b.getHeight()) * 0.5f;
    const float u = juce::jmax (0.6f, r / 45.f);                  // todo escala con el tamano de la perilla
    const auto c = b.getCentre();
    const float angle = startAngle + pos * (endAngle - startAngle);
    const float arcR = r * 0.84f, capR = r * 0.64f;

    auto pt = [&] (float a, float rad) { return juce::Point<float> (c.x + std::sin (a) * rad, c.y - std::cos (a) * rad); };

    // marcas alrededor
    for (int i = 0; i <= 10; ++i)
    {
        const float a = startAngle + (float) i / 10.f * (endAngle - startAngle);
        g.setColour (i % 5 == 0 ? accHi.withAlpha (0.9f) : juce::Colour (0xff2a4670));
        g.drawLine (juce::Line<float> (pt (a, r * 0.98f), pt (a, r * 1.08f)), (i % 5 == 0 ? 2.f : 1.f) * u);
    }

    // pista y arco de valor con brillo neon moderado
    juce::Path track;
    track.addCentredArc (c.x, c.y, arcR, arcR, 0.f, startAngle, endAngle, true);
    g.setColour (juce::Colour (0xff0e1b33));
    g.strokePath (track, juce::PathStrokeType (4.f * u, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    if (pos > 0.001f)
    {
        juce::Path val;
        val.addCentredArc (c.x, c.y, arcR, arcR, 0.f, startAngle, angle, true);
        g.setColour (acc.withAlpha (0.18f));
        g.strokePath (val, juce::PathStrokeType (11.f * u, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        g.setColour (acc.withAlpha (0.45f));
        g.strokePath (val, juce::PathStrokeType (6.f * u, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        g.setColour (accHi);
        g.strokePath (val, juce::PathStrokeType (3.f * u, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    }

    // sombra + tapa de metal oscuro cepillado
    g.setColour (juce::Colour (0x99000000));
    g.fillEllipse (c.x - capR - 1.f, c.y - capR + 4.f * u, (capR + 1.f) * 2.f, (capR + 1.f) * 2.f);
    g.setGradientFill (juce::ColourGradient (juce::Colour (0xff6a7488), c.x - capR * 0.35f, c.y - capR * 0.45f,
                                             juce::Colour (0xff0b0e16), c.x + capR * 0.9f, c.y + capR * 0.9f, true));
    g.fillEllipse (c.x - capR, c.y - capR, capR * 2.f, capR * 2.f);

    // cepillado radial
    g.setColour (juce::Colours::white.withAlpha (0.05f));
    for (int i = 0; i < 40; ++i)
    {
        const float a = (float) i * juce::MathConstants<float>::twoPi / 40.f;
        g.drawLine (juce::Line<float> (pt (a, capR * 0.10f), pt (a, capR * 0.97f)), 1.f * u);
    }

    // borde con luz azul
    g.setColour (accHi.withAlpha (0.45f));
    g.drawEllipse (c.x - capR, c.y - capR, capR * 2.f, capR * 2.f, 1.6f * u);
    g.setColour (juce::Colour (0xff02040a));
    g.drawEllipse (c.x - capR * 0.97f, c.y - capR * 0.97f, capR * 1.94f, capR * 1.94f, 1.f * u);

    // puntero blanco
    const auto p1 = pt (angle, capR * 0.30f), p2 = pt (angle, capR * 0.92f);
    g.setColour (juce::Colours::white.withAlpha (0.22f));
    g.drawLine (juce::Line<float> (p1, p2), 6.f * u);
    g.setColour (juce::Colours::white);
    g.drawLine (juce::Line<float> (p1, p2), 3.f * u);
}

void DioLookAndFeel::drawToggleButton (juce::Graphics& g, juce::ToggleButton& bt, bool highlighted, bool)
{
    const bool on = bt.getToggleState();

    if (bt.getProperties().contains ("power"))          // boton de encendido redondo
    {
        auto r = bt.getLocalBounds().toFloat().reduced (5.f);
        const auto c = r.getCentre();
        const float rad = juce::jmin (r.getWidth(), r.getHeight()) * 0.5f;
        g.setGradientFill (juce::ColourGradient (juce::Colour (0xff17284a), c.x, r.getY(), juce::Colour (0xff060b16), c.x, r.getBottom(), false));
        g.fillEllipse (r);
        if (on) { g.setColour (electric.withAlpha (0.28f)); g.drawEllipse (r.expanded (3.f), 6.f); }
        g.setColour (on ? cyan : juce::Colour (0xff2a4670));
        g.drawEllipse (r, 2.5f);
        juce::Path p;
        p.addCentredArc (c.x, c.y, rad * 0.5f, rad * 0.5f, 0.f, 0.6f, juce::MathConstants<float>::twoPi - 0.6f, true);
        p.startNewSubPath (c.x, c.y - rad * 0.62f);
        p.lineTo (c.x, c.y - rad * 0.08f);
        if (on) { g.setColour (cyan.withAlpha (0.35f)); g.strokePath (p, juce::PathStrokeType (8.f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded)); }
        g.setColour (on ? juce::Colours::white : juce::Colour (0xff6b86ad));
        g.strokePath (p, juce::PathStrokeType (4.f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        return;
    }

    auto r = bt.getLocalBounds().toFloat().reduced (3.f);
    const float rad = juce::jmin (12.f, r.getHeight() * 0.22f);
    if (on)
    {
        g.setColour (electric.withAlpha (0.28f));
        g.fillRoundedRectangle (r.expanded (2.f), rad + 2.f);
        g.setGradientFill (juce::ColourGradient (cyan.interpolatedWith (electric, 0.45f), r.getX(), r.getY(),
                                                 electric.darker (0.5f), r.getX(), r.getBottom(), false));
    }
    else
        g.setColour (juce::Colour (highlighted ? 0xff14233f : 0xff0b1426));
    g.fillRoundedRectangle (r, rad);
    g.setColour (on ? cyan : juce::Colour (0xff22395e));
    g.drawRoundedRectangle (r, rad, 1.6f);
    g.setColour (on ? juce::Colours::white : juce::Colour (0xffa9c4e6));
    g.setFont (juce::Font (juce::FontOptions (juce::jmin (21.f, r.getHeight() * 0.5f), juce::Font::bold)));
    g.drawText (bt.getButtonText(), bt.getLocalBounds(), juce::Justification::centred);
}

void DioLookAndFeel::drawButtonBackground (juce::Graphics& g, juce::Button& b, const juce::Colour&, bool highlighted, bool down)
{
    auto r = b.getLocalBounds().toFloat().reduced (2.f);
    const float rad = juce::jmin (12.f, r.getHeight() * 0.22f);
    const bool on = b.getToggleState();
    if (on)
    {
        g.setColour (electric.withAlpha (0.28f));
        g.fillRoundedRectangle (r.expanded (2.f), rad + 2.f);
        g.setGradientFill (juce::ColourGradient (electric, r.getX(), r.getY(), electric.darker (0.6f), r.getX(), r.getBottom(), false));
    }
    else
        g.setColour (juce::Colour (down ? 0xff1a2e52 : highlighted ? 0xff13223d : 0xff0a1224));
    g.fillRoundedRectangle (r, rad);
    g.setColour (on ? cyan : juce::Colour (0xff22395e));
    g.drawRoundedRectangle (r, rad, 1.6f);
}

// ================================================================ helpers
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
        g.setColour (glow.withAlpha ((k == 4.f ? 0.10f : k == 2.5f ? 0.22f : 0.45f) * intensity));
        g.strokePath (p, juce::PathStrokeType (thick * k, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    }
    g.setColour (core);
    g.strokePath (p, juce::PathStrokeType (thick, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
}

static void drawScrew (juce::Graphics& g, float cx, float cy, float r)
{
    g.setGradientFill (juce::ColourGradient (juce::Colour (0xffd8e2f2), cx - r * 0.5f, cy - r * 0.6f,
                                             juce::Colour (0xff2d3648), cx + r * 0.7f, cy + r * 0.8f, false));
    g.fillEllipse (cx - r, cy - r, r * 2.f, r * 2.f);
    g.setColour (juce::Colour (0xff070a10));
    g.drawEllipse (cx - r, cy - r, r * 2.f, r * 2.f, 1.2f);
    g.drawLine (cx - r * 0.6f, cy + r * 0.3f, cx + r * 0.6f, cy - r * 0.3f, 1.8f);
}

static void drawLedBar (juce::Graphics& g, juce::Rectangle<float> r, float db, const juce::String& label)
{
    g.setColour (cyan);
    g.setFont (juce::Font (juce::FontOptions (19.f, juce::Font::bold)));
    g.drawText (label, r.withHeight (22.f).translated (0.f, -26.f), juce::Justification::centredLeft);
    g.setColour (ice);
    g.drawText (db <= -79.f ? juce::String ("-inf dB") : juce::String (db, 1) + " dB",
                r.withHeight (22.f).translated (0.f, -26.f), juce::Justification::centredRight);

    g.setColour (juce::Colour (0xff050a14));
    g.fillRoundedRectangle (r.expanded (4.f), 6.f);
    const int segs = 40;
    const float sw = r.getWidth() / (float) segs;
    const int lit = juce::roundToInt (juce::jlimit (0.f, 1.f, (db + 48.f) / 48.f) * (float) segs);
    for (int i = 0; i < segs; ++i)
    {
        const float t = (float) i / (float) segs;
        const juce::Colour on = t < 0.72f ? cyan : t < 0.92f ? ice : juce::Colour (0xffff6b7d);
        g.setColour (i < lit ? on : juce::Colour (0xff0e1a30));
        g.fillRoundedRectangle (r.getX() + (float) i * sw + 1.5f, r.getY(), sw - 3.f, r.getHeight(), 2.f);
    }
}

// fondo por topologia: negro profundo + humo azul + textura propia de cada una
static juce::Image makeBackground (int topo)
{
    const int w = DioPanel::kW, h = DioPanel::kH;
    juce::Image img (juce::Image::ARGB, w, h, true);
    juce::Graphics g (img);
    g.setGradientFill (juce::ColourGradient::vertical (juce::Colour (0xff020409), 0.f, juce::Colour (0xff060d1c), (float) h));
    g.fillAll();

    juce::Random rnd (4242 + topo);
    for (int i = 0; i < 40; ++i)
    {
        const float cx = rnd.nextFloat() * (float) w, cy = rnd.nextFloat() * (float) h;
        const float rad = 120.f + rnd.nextFloat() * 280.f;
        const auto c = juce::Colour::fromHSV (0.57f + rnd.nextFloat() * 0.05f, 0.85f,
                                              0.45f + 0.5f * rnd.nextFloat(), 0.04f + 0.07f * rnd.nextFloat());
        g.setGradientFill (juce::ColourGradient (c, cx, cy, c.withAlpha (0.f), cx + rad, cy, true));
        g.fillEllipse (cx - rad, cy - rad, rad * 2.f, rad * 2.f);
    }
    // textura del panel (distinta en cada topologia)
    if (topo == 0)        // VCA: metal cepillado horizontal
    {
        for (int y = 164; y < 938; y += 3)
        {
            g.setColour (juce::Colours::white.withAlpha (0.008f + 0.016f * rnd.nextFloat()));
            g.drawHorizontalLine (y, 28.f, 1372.f);
        }
    }
    else if (topo == 1)   // OPTICO: trama vertical fina
    {
        for (int x = 30; x < 1372; x += 6)
        {
            g.setColour (juce::Colours::white.withAlpha (0.006f + 0.012f * rnd.nextFloat()));
            g.drawVerticalLine (x, 164.f, 938.f);
        }
    }
    else                  // VARI-MU: rejilla de puntos
    {
        g.setColour (juce::Colours::white.withAlpha (0.035f));
        for (int y = 170; y < 936; y += 18)
            for (int x = 34; x < 1372; x += 18)
                g.fillEllipse ((float) x, (float) y, 2.f, 2.f);
    }
    g.setGradientFill (juce::ColourGradient (juce::Colours::transparentBlack, (float) w * 0.5f, (float) h * 0.5f,
                                             juce::Colour (0xe6000000), 0.f, 0.f, true));
    g.fillAll();
    return img;
}

// ================================================================ cuadro de dialogo dentro del plugin
class PromptOverlay : public juce::Component
{
public:
    PromptOverlay (juce::LookAndFeel& lf, const juce::String& titleText, const juce::String& messageText,
                   const juce::String& defaultText, bool withText, const juce::String& okText,
                   std::function<void (bool, juce::String)> finish)
        : laf (lf), title (titleText), message (messageText), hasText (withText), onFinish (std::move (finish))
    {
        okBtn.setLookAndFeel (&laf);
        cancelBtn.setLookAndFeel (&laf);
        okBtn.setButtonText (okText);
        cancelBtn.setButtonText ("CANCEL");
        okBtn.onClick = [this] { done (true); };
        cancelBtn.onClick = [this] { done (false); };
        addAndMakeVisible (okBtn);
        addAndMakeVisible (cancelBtn);
        if (hasText)
        {
            editor.setText (defaultText, false);
            editor.setFont (juce::Font (juce::FontOptions (30.f, juce::Font::bold)));
            editor.setColour (juce::TextEditor::backgroundColourId, juce::Colour (0xff050a14));
            editor.setColour (juce::TextEditor::textColourId, ice);
            editor.setColour (juce::TextEditor::outlineColourId, electric.withAlpha (0.6f));
            editor.setColour (juce::TextEditor::focusedOutlineColourId, cyan);
            editor.setColour (juce::TextEditor::highlightColourId, electric);
            editor.setInputRestrictions (40);
            editor.onReturnKey = [this] { done (true); };
            editor.onEscapeKey = [this] { done (false); };
            addAndMakeVisible (editor);
        }
    }

    ~PromptOverlay() override
    {
        okBtn.setLookAndFeel (nullptr);
        cancelBtn.setLookAndFeel (nullptr);
    }

    void focusEditor()
    {
        if (hasText) { editor.grabKeyboardFocus(); editor.selectAll(); }
    }

    void paint (juce::Graphics& g) override
    {
        g.fillAll (juce::Colour (0xd0020409));
        const auto card = cardBounds().toFloat();
        g.setGradientFill (juce::ColourGradient (juce::Colour (0xff10203c), card.getX(), card.getY(),
                                                 juce::Colour (0xff050a14), card.getX(), card.getBottom(), false));
        g.fillRoundedRectangle (card, 18.f);
        g.setColour (electric.withAlpha (0.8f));
        g.drawRoundedRectangle (card, 18.f, 2.f);
        g.setColour (cyan);
        g.setFont (juce::Font (juce::FontOptions (36.f, juce::Font::bold)));
        g.drawText (title, cardBounds().withTrimmedTop (28).withHeight (50), juce::Justification::centred);
        g.setColour (ice);
        g.setFont (juce::Font (juce::FontOptions (24.f)));
        g.drawFittedText (message, cardBounds().reduced (50, 0).withTrimmedTop (86).withHeight (hasText ? 50 : 120),
                          juce::Justification::centred, 3);
    }

    void resized() override
    {
        const auto c = cardBounds();
        if (hasText) editor.setBounds (c.getX() + 60, c.getY() + 150, c.getWidth() - 120, 66);
        okBtn.setBounds (c.getCentreX() - 200, c.getBottom() - 110, 180, 64);
        cancelBtn.setBounds (c.getCentreX() + 20, c.getBottom() - 110, 180, 64);
    }

    void mouseDown (const juce::MouseEvent&) override {}      // bloquea los controles de abajo

private:
    juce::Rectangle<int> cardBounds() const
    {
        return juce::Rectangle<int> (DioPanel::kW / 2 - 400, DioPanel::kH / 2 - 200, 800, 400);
    }

    void done (bool ok)
    {
        if (finished) return;
        finished = true;
        if (onFinish) onFinish (ok, hasText ? editor.getText() : juce::String());
    }

    juce::LookAndFeel& laf;
    juce::String title, message;
    bool hasText = false, finished = false;
    std::function<void (bool, juce::String)> onFinish;
    juce::TextEditor editor;
    juce::TextButton okBtn, cancelBtn;
};

// ================================================================ Panel
DioPanel::DioPanel (DioCompressProcessor& p)
    : proc (p)
{
    for (auto& s : mainSlots)   initSlot (s);
    for (auto& s : sharedSlots) initSlot (s);

    // secciones globales (independientes de la topologia)
    configureSlot (sharedSlots[0], "drive",  "DRIVE",   " %",  ice);
    configureSlot (sharedSlots[1], "color",  "COLOR",   " %",  ice);
    configureSlot (sharedSlots[2], "amix",   "MIX",     " %",  ice);
    configureSlot (sharedSlots[3], "bpamt",  "AMOUNT",  " %",  cyan);
    configureSlot (sharedSlots[4], "ceil",   "CEILING", " dB", steel);

    const char* topoNames[3] = { "VCA", "OPTICO", "VARI-MU" };
    for (int i = 0; i < 3; ++i)
    {
        auto& tb = topoBtn[(size_t) i];
        tb.setLookAndFeel (&laf);
        tb.setButtonText (topoNames[i]);
        tb.setRadioGroupId (101);
        tb.setClickingTogglesState (true);
        tb.onClick = [this, i] { setChoice ("topology", i); };
        addAndMakeVisible (tb);

        auto& mb = modeBtn[(size_t) i];
        mb.setLookAndFeel (&laf);
        mb.setRadioGroupId (102);
        mb.setClickingTogglesState (true);
        mb.onClick = [this, i] { setChoice (uiTopo == 0 ? "vmode" : "olimit", i); };
        addAndMakeVisible (mb);
    }

    powerBtn.getProperties().set ("power", true);
    powerBtn.setLookAndFeel (&laf);
    powerBtn.onClick = [this] { setBool ("bypass", ! powerBtn.getToggleState()); };
    addAndMakeVisible (powerBtn);

    autoBtn.setLookAndFeel (&laf);
    autoBtn.setButtonText ("AUTO");
    addAndMakeVisible (autoBtn);

    analogBtn.setLookAndFeel (&laf);
    analogBtn.setButtonText ("ANALOG ON");
    addAndMakeVisible (analogBtn);
    analogAtt = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (proc.apvts, "analogon", analogBtn);

    bpBtn.setLookAndFeel (&laf);
    bpBtn.setButtonText ("PRESERVE ON");
    addAndMakeVisible (bpBtn);
    bpAtt = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (proc.apvts, "bpon", bpBtn);

    safetyBox.setLookAndFeel (&laf);
    safetyBox.addItemList (juce::StringArray { "OFF", "LIMIT", "CLIP" }, 1);
    addAndMakeVisible (safetyBox);
    safetyAtt = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (proc.apvts, "safety", safetyBox);

    // ---- gestor de presets
    presetBox.setLookAndFeel (&laf);
    presetBox.onChange = [this]
    {
        const int id = presetBox.getSelectedId();
        if (id >= 1000 && id < 99999)
        {
            const int idx = id - 1000;
            if (idx < userNames.size()) proc.loadUserPreset (userNames[idx]);
        }
        else if (id >= 1 && id < 1000)
            proc.loadFactoryPreset (id - 1);
        profileBox.setSelectedId (0, juce::dontSendNotification);
        lastPresetName = proc.getCurrentPresetName();
        lastPresetUser = proc.isCurrentPresetUser();
    };
    addAndMakeVisible (presetBox);

    auto step = [this] (int dir)
    {
        if (presetIds.empty()) return;
        const int cur = presetBox.getSelectedId();
        int idx = 0;
        for (size_t k = 0; k < presetIds.size(); ++k) if (presetIds[k] == cur) idx = (int) k;
        const int n = (int) presetIds.size();
        presetBox.setSelectedId (presetIds[(size_t) ((idx + dir + n) % n)]);
    };
    prevBtn.setLookAndFeel (&laf);  prevBtn.setButtonText ("<");
    nextBtn.setLookAndFeel (&laf);  nextBtn.setButtonText (">");
    prevBtn.onClick = [step] { step (-1); };
    nextBtn.onClick = [step] { step (1); };
    addAndMakeVisible (prevBtn);
    addAndMakeVisible (nextBtn);

    saveBtn.setLookAndFeel (&laf);    saveBtn.setButtonText ("SAVE");
    manageBtn.setLookAndFeel (&laf);  manageBtn.setButtonText ("MANAGE");
    saveBtn.onClick = [this] { showSaveDialog(); };
    manageBtn.onClick = [this] { showManageMenu(); };
    addAndMakeVisible (saveBtn);
    addAndMakeVisible (manageBtn);

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
    refreshPresetList();
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
    autoBtn.setLookAndFeel (nullptr);
    analogBtn.setLookAndFeel (nullptr);
    bpBtn.setLookAndFeel (nullptr);
    safetyBox.setLookAndFeel (nullptr);
    presetBox.setLookAndFeel (nullptr);
    profileBox.setLookAndFeel (nullptr);
    prevBtn.setLookAndFeel (nullptr);
    nextBtn.setLookAndFeel (nullptr);
    saveBtn.setLookAndFeel (nullptr);
    manageBtn.setLookAndFeel (nullptr);
    settingsBtn.setLookAndFeel (nullptr);
    for (auto& s : mainSlots)   s.s.setLookAndFeel (nullptr);
    for (auto& s : sharedSlots) s.s.setLookAndFeel (nullptr);
}

juce::Colour DioPanel::accent() const
{
    return uiTopo == 0 ? electric : uiTopo == 1 ? juce::Colour (0xff4fd2ff) : juce::Colour (0xff6b8cff);
}

float DioPanel::meterMax() const { return uiTopo == 0 ? 20.f : uiTopo == 1 ? 12.f : 16.f; }

void DioPanel::initSlot (Slot& sl)
{
    sl.s.setLookAndFeel (&laf);
    sl.s.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    sl.s.setRotaryParameters (juce::MathConstants<float>::pi * 1.2f, juce::MathConstants<float>::pi * 2.8f, true);
    sl.s.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 170, 30);
    sl.s.setColour (juce::Slider::textBoxTextColourId, ice);
    sl.s.setColour (juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    sl.l.setJustificationType (juce::Justification::centred);
    sl.l.setFont (juce::Font (juce::FontOptions (23.f, juce::Font::bold)));
    addChildComponent (sl.s);          // ocultos hasta que se configuren
    addChildComponent (sl.l);
}

void DioPanel::configureSlot (Slot& sl, const char* id, const juce::String& name, const juce::String& suffix, juce::Colour acc)
{
    sl.a.reset();                                           // desconecta del parametro anterior
    sl.s.getProperties().set ("accent", (int) acc.getARGB());
    sl.s.setTextValueSuffix (suffix);
    sl.l.setText (name, juce::dontSendNotification);
    sl.l.setColour (juce::Label::textColourId, acc.interpolatedWith (juce::Colours::white, 0.5f));
    sl.a = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (proc.apvts, id, sl.s);
    sl.s.setVisible (true);
    sl.l.setVisible (true);
    sl.s.repaint();
}

void DioPanel::hideSlot (Slot& sl)
{
    sl.a.reset();
    sl.s.setVisible (false);
    sl.l.setVisible (false);
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

// cada topologia tiene SUS controles, etiquetas, unidades y rangos (los valores son independientes)
void DioPanel::applyTopologyUi()
{
    const auto acc = accent();
    juce::String autoId;
    const char* modeNames3[3] = { "SMOOTH", "PUNCH", "AGGRESSIVE" };
    const char* modeNames2[3] = { "COMPRESS", "LIMIT", "" };
    const char** modeNames = modeNames3;

    if (uiTopo == 0)            // VCA: precision y punch
    {
        configureSlot (mainSlots[0], "vthr",     "THRESHOLD", " dB", acc);
        configureSlot (mainSlots[1], "vratio",   "RATIO",     ":1",  acc);
        configureSlot (mainSlots[2], "vattack",  "ATTACK",    " ms", acc);
        configureSlot (mainSlots[3], "vrelease", "RELEASE",   " ms", acc);
        configureSlot (mainSlots[4], "vknee",    "KNEE",      " dB", acc);
        configureSlot (mainSlots[5], "vhpf",     "SC HPF",    " Hz", acc);
        configureSlot (mainSlots[6], "vmakeup",  "MAKEUP",    " dB", acc);
        configureSlot (mainSlots[7], "vmix",     "MIX",       " %",  acc);
        autoOwner = 6;  autoId = "vauto";  modeCount = 3;
    }
    else if (uiTopo == 1)       // OPTICO: musicalidad y densidad (sin attack/release convencionales)
    {
        configureSlot (mainSlots[0], "opeak", "PEAK REDUCTION", "",     acc);
        configureSlot (mainSlots[1], "ogain", "GAIN",           " dB",  acc);
        configureSlot (mainSlots[2], "oemph", "EMPHASIS",       " %",   acc);
        configureSlot (mainSlots[3], "omix",  "MIX",            " %",   acc);
        for (int i = 4; i < 8; ++i) hideSlot (mainSlots[(size_t) i]);
        autoOwner = 1;  autoId = "oauto";  modeCount = 2;  modeNames = modeNames2;
    }
    else                        // VARI-MU: compresion progresiva
    {
        configureSlot (mainSlots[0], "minput",  "INPUT",     " dB", acc);
        configureSlot (mainSlots[1], "moutput", "OUTPUT",    " dB", acc);
        configureSlot (mainSlots[2], "mthr",    "THRESHOLD", " dB", acc);
        configureSlot (mainSlots[3], "mdens",   "DENSITY",   "",    acc);
        configureSlot (mainSlots[4], "mbias",   "BIAS",      " %",  acc);
        configureSlot (mainSlots[5], "mrecov",  "RECOVERY",  "",    acc);
        configureSlot (mainSlots[6], "mmix",    "MIX",       " %",  acc);
        hideSlot (mainSlots[7]);
        autoOwner = 1;  autoId = "mauto";  modeCount = 0;
    }

    autoAtt.reset();
    autoAtt = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (proc.apvts, autoId, autoBtn);

    for (int i = 0; i < 3; ++i)
    {
        modeBtn[(size_t) i].setButtonText (modeNames[i]);
        modeBtn[(size_t) i].setVisible (i < modeCount);
    }
    layoutControls();
    repaint();
}

void DioPanel::layoutControls()
{
    // perillas grandes
    auto placeBig = [] (Slot& s, int x)
    {
        s.l.setBounds (x, 176, 300, 28);
        s.s.setBounds (x, 204, 300, 296);
    };
    placeBig (mainSlots[0], 60);
    placeBig (mainSlots[1], 1040);

    // fila inferior: solo las que existen en la topologia, centradas
    std::vector<size_t> vis;
    for (size_t i = 2; i < 8; ++i) if (mainSlots[i].s.isVisible()) vis.push_back (i);
    const int cnt = (int) vis.size(), cell = 210, w = 200;
    const int startX = (kW - (cnt * cell - 10)) / 2;
    for (int k = 0; k < cnt; ++k)
    {
        auto& s = mainSlots[vis[(size_t) k]];
        const int x = startX + k * cell;
        s.l.setBounds (x, 512, w, 26);
        s.s.setBounds (x, 538, w, 150);
    }

    // boton AUTO junto a la etiqueta del makeup / gain / output
    if (autoOwner == 6)
    {
        auto& s = mainSlots[6];
        const auto lb = s.l.getBounds();
        s.l.setBounds (lb.getX(), lb.getY(), 118, lb.getHeight());
        autoBtn.setBounds (lb.getX() + 122, lb.getY() - 3, 78, 32);
    }
    else
    {
        mainSlots[1].l.setBounds (1040, 176, 206, 28);
        autoBtn.setBounds (1252, 174, 88, 32);
    }

    // zona global inferior
    const int xs[3] = { 64, 240, 416 };
    for (int i = 0; i < 3; ++i)
    {
        sharedSlots[(size_t) i].l.setBounds (xs[i], 740, 170, 26);
        sharedSlots[(size_t) i].s.setBounds (xs[i], 766, 170, 160);
    }
    sharedSlots[3].l.setBounds (784, 740, 170, 26);   sharedSlots[3].s.setBounds (784, 766, 170, 160);
    sharedSlots[4].l.setBounds (1178, 740, 170, 26);  sharedSlots[4].s.setBounds (1178, 766, 170, 160);
    analogBtn.setBounds (602, 756, 132, 58);
    bpBtn.setBounds (968, 756, 152, 58);
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
    const juce::Rectangle<float> area (96.f, 28.f, 262.f, 46.f);
    const float s = juce::jmin (area.getWidth() / b.getWidth(), area.getHeight() / b.getHeight());
    const float ty = area.getCentreY() - b.getHeight() * s * 0.5f;
    titlePath.applyTransform (juce::AffineTransform::translation (-b.getX(), -b.getY()).scaled (s).translated (area.getX(), ty));
}

juce::Image& DioPanel::getBackground (int topo)
{
    auto& img = bgs[(size_t) juce::jlimit (0, 2, topo)];
    if (img.isNull()) img = makeBackground (juce::jlimit (0, 2, topo));
    return img;
}

// ---------------------------------------------------------------- presets
void DioPanel::refreshPresetList()
{
    presetBox.clear (juce::dontSendNotification);
    presetIds.clear();
    presetBox.addSectionHeading ("FACTORY");
    for (int i = 0; i < proc.getNumPrograms(); ++i)
    {
        presetBox.addItem (proc.getProgramName (i), i + 1);
        presetIds.push_back (i + 1);
    }
    userNames = proc.getUserPresetNames();
    presetBox.addSeparator();
    presetBox.addSectionHeading ("USER");
    if (userNames.isEmpty())
    {
        presetBox.addItem ("(no user presets yet - use SAVE)", 99999);
        presetBox.setItemEnabled (99999, false);
    }
    for (int i = 0; i < userNames.size(); ++i)
    {
        presetBox.addItem (userNames[i], 1000 + i);
        presetIds.push_back (1000 + i);
    }
    selectCurrentPreset();
}

void DioPanel::selectCurrentPreset()
{
    int id = proc.getCurrentProgram() + 1;
    if (proc.isCurrentPresetUser())
    {
        const int idx = userNames.indexOf (proc.getCurrentPresetName());
        if (idx >= 0) id = 1000 + idx;
    }
    presetBox.setSelectedId (id, juce::dontSendNotification);
    lastPresetName = proc.getCurrentPresetName();
    lastPresetUser = proc.isCurrentPresetUser();
}

void DioPanel::showPrompt (const juce::String& title, const juce::String& message, const juce::String& defaultText,
                           bool withText, const juce::String& okText, std::function<void (juce::String)> onOk)
{
    pendingAction = std::move (onOk);
    juce::Component::SafePointer<DioPanel> safe (this);
    overlay = std::make_unique<PromptOverlay> (laf, title, message, defaultText, withText, okText,
        [safe] (bool accepted, juce::String text)
        {
            juce::MessageManager::callAsync ([safe, accepted, text]
            {
                if (safe == nullptr) return;
                auto fn = std::move (safe->pendingAction);
                safe->pendingAction = nullptr;
                safe->overlay.reset();
                if (accepted && fn) fn (text);
            });
        });
    addAndMakeVisible (*overlay);
    overlay->setBounds (getLocalBounds());
    overlay->toFront (true);
    overlay->focusEditor();
}

void DioPanel::showSaveDialog()
{
    const juce::String def = proc.isCurrentPresetUser() ? proc.getCurrentPresetName() : juce::String ("My Preset");
    juce::Component::SafePointer<DioPanel> safe (this);
    showPrompt ("SAVE PRESET", "Name for your preset (all controls of every topology, Analog Color and Bass Preserve are saved):",
                def, true, "SAVE",
                [safe] (juce::String raw)
                {
                    if (safe == nullptr) return;
                    const auto name = juce::File::createLegalFileName (raw.trim()).substring (0, 40).trim();
                    if (name.isEmpty()) return;
                    auto doSave = [safe, name] (bool overwrite)
                    {
                        if (safe == nullptr) return;
                        if (safe->proc.saveUserPreset (name, overwrite))
                            safe->refreshPresetList();
                        else
                            safe->showPrompt ("COULD NOT SAVE", "The preset could not be written to:\n"
                                              + DioCompressProcessor::getUserPresetDir().getFullPathName(),
                                              "", false, "OK", [] (juce::String) {});
                    };
                    if (safe->proc.userPresetExists (name))
                        safe->showPrompt ("OVERWRITE PRESET", "A preset named \"" + name + "\" already exists. Overwrite it?",
                                          "", false, "OVERWRITE", [doSave] (juce::String) { doSave (true); });
                    else
                        doSave (false);
                });
}

void DioPanel::showManageMenu()
{
    const bool user = proc.isCurrentPresetUser();
    const juce::String name = proc.getCurrentPresetName();
    juce::PopupMenu m;
    m.setLookAndFeel (&laf);
    m.addSectionHeader ("MANAGE PRESETS");
    m.addItem (1, "Rename \"" + name + "\"...", user);
    m.addItem (2, "Delete \"" + name + "\"...", user);
    m.addSeparator();
    m.addItem (3, "Open user preset folder");
    if (! user) m.addItem (4, "(select a USER preset to rename or delete it)", false);

    juce::Component::SafePointer<DioPanel> safe (this);
    m.showMenuAsync (juce::PopupMenu::Options().withTargetComponent (manageBtn),
        [safe, name] (int result)
        {
            if (safe == nullptr || result <= 0) return;
            if (result == 3)
            {
                auto dir = DioCompressProcessor::getUserPresetDir();
                dir.createDirectory();
                dir.revealToUser();
            }
            else if (result == 1)
            {
                safe->showPrompt ("RENAME PRESET", "New name for \"" + name + "\":", name, true, "RENAME",
                                  [safe, name] (juce::String raw)
                                  {
                                      if (safe == nullptr) return;
                                      if (safe->proc.renameUserPreset (name, raw))
                                          safe->refreshPresetList();
                                      else
                                          safe->showPrompt ("COULD NOT RENAME", "Invalid name, or a preset with that name already exists.",
                                                            "", false, "OK", [] (juce::String) {});
                                  });
            }
            else if (result == 2)
            {
                safe->showPrompt ("DELETE PRESET", "Delete \"" + name + "\"? This cannot be undone.", "", false, "DELETE",
                                  [safe, name] (juce::String)
                                  {
                                      if (safe == nullptr) return;
                                      safe->proc.deleteUserPreset (name);
                                      safe->refreshPresetList();
                                  });
            }
        });
}

void DioPanel::showSettingsMenu()
{
    juce::PopupMenu m;
    m.setLookAndFeel (&laf);
    m.addSectionHeader ("WINDOW SIZE (always opens at 60%)");
    const int cur = getSizePercent ? getSizePercent() : 60;
    for (int pct : { 40, 50, 60, 70, 80, 90, 100, 125 })
        m.addItem (pct, juce::String (pct) + "%" + (pct == 60 ? "  (default)" : ""), true, std::abs (cur - pct) < 3);

    juce::Component::SafePointer<DioPanel> safe (this);
    m.showMenuAsync (juce::PopupMenu::Options().withTargetComponent (settingsBtn),
                     [safe] (int result)
                     {
                         if (safe != nullptr && result > 0 && safe->onSizePercent)
                             safe->onSizePercent (result);
                     });
}

// ---------------------------------------------------------------- tiempo real de la interfaz
void DioPanel::timerCallback()
{
    grDisp  = juce::jmax (proc.grMeter.load(), grDisp * 0.88f);
    inDisp  = juce::jmax (proc.inMeter.load(),  inDisp - 1.2f);
    outDisp = juce::jmax (proc.outMeter.load(), outDisp - 1.2f);

    const float posT = 1.f - std::sqrt (juce::jlimit (0.f, 1.f, grDisp / meterMax()));
    needlePos += (posT - needlePos) * 0.30f;

    phase += 0.10f;
    if (phase > juce::MathConstants<float>::twoPi) phase -= juce::MathConstants<float>::twoPi;

    // sincroniza la interfaz con los parametros (presets, automatizacion, cambios del DAW)
    const int topo = getChoice ("topology");
    if (topo != uiTopo) { uiTopo = topo; applyTopologyUi(); }
    for (int i = 0; i < 3; ++i)
    {
        topoBtn[(size_t) i].setToggleState (i == topo, juce::dontSendNotification);
        const int modeNow = (uiTopo == 0) ? getChoice ("vmode") : getChoice ("olimit");
        modeBtn[(size_t) i].setToggleState (i == modeNow, juce::dontSendNotification);
    }
    powerBtn.setToggleState (getChoice ("bypass") == 0, juce::dontSendNotification);

    if (proc.getCurrentPresetName() != lastPresetName || proc.isCurrentPresetUser() != lastPresetUser)
        selectCurrentPreset();

    const bool an = analogBtn.getToggleState(), bp = bpBtn.getToggleState();
    if (an != analogUiOn)
    {
        analogUiOn = an;
        for (size_t i = 0; i < 3; ++i)
        {
            sharedSlots[i].s.setAlpha (an ? 1.f : 0.4f);
            sharedSlots[i].l.setAlpha (an ? 1.f : 0.4f);
        }
        repaint (48, 704, 700, 226);
    }
    if (bp != bpUiOn)
    {
        bpUiOn = bp;
        sharedSlots[3].s.setAlpha (bp ? 1.f : 0.4f);
        sharedSlots[3].l.setAlpha (bp ? 1.f : 0.4f);
        repaint (768, 704, 380, 226);
    }

    repaint (meterArea.expanded (14));
    repaint (400, 462, 600, 64);                                  // medidores de entrada / salida
    if ((++tick & 1) == 0) repaint (0, 0, kW, 110);               // logo y power palpitan
}

// ---------------------------------------------------------------- medidor de aguja
void DioPanel::drawMeter (juce::Graphics& g, juce::Rectangle<float> r)
{
    const auto acc = accent();
    const float gmax = meterMax();

    g.setGradientFill (juce::ColourGradient (juce::Colour (0xff222b3c), r.getX(), r.getY(), juce::Colour (0xff070b14), r.getX(), r.getBottom(), false));
    g.fillRoundedRectangle (r, 16.f);
    g.setColour (juce::Colour (0xff03060c));
    g.drawRoundedRectangle (r, 16.f, 3.f);

    auto face = r.reduced (24.f);
    g.setColour (juce::Colour (0xff040810));
    g.fillRoundedRectangle (face.expanded (4.f), 10.f);
    g.setGradientFill (juce::ColourGradient (ice.interpolatedWith (cyan, 0.2f), face.getCentreX(), face.getBottom() - 6.f,
                                             acc.darker (0.35f), face.getX(), face.getY(), true));
    g.fillRoundedRectangle (face, 9.f);

    {
        juce::Graphics::ScopedSaveState ss (g);
        g.reduceClipRegion (face.toNearestInt());

        const juce::Point<float> P (face.getCentreX(), face.getBottom() + face.getHeight() * 0.22f);
        const float R = face.getHeight() * 1.02f;
        const float half = juce::degreesToRadians (44.f);
        auto polar = [&] (float ang, float rad) { return juce::Point<float> (P.x + std::sin (ang) * rad, P.y - std::cos (ang) * rad); };
        auto posOf = [gmax] (float gr) { return 1.f - std::sqrt (juce::jlimit (0.f, 1.f, gr / gmax)); };   // escala no lineal
        auto angOf = [half] (float pos) { return -half + pos * 2.f * half; };
        const juce::Colour ink (0xff061226), hotc (0xffc0213f);

        juce::Path hotArc;
        hotArc.addCentredArc (P.x, P.y, R * 0.80f, R * 0.80f, 0.f, angOf (posOf (gmax)), angOf (posOf (gmax * 0.5f)), true);
        g.setColour (hotc.withAlpha (0.75f));
        g.strokePath (hotArc, juce::PathStrokeType (7.f));

        juce::Path arc;
        arc.addCentredArc (P.x, P.y, R * 0.80f, R * 0.80f, 0.f, -half, half, true);
        g.setColour (ink);
        g.strokePath (arc, juce::PathStrokeType (2.2f));

        for (float v = 0.f; v <= gmax + 0.01f; v += 1.f)
        {
            const float a = angOf (posOf (v));
            g.drawLine (juce::Line<float> (polar (a, R * 0.80f), polar (a, R * 0.75f)), 1.5f);
        }
        std::vector<float> majors;
        if (uiTopo == 0)      majors = { 0.f, 1.f, 3.f, 5.f, 7.f, 10.f, 20.f };
        else if (uiTopo == 1) majors = { 0.f, 1.f, 2.f, 3.f, 5.f, 8.f, 12.f };
        else                  majors = { 0.f, 1.f, 2.f, 4.f, 6.f, 10.f, 16.f };
        g.setFont (juce::Font (juce::FontOptions (23.f, juce::Font::bold)));
        for (float v : majors)
        {
            const float a = angOf (posOf (v));
            g.setColour (v >= gmax * 0.5f ? hotc : ink);
            g.drawLine (juce::Line<float> (polar (a, R * 0.80f), polar (a, R * 0.69f)), 3.f);
            const auto tp = polar (a, R * 0.60f);
            g.drawText (v == 0.f ? juce::String ("0") : "-" + juce::String ((int) v),
                        juce::Rectangle<float> (tp.x - 30.f, tp.y - 13.f, 60.f, 26.f), juce::Justification::centred);
        }

        g.setColour (ink.withAlpha (0.9f));
        g.setFont (juce::Font (juce::FontOptions (28.f, juce::Font::bold)));
        g.drawText ("dB", face.withTrimmedTop (face.getHeight() * 0.56f).withHeight (32.f), juce::Justification::centred);
        g.setFont (juce::Font (juce::FontOptions (22.f, juce::Font::bold)));
        g.drawText ("GAIN REDUCTION", face.withTrimmedTop (face.getHeight() * 0.72f).withHeight (28.f), juce::Justification::centred);

        // aguja con sombra
        const float na = angOf (juce::jlimit (0.f, 1.f, needlePos));
        const auto tip = polar (na, R * 0.98f);
        g.setColour (juce::Colour (0x55000000));
        g.drawLine (juce::Line<float> (P.translated (4.f, 4.f), tip.translated (4.f, 4.f)), 4.f);
        g.setColour (juce::Colour (0xff050912));
        g.drawLine (juce::Line<float> (P, tip), 3.4f);

        // reflejo del vidrio
        g.setGradientFill (juce::ColourGradient (juce::Colours::white.withAlpha (0.28f), face.getCentreX(), face.getY(),
                                                 juce::Colours::white.withAlpha (0.f), face.getCentreX(), face.getCentreY() + 12.f, false));
        g.fillRect (face.withHeight (face.getHeight() * 0.6f));
    }

    drawScrew (g, r.getX() + 13.f, r.getY() + 13.f, 5.f);
    drawScrew (g, r.getRight() - 13.f, r.getY() + 13.f, 5.f);
    drawScrew (g, r.getX() + 13.f, r.getBottom() - 13.f, 5.f);
    drawScrew (g, r.getRight() - 13.f, r.getBottom() - 13.f, 5.f);
}

// ---------------------------------------------------------------- dibujo general
void DioPanel::paint (juce::Graphics& g)
{
    g.drawImageAt (getBackground (uiTopo), 0, 0);
    const auto acc = accent();
    const float pulse = 0.85f + 0.15f * std::sin (phase);

    // ---- barra superior
    const juce::Rectangle<float> head (24.f, 16.f, 1352.f, 84.f);
    g.setGradientFill (juce::ColourGradient (juce::Colour (0xf0101b33), 0.f, head.getY(), juce::Colour (0xf0050a14), 0.f, head.getBottom(), false));
    g.fillRoundedRectangle (head, 16.f);
    g.setColour (electric.withAlpha (0.30f + 0.15f * pulse));
    g.drawRoundedRectangle (head, 16.f, 2.f);
    g.setColour (juce::Colour (0x22ffffff));
    g.drawLine (40.f, head.getY() + 2.f, head.getRight() - 40.f, head.getY() + 2.f, 1.5f);

    drawCrown (g, { 38.f, 32.f, 44.f, 32.f }, ice, cyan, 2.4f, pulse);
    g.setColour (electric.withAlpha (0.22f * pulse));
    g.strokePath (titlePath, juce::PathStrokeType (9.f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    g.setColour (electric.withAlpha (0.8f));
    g.strokePath (titlePath, juce::PathStrokeType (4.f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    const auto tb = titlePath.getBounds();
    g.setGradientFill (juce::ColourGradient::vertical (juce::Colour (0xfff5fbff), tb.getY(), juce::Colour (0xff8db8ff), tb.getBottom()));
    g.fillPath (titlePath);
    g.setColour (cyan.withAlpha (0.85f));
    g.setFont (juce::Font (juce::FontOptions (15.f, juce::Font::bold)));
    g.drawText ("3 - I N - 1   C O M P R E S S O R", 96, 76, 280, 18, juce::Justification::centredLeft);

    // ---- barra secundaria
    g.setColour (cyan);
    g.setFont (juce::Font (juce::FontOptions (20.f, juce::Font::bold)));
    g.drawText ("SOURCE", 40, 108, 100, 44, juce::Justification::centredLeft);
    g.drawText ("SAFETY", 900, 108, 110, 44, juce::Justification::centredRight);
    if (uiTopo == 2)
    {
        g.setColour (ice.withAlpha (0.85f));
        g.setFont (juce::Font (juce::FontOptions (21.f, juce::Font::bold | juce::Font::italic)));
        g.drawText ("PROGRESSIVE RATIO  -  DENSITY, BIAS & RECOVERY", 440, 108, 520, 44, juce::Justification::centred);
    }

    // ---- panel principal
    const juce::Rectangle<float> panel (24.f, 160.f, 1352.f, 780.f);
    g.setGradientFill (juce::ColourGradient (juce::Colour (0xd0101a30), 0.f, panel.getY(), juce::Colour (0xd003060e), 0.f, panel.getBottom(), false));
    g.fillRoundedRectangle (panel, 18.f);
    g.setColour (juce::Colour (0x22ffffff));
    g.drawLine (44.f, panel.getY() + 2.f, panel.getRight() - 44.f, panel.getY() + 2.f, 1.5f);
    g.setColour (acc.withAlpha (0.45f));
    g.drawRoundedRectangle (panel, 18.f, 2.f);
    drawScrew (g, 42.f, 178.f, 6.f);   drawScrew (g, 1358.f, 178.f, 6.f);
    drawScrew (g, 42.f, 922.f, 6.f);   drawScrew (g, 1358.f, 922.f, 6.f);

    // titulo sobre el medidor (segun topologia)
    g.setColour (acc.interpolatedWith (juce::Colours::white, 0.25f));
    g.setFont (juce::Font (juce::FontOptions (28.f, juce::Font::bold)));
    g.drawText (uiTopo == 0 ? "V C A   C O M P R E S S O R"
              : uiTopo == 1 ? "O P T I C A L   C O M P R E S S O R"
                            : "V A R I - M U   C O M P R E S S O R",
                meterArea.getX(), 164, meterArea.getWidth(), 34, juce::Justification::centred);

    drawMeter (g, meterArea.toFloat());
    drawLedBar (g, { 400.f, 488.f, 290.f, 14.f }, inDisp, "INPUT");
    drawLedBar (g, { 710.f, 488.f, 290.f, 14.f }, outDisp, "OUTPUT");

    g.setColour (ice.withAlpha (0.12f));
    g.drawLine (60.f, 696.f, 1340.f, 696.f, 1.5f);

    // ---- zona inferior: ANALOG COLOR | BASS PRESERVE | SAFETY CEILING
    auto section = [&] (juce::Rectangle<float> rc, const juce::String& title, bool active, juce::Colour c)
    {
        g.setGradientFill (juce::ColourGradient (juce::Colour (0x66102040), rc.getX(), rc.getY(), juce::Colour (0x66050a14), rc.getX(), rc.getBottom(), false));
        g.fillRoundedRectangle (rc, 16.f);
        g.setColour (c.withAlpha (active ? 0.75f : 0.30f));
        g.drawRoundedRectangle (rc, 16.f, 2.f);
        g.setColour (c.interpolatedWith (juce::Colours::white, 0.4f));
        g.setFont (juce::Font (juce::FontOptions (21.f, juce::Font::bold)));
        g.drawText (title, rc.getX() + 20.f, rc.getY() + 8.f, rc.getWidth() - 30.f, 28.f, juce::Justification::centredLeft);
    };
    section ({ 48.f, 704.f, 700.f, 226.f }, "A N A L O G   C O L O R", analogUiOn, ice);
    section ({ 768.f, 704.f, 380.f, 226.f }, "B A S S   P R E S E R V E", bpUiOn, cyan);
    section ({ 1164.f, 704.f, 188.f, 226.f }, "S A F E T Y", false, steel);

    // pie
    g.setColour (ice.withAlpha (0.65f));
    g.setFont (juce::Font (juce::FontOptions (17.f, juce::Font::bold)));
    g.drawText (juce::String::fromUTF8 ("@DioOnTheBeat  \xe2\x80\xa2  BEATS  \xe2\x80\xa2  MIX  \xe2\x80\xa2  MASTER  \xe2\x80\xa2  CREATIVE SOUNDS"),
                0, 942, kW, 18, juce::Justification::centred);
}

void DioPanel::resized()
{
    rebuildTitle();

    // barra superior
    prevBtn.setBounds (372, 34, 40, 48);
    presetBox.setBounds (416, 34, 300, 48);
    nextBtn.setBounds (720, 34, 40, 48);
    saveBtn.setBounds (774, 34, 84, 48);
    manageBtn.setBounds (862, 34, 100, 48);
    for (int i = 0; i < 3; ++i) topoBtn[(size_t) i].setBounds (984 + i * 102, 34, 96, 48);
    powerBtn.setBounds (1296, 24, 68, 68);

    // barra secundaria
    profileBox.setBounds (146, 112, 270, 40);
    for (int i = 0; i < 3; ++i) modeBtn[(size_t) i].setBounds (484 + i * 150, 112, 144, 40);
    safetyBox.setBounds (1018, 112, 176, 40);
    settingsBtn.setBounds (1250, 112, 126, 40);

    layoutControls();
    if (overlay != nullptr) overlay->setBounds (getLocalBounds());
}

// ================================================================ Ventana escalable
DioCompressEditor::DioCompressEditor (DioCompressProcessor& p)
    : AudioProcessorEditor (&p), panel (p)
{
    addAndMakeVisible (panel);
    setResizeLimits (560, 384, 2100, 1440);                      // 40% a 150%
    getConstrainer()->setFixedAspectRatio ((double) DioPanel::kW / (double) DioPanel::kH);
    setResizable (true, true);

    panel.onSizePercent = [this] (int pct)
    {
        setSize (juce::roundToInt ((float) DioPanel::kW * (float) pct / 100.f),
                 juce::roundToInt ((float) DioPanel::kH * (float) pct / 100.f));
    };
    panel.getSizePercent = [this] { return juce::roundToInt (100.f * (float) getWidth() / (float) DioPanel::kW); };

    // escala predeterminada: SIEMPRE 60% al abrir (840 x 576)
    setSize (juce::roundToInt ((float) DioPanel::kW * 0.60f), juce::roundToInt ((float) DioPanel::kH * 0.60f));
}

void DioCompressEditor::resized()
{
    const float s = (float) getWidth() / (float) DioPanel::kW;
    panel.setBounds (0, 0, DioPanel::kW, DioPanel::kH);
    panel.setTransform (juce::AffineTransform::scale (s));
}
