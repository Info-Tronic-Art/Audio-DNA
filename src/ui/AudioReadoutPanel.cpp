#include "AudioReadoutPanel.h"
#include "ui/LookAndFeel.h"
#include <cmath>
#include <algorithm>

AudioReadoutPanel::AudioReadoutPanel(const AnalysisThread& /* analysisThread */,
                                     FeatureBus& featureBus)
    : featureBus_(featureBus)
{
    displaySnap_.clear();
    startTimerHz(30);
}

void AudioReadoutPanel::timerCallback()
{
    // Try to acquire new data; fall back to last-read snapshot
    const FeatureSnapshot* newSnap = featureBus_.acquireRead();
    const FeatureSnapshot* snap = newSnap ? newSnap : featureBus_.getLatestRead();
    if (snap == nullptr)
        return;

    hasData_ = true;

    // EMA smooth all continuous values
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

    // Discrete values — no smoothing
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

    float w = bounds.getWidth() - 16.0f;
    float y = bounds.getY() + 8.0f;
    float x = bounds.getX() + 8.0f;

    // === AMPLITUDE ===
    y = drawSection(g, y, w, "Amplitude");
    y = drawMeter(g, y, w, "RMS", displaySnap_.rms,
                  juce::Colour(AudioDNALookAndFeel::kMeterGreen));
    y = drawMeter(g, y, w, "Peak", displaySnap_.peak,
                  juce::Colour(AudioDNALookAndFeel::kMeterYellow));
    y = drawLabel(g, y, w, "RMS dB", juce::String(displaySnap_.rmsDB, 1) + " dB");
    y = drawLabel(g, y, w, "LUFS", juce::String(displaySnap_.lufs, 1));
    y = drawLabel(g, y, w, "Crest", juce::String(displaySnap_.dynamicRange, 1));

    y += 4.0f;

    // === RHYTHM ===
    y = drawSection(g, y, w, "Rhythm");
    y = drawLabel(g, y, w, "BPM", juce::String(static_cast<int>(displaySnap_.bpm + 0.5f)));

    // Beat phase — pulsing bar
    {
        float barY = y;
        g.setColour(juce::Colour(AudioDNALookAndFeel::kTextSecondary));
        g.setFont(juce::Font(juce::FontOptions(11.0f)));
        g.drawText("Beat", juce::Rectangle<float>(x, barY, 40.0f, 14.0f),
                   juce::Justification::centredLeft);

        auto meterBg = juce::Rectangle<float>(x + 44.0f, barY + 2.0f, w - 44.0f, 10.0f);
        g.setColour(juce::Colour(AudioDNALookAndFeel::kBackground));
        g.fillRoundedRectangle(meterBg, 2.0f);

        float phase = std::clamp(displaySnap_.beatPhase, 0.0f, 1.0f);
        float brightness = 1.0f - phase;  // bright on beat, fade to dark
        g.setColour(juce::Colour(AudioDNALookAndFeel::kAccentMagenta).withAlpha(brightness));
        g.fillRoundedRectangle(meterBg.withWidth(meterBg.getWidth() * phase), 2.0f);
        y = barY + 16.0f;
    }

    // Onset flash indicator
    {
        g.setColour(juce::Colour(AudioDNALookAndFeel::kTextSecondary));
        g.setFont(juce::Font(juce::FontOptions(11.0f)));
        g.drawText("Onset", juce::Rectangle<float>(x, y, 40.0f, 14.0f),
                   juce::Justification::centredLeft);

        auto flashRect = juce::Rectangle<float>(x + 44.0f, y + 1.0f, 12.0f, 12.0f);
        g.setColour(juce::Colour(AudioDNALookAndFeel::kAccentCyan).withAlpha(onsetFlash_));
        g.fillEllipse(flashRect);
        g.setColour(juce::Colour(AudioDNALookAndFeel::kTextSecondary).withAlpha(0.5f));
        g.drawEllipse(flashRect, 1.0f);

        y = drawLabel(g, y + 16.0f, w, "Density",
                      juce::String(displaySnap_.transientDensity, 1) + "/s");
    }

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
    y = drawLabel(g, y, w, "State", structStateName(displaySnap_.structuralState));
}

float AudioReadoutPanel::drawSection(juce::Graphics& g, float y, float width,
                                      const juce::String& title)
{
    float x = static_cast<float>(getLocalBounds().getX()) + 8.0f;
    g.setColour(juce::Colour(AudioDNALookAndFeel::kAccentCyan));
    g.setFont(juce::Font(juce::FontOptions(12.0f, juce::Font::bold)));
    g.drawText(title, juce::Rectangle<float>(x, y, width, 16.0f),
               juce::Justification::centredLeft);
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

    auto meterBg = juce::Rectangle<float>(x + 44.0f, y + 2.0f, width - 44.0f, 10.0f);
    g.setColour(juce::Colour(AudioDNALookAndFeel::kBackground));
    g.fillRoundedRectangle(meterBg, 2.0f);

    float clamped = std::clamp(value, 0.0f, 1.0f);
    g.setColour(color);
    g.fillRoundedRectangle(meterBg.withWidth(meterBg.getWidth() * clamped), 2.0f);

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
