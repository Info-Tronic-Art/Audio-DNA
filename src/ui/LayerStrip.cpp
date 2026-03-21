#include "ui/LayerStrip.h"

// ID scheme for V dropdown: 1-100 = MixMode, 101+ = KeyingMode
static constexpr int kKeyingIdOffset = 101;

// Helper: populate a ComboBox with all MixMode entries
static void populateMixModes(juce::ComboBox& cb, int idOffset = 1)
{
    using M = Layer::MixMode;

    cb.addSectionHeading("Compositing");
    cb.addItem("Alpha", idOffset + static_cast<int>(M::Normal));
    cb.addItem("Add", idOffset + static_cast<int>(M::Additive));
    cb.addItem("Screen", idOffset + static_cast<int>(M::Screen));
    cb.addItem("Multiply", idOffset + static_cast<int>(M::Multiply));
    cb.addItem("Overlay", idOffset + static_cast<int>(M::Overlay));

    cb.addSectionHeading("Light");
    cb.addItem("Soft Light", idOffset + static_cast<int>(M::SoftLight));
    cb.addItem("Hard Light", idOffset + static_cast<int>(M::HardLight));
    cb.addItem("Vivid Light", idOffset + static_cast<int>(M::VividLight));
    cb.addItem("Linear Light", idOffset + static_cast<int>(M::LinearLight));
    cb.addItem("Pin Light", idOffset + static_cast<int>(M::PinLight));
    cb.addItem("Hard Mix", idOffset + static_cast<int>(M::HardMix));

    cb.addSectionHeading("Compare");
    cb.addItem("Darken", idOffset + static_cast<int>(M::Darken));
    cb.addItem("Lighten", idOffset + static_cast<int>(M::Lighten));
    cb.addItem("Darker Color", idOffset + static_cast<int>(M::DarkerColor));
    cb.addItem("Lighter Color", idOffset + static_cast<int>(M::LighterColor));

    cb.addSectionHeading("Dodge / Burn");
    cb.addItem("Color Dodge", idOffset + static_cast<int>(M::ColorDodge));
    cb.addItem("Color Burn", idOffset + static_cast<int>(M::ColorBurn));

    cb.addSectionHeading("Inversion");
    cb.addItem("Difference", idOffset + static_cast<int>(M::Difference));
    cb.addItem("Exclusion", idOffset + static_cast<int>(M::Exclusion));
    cb.addItem("Subtract", idOffset + static_cast<int>(M::Subtract));

    cb.addSectionHeading("Component");
    cb.addItem("Hue", idOffset + static_cast<int>(M::Hue));
    cb.addItem("Saturation", idOffset + static_cast<int>(M::Saturation));
    cb.addItem("Color", idOffset + static_cast<int>(M::Color));
    cb.addItem("Luminosity", idOffset + static_cast<int>(M::Luminosity));

    cb.addSectionHeading("Special");
    cb.addItem("Dissolve", idOffset + static_cast<int>(M::Dissolve));
    cb.addItem("Cut", idOffset + static_cast<int>(M::Cut));

    cb.addSectionHeading("Wipe");
    cb.addItem("Wipe Left", idOffset + static_cast<int>(M::WipeLeft));
    cb.addItem("Wipe Right", idOffset + static_cast<int>(M::WipeRight));
    cb.addItem("Wipe Up", idOffset + static_cast<int>(M::WipeUp));
    cb.addItem("Wipe Down", idOffset + static_cast<int>(M::WipeDown));
    cb.addItem("Wipe Ellipse", idOffset + static_cast<int>(M::WipeEllipse));
    cb.addItem("Wipe Diagonal", idOffset + static_cast<int>(M::WipeDiagonal));

    cb.addSectionHeading("Push");
    cb.addItem("Push Left", idOffset + static_cast<int>(M::PushLeft));
    cb.addItem("Push Right", idOffset + static_cast<int>(M::PushRight));
    cb.addItem("Push Up", idOffset + static_cast<int>(M::PushUp));
    cb.addItem("Push Down", idOffset + static_cast<int>(M::PushDown));

    cb.addSectionHeading("Zoom");
    cb.addItem("Zoom In", idOffset + static_cast<int>(M::ZoomIn));
    cb.addItem("Zoom Out", idOffset + static_cast<int>(M::ZoomOut));

    cb.addSectionHeading("3D");
    cb.addItem("Rotate X", idOffset + static_cast<int>(M::RotateX));
    cb.addItem("Rotate Y", idOffset + static_cast<int>(M::RotateY));
    cb.addItem("Spin", idOffset + static_cast<int>(M::Spin));
    cb.addItem("Cube", idOffset + static_cast<int>(M::Cube));
    cb.addItem("Flip", idOffset + static_cast<int>(M::Flip));
    cb.addItem("Fold", idOffset + static_cast<int>(M::Fold));

    cb.addSectionHeading("Color Fade");
    cb.addItem("To Black", idOffset + static_cast<int>(M::ToBlack));
    cb.addItem("To White", idOffset + static_cast<int>(M::ToWhite));

    cb.addSectionHeading("Creative");
    cb.addItem("Pixelate", idOffset + static_cast<int>(M::Pixelate));
    cb.addItem("Blur", idOffset + static_cast<int>(M::Blur));
    cb.addItem("Noise", idOffset + static_cast<int>(M::Noise));
    cb.addItem("RGB Split", idOffset + static_cast<int>(M::RGBSplit));
    cb.addItem("Glitch Blocks", idOffset + static_cast<int>(M::GlitchBlocks));
    cb.addItem("Strobe", idOffset + static_cast<int>(M::Strobe));
    cb.addItem("Slide", idOffset + static_cast<int>(M::Slide));
    cb.addItem("Stretch", idOffset + static_cast<int>(M::Stretch));
    cb.addItem("Displace", idOffset + static_cast<int>(M::Displace));
}

