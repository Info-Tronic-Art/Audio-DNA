#include "MappingSuggester.h"
#include <algorithm>
#include <cmath>

float MappingSuggester::featureActivity(float value, float lowThreshold, float highThreshold)
{
    if (value < lowThreshold) return 0.0f;
    if (value > highThreshold) return 1.0f;
    return (value - lowThreshold) / (highThreshold - lowThreshold);
}

std::vector<MappingSuggester::Suggestion> MappingSuggester::suggestMappings(
    const FeatureSnapshot& snapshot, int maxSuggestions) const
{
    std::vector<Suggestion> suggestions;
    suggestions.reserve(20);

    addUniversalSuggestions(suggestions, snapshot);
    addGenreSuggestions(suggestions, snapshot.detectedGenre);

    // Sort by relevance (highest first)
    std::sort(suggestions.begin(), suggestions.end(),
              [](const Suggestion& a, const Suggestion& b) {
                  return a.relevance > b.relevance;
              });

    // Trim to max
    if (static_cast<int>(suggestions.size()) > maxSuggestions)
        suggestions.resize(static_cast<size_t>(maxSuggestions));

    return suggestions;
}

std::vector<MappingSuggester::Suggestion> MappingSuggester::suggestGenreMappings(
    uint8_t genre, int maxSuggestions) const
{
    std::vector<Suggestion> suggestions;
    suggestions.reserve(12);

    addGenreSuggestions(suggestions, genre);

    std::sort(suggestions.begin(), suggestions.end(),
              [](const Suggestion& a, const Suggestion& b) {
                  return a.relevance > b.relevance;
              });

    if (static_cast<int>(suggestions.size()) > maxSuggestions)
        suggestions.resize(static_cast<size_t>(maxSuggestions));

    return suggestions;
}

void MappingSuggester::addUniversalSuggestions(std::vector<Suggestion>& out,
                                                const FeatureSnapshot& snapshot) const
{
    float bassActivity = featureActivity(
        snapshot.bandEnergies[0] + snapshot.bandEnergies[1], 0.1f, 0.6f);
    float midActivity = featureActivity(
        snapshot.bandEnergies[2] + snapshot.bandEnergies[3], 0.1f, 0.6f);
    float highActivity = featureActivity(
        snapshot.bandEnergies[4] + snapshot.bandEnergies[5] + snapshot.bandEnergies[6], 0.05f, 0.4f);
    float beatRelevance = (snapshot.trackerState == 2) ? 0.9f : 0.3f;
    float onsetActivity = featureActivity(snapshot.transientDensity, 1.0f, 8.0f);

    // Bass → Warp effects (always relevant when bass is present)
    if (bassActivity > 0.2f)
    {
        out.push_back({"Bass", "warp", "Ripple", "intensity",
                        "Exponential", "Bass pulses create rhythmic distortion waves",
                        0.8f * bassActivity});
        out.push_back({"Bass", "warp", "Bulge", "intensity",
                        "Exponential", "Bass hits inflate the image from center",
                        0.7f * bassActivity});
    }

    // Beat Phase → Animation (always relevant when BPM is locked)
    out.push_back({"Beat Phase", "animation", "Pulse", "intensity",
                    "Linear", "Smooth pulsing synced to the beat",
                    0.85f * beatRelevance});
    out.push_back({"Bar Phase", "color", "Hue Shift", "amount",
                    "Linear", "Continuous hue rotation over each bar",
                    0.6f * beatRelevance});

    // Spectral Flux → Glitch effects (when audio is dynamic)
    if (onsetActivity > 0.3f)
    {
        out.push_back({"Spectral Flux", "glitch", "Block Glitch", "intensity",
                        "S-Curve", "Sudden spectral changes trigger glitch bursts",
                        0.75f * onsetActivity});
        out.push_back({"Onset Strength", "glitch", "RGB Split", "amount",
                        "Exponential", "Transient hits split RGB channels",
                        0.7f * onsetActivity});
    }

    // Spectral Centroid → Color brightness/temperature
    float centroidActivity = featureActivity(snapshot.spectralCentroid, 1000.0f, 8000.0f);
    if (centroidActivity > 0.1f)
    {
        out.push_back({"Spectral Centroid", "color", "Brightness", "amount",
                        "Logarithmic", "Brighter audio produces brighter visuals",
                        0.6f * centroidActivity});
    }

    // RMS → Overall intensity effects
    float rmsActivity = featureActivity(snapshot.rms, 0.05f, 0.5f);
    if (rmsActivity > 0.2f)
    {
        out.push_back({"RMS", "blur", "Zoom Blur", "intensity",
                        "Exponential", "Loud moments create zoom blur energy",
                        0.65f * rmsActivity});
        out.push_back({"RMS", "blur", "Glow", "intensity",
                        "Linear", "Volume level drives bloom/glow amount",
                        0.55f * rmsActivity});
    }

    // Mids → Pattern effects (when mids are active)
    if (midActivity > 0.3f)
    {
        out.push_back({"Mid", "warp", "Wave", "intensity",
                        "Linear", "Mid-frequency content drives wave deformation",
                        0.6f * midActivity});
    }

    // Highs → Detail effects
    if (highActivity > 0.3f)
    {
        out.push_back({"Brilliance", "color", "Chromatic Aberration", "amount",
                        "Logarithmic", "High frequencies drive prismatic color splitting",
                        0.55f * highActivity});
    }

    // Phrase Phase → Large-scale structure
    out.push_back({"Phrase Phase", "color", "Color Shift", "amount",
                    "Linear", "Color palette evolves over musical phrases",
                    0.5f * beatRelevance});
}

