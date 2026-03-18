#include "ui/SignalInspector.h"

SignalInspector::SignalInspector()
{
    auto setupSlider = [](juce::Slider& s, double min, double max, double val,
                          const juce::String& suffix = {}) {
        s.setSliderStyle(juce::Slider::LinearHorizontal);
        s.setTextBoxStyle(juce::Slider::TextBoxRight, false, 45, 20);
        s.setRange(min, max, 0.01);
        s.setValue(val, juce::dontSendNotification);
        s.setColour(juce::Slider::thumbColourId,
                    juce::Colour(AudioDNALookAndFeel::kAccentCyan));
        if (suffix.isNotEmpty())
            s.setTextValueSuffix(suffix);
    };

    // Audio signal controls
    setupSlider(thresholdSlider_, 0.0, 1.0, 0.0);
    setupSlider(gainSlider_, 0.0, 4.0, 1.0, "x");
    setupSlider(falloffSlider_, 0.0, 1.0, 0.1, "s");
    addChildComponent(thresholdSlider_);
    addChildComponent(gainSlider_);
    addChildComponent(falloffSlider_);

    // Oscillator controls
    waveShapeSelector_.addItem("Sine", 1);
    waveShapeSelector_.addItem("Saw Up", 2);
    waveShapeSelector_.addItem("Saw Down", 3);
    waveShapeSelector_.addItem("Triangle", 4);
    waveShapeSelector_.addItem("Square", 5);
    waveShapeSelector_.setSelectedId(1, juce::dontSendNotification);
    waveShapeSelector_.onChange = [this] {
        if (!signal_ || signal_->getType() != Signal::Type::Oscillator) return;
        auto* osc = static_cast<OscillatorSignal*>(signal_);
        osc->setShape(static_cast<OscillatorSignal::WaveShape>(
            waveShapeSelector_.getSelectedId() - 1));
    };
    addChildComponent(waveShapeSelector_);

    beatDurationSelector_.addItem("1/4 Beat", 1);
    beatDurationSelector_.addItem("1/2 Beat", 2);
    beatDurationSelector_.addItem("1 Beat", 3);
    beatDurationSelector_.addItem("2 Beats", 4);
    beatDurationSelector_.addItem("4 Beats", 5);
    beatDurationSelector_.addItem("8 Beats", 6);
    beatDurationSelector_.setSelectedId(3, juce::dontSendNotification);
    beatDurationSelector_.onChange = [this] {
        if (!signal_ || signal_->getType() != Signal::Type::Oscillator) return;
        auto* osc = static_cast<OscillatorSignal*>(signal_);
        static const float durations[] = {0.25f, 0.5f, 1.0f, 2.0f, 4.0f, 8.0f};
        int sel = beatDurationSelector_.getSelectedId() - 1;
        if (sel >= 0 && sel < 6)
            osc->setBeatDuration(durations[sel]);
    };
    addChildComponent(beatDurationSelector_);

    setupSlider(amplitudeSlider_, 0.0, 1.0, 1.0);
    amplitudeSlider_.onValueChange = [this] {
        if (!signal_ || signal_->getType() != Signal::Type::Oscillator) return;
        auto* osc = static_cast<OscillatorSignal*>(signal_);
        osc->setAmplitude(static_cast<float>(amplitudeSlider_.getValue()));
    };
    addChildComponent(amplitudeSlider_);

    setupSlider(phaseOffsetSlider_, 0.0, 1.0, 0.0);
    phaseOffsetSlider_.onValueChange = [this] {
        if (!signal_ || signal_->getType() != Signal::Type::Oscillator) return;
        auto* osc = static_cast<OscillatorSignal*>(signal_);
        osc->setPhaseOffset(static_cast<float>(phaseOffsetSlider_.getValue()));
    };
    addChildComponent(phaseOffsetSlider_);

    // Envelope controls
    curveTypeSelector_.addItem("Linear", 1);
    curveTypeSelector_.addItem("Exponential", 2);
    curveTypeSelector_.addItem("S-Curve", 3);
    curveTypeSelector_.setSelectedId(1, juce::dontSendNotification);
    curveTypeSelector_.onChange = [this] {
        if (!signal_ || signal_->getType() != Signal::Type::Envelope) return;
        auto* env = static_cast<EnvelopeSignal*>(signal_);
        env->setCurveType(static_cast<EnvelopeSignal::CurveType>(
            curveTypeSelector_.getSelectedId() - 1));
    };
    addChildComponent(curveTypeSelector_);

    envBeatDurationSelector_.addItem("1 Beat", 1);
    envBeatDurationSelector_.addItem("2 Beats", 2);
    envBeatDurationSelector_.addItem("4 Beats", 3);
    envBeatDurationSelector_.addItem("8 Beats", 4);
    envBeatDurationSelector_.addItem("16 Beats", 5);
    envBeatDurationSelector_.setSelectedId(3, juce::dontSendNotification);
    envBeatDurationSelector_.onChange = [this] {
        if (!signal_ || signal_->getType() != Signal::Type::Envelope) return;
        auto* env = static_cast<EnvelopeSignal*>(signal_);
        static const float durations[] = {1.0f, 2.0f, 4.0f, 8.0f, 16.0f};
        int sel = envBeatDurationSelector_.getSelectedId() - 1;
        if (sel >= 0 && sel < 5)
            env->setBeatDuration(durations[sel]);
    };
    addChildComponent(envBeatDurationSelector_);

    setupSlider(envAmplitudeSlider_, 0.0, 1.0, 1.0);
    envAmplitudeSlider_.onValueChange = [this] {
        if (!signal_ || signal_->getType() != Signal::Type::Envelope) return;
        auto* env = static_cast<EnvelopeSignal*>(signal_);
        env->setAmplitude(static_cast<float>(envAmplitudeSlider_.getValue()));
    };
    addChildComponent(envAmplitudeSlider_);

    setupSlider(envPhaseSlider_, 0.0, 1.0, 0.0);
    envPhaseSlider_.onValueChange = [this] {
        if (!signal_ || signal_->getType() != Signal::Type::Envelope) return;
        auto* env = static_cast<EnvelopeSignal*>(signal_);
        env->setPhaseOffset(static_cast<float>(envPhaseSlider_.getValue()));
    };
    addChildComponent(envPhaseSlider_);

    loopingToggle_.setColour(juce::ToggleButton::textColourId,
                             juce::Colour(AudioDNALookAndFeel::kTextPrimary));
    loopingToggle_.onStateChange = [this] {
        if (!signal_ || signal_->getType() != Signal::Type::Envelope) return;
        auto* env = static_cast<EnvelopeSignal*>(signal_);
        env->setLooping(loopingToggle_.getToggleState());
    };
    addChildComponent(loopingToggle_);

    oneShotToggle_.setColour(juce::ToggleButton::textColourId,
                             juce::Colour(AudioDNALookAndFeel::kTextPrimary));
    oneShotToggle_.onStateChange = [this] {
        if (!signal_ || signal_->getType() != Signal::Type::Envelope) return;
        auto* env = static_cast<EnvelopeSignal*>(signal_);
        env->setOneShot(oneShotToggle_.getToggleState());
    };
    addChildComponent(oneShotToggle_);
}