// Base class that removes all slider padding so track fills entire component
class FullBoundsSliderLAF : public juce::LookAndFeel_V4
{
public:
    juce::Slider::SliderLayout getSliderLayout(juce::Slider& slider) override
    {
        juce::Slider::SliderLayout layout;
        layout.sliderBounds = slider.getLocalBounds();
        layout.textBoxBounds = {};
        return layout;
    }
};

// Vertical opacity slider — teal fill from bottom, "V" label at top
class OpacitySliderLookAndFeel : public FullBoundsSliderLAF
{
public:
    void drawLinearSlider(juce::Graphics& g, int x, int y, int width, int height,
                          float sliderPos, float, float,
                          juce::Slider::SliderStyle, juce::Slider&) override
    {
        auto bounds = juce::Rectangle<float>((float)x, (float)y, (float)width, (float)height);

        g.setColour(juce::Colour(0xff2a2a2a));
        g.fillRect(bounds);

        float fillHeight = bounds.getBottom() - sliderPos;
        if (fillHeight > 0.0f)
        {
            g.setColour(juce::Colour(0xff4a7a6a));
            g.fillRect(juce::Rectangle<float>(bounds.getX(), sliderPos,
                                               bounds.getWidth(), fillHeight));
        }

        g.setColour(juce::Colours::white.withAlpha(0.6f));
        g.setFont(juce::Font(juce::FontOptions(9.0f)));
        g.drawText("V", bounds.removeFromTop(14.0f), juce::Justification::centred);

        g.setColour(juce::Colour(0xff1a1a1a));
        g.drawRect(juce::Rectangle<float>((float)x, (float)y, (float)width, (float)height), 1.0f);
    }
};

// Vertical fade speed slider — same style as V and K, with "F" label
class FadeSpeedSliderLookAndFeel : public FullBoundsSliderLAF
{
public:
    void drawLinearSlider(juce::Graphics& g, int x, int y, int width, int height,
                          float sliderPos, float, float,
                          juce::Slider::SliderStyle, juce::Slider&) override
    {
        auto bounds = juce::Rectangle<float>((float)x, (float)y, (float)width, (float)height);

        g.setColour(juce::Colour(0xff2a2a2a));
        g.fillRect(bounds);

        float fillHeight = bounds.getBottom() - sliderPos;
        if (fillHeight > 0.0f)
        {
            g.setColour(juce::Colour(0xff5a6a7a));
            g.fillRect(juce::Rectangle<float>(bounds.getX(), sliderPos,
                                               bounds.getWidth(), fillHeight));
        }

        g.setColour(juce::Colours::white.withAlpha(0.6f));
        g.setFont(juce::Font(juce::FontOptions(9.0f)));
        g.drawText("F", bounds.removeFromTop(14.0f), juce::Justification::centred);

        g.setColour(juce::Colour(0xff1a1a1a));
        g.drawRect(juce::Rectangle<float>((float)x, (float)y, (float)width, (float)height), 1.0f);
    }
};

// Speed slider — cyan fill from bottom, "S" label at top (centered at 0.5 = 1x)
class SpeedSliderLookAndFeel : public FullBoundsSliderLAF
{
public:
    void drawLinearSlider(juce::Graphics& g, int x, int y, int width, int height,
                          float sliderPos, float, float,
                          juce::Slider::SliderStyle, juce::Slider&) override
    {
        auto bounds = juce::Rectangle<float>((float)x, (float)y, (float)width, (float)height);

        g.setColour(juce::Colour(0xff2a2a2a));
        g.fillRect(bounds);

        float fillHeight = bounds.getBottom() - sliderPos;
        if (fillHeight > 0.0f)
        {
            g.setColour(juce::Colour(0xff3a6a7a));
            g.fillRect(juce::Rectangle<float>(bounds.getX(), sliderPos,
                                               bounds.getWidth(), fillHeight));
        }

        g.setColour(juce::Colours::white.withAlpha(0.6f));
        g.setFont(juce::Font(juce::FontOptions(9.0f)));
        g.drawText("S", bounds.removeFromTop(14.0f), juce::Justification::centred);

        g.setColour(juce::Colour(0xff1a1a1a));
        g.drawRect(juce::Rectangle<float>((float)x, (float)y, (float)width, (float)height), 1.0f);
    }
};

