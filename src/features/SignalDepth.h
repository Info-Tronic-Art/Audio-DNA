#pragma once

// Master Signal (s-rta-0925 mastersignal Step 1): the ONE function that
// scales how far a signal-driven value pulls a control away from its own
// manually-set/default value. Header-only, no JUCE dependency, so it is
// reachable from every consumer (ConnectionEngine::evaluate, MacroBank::
// updateValues, v1 MappingEngine::processFrame) without pulling in a heavier
// include. NOT applied on any GL/render thread (Boris Q2: effects/sources
// reading the beat clock or audio uniforms directly keep pulsing at 0%).
//
// depth == 1 -> driven (bit-identical to no fader at all, no interpolation
// arithmetic run at all -- the guard, not the lerp, is what makes 100%
// exact). depth == 0 -> manual (every signal-connected control sits at its
// hand value). In between -> linear blend. The >=1 / <=0 guards are load-
// bearing: an out-of-range OSC/REST/MIDI write degrades to 1/0 instead of
// over/under-shooting the blend.
inline float applyDepth(float manual, float driven, float depth) noexcept
{
    if (depth >= 1.0f) return driven;
    if (depth <= 0.0f) return manual;
    return manual + depth * (driven - manual);
}