void SignalInspector::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff1a1a1a));

    if (!signal_)
    {
        g.setColour(juce::Colour(AudioDNALookAndFeel::kTextSecondary));
        g.setFont(juce::Font(juce::FontOptions(12.0f)));
        g.drawText("No signal selected", getLocalBounds(),
                   juce::Justification::centred, false);
        return;
    }

    // Signal name header
    auto headerBounds = getLocalBounds().removeFromTop(kSectionHeaderHeight + 4);
    g.setColour(juce::Colour(AudioDNALookAndFeel::kTextPrimary));
    g.setFont(juce::Font(juce::FontOptions(14.0f)).boldened());
    g.drawText(juce::String(signal_->getName()),
               headerBounds.withTrimmedLeft(4),
               juce::Justification::centredLeft, true);

    // Type badge
    juce::String typeStr;
    switch (signal_->getType())
    {
        case Signal::Type::Audio:      typeStr = "AUDIO"; break;
        case Signal::Type::Oscillator: typeStr = "OSC"; break;
        case Signal::Type::Envelope:   typeStr = "ENV"; break;
    }
    g.setColour(juce::Colour(AudioDNALookAndFeel::kAccentCyan).withAlpha(0.3f));
    auto badgeBounds = headerBounds.removeFromRight(50).reduced(4, 4);
    g.fillRect(badgeBounds);
    g.setColour(juce::Colour(AudioDNALookAndFeel::kAccentCyan));
    g.setFont(juce::Font(juce::FontOptions(10.0f)));
    g.drawText(typeStr, badgeBounds, juce::Justification::centred, false);

    // Paint section headers based on signal type
    int y = kSectionHeaderHeight + 4 + kSectionGap;

    if (signal_->getType() == Signal::Type::Audio)
    {
        paintSectionHeader(g, {0, y, getWidth(), kSectionHeaderHeight}, "THRESHOLD / GAIN / FALLOFF");
    }
    else if (signal_->getType() == Signal::Type::Oscillator)
    {
        paintSectionHeader(g, {0, y, getWidth(), kSectionHeaderHeight}, "OSCILLATOR SETTINGS");
    }
    else if (signal_->getType() == Signal::Type::Envelope)
    {
        paintSectionHeader(g, {0, y, getWidth(), kSectionHeaderHeight}, "ENVELOPE SETTINGS");
        // Paint curve editor
        paintCurveEditor(g);
    }
}