// Keying threshold slider — amber fill from bottom, "K" label at top
class KeyingSliderLookAndFeel : public FullBoundsSliderLAF
{
public:
    void drawLinearSlider(juce::Graphics& g, int x, int y, int width, int height,
                          float sliderPos, float, float,
                          juce::Slider::SliderStyle, juce::Slider&) override
    {
        auto bounds = juce::Rectangle<float>((float)x, (float)y, (float)width, (float)height);

        g.setColour(juce::Colour(0xff2a2a2a));
        g.fillRect(bounds);

        float fillHeight = bounds.getBottom() - sliderPos;
        if (fillHeight > 0.0f)
        {
            g.setColour(juce::Colour(0xff7a6a3a));
            g.fillRect(juce::Rectangle<float>(bounds.getX(), sliderPos,
                                               bounds.getWidth(), fillHeight));
        }

        g.setColour(juce::Colours::white.withAlpha(0.6f));
        g.setFont(juce::Font(juce::FontOptions(9.0f)));
        g.drawText("K", bounds.removeFromTop(14.0f), juce::Justification::centred);

        g.setColour(juce::Colour(0xff1a1a1a));
        g.drawRect(juce::Rectangle<float>((float)x, (float)y, (float)width, (float)height), 1.0f);
    }
};

// ComboBox that shows only a teal caret triangle, no text
class CaretOnlyComboBoxLookAndFeel : public juce::LookAndFeel_V4
{
public:
    void drawComboBox(juce::Graphics& g, int width, int height, bool,
                      int, int, int, int, juce::ComboBox&) override
    {
        auto bounds = juce::Rectangle<float>(0.0f, 0.0f, (float)width, (float)height);

        g.setColour(juce::Colour(0xff333333));
        g.fillRect(bounds);

        // Teal downward caret centered
        float cx = (float)width * 0.5f;
        float cy = (float)height * 0.5f;
        float triW = juce::jmin((float)width * 0.5f, 10.0f);
        float triH = triW * 0.6f;
        juce::Path caret;
        caret.addTriangle(cx - triW * 0.5f, cy - triH * 0.5f,
                          cx + triW * 0.5f, cy - triH * 0.5f,
                          cx, cy + triH * 0.5f);
        g.setColour(juce::Colour(0xff4a7a6a));
        g.fillPath(caret);

        g.setColour(juce::Colour(0xff1a1a1a));
        g.drawRect(bounds, 1.0f);
    }

    void positionComboBoxText(juce::ComboBox&, juce::Label& label) override
    {
        // Hide the text label entirely
        label.setBounds(0, 0, 0, 0);
        label.setVisible(false);
    }
};

// Flat square button — no rounding, dark border, respects button colours
class FlatButtonLookAndFeel : public juce::LookAndFeel_V4
{
public:
    void drawButtonBackground(juce::Graphics& g, juce::Button& button,
                              const juce::Colour& backgroundColour,
                              bool isMouseOver, bool isButtonDown) override
    {
        auto bounds = button.getLocalBounds().toFloat();
        auto baseColour = backgroundColour;

        if (isButtonDown)
            baseColour = baseColour.brighter(0.15f);
        else if (isMouseOver)
            baseColour = baseColour.brighter(0.08f);

        g.setColour(baseColour);
        g.fillRect(bounds);

        g.setColour(juce::Colour(0xff1a1a1a));
        g.drawRect(bounds, 1.0f);
    }
};

// Flat square ComboBox — no rounding
class FlatComboBoxLookAndFeel : public juce::LookAndFeel_V4
{
public:
    void drawComboBox(juce::Graphics& g, int width, int height, bool,
                      int, int, int, int, juce::ComboBox&) override
    {
        auto bounds = juce::Rectangle<float>(0.0f, 0.0f, (float)width, (float)height);

        g.setColour(juce::Colour(0xff333333));
        g.fillRect(bounds);

        // Teal down arrow on right
        float arrowX = (float)width - 14.0f;
        float cy = (float)height * 0.5f;
        juce::Path arrow;
        arrow.addTriangle(arrowX - 4.0f, cy - 2.0f,
                          arrowX + 4.0f, cy - 2.0f,
                          arrowX, cy + 3.0f);
        g.setColour(juce::Colour(0xff4a7a6a));
        g.fillPath(arrow);

        g.setColour(juce::Colour(0xff1a1a1a));
        g.drawRect(bounds, 1.0f);
    }
};

