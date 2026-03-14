#include "AudioReadoutPanel.h"
#include "ui/LookAndFeel.h"
#include <cmath>
#include <algorithm>

AudioReadoutPanel::AudioReadoutPanel(const AnalysisThread& /* analysisThread */,
                                     FeatureBus& featureBus)
    : featureBus_(featureBus)
{
    displaySnap_.clear();
    std::fill(std::begin(displayBands_), std::end(displayBands_), 0.0f);
    startTimerHz(30);
}

void AudioReadoutPanel::timerCallback()
{
    const FeatureSnapshot* newSnap = featureBus_.acquireRead();
    const FeatureSnapshot* snap = newSnap ? newSnap : featureBus_.getLatestRead();
    if (snap == nullptr)
        return;

    hasData_ = true;
    constexpr float a = 0.3f;

    displaySnap_.rms   += a * (snap->rms   - displaySnap_.rms);
    displaySnap_.peak  += a * (snap->peak  - displaySnap_.peak);
    displaySnap_.rmsDB += a * (snap->rmsDB - displaySnap_.rmsDB);
    displaySnap_.lufs  += a * (snap->lufs  - displaySnap_.lufs);
    displaySnap_.dynamicRange += a * (snap->dynamicRange - displaySnap_.dynamicRange);
    displaySnap_.transientDensity += a * (snap->transientDensity - displaySnap_.transientDensity);

    displaySnap_.spectralCentroid  += a * (snap->spectralCentroid  - displaySnap_.spectralCentroid);
    displaySnap_.spectralFlux      += a * (snap->spectralFlux      - displaySnap_.spectralFlux);
    displaySnap_.spectralFlatness  += a * (snap->spectralFlatness  - displaySnap_.spectralFlatness);
    displaySnap_.spectralRolloff   += a * (snap->spectralRolloff   - displaySnap_.spectralRolloff);

    displaySnap_.bpm       += a * (snap->bpm       - displaySnap_.bpm);
    displaySnap_.beatPhase  = snap->beatPhase;  // no smoothing — sawtooth
    displaySnap_.dominantPitch   += a * (snap->dominantPitch   - displaySnap_.dominantPitch);
    displaySnap_.pitchConfidence += a * (snap->pitchConfidence - displaySnap_.pitchConfidence);
    displaySnap_.harmonicChangeDetection += a * (snap->harmonicChangeDetection - displaySnap_.harmonicChangeDetection);

    // Smooth band energies
    for (int i = 0; i < 7; ++i)
        displayBands_[i] += a * (snap->bandEnergies[i] - displayBands_[i]);

    // Discrete values
    displaySnap_.onsetDetected  = snap->onsetDetected;
    displaySnap_.onsetStrength  = snap->onsetStrength;
    displaySnap_.detectedKey    = snap->detectedKey;
    displaySnap_.keyIsMajor     = snap->keyIsMajor;
    displaySnap_.structuralState = snap->structuralState;

    // Onset flash: spike on onset, fast decay
    if (snap->onsetDetected)
        onsetFlash_ = 1.0f;
    else
        onsetFlash_ *= 0.85f;

    repaint();
}

