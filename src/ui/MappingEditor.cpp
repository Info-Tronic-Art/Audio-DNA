#include "MappingEditor.h"
#include "ui/LookAndFeel.h"

// Human-readable names for MappingSource enum values
static juce::String getSourceName(MappingSource source)
{
    switch (source)
    {
        case MappingSource::RMS:               return "RMS";
        case MappingSource::Peak:              return "Peak";
        case MappingSource::RmsDB:             return "RMS (dB)";
        case MappingSource::LUFS:              return "LUFS";
        case MappingSource::DynamicRange:      return "Dynamic Range";
        case MappingSource::TransientDensity:  return "Transient Density";
        case MappingSource::SpectralCentroid:  return "Spectral Centroid";
        case MappingSource::SpectralFlux:      return "Spectral Flux";
        case MappingSource::SpectralFlatness:  return "Spectral Flatness";
        case MappingSource::SpectralRolloff:   return "Spectral Rolloff";
        case MappingSource::BandSub:           return "Band: Sub";
        case MappingSource::BandBass:          return "Band: Bass";
        case MappingSource::BandLowMid:        return "Band: Low Mid";
        case MappingSource::BandMid:           return "Band: Mid";
        case MappingSource::BandHighMid:       return "Band: High Mid";
        case MappingSource::BandPresence:      return "Band: Presence";
        case MappingSource::BandBrilliance:    return "Band: Brilliance";
        case MappingSource::OnsetStrength:     return "Onset Strength";
        case MappingSource::BeatPhase:         return "Beat Phase";
        case MappingSource::BPM:               return "BPM";
        case MappingSource::StructuralState:   return "Structural State";
        case MappingSource::DominantPitch:     return "Dominant Pitch";
        case MappingSource::PitchConfidence:   return "Pitch Confidence";
        case MappingSource::DetectedKey:       return "Detected Key";
        case MappingSource::HarmonicChange:    return "Harmonic Change";
        case MappingSource::MFCC0:             return "MFCC 0";
        case MappingSource::MFCC1:             return "MFCC 1";
        case MappingSource::MFCC2:             return "MFCC 2";
        case MappingSource::MFCC3:             return "MFCC 3";
        case MappingSource::MFCC4:             return "MFCC 4";
        case MappingSource::MFCC5:             return "MFCC 5";
        case MappingSource::MFCC6:             return "MFCC 6";
        case MappingSource::MFCC7:             return "MFCC 7";
        case MappingSource::MFCC8:             return "MFCC 8";
        case MappingSource::MFCC9:             return "MFCC 9";
        case MappingSource::MFCC10:            return "MFCC 10";
        case MappingSource::MFCC11:            return "MFCC 11";
        case MappingSource::MFCC12:            return "MFCC 12";
        case MappingSource::ChromaC:           return "Chroma C";
        case MappingSource::ChromaCs:          return "Chroma C#";
        case MappingSource::ChromaD:           return "Chroma D";
        case MappingSource::ChromaDs:          return "Chroma D#";
        case MappingSource::ChromaE:           return "Chroma E";
        case MappingSource::ChromaF:           return "Chroma F";
        case MappingSource::ChromaFs:          return "Chroma F#";
        case MappingSource::ChromaG:           return "Chroma G";
        case MappingSource::ChromaGs:          return "Chroma G#";
        case MappingSource::ChromaA:           return "Chroma A";
        case MappingSource::ChromaAs:          return "Chroma A#";
        case MappingSource::ChromaB:           return "Chroma B";
        case MappingSource::Count:             return "---";
    }
    return "Unknown";
}

static juce::String getCurveName(MappingCurve curve)
{
    switch (curve)
    {
        case MappingCurve::Linear:      return "Linear";
        case MappingCurve::Exponential: return "Exponential";
        case MappingCurve::Logarithmic: return "Logarithmic";
        case MappingCurve::SCurve:      return "S-Curve";
        case MappingCurve::Stepped:     return "Stepped";
        case MappingCurve::Count:       return "---";
    }
    return "Unknown";
}