static SpeedSliderLookAndFeel sSpeedLAF;
static OpacitySliderLookAndFeel sOpacityLAF;
static FadeSpeedSliderLookAndFeel sFadeLAF;
static KeyingSliderLookAndFeel sKeyingLAF;
static CaretOnlyComboBoxLookAndFeel sCaretComboLAF;
static FlatButtonLookAndFeel sFlatBtnLAF;
static FlatComboBoxLookAndFeel sFlatComboLAF;

LayerStrip::LayerStrip()
{
    setOpaque(true);
    startTimerHz(30); // 30fps playhead update

    setupFlatButton(clearBtn_);
    setupFlatButton(bypassBtn_);
    setupFlatButton(soloBtn_);

    clearBtn_.onClick = [this] {
        if (onClearClip && layer_) onClearClip(layerIndex_);
    };
    bypassBtn_.onClick = [this] {
        if (!layer_) return;
        layer_->bypassed = !layer_->bypassed;
        updateButtonStates();
        if (onBypass) onBypass(layerIndex_, layer_->bypassed);
    };
    soloBtn_.onClick = [this] {
        if (!layer_) return;
        layer_->solo = !layer_->solo;
        updateButtonStates();
        if (onSolo) onSolo(layerIndex_, layer_->solo);
    };

    // Transport controls
    setupFlatButton(transportBackBtn_);
    setupFlatButton(transportPauseBtn_);
    setupFlatButton(transportPlayBtn_);
    setupFlatButton(transportForwardBtn_);
    transportBackBtn_.setButtonText("<");
    transportPauseBtn_.setButtonText("||");
    transportPlayBtn_.setButtonText(">");
    transportForwardBtn_.setButtonText(">|");

    transportBackBtn_.onClick = [this] {
        if (!layer_) return;
        auto* clip = layer_->getActiveClip();
        if (clip) { clip->reverse = true; clip->playing = true; }
        if (onTransportBack) onTransportBack(layerIndex_);
    };
    transportPauseBtn_.onClick = [this] {
        if (!layer_) return;
        auto* clip = layer_->getActiveClip();
        if (clip) clip->playing = false;
        if (onTransportPause) onTransportPause(layerIndex_);
    };
    transportPlayBtn_.onClick = [this] {
        if (!layer_) return;
        auto* clip = layer_->getActiveClip();
        if (clip) { clip->reverse = false; clip->playing = true; }
        if (onTransportPlay) onTransportPlay(layerIndex_);
    };
    transportForwardBtn_.onClick = [this] {
        if (!layer_) return;
        auto* clip = layer_->getActiveClip();
        if (clip) { clip->reverse = false; clip->playing = true; clip->speed = std::min(clip->speed * 2.0f, 4.0f); }
        if (onTransportForward) onTransportForward(layerIndex_);
    };

    // S = speed slider (0 = 0x, 0.25 = 1x default, 1.0 = 4x)
    addAndMakeVisible(speedSlider_);
    speedSlider_.setRange(0.0, 1.0, 0.01);
    speedSlider_.setValue(0.25, juce::dontSendNotification);
    speedSlider_.setDefaultValue(0.25);
    speedSlider_.setSliderStyle(juce::Slider::LinearVertical);
    speedSlider_.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
    speedSlider_.setLookAndFeel(&sSpeedLAF);
    speedSlider_.onValueChange = [this] {
        if (!layer_) return;
        auto* clip = layer_->getActiveClip();
        if (clip)
        {
            // Map 0-1 slider to 0x-4x speed (0.25 = 1x)
            float v = static_cast<float>(speedSlider_.getValue());
            clip->speed = v * 4.0f;
        }
    };

    // K = keying threshold slider (no dropdown)
    addAndMakeVisible(keyingSlider_);
    keyingSlider_.setRange(0.0, 1.0, 0.01);
    keyingSlider_.setValue(0.1, juce::dontSendNotification);
    keyingSlider_.setDefaultValue(0.1);
    keyingSlider_.setSliderStyle(juce::Slider::LinearVertical);
    keyingSlider_.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
    keyingSlider_.setLookAndFeel(&sKeyingLAF);
    keyingSlider_.onValueChange = [this] {
        if (layer_)
            layer_->keyThreshold = static_cast<float>(keyingSlider_.getValue());
    };

    // V = opacity slider
    addAndMakeVisible(opacitySlider_);
    opacitySlider_.setRange(0.0, 1.0, 0.01);
    opacitySlider_.setValue(1.0, juce::dontSendNotification);
    opacitySlider_.setDefaultValue(1.0);
    opacitySlider_.setSliderStyle(juce::Slider::LinearVertical);
    opacitySlider_.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
    opacitySlider_.setLookAndFeel(&sOpacityLAF);
    opacitySlider_.onValueChange = [this] {
        if (layer_)
            layer_->opacity = static_cast<float>(opacitySlider_.getValue());
    };

    // V dropdown = keying types + mix modes combined
    populateBlendDropdown();
    addAndMakeVisible(blendDropdown_);
    blendDropdown_.setLookAndFeel(&sFlatComboLAF);
    blendDropdown_.setColour(juce::ComboBox::backgroundColourId, juce::Colour(kBtnBg));
    blendDropdown_.setColour(juce::ComboBox::outlineColourId, juce::Colour(kBtnBorder));
    blendDropdown_.setColour(juce::ComboBox::textColourId, juce::Colour(kTextDim));
    blendDropdown_.onChange = [this] {
        if (!layer_) return;
        int sel = blendDropdown_.getSelectedId();
        if (sel >= kKeyingIdOffset)
        {
            // Keying mode selected
            layer_->keyingMode = static_cast<Layer::KeyingMode>(sel - kKeyingIdOffset);
        }
        else if (sel >= 1)
        {
            // Mix mode selected
            layer_->blendMode = static_cast<Layer::MixMode>(sel - 1);
            if (onBlendModeChanged)
                onBlendModeChanged(layerIndex_, layer_->blendMode);
        }
    };

    // F = fade speed slider
    addAndMakeVisible(fadeTimeSlider_);
    fadeTimeSlider_.setRange(0.0, 4.0, 0.1);
    fadeTimeSlider_.setValue(0.3, juce::dontSendNotification);
    fadeTimeSlider_.setDefaultValue(0.3);
    fadeTimeSlider_.setSliderStyle(juce::Slider::LinearVertical);
    fadeTimeSlider_.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
    fadeTimeSlider_.setLookAndFeel(&sFadeLAF);
    fadeTimeSlider_.onValueChange = [this] {
        if (layer_)
            layer_->transitionSpeed = static_cast<float>(fadeTimeSlider_.getValue());
    };

    // F dropdown = transition mix mode (caret only, no text)
    populateTransitionDropdown();
    addAndMakeVisible(transitionDropdown_);
    transitionDropdown_.setLookAndFeel(&sCaretComboLAF);
    transitionDropdown_.setColour(juce::ComboBox::backgroundColourId, juce::Colour(kBtnBg));
    transitionDropdown_.setColour(juce::ComboBox::outlineColourId, juce::Colour(kBtnBorder));
    transitionDropdown_.setColour(juce::ComboBox::textColourId, juce::Colour(kTextDim));
    transitionDropdown_.onChange = [this] {
        if (!layer_) return;
        int sel = transitionDropdown_.getSelectedId();
        if (sel >= 1)
            layer_->transitionMode = static_cast<Layer::MixMode>(sel - 1);
    };
}

