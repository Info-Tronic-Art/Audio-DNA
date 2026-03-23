#pragma once
#include "analysis/FeatureSnapshot.h"
#include "analysis/GenreDetector.h"
#include <string>
#include <vector>

// MappingSuggester: analyzes audio features and suggests optimal signal-to-parameter
// mappings based on genre, energy state, and active audio features.
//
// Each suggestion includes a source feature, recommended target effect category,
// curve type, and a human-readable reason.
//
// This is a stateless utility — call suggestMappings() with a snapshot to get
// recommendations. No allocation in steady state (pre-sized output vector).
class MappingSuggester
{
public:
    struct Suggestion
    {
        std::string sourceName;        // e.g., "Bass", "Beat Phase", "Spectral Flux"
        std::string targetCategory;    // e.g., "warp", "color", "glitch"
        std::string targetEffect;      // e.g., "Ripple", "Hue Shift" (recommended)
        std::string targetParam;       // e.g., "intensity", "amount"
        std::string curveType;         // e.g., "Exponential", "Linear", "S-Curve"
        std::string reason;            // Why this mapping works well
        float relevance = 0.0f;       // [0, 1] — how relevant this suggestion is
    };

    MappingSuggester() = default;

    // Generate mapping suggestions based on the current audio snapshot.
    // Returns suggestions sorted by relevance (highest first).
    // maxSuggestions: maximum number of suggestions to return.
    std::vector<Suggestion> suggestMappings(const FeatureSnapshot& snapshot,
                                             int maxSuggestions = 8) const;

    // Generate genre-specific mapping suggestions.
    // Uses the detected genre to recommend mappings that work best for that style.
    std::vector<Suggestion> suggestGenreMappings(uint8_t genre,
                                                  int maxSuggestions = 6) const;

private:
    // Score a feature's relevance based on its current activity level
    static float featureActivity(float value, float lowThreshold, float highThreshold);

    // Add universal mappings that work for any genre
    void addUniversalSuggestions(std::vector<Suggestion>& out,
                                 const FeatureSnapshot& snapshot) const;

    // Add genre-specific mappings
    void addGenreSuggestions(std::vector<Suggestion>& out,
                             uint8_t genre) const;
};