void MappingSuggester::addGenreSuggestions(std::vector<Suggestion>& out,
                                            uint8_t genre) const
{
    switch (genre)
    {
        case GenreDetector::kHouse:
            out.push_back({"Bass", "warp", "Ripple", "intensity",
                            "Exponential", "House: 4-on-floor kick drives rhythmic ripples",
                            0.9f});
            out.push_back({"Beat Phase", "animation", "Strobe", "intensity",
                            "Stepped", "House: strobe on every kick for club feel",
                            0.75f});
            out.push_back({"Phrase Phase", "color", "Hue Shift", "amount",
                            "Linear", "House: color evolves over 8-bar phrases",
                            0.7f});
            out.push_back({"Mid", "warp", "Kaleidoscope", "rotation",
                            "Linear", "House: synth stabs rotate kaleidoscope pattern",
                            0.65f});
            break;

        case GenreDetector::kTechno:
            out.push_back({"Spectral Flux", "glitch", "Block Glitch", "intensity",
                            "Exponential", "Techno: harsh spectral changes trigger industrial glitch",
                            0.9f});
            out.push_back({"Bass", "warp", "Bulge", "intensity",
                            "Exponential", "Techno: relentless kick inflates the image",
                            0.85f});
            out.push_back({"Transient Density", "glitch", "Scanlines", "intensity",
                            "Linear", "Techno: busier rhythms increase scan interference",
                            0.7f});
            out.push_back({"Onset Strength", "color", "Invert", "amount",
                            "Stepped", "Techno: hard transients flash-invert colors",
                            0.65f});
            break;

        case GenreDetector::kDnB:
            out.push_back({"Bass", "warp", "Liquid", "intensity",
                            "Exponential", "DnB: heavy bass creates liquid distortion",
                            0.9f});
            out.push_back({"Onset Strength", "glitch", "RGB Split", "amount",
                            "Exponential", "DnB: fast breakbeats scatter RGB channels",
                            0.85f});
            out.push_back({"Beat Phase", "warp", "Shake", "intensity",
                            "Exponential", "DnB: beat-synced camera shake for energy",
                            0.75f});
            out.push_back({"Spectral Centroid", "color", "Duotone", "mix",
                            "Logarithmic", "DnB: brightness tracks frequency content",
                            0.6f});
            break;

        case GenreDetector::kHipHop:
            out.push_back({"Bass", "warp", "Bulge", "intensity",
                            "Exponential", "Hip-Hop: bass hits create punchy zoom effect",
                            0.85f});
            out.push_back({"Beat Phase", "animation", "Pulse", "intensity",
                            "S-Curve", "Hip-Hop: smooth pulse on the groove",
                            0.8f});
            out.push_back({"RMS", "color", "Contrast", "amount",
                            "Linear", "Hip-Hop: volume drives contrast for impact",
                            0.7f});
            out.push_back({"Mid", "blur", "Motion Blur", "intensity",
                            "Linear", "Hip-Hop: vocal/synth content adds motion blur",
                            0.6f});
            break;

        case GenreDetector::kAmbient:
            out.push_back({"Spectral Centroid", "color", "Hue Shift", "amount",
                            "Logarithmic", "Ambient: tonal color slowly shifts with frequency",
                            0.85f});
            out.push_back({"RMS", "blur", "Gaussian Blur", "intensity",
                            "Linear", "Ambient: volume gently controls focus/blur",
                            0.8f});
            out.push_back({"Spectral Flatness", "color", "Saturation", "amount",
                            "Linear", "Ambient: tonal content saturates color, noise desaturates",
                            0.7f});
            out.push_back({"Phrase Phase", "warp", "Ripple", "intensity",
                            "S-Curve", "Ambient: very slow ripple evolution over phrases",
                            0.6f});
            break;

        case GenreDetector::kRock:
            out.push_back({"RMS", "warp", "Shake", "intensity",
                            "Exponential", "Rock: loud moments shake the camera",
                            0.85f});
            out.push_back({"Bass", "blur", "Zoom Blur", "intensity",
                            "Exponential", "Rock: bass hits create zoom blur energy",
                            0.8f});
            out.push_back({"Onset Strength", "glitch", "Pixel Scatter", "intensity",
                            "Exponential", "Rock: drum hits scatter pixels",
                            0.75f});
            out.push_back({"Mid", "color", "Contrast", "amount",
                            "Linear", "Rock: guitar/vocal energy drives contrast",
                            0.65f});
            break;

        case GenreDetector::kPopElectronic:
            out.push_back({"Beat Phase", "animation", "Pulse", "intensity",
                            "S-Curve", "Pop: clean beat-synced pulsing",
                            0.85f});
            out.push_back({"Bass", "warp", "Ripple", "intensity",
                            "Exponential", "Pop: bass creates gentle ripple effects",
                            0.75f});
            out.push_back({"Spectral Centroid", "color", "Chromatic Aberration", "amount",
                            "Logarithmic", "Pop: bright production drives prismatic color",
                            0.7f});
            out.push_back({"Bar Phase", "color", "Color Shift", "amount",
                            "Linear", "Pop: color rotates each bar for variety",
                            0.65f});
            break;

        case GenreDetector::kJazzOther:
            out.push_back({"Harmonic Change", "color", "Hue Shift", "amount",
                            "Linear", "Jazz: chord changes shift the color palette",
                            0.85f});
            out.push_back({"Spectral Centroid", "color", "Brightness", "amount",
                            "Logarithmic", "Jazz: tonal brightness tracks frequency content",
                            0.75f});
            out.push_back({"RMS", "blur", "Gaussian Blur", "intensity",
                            "S-Curve", "Jazz: dynamics control depth of field",
                            0.7f});
            out.push_back({"Dominant Pitch", "warp", "Wave", "frequency",
                            "Linear", "Jazz: melodic pitch controls wave frequency",
                            0.6f});
            break;

        default:
            break;
    }
}