void LayerStrip::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds();

    g.setColour(juce::Colour(kStripBg));
    g.fillRect(bounds);

    // Thumbnail area
    if (!thumbnailBounds_.isEmpty())
    {
        auto tb = thumbnailBounds_.toFloat();
        if (thumbnail_.isValid())
        {
            g.drawImage(thumbnail_, tb,
                        juce::RectanglePlacement::centred
                        | juce::RectanglePlacement::fillDestination);
        }
        else
        {
            g.setColour(juce::Colour(kThumbBg));
            g.fillRect(tb);
        }
        g.setColour(juce::Colour(kBtnBorder));
        g.drawRect(tb, 1.0f);
    }

    // Transport/playhead display (between left buttons and right sliders)
    if (!transportBounds_.isEmpty())
    {
        auto tb = transportBounds_.toFloat();
        g.setColour(juce::Colour(0xff111111));
        g.fillRect(tb);

        if (layer_)
        {
            auto* clip = layer_->getActiveClip();
            if (clip && clip->isPlayable())
            {
                // Draw in/out region
                float inX = tb.getX() + clip->inPoint * tb.getWidth();
                float outX = tb.getX() + clip->outPoint * tb.getWidth();
                g.setColour(juce::Colour(0xff1a2a2a));
                g.fillRect(juce::Rectangle<float>(inX, tb.getY(), outX - inX, tb.getHeight()));

                // Playhead line
                float pos = static_cast<float>(clip->playheadPosition);
                float xPos = tb.getX() + pos * tb.getWidth();
                g.setColour(juce::Colour(AudioDNALookAndFeel::kAccentCyan));
                g.drawVerticalLine(static_cast<int>(xPos), tb.getY(), tb.getBottom());
            }
        }

        g.setColour(juce::Colour(kBtnBorder));
        g.drawRect(tb, 1.0f);
    }

    // Layer name box (painted manually — same as ComboBox rendering)
    if (!nameBounds_.isEmpty())
    {
        auto nb = nameBounds_.toFloat();
        g.setColour(juce::Colour(kBtnBg));
        g.fillRect(nb);
        g.setColour(juce::Colour(kBtnBorder));
        g.drawRect(nb, 1.0f);
        g.setColour(juce::Colour(0xffe0e0e0));
        g.setFont(juce::Font(juce::FontOptions(13.0f)));
        g.drawText(layerName_, nb.reduced(5.0f, 0.0f), juce::Justification::centredLeft, true);
    }

    // Clip name box with transport playhead overlay
    if (!clipNameBounds_.isEmpty())
    {
        auto cb = clipNameBounds_.toFloat();
        g.setColour(juce::Colour(kBtnBg));
        g.fillRect(cb);
        g.setColour(juce::Colour(kBtnBorder));
        g.drawRect(cb, 1.0f);
        g.setColour(juce::Colour(0xffe0e0e0));
        g.setFont(juce::Font(juce::FontOptions(10.0f)));
        g.drawText(clipName_, cb.reduced(3.0f, 0.0f), juce::Justification::centred, true);
    }

    // Selection outline — cyan border around the name box only
    if (selected_ && !nameBounds_.isEmpty())
    {
        g.setColour(juce::Colour(AudioDNALookAndFeel::kAccentCyan));
        g.drawRect(nameBounds_.toFloat(), 2.0f);
    }

    // Bottom edge
    g.setColour(juce::Colour(kBtnBorder));
    g.drawHorizontalLine(bounds.getHeight() - 1, 0.0f, (float)bounds.getWidth());
}