MappingEditor::MappingEditor()
{
    // Title
    titleLabel_.setText("Mapping Editor", juce::dontSendNotification);
    titleLabel_.setFont(juce::Font(14.0f, juce::Font::bold));
    titleLabel_.setColour(juce::Label::textColourId,
                          juce::Colour(AudioDNALookAndFeel::kAccentCyan));
    addAndMakeVisible(titleLabel_);

    // Source
    sourceLabel_.setText("Source:", juce::dontSendNotification);
    sourceLabel_.setColour(juce::Label::textColourId,
                           juce::Colour(AudioDNALookAndFeel::kTextSecondary));
    addAndMakeVisible(sourceLabel_);
    populateSourceCombo();
    addAndMakeVisible(sourceCombo_);
    sourceCombo_.onChange = [this] { notifyChanged(); };

    // Curve
    curveLabel_.setText("Curve:", juce::dontSendNotification);
    curveLabel_.setColour(juce::Label::textColourId,
                          juce::Colour(AudioDNALookAndFeel::kTextSecondary));
    addAndMakeVisible(curveLabel_);
    populateCurveCombo();
    addAndMakeVisible(curveCombo_);
    curveCombo_.onChange = [this] { notifyChanged(); };

    // Slider setup helper
    auto setupSlider = [this](juce::Slider& slider, juce::Label& label,
                              const juce::String& text, double min, double max,
                              double defaultVal)
    {
        label.setText(text, juce::dontSendNotification);
        label.setColour(juce::Label::textColourId,
                        juce::Colour(AudioDNALookAndFeel::kTextSecondary));
        addAndMakeVisible(label);

        slider.setRange(min, max, 0.001);
        slider.setValue(defaultVal, juce::dontSendNotification);
        slider.setSliderStyle(juce::Slider::LinearHorizontal);
        slider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 50, 20);
        slider.setColour(juce::Slider::thumbColourId,
                         juce::Colour(AudioDNALookAndFeel::kAccentCyan));
        addAndMakeVisible(slider);
        slider.onValueChange = [this] { notifyChanged(); };
    };

    setupSlider(inputMinSlider_,  inputMinLabel_,  "In Min:",  -1000.0, 1000.0, 0.0);
    setupSlider(inputMaxSlider_,  inputMaxLabel_,  "In Max:",  -1000.0, 1000.0, 1.0);
    setupSlider(outputMinSlider_, outputMinLabel_, "Out Min:", 0.0, 1.0, 0.0);
    setupSlider(outputMaxSlider_, outputMaxLabel_, "Out Max:", 0.0, 1.0, 1.0);
    setupSlider(smoothingSlider_, smoothingLabel_, "Smooth:",  0.01, 1.0, 0.15);

    // Enable toggle
    enableToggle_.setToggleState(true, juce::dontSendNotification);
    enableToggle_.setColour(juce::ToggleButton::textColourId,
                            juce::Colour(AudioDNALookAndFeel::kTextPrimary));
    addAndMakeVisible(enableToggle_);
    enableToggle_.onClick = [this] { notifyChanged(); };

    // Delete button
    addAndMakeVisible(deleteButton_);
    deleteButton_.onClick = [this]
    {
        listeners_.call([this](Listener& l) { l.mappingEditorDeleteRequested(this); });
    };
}

void MappingEditor::paint(juce::Graphics& g)
{
    g.setColour(juce::Colour(AudioDNALookAndFeel::kSurface));
    g.fillRoundedRectangle(getLocalBounds().toFloat(), 4.0f);

    g.setColour(juce::Colour(AudioDNALookAndFeel::kSurfaceLight));
    g.drawRoundedRectangle(getLocalBounds().toFloat().reduced(0.5f), 4.0f, 1.0f);
}