void SignalInspector::resized()
{
    auto area = getLocalBounds().reduced(4, 0);
    int y = kSectionHeaderHeight + 4 + kSectionGap;

    hideAllControls();

    if (!signal_) return;

    switch (signal_->getType())
    {
        case Signal::Type::Audio:
            showAudioControls();
            y += kSectionHeaderHeight;
            thresholdSlider_.setBounds(area.getX(), y, area.getWidth(), kRowHeight); y += kRowHeight;
            gainSlider_.setBounds(area.getX(), y, area.getWidth(), kRowHeight); y += kRowHeight;
            falloffSlider_.setBounds(area.getX(), y, area.getWidth(), kRowHeight);
            break;

        case Signal::Type::Oscillator:
            showOscillatorControls();
            y += kSectionHeaderHeight;
            waveShapeSelector_.setBounds(area.getX(), y, area.getWidth(), kRowHeight); y += kRowHeight;
            beatDurationSelector_.setBounds(area.getX(), y, area.getWidth(), kRowHeight); y += kRowHeight;
            amplitudeSlider_.setBounds(area.getX(), y, area.getWidth(), kRowHeight); y += kRowHeight;
            phaseOffsetSlider_.setBounds(area.getX(), y, area.getWidth(), kRowHeight);
            break;

        case Signal::Type::Envelope:
            showEnvelopeControls();
            y += kSectionHeaderHeight;
            curveEditorBounds_ = juce::Rectangle<int>(area.getX(), y,
                                                       area.getWidth(), kCurveEditorHeight);
            y += kCurveEditorHeight + kSectionGap;
            curveTypeSelector_.setBounds(area.getX(), y, area.getWidth(), kRowHeight); y += kRowHeight;
            envBeatDurationSelector_.setBounds(area.getX(), y, area.getWidth(), kRowHeight); y += kRowHeight;
            envAmplitudeSlider_.setBounds(area.getX(), y, area.getWidth(), kRowHeight); y += kRowHeight;
            envPhaseSlider_.setBounds(area.getX(), y, area.getWidth(), kRowHeight); y += kRowHeight;
            {
                auto toggleRow = juce::Rectangle<int>(area.getX(), y, area.getWidth(), kRowHeight);
                loopingToggle_.setBounds(toggleRow.removeFromLeft(area.getWidth() / 2));
                oneShotToggle_.setBounds(toggleRow);
            }
            break;
    }
}

void SignalInspector::setSignal(Signal* signal)
{
    signal_ = signal;
    refresh();
    resized();
    repaint();
}

void SignalInspector::refresh()
{
    if (!signal_) return;

    if (signal_->getType() == Signal::Type::Oscillator)
    {
        auto* osc = static_cast<OscillatorSignal*>(signal_);
        waveShapeSelector_.setSelectedId(static_cast<int>(osc->getShape()) + 1,
                                          juce::dontSendNotification);

        // Map beat duration to selector
        float bd = osc->getBeatDuration();
        int sel = 3; // default 1 beat
        if (bd <= 0.25f) sel = 1;
        else if (bd <= 0.5f) sel = 2;
        else if (bd <= 1.0f) sel = 3;
        else if (bd <= 2.0f) sel = 4;
        else if (bd <= 4.0f) sel = 5;
        else sel = 6;
        beatDurationSelector_.setSelectedId(sel, juce::dontSendNotification);

        amplitudeSlider_.setValue(static_cast<double>(osc->getAmplitude()),
                                  juce::dontSendNotification);
        phaseOffsetSlider_.setValue(static_cast<double>(osc->getPhaseOffset()),
                                    juce::dontSendNotification);
    }
    else if (signal_->getType() == Signal::Type::Envelope)
    {
        auto* env = static_cast<EnvelopeSignal*>(signal_);
        curveTypeSelector_.setSelectedId(static_cast<int>(env->getCurveType()) + 1,
                                          juce::dontSendNotification);

        float bd = env->getBeatDuration();
        int sel = 3;
        if (bd <= 1.0f) sel = 1;
        else if (bd <= 2.0f) sel = 2;
        else if (bd <= 4.0f) sel = 3;
        else if (bd <= 8.0f) sel = 4;
        else sel = 5;
        envBeatDurationSelector_.setSelectedId(sel, juce::dontSendNotification);

        envAmplitudeSlider_.setValue(static_cast<double>(env->getAmplitude()),
                                     juce::dontSendNotification);
        envPhaseSlider_.setValue(static_cast<double>(env->getPhaseOffset()),
                                 juce::dontSendNotification);
        loopingToggle_.setToggleState(env->isLooping(), juce::dontSendNotification);
        oneShotToggle_.setToggleState(env->isOneShot(), juce::dontSendNotification);
    }
}