void LayerStrip::resized()
{
    auto bounds = getLocalBounds();
    int h = bounds.getHeight();
    int w = bounds.getWidth();

    int btnSize = 26;
    int leftColW = btnSize * 3;
    int sliderW = 22;           // K, V, and F all same width
    int dropdownH = 20;

    int mainH = h - dropdownH;
    if (mainH < 30) mainH = h;

    // Thumbnail: always square using mainH
    int thumbW = mainH;

    // === Left column: X B S on top, < || > below, transport bar fills rest ===
    clearBtn_.setBounds(0, 0, btnSize, btnSize);
    bypassBtn_.setBounds(btnSize, 0, btnSize, btnSize);
    soloBtn_.setBounds(btnSize * 2, 0, btnSize, btnSize);

    // Transport buttons (< || >) below X/B/S
    int tBtnH = btnSize;
    int tY = btnSize;
    {
        int tBtnW = btnSize;
        if (mainH - btnSize > 20)
        {
            transportBackBtn_.setBounds(0, tY, tBtnW, tBtnH);
            transportPauseBtn_.setBounds(tBtnW, tY, tBtnW, tBtnH);
            transportPlayBtn_.setBounds(tBtnW * 2, tY, tBtnW, tBtnH);
            if (leftColW > tBtnW * 3)
            {
                transportForwardBtn_.setBounds(tBtnW * 3, tY, leftColW - tBtnW * 3, tBtnH);
                transportForwardBtn_.setVisible(true);
            }
            else
                transportForwardBtn_.setVisible(false);
            transportBackBtn_.setVisible(true);
            transportPauseBtn_.setVisible(true);
            transportPlayBtn_.setVisible(true);
        }
        else
        {
            transportBackBtn_.setVisible(false);
            transportPauseBtn_.setVisible(false);
            transportPlayBtn_.setVisible(false);
            transportForwardBtn_.setVisible(false);
        }
    }

    // Transport playhead bar: fills remaining space below < || > buttons, above name row
    int transportBarY = tY + tBtnH;
    int transportBarH = mainH - transportBarY;
    if (transportBarH < 4) transportBarH = 4;
    transportBounds_ = juce::Rectangle<int>(0, transportBarY, leftColW, transportBarH);

    // === Right section: S | K | V | thumbnail(square) | F — flush against left column ===
    int sX = leftColW;
    int kX = sX + sliderW;
    int vX = kX + sliderW;
    int thumbX = vX + sliderW;
    int fX = thumbX + thumbW;

    speedSlider_.setBounds(sX, 0, sliderW, mainH);
    keyingSlider_.setBounds(kX, 0, sliderW, mainH);
    opacitySlider_.setBounds(vX, 0, sliderW, mainH);
    thumbnailBounds_ = juce::Rectangle<int>(thumbX, 0, thumbW, mainH);
    fadeTimeSlider_.setBounds(fX, 0, sliderW, mainH);

    // === Dropdown row ===
    int rowY = mainH;
    int rowH = dropdownH;

    int nameW = leftColW;
    if (nameW < 30) nameW = 30;
    nameBounds_ = juce::Rectangle<int>(0, rowY, nameW, rowH);
    blendDropdown_.setBounds(sX, rowY, sliderW * 3, rowH);
    clipNameBounds_ = juce::Rectangle<int>(thumbX, rowY, thumbW, rowH);
    transitionDropdown_.setBounds(fX, rowY, sliderW, rowH);
}

