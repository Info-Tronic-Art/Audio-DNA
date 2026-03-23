#pragma once
#include <cstdint>

// GenreSmoothing: provides genre-specific envelope attack/release parameters
// for audio feature smoothing. Different genres need different reactivity:
//
//   - Techno/DnB: fast attack, fast release (transient-heavy, need snappy response)
//   - Ambient: slow attack, slow release (smooth, sustained changes)
//   - House: moderate attack, moderate release (balanced 4-on-floor)
//   - Hip-Hop: moderate attack, slower release (punchy but smooth)
//   - Rock: fast attack, moderate release (dynamic but not twitchy)
//   - Jazz: moderate, slightly slower (fluid, complex changes)
//   - Pop: balanced defaults
//
// These parameters are used by the MappingEngine's per-mapping smoothers
// and can be used to auto-tune OneEuroFilter beta values.
//
// All pre-computed constants — zero allocation.
struct GenreSmoothing
{
    float attackAlpha;     // EMA alpha for rising signals (0-1, higher = faster)
    float releaseAlpha;    // EMA alpha for falling signals
    float oneEuroBeta;     // One-Euro filter speed coefficient
    float oneEuroMinCutoff; // One-Euro minimum cutoff Hz

    // Get recommended smoothing for a genre ID (0-7)
    static GenreSmoothing forGenre(uint8_t genre)
    {
        // Genre IDs from GenreDetector
        switch (genre)
        {
            case 0: // House — balanced 4-on-floor
                return { 0.35f, 0.20f, 0.008f, 1.2f };

            case 1: // Techno — fast, snappy, driving
                return { 0.50f, 0.35f, 0.015f, 2.0f };

            case 2: // DnB — very fast, breakbeat-responsive
                return { 0.55f, 0.40f, 0.020f, 2.5f };

            case 3: // Hip-Hop — punchy attack, smooth release
                return { 0.35f, 0.15f, 0.006f, 0.8f };

            case 4: // Ambient — slow, dreamy, sustained
                return { 0.12f, 0.08f, 0.003f, 0.3f };

            case 5: // Rock — dynamic attack, moderate release
                return { 0.40f, 0.25f, 0.010f, 1.5f };

            case 6: // Pop/Electronic — balanced defaults
                return { 0.30f, 0.20f, 0.007f, 1.0f };

            case 7: // Jazz/Other — fluid, slightly slower
                return { 0.25f, 0.15f, 0.005f, 0.7f };

            default:
                return { 0.30f, 0.20f, 0.007f, 1.0f };
        }
    }

    // Interpolate between two smoothing configs (for genre transitions)
    static GenreSmoothing lerp(const GenreSmoothing& a, const GenreSmoothing& b, float t)
    {
        float oneMinusT = 1.0f - t;
        return {
            a.attackAlpha * oneMinusT + b.attackAlpha * t,
            a.releaseAlpha * oneMinusT + b.releaseAlpha * t,
            a.oneEuroBeta * oneMinusT + b.oneEuroBeta * t,
            a.oneEuroMinCutoff * oneMinusT + b.oneEuroMinCutoff * t
        };
    }
};