int SignalInspector::getPreferredHeight() const
{
    if (!signal_) return 100;

    int h = kSectionHeaderHeight + 4 + kSectionGap + kSectionHeaderHeight;
    switch (signal_->getType())
    {
        case Signal::Type::Audio:      h += kRowHeight * 3; break;
        case Signal::Type::Oscillator: h += kRowHeight * 4; break;
        case Signal::Type::Envelope:   h += kCurveEditorHeight + kSectionGap + kRowHeight * 5; break;
    }
    return h + 16;
}

void SignalInspector::hideAllControls()
{
    thresholdSlider_.setVisible(false);
    gainSlider_.setVisible(false);
    falloffSlider_.setVisible(false);
    waveShapeSelector_.setVisible(false);
    beatDurationSelector_.setVisible(false);
    amplitudeSlider_.setVisible(false);
    phaseOffsetSlider_.setVisible(false);
    curveTypeSelector_.setVisible(false);
    envBeatDurationSelector_.setVisible(false);
    envAmplitudeSlider_.setVisible(false);
    envPhaseSlider_.setVisible(false);
    loopingToggle_.setVisible(false);
    oneShotToggle_.setVisible(false);
}

void SignalInspector::showAudioControls()
{
    thresholdSlider_.setVisible(true);
    gainSlider_.setVisible(true);
    falloffSlider_.setVisible(true);
}

void SignalInspector::showOscillatorControls()
{
    waveShapeSelector_.setVisible(true);
    beatDurationSelector_.setVisible(true);
    amplitudeSlider_.setVisible(true);
    phaseOffsetSlider_.setVisible(true);
}

void SignalInspector::showEnvelopeControls()
{
    curveTypeSelector_.setVisible(true);
    envBeatDurationSelector_.setVisible(true);
    envAmplitudeSlider_.setVisible(true);
    envPhaseSlider_.setVisible(true);
    loopingToggle_.setVisible(true);
    oneShotToggle_.setVisible(true);
}

void SignalInspector::paintSectionHeader(juce::Graphics& g,
                                          const juce::Rectangle<int>& bounds,
                                          const juce::String& title)
{
    g.setColour(juce::Colour(0xff222222));
    g.fillRect(bounds);
    g.setColour(juce::Colour(AudioDNALookAndFeel::kTextSecondary));
    g.setFont(juce::Font(juce::FontOptions(10.0f)).boldened());
    g.drawText(title, bounds.withTrimmedLeft(4), juce::Justification::centredLeft, false);
}

void SignalInspector::paintCurveEditor(juce::Graphics& g)
{
    if (!signal_ || signal_->getType() != Signal::Type::Envelope) return;
    auto* env = static_cast<EnvelopeSignal*>(signal_);

    auto bounds = curveEditorBounds_.toFloat();
    if (bounds.isEmpty()) return;

    // Background
    g.setColour(juce::Colour(0xff111111));
    g.fillRect(bounds);

    g.setColour(juce::Colour(0xff2a2a2a));
    g.drawRect(bounds, 1.0f);

    // Grid lines
    g.setColour(juce::Colour(0xff222222));
    for (int i = 1; i < 4; ++i)
    {
        float x = bounds.getX() + bounds.getWidth() * (static_cast<float>(i) / 4.0f);
        g.drawVerticalLine(static_cast<int>(x), bounds.getY(), bounds.getBottom());
    }
    for (int i = 1; i < 4; ++i)
    {
        float y = bounds.getY() + bounds.getHeight() * (static_cast<float>(i) / 4.0f);
        g.drawHorizontalLine(static_cast<int>(y), bounds.getX(), bounds.getRight());
    }

    // Draw curve
    auto& points = env->getPoints();
    if (points.size() < 2) return;

    juce::Path curvePath;
    for (size_t i = 0; i < points.size(); ++i)
    {
        float x = bounds.getX() + points[i].position * bounds.getWidth();
        float y = bounds.getBottom() - points[i].value * bounds.getHeight();

        if (i == 0)
            curvePath.startNewSubPath(x, y);
        else
            curvePath.lineTo(x, y);
    }

    g.setColour(juce::Colour(AudioDNALookAndFeel::kAccentCyan));
    g.strokePath(curvePath, juce::PathStrokeType(2.0f));

    // Draw control points
    for (auto& pt : points)
    {
        float x = bounds.getX() + pt.position * bounds.getWidth();
        float y = bounds.getBottom() - pt.value * bounds.getHeight();
        g.setColour(juce::Colour(AudioDNALookAndFeel::kAccentCyan));
        g.fillEllipse(x - 4.0f, y - 4.0f, 8.0f, 8.0f);
        g.setColour(juce::Colour(0xff111111));
        g.fillEllipse(x - 2.0f, y - 2.0f, 4.0f, 4.0f);
    }
}