void LayerStrip::setLayer(Layer* layer, int index)
{
    layer_ = layer;
    layerIndex_ = index;

    if (layer_)
    {
        layerName_ = juce::String(layer_->name);
        opacitySlider_.setValue(layer_->opacity, juce::dontSendNotification);
        keyingSlider_.setValue(layer_->keyThreshold, juce::dontSendNotification);
        blendDropdown_.setSelectedId(static_cast<int>(layer_->blendMode) + 1,
                                     juce::dontSendNotification);

        float fade = layer_->transitionSpeed;
        if (fade < 0.0f) fade = 0.3f;
        fadeTimeSlider_.setValue(fade, juce::dontSendNotification);

        transitionDropdown_.setSelectedId(static_cast<int>(layer_->transitionMode) + 1,
                                          juce::dontSendNotification);

        // Speed slider: read from active clip (0.25 = 1x)
        auto* clip = layer_->getActiveClip();
        if (clip)
            speedSlider_.setValue(static_cast<double>(clip->speed / 4.0f), juce::dontSendNotification);
        else
            speedSlider_.setValue(0.25, juce::dontSendNotification);

        updateButtonStates();
        updateThumbnail();
        updateClipName();
    }
}

void LayerStrip::refresh()
{
    if (!layer_) return;
    layerName_ = juce::String(layer_->name);
    updateThumbnail();
    updateClipName();
    repaint();
}

void LayerStrip::timerCallback()
{
    // Repaint transport and clip name areas to animate the playhead
    if (!transportBounds_.isEmpty())
        repaint(transportBounds_);
    if (!clipNameBounds_.isEmpty())
        repaint(clipNameBounds_);
}

void LayerStrip::mouseDown(const juce::MouseEvent& event)
{
    // Scrub playhead if clicking in the transport bar area
    if (!transportBounds_.isEmpty() && transportBounds_.contains(event.getPosition()))
    {
        scrubPlayhead(event.getPosition());
        return;
    }

    if (!event.mods.isRightButtonDown())
    {
        if (onSelect) onSelect(layerIndex_);
    }
}

void LayerStrip::mouseDrag(const juce::MouseEvent& event)
{
    if (!transportBounds_.isEmpty() && transportBounds_.contains(event.getMouseDownPosition()))
    {
        scrubPlayhead(event.getPosition());
    }
}

void LayerStrip::scrubPlayhead(juce::Point<int> pos)
{
    if (!layer_ || transportBounds_.isEmpty()) return;
    auto* clip = layer_->getActiveClip();
    if (!clip || !clip->isPlayable()) return;

    float normalized = static_cast<float>(pos.x - transportBounds_.getX())
                     / static_cast<float>(transportBounds_.getWidth());
    normalized = juce::jlimit(0.0f, 1.0f, normalized);
    clip->playheadPosition = static_cast<double>(normalized);
}

void LayerStrip::setupFlatButton(juce::TextButton& btn)
{
    addAndMakeVisible(btn);
    btn.setLookAndFeel(&sFlatBtnLAF);
    btn.setColour(juce::TextButton::buttonColourId, juce::Colour(kBtnBg));
    btn.setColour(juce::TextButton::buttonOnColourId, juce::Colour(kBtnBg));
    btn.setColour(juce::TextButton::textColourOffId, juce::Colour(kTextDim));
    btn.setColour(juce::TextButton::textColourOnId, juce::Colour(kTextDim));
}

void LayerStrip::updateButtonStates()
{
    if (!layer_) return;

    auto inactive = juce::Colour(kBtnBg);

    bypassBtn_.setColour(juce::TextButton::buttonColourId,
                         layer_->bypassed ? juce::Colour(kBypassActive) : inactive);
    bypassBtn_.setColour(juce::TextButton::textColourOffId,
                         layer_->bypassed ? juce::Colours::white : juce::Colour(kTextDim));

    soloBtn_.setColour(juce::TextButton::buttonColourId,
                       layer_->solo ? juce::Colour(kSoloActive) : inactive);
    soloBtn_.setColour(juce::TextButton::textColourOffId,
                       layer_->solo ? juce::Colours::white : juce::Colour(kTextDim));
}