void AudioReadoutPanel::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();

    g.setColour(juce::Colour(AudioDNALookAndFeel::kSurface));
    g.fillRoundedRectangle(bounds, 4.0f);

    // Panel border
    g.setColour(juce::Colour(AudioDNALookAndFeel::kPanelBorder));
    g.drawRoundedRectangle(bounds.reduced(0.5f), 4.0f, 1.0f);

    float w = bounds.getWidth() - 16.0f;
    float y = bounds.getY() + 8.0f;

    // === AMPLITUDE ===
    y = drawSection(g, y, w, "Amplitude");
    y = drawMeter(g, y, w, "RMS", displaySnap_.rms,
                  juce::Colour(AudioDNALookAndFeel::kMeterGreen));
    y = drawMeter(g, y, w, "Peak", displaySnap_.peak,
                  juce::Colour(AudioDNALookAndFeel::kMeterYellow));
    y = drawDbMeter(g, y, w, "dBFS", displaySnap_.rmsDB, -60.0f, 0.0f);
    y = drawLabel(g, y, w, "LUFS", juce::String(displaySnap_.lufs, 1));
    y = drawLabel(g, y, w, "Crest", juce::String(displaySnap_.dynamicRange, 1));

    y += 4.0f;

    // === BANDS ===
    y = drawSection(g, y, w, "Frequency Bands");
    y = drawBandMeters(g, y, w);

    y += 4.0f;

    // === RHYTHM ===
    y = drawSection(g, y, w, "Rhythm");
    y = drawLabel(g, y, w, "BPM", juce::String(static_cast<int>(displaySnap_.bpm + 0.5f)));
    y = drawBeatPhase(g, y, w);
    y = drawOnsetIndicator(g, y, w);
    y = drawLabel(g, y, w, "Density",
                  juce::String(displaySnap_.transientDensity, 1) + "/s");

    y += 4.0f;

    // === SPECTRAL ===
    y = drawSection(g, y, w, "Spectral");
    y = drawLabel(g, y, w, "Centroid", juce::String(static_cast<int>(displaySnap_.spectralCentroid)) + " Hz");
    y = drawLabel(g, y, w, "Flux", juce::String(displaySnap_.spectralFlux, 2));
    y = drawLabel(g, y, w, "Flat", juce::String(displaySnap_.spectralFlatness, 3));
    y = drawLabel(g, y, w, "Rolloff", juce::String(static_cast<int>(displaySnap_.spectralRolloff)) + " Hz");

    y += 4.0f;

    // === PITCH & KEY ===
    y = drawSection(g, y, w, "Pitch & Key");
    y = drawLabel(g, y, w, "Pitch",
                  juce::String(static_cast<int>(displaySnap_.dominantPitch)) + " Hz");
    y = drawLabel(g, y, w, "Conf", juce::String(displaySnap_.pitchConfidence, 2));
    y = drawLabel(g, y, w, "Key", keyName(displaySnap_.detectedKey, displaySnap_.keyIsMajor));
    y = drawLabel(g, y, w, "HCDF", juce::String(displaySnap_.harmonicChangeDetection, 3));

    y += 4.0f;

    // === STRUCTURAL ===
    y = drawSection(g, y, w, "Structure");
    y = drawStructuralState(g, y, w);
}

//==============================================================================

float AudioReadoutPanel::drawSection(juce::Graphics& g, float y, float width,
                                      const juce::String& title)
{
    float x = static_cast<float>(getLocalBounds().getX()) + 8.0f;
    g.setColour(juce::Colour(AudioDNALookAndFeel::kAccentCyan));
    g.setFont(juce::Font(juce::FontOptions(12.0f, juce::Font::bold)));
    g.drawText(title, juce::Rectangle<float>(x, y, width, 16.0f),
               juce::Justification::centredLeft);

    // Subtle separator line
    g.setColour(juce::Colour(AudioDNALookAndFeel::kPanelBorder));
    g.fillRect(x, y + 15.0f, width, 1.0f);

    return y + 18.0f;
}

float AudioReadoutPanel::drawMeter(juce::Graphics& g, float y, float width,
                                    const juce::String& label, float value,
                                    juce::Colour color)
{
    float x = static_cast<float>(getLocalBounds().getX()) + 8.0f;

    g.setColour(juce::Colour(AudioDNALookAndFeel::kTextSecondary));
    g.setFont(juce::Font(juce::FontOptions(11.0f)));
    g.drawText(label, juce::Rectangle<float>(x, y, 40.0f, 14.0f),
               juce::Justification::centredLeft);

    auto meterBg = juce::Rectangle<float>(x + 44.0f, y + 2.0f, width - 74.0f, 10.0f);
    g.setColour(juce::Colour(AudioDNALookAndFeel::kBackground));
    g.fillRoundedRectangle(meterBg, 2.0f);

    float clamped = std::clamp(value, 0.0f, 1.0f);
    g.setColour(color);
    g.fillRoundedRectangle(meterBg.withWidth(meterBg.getWidth() * clamped), 2.0f);

    // Value text
    g.setColour(juce::Colour(AudioDNALookAndFeel::kTextPrimary));
    g.setFont(juce::Font(juce::FontOptions(10.0f)));
    g.drawText(juce::String(value, 2),
               juce::Rectangle<float>(meterBg.getRight() + 2.0f, y, 28.0f, 14.0f),
               juce::Justification::centredRight);

    return y + 16.0f;
}