void MappingEditor::resized()
{
    auto area = getLocalBounds().reduced(6);
    int rowHeight = 22;
    int spacing = 3;
    int labelWidth = 55;

    titleLabel_.setBounds(area.removeFromTop(rowHeight));
    area.removeFromTop(spacing);

    // Source row
    auto row = area.removeFromTop(rowHeight);
    sourceLabel_.setBounds(row.removeFromLeft(labelWidth));
    sourceCombo_.setBounds(row);
    area.removeFromTop(spacing);

    // Curve row
    row = area.removeFromTop(rowHeight);
    curveLabel_.setBounds(row.removeFromLeft(labelWidth));
    curveCombo_.setBounds(row);
    area.removeFromTop(spacing);

    // Slider rows
    auto layoutSliderRow = [&](juce::Label& label, juce::Slider& slider)
    {
        auto r = area.removeFromTop(rowHeight);
        label.setBounds(r.removeFromLeft(labelWidth));
        slider.setBounds(r);
        area.removeFromTop(spacing);
    };

    layoutSliderRow(inputMinLabel_,  inputMinSlider_);
    layoutSliderRow(inputMaxLabel_,  inputMaxSlider_);
    layoutSliderRow(outputMinLabel_, outputMinSlider_);
    layoutSliderRow(outputMaxLabel_, outputMaxSlider_);
    layoutSliderRow(smoothingLabel_, smoothingSlider_);

    // Enable toggle + delete button
    row = area.removeFromTop(rowHeight + 4);
    enableToggle_.setBounds(row.removeFromLeft(90));
    row.removeFromLeft(8);
    deleteButton_.setBounds(row);
}

void MappingEditor::setMapping(const Mapping& mapping, int mappingIndex)
{
    mappingIndex_ = mappingIndex;

    sourceCombo_.setSelectedId(static_cast<int>(mapping.source) + 1,
                               juce::dontSendNotification);
    curveCombo_.setSelectedId(static_cast<int>(mapping.curve) + 1,
                              juce::dontSendNotification);
    inputMinSlider_.setValue(mapping.inputMin, juce::dontSendNotification);
    inputMaxSlider_.setValue(mapping.inputMax, juce::dontSendNotification);
    outputMinSlider_.setValue(mapping.outputMin, juce::dontSendNotification);
    outputMaxSlider_.setValue(mapping.outputMax, juce::dontSendNotification);
    smoothingSlider_.setValue(mapping.smoothing, juce::dontSendNotification);
    enableToggle_.setToggleState(mapping.enabled, juce::dontSendNotification);
}

Mapping MappingEditor::getMapping() const
{
    Mapping m;
    m.source = static_cast<MappingSource>(sourceCombo_.getSelectedId() - 1);
    m.curve = static_cast<MappingCurve>(curveCombo_.getSelectedId() - 1);
    m.inputMin = static_cast<float>(inputMinSlider_.getValue());
    m.inputMax = static_cast<float>(inputMaxSlider_.getValue());
    m.outputMin = static_cast<float>(outputMinSlider_.getValue());
    m.outputMax = static_cast<float>(outputMaxSlider_.getValue());
    m.smoothing = static_cast<float>(smoothingSlider_.getValue());
    m.enabled = enableToggle_.getToggleState();
    return m;
}

void MappingEditor::addListener(Listener* listener)
{
    listeners_.add(listener);
}

void MappingEditor::removeListener(Listener* listener)
{
    listeners_.remove(listener);
}

void MappingEditor::populateSourceCombo()
{
    int count = static_cast<int>(MappingSource::Count);
    for (int i = 0; i < count; ++i)
        sourceCombo_.addItem(getSourceName(static_cast<MappingSource>(i)), i + 1);
    sourceCombo_.setSelectedId(1, juce::dontSendNotification);
}

void MappingEditor::populateCurveCombo()
{
    int count = static_cast<int>(MappingCurve::Count);
    for (int i = 0; i < count; ++i)
        curveCombo_.addItem(getCurveName(static_cast<MappingCurve>(i)), i + 1);
    curveCombo_.setSelectedId(1, juce::dontSendNotification);
}

void MappingEditor::notifyChanged()
{
    listeners_.call([this](Listener& l) { l.mappingEditorChanged(this); });
}