void LayerStrip::updateThumbnail()
{
    thumbnail_ = juce::Image();
    if (!layer_) return;

    auto* clip = layer_->getActiveClip();
    if (!clip) return;

    int sz = thumbnailBounds_.getHeight();
    if (sz < 1) sz = 64;

    // Use cached thumbnail from clip if available (video, image sequence)
    if (clip->thumbnail.isValid())
    {
        thumbnail_ = clip->thumbnail.rescaled(sz, sz, juce::Graphics::lowResamplingQuality);
        repaint(thumbnailBounds_);
        return;
    }

    if (clip->mediaType == Clip::MediaType::Image && clip->mediaFile.existsAsFile())
    {
        auto img = juce::ImageFileFormat::loadFrom(clip->mediaFile);
        if (img.isValid())
        {
            thumbnail_ = img.rescaled(sz, sz, juce::Graphics::lowResamplingQuality);
            repaint(thumbnailBounds_);
            return;
        }
    }

    // Generate placeholder thumbnails for Source and FX-only clips
    if (clip->mediaType == Clip::MediaType::Source || (clip->hasEffects() && !clip->hasMedia()))
    {
        thumbnail_ = juce::Image(juce::Image::ARGB, sz, sz, true);
        juce::Graphics g(thumbnail_);

        if (clip->mediaType == Clip::MediaType::Source)
        {
            g.fillAll(juce::Colour(0xff2a2040));
            g.setColour(juce::Colour(0xffbb88ff));
            g.setFont(juce::Font(juce::FontOptions(static_cast<float>(sz) * 0.25f).withStyle("Bold")));
            g.drawText("SRC", thumbnail_.getBounds(), juce::Justification::centred);
        }
        else
        {
            g.fillAll(juce::Colour(0xff3a2a3a));
            g.setColour(juce::Colour(0xffe0e0e0));
            g.setFont(juce::Font(juce::FontOptions(static_cast<float>(sz) * 0.3f).withStyle("Bold")));
            g.drawText("FX", thumbnail_.getBounds(), juce::Justification::centred);
        }
    }

    repaint(thumbnailBounds_);
}

void LayerStrip::updateClipName()
{
    if (!layer_)
    {
        clipName_ = "";
        return;
    }

    auto* clip = layer_->getActiveClip();
    if (!clip)
    {
        clipName_ = "";
    }
    else if (clip->mediaType == Clip::MediaType::Source)
    {
        clipName_ = juce::String(clip->name);
    }
    else if (clip->hasEffects() && !clip->hasMedia())
    {
        clipName_ = juce::String(clip->name);
    }
    else if (clip->hasMedia())
    {
        clipName_ = clip->mediaFile.getFileNameWithoutExtension();
    }
    else
    {
        clipName_ = "";
    }
    repaint(clipNameBounds_);
}

void LayerStrip::populateBlendDropdown()
{
    using K = Layer::KeyingMode;
    blendDropdown_.clear(juce::dontSendNotification);

    // Keying types first (these control transparency extraction)
    blendDropdown_.addSectionHeading("Keying");
    blendDropdown_.addItem("Alpha", kKeyingIdOffset + static_cast<int>(K::Alpha));
    blendDropdown_.addItem("Luma Key", kKeyingIdOffset + static_cast<int>(K::LumaKey));
    blendDropdown_.addItem("Inverted Luma Key", kKeyingIdOffset + static_cast<int>(K::InvertedLumaKey));
    blendDropdown_.addItem("Luma Is Alpha", kKeyingIdOffset + static_cast<int>(K::LumaIsAlpha));
    blendDropdown_.addItem("Inverted Luma Is Alpha", kKeyingIdOffset + static_cast<int>(K::InvertedLumaIsAlpha));
    blendDropdown_.addItem("Chroma Key", kKeyingIdOffset + static_cast<int>(K::ChromaKey));
    blendDropdown_.addItem("Max RGB", kKeyingIdOffset + static_cast<int>(K::MaxRGB));
    blendDropdown_.addItem("Saturation Key", kKeyingIdOffset + static_cast<int>(K::SaturationKey));
    blendDropdown_.addItem("Edge Detection", kKeyingIdOffset + static_cast<int>(K::EdgeDetection));
    blendDropdown_.addItem("Threshold Mask", kKeyingIdOffset + static_cast<int>(K::ThresholdMask));
    blendDropdown_.addItem("Channel Red", kKeyingIdOffset + static_cast<int>(K::ChannelR));
    blendDropdown_.addItem("Channel Green", kKeyingIdOffset + static_cast<int>(K::ChannelG));
    blendDropdown_.addItem("Channel Blue", kKeyingIdOffset + static_cast<int>(K::ChannelB));

    // Then all mix modes
    populateMixModes(blendDropdown_, 1);

    blendDropdown_.setSelectedId(1 + static_cast<int>(Layer::MixMode::Additive),
                                 juce::dontSendNotification);
}

void LayerStrip::populateTransitionDropdown()
{
    transitionDropdown_.clear(juce::dontSendNotification);
    populateMixModes(transitionDropdown_, 1);
    transitionDropdown_.setSelectedId(1 + static_cast<int>(Layer::MixMode::Dissolve),
                                      juce::dontSendNotification);
}