float AudioReadoutPanel::drawDbMeter(juce::Graphics& g, float y, float width,
                                      const juce::String& label, float dbValue,
                                      float minDb, float maxDb)
{
    float x = static_cast<float>(getLocalBounds().getX()) + 8.0f;

    g.setColour(juce::Colour(AudioDNALookAndFeel::kTextSecondary));
    g.setFont(juce::Font(juce::FontOptions(11.0f)));
    g.drawText(label, juce::Rectangle<float>(x, y, 40.0f, 14.0f),
               juce::Justification::centredLeft);

    auto meterBg = juce::Rectangle<float>(x + 44.0f, y + 2.0f, width - 74.0f, 10.0f);
    g.setColour(juce::Colour(AudioDNALookAndFeel::kBackground));
    g.fillRoundedRectangle(meterBg, 2.0f);

    float normalized = std::clamp((dbValue - minDb) / (maxDb - minDb), 0.0f, 1.0f);

    // Color gradient: green -> yellow -> red
    juce::Colour meterCol;
    if (normalized < 0.6f)
        meterCol = juce::Colour(AudioDNALookAndFeel::kMeterGreen);
    else if (normalized < 0.85f)
        meterCol = juce::Colour(AudioDNALookAndFeel::kMeterYellow);
    else
        meterCol = juce::Colour(AudioDNALookAndFeel::kMeterRed);

    g.setColour(meterCol);
    g.fillRoundedRectangle(meterBg.withWidth(meterBg.getWidth() * normalized), 2.0f);

    // dB value text
    g.setColour(juce::Colour(AudioDNALookAndFeel::kTextPrimary));
    g.setFont(juce::Font(juce::FontOptions(10.0f)));
    g.drawText(juce::String(dbValue, 1),
               juce::Rectangle<float>(meterBg.getRight() + 2.0f, y, 28.0f, 14.0f),
               juce::Justification::centredRight);

    return y + 16.0f;
}

float AudioReadoutPanel::drawLabel(juce::Graphics& g, float y, float width,
                                    const juce::String& label, const juce::String& value)
{
    float x = static_cast<float>(getLocalBounds().getX()) + 8.0f;

    g.setFont(juce::Font(juce::FontOptions(11.0f)));
    g.setColour(juce::Colour(AudioDNALookAndFeel::kTextSecondary));
    g.drawText(label, juce::Rectangle<float>(x, y, 50.0f, 14.0f),
               juce::Justification::centredLeft);
    g.setColour(juce::Colour(AudioDNALookAndFeel::kTextPrimary));
    g.drawText(value, juce::Rectangle<float>(x + 52.0f, y, width - 52.0f, 14.0f),
               juce::Justification::centredLeft);

    return y + 16.0f;
}

float AudioReadoutPanel::drawBandMeters(juce::Graphics& g, float y, float width)
{
    float x = static_cast<float>(getLocalBounds().getX()) + 8.0f;

    for (int i = 0; i < 7; ++i)
    {
        g.setColour(juce::Colour(AudioDNALookAndFeel::kTextSecondary));
        g.setFont(juce::Font(juce::FontOptions(10.0f)));
        g.drawText(kBandNames[i], juce::Rectangle<float>(x, y, 30.0f, 12.0f),
                   juce::Justification::centredLeft);

        auto meterBg = juce::Rectangle<float>(x + 32.0f, y + 1.0f, width - 32.0f, 10.0f);
        g.setColour(juce::Colour(AudioDNALookAndFeel::kBackground));
        g.fillRoundedRectangle(meterBg, 2.0f);

        float clamped = std::clamp(displayBands_[i], 0.0f, 1.0f);
        g.setColour(juce::Colour(kBandColors[i]));
        g.fillRoundedRectangle(meterBg.withWidth(meterBg.getWidth() * clamped), 2.0f);

        y += 14.0f;
    }

    return y;
}

float AudioReadoutPanel::drawBeatPhase(juce::Graphics& g, float y, float width)
{
    float x = static_cast<float>(getLocalBounds().getX()) + 8.0f;

    g.setColour(juce::Colour(AudioDNALookAndFeel::kTextSecondary));
    g.setFont(juce::Font(juce::FontOptions(11.0f)));
    g.drawText("Beat", juce::Rectangle<float>(x, y, 40.0f, 14.0f),
               juce::Justification::centredLeft);

    auto meterBg = juce::Rectangle<float>(x + 44.0f, y + 2.0f, width - 44.0f, 10.0f);
    g.setColour(juce::Colour(AudioDNALookAndFeel::kBackground));
    g.fillRoundedRectangle(meterBg, 2.0f);

    float phase = std::clamp(displaySnap_.beatPhase, 0.0f, 1.0f);
    float brightness = 1.0f - phase;  // bright on beat, fade to dark
    g.setColour(juce::Colour(AudioDNALookAndFeel::kAccentMagenta).withAlpha(0.3f + brightness * 0.7f));
    g.fillRoundedRectangle(meterBg.withWidth(meterBg.getWidth() * phase), 2.0f);

    // Beat pip (bright dot at current position)
    float pipX = meterBg.getX() + meterBg.getWidth() * phase;
    g.setColour(juce::Colour(AudioDNALookAndFeel::kAccentMagenta));
    g.fillEllipse(pipX - 4.0f, y + 3.0f, 8.0f, 8.0f);

    return y + 16.0f;
}

float AudioReadoutPanel::drawOnsetIndicator(juce::Graphics& g, float y, float /*width*/)
{
    float x = static_cast<float>(getLocalBounds().getX()) + 8.0f;

    g.setColour(juce::Colour(AudioDNALookAndFeel::kTextSecondary));
    g.setFont(juce::Font(juce::FontOptions(11.0f)));
    g.drawText("Onset", juce::Rectangle<float>(x, y, 40.0f, 14.0f),
               juce::Justification::centredLeft);

    // Flash circle
    auto flashRect = juce::Rectangle<float>(x + 44.0f, y + 1.0f, 12.0f, 12.0f);

    // Glow behind the circle when flashing
    if (onsetFlash_ > 0.1f)
    {
        g.setColour(juce::Colour(AudioDNALookAndFeel::kAccentCyan).withAlpha(onsetFlash_ * 0.3f));
        g.fillEllipse(flashRect.expanded(4.0f));
    }

    g.setColour(juce::Colour(AudioDNALookAndFeel::kAccentCyan).withAlpha(onsetFlash_));
    g.fillEllipse(flashRect);
    g.setColour(juce::Colour(AudioDNALookAndFeel::kPanelBorder));
    g.drawEllipse(flashRect, 1.0f);

    // Strength value
    g.setColour(juce::Colour(AudioDNALookAndFeel::kTextPrimary));
    g.setFont(juce::Font(juce::FontOptions(10.0f)));
    g.drawText(juce::String(displaySnap_.onsetStrength, 2),
               juce::Rectangle<float>(x + 60.0f, y, 50.0f, 14.0f),
               juce::Justification::centredLeft);

    return y + 16.0f;
}

float AudioReadoutPanel::drawStructuralState(juce::Graphics& g, float y, float width)
{
    float x = static_cast<float>(getLocalBounds().getX()) + 8.0f;

    g.setColour(juce::Colour(AudioDNALookAndFeel::kTextSecondary));
    g.setFont(juce::Font(juce::FontOptions(11.0f)));
    g.drawText("State", juce::Rectangle<float>(x, y, 40.0f, 14.0f),
               juce::Justification::centredLeft);

    // Colored state indicator
    auto stateCol = structStateColour(displaySnap_.structuralState);
    auto stateName = structStateName(displaySnap_.structuralState);

    // Small colored dot
    g.setColour(stateCol);
    g.fillEllipse(x + 44.0f, y + 3.0f, 8.0f, 8.0f);

    // State name in matching color
    g.setColour(stateCol);
    g.setFont(juce::Font(juce::FontOptions(11.0f, juce::Font::bold)));
    g.drawText(stateName, juce::Rectangle<float>(x + 56.0f, y, width - 56.0f, 14.0f),
               juce::Justification::centredLeft);

    return y + 16.0f;
}

//==============================================================================

juce::String AudioReadoutPanel::keyName(int key, bool isMajor)
{
    if (key < 0 || key > 11)
        return "--";

    static const char* kNotes[12] = {
        "C", "C#", "D", "D#", "E", "F",
        "F#", "G", "G#", "A", "A#", "B"
    };

    return juce::String(kNotes[key]) + (isMajor ? " maj" : " min");
}

juce::String AudioReadoutPanel::structStateName(uint8_t state)
{
    switch (state)
    {
        case 0: return "Normal";
        case 1: return "Buildup";
        case 2: return "Drop";
        case 3: return "Breakdown";
        default: return "?";
    }
}

juce::Colour AudioReadoutPanel::structStateColour(uint8_t state)
{
    switch (state)
    {
        case 0: return juce::Colour(AudioDNALookAndFeel::kTextSecondary);   // Normal - dim
        case 1: return juce::Colour(AudioDNALookAndFeel::kMeterYellow);     // Buildup - yellow
        case 2: return juce::Colour(AudioDNALookAndFeel::kMeterRed);        // Drop - red
        case 3: return juce::Colour(AudioDNALookAndFeel::kAccentCyan);      // Breakdown - cyan
        default: return juce::Colour(AudioDNALookAndFeel::kTextSecondary);
    }
}
