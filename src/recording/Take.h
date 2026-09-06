#pragma once
#include "model/ControlPath.h"
#include "recording/Lane.h"
#include "recording/TempoMap.h"
#include "recording/PerfState.h"
#include <juce_core/juce_core.h>
#include <map>
#include <vector>
#include <string>
#include <cstdint>
#include <optional>
#include <span>

// AudioRef -- D10.4's take.json "audio" section. The tap itself
// (src/recording/AudioTap.*) is step 2; this packet only needs the shape
// so Take round-trips it (empty/default here is a legitimate, un-recorded
// take -- "mode" stays empty, `segments` stays empty).
struct AudioRef
{
    struct Segment
    {
        std::string file;
        uint64_t firstSample = 0;
        uint64_t frames = 0;
        double rate = 0.0;
        int channels = 0;
        std::string sha1Head;

        juce::var toVar() const;
        static Segment fromVar(const juce::var& v);
    };

    std::vector<Segment> segments;
    std::string mode;             // "file" | "input" | "" (no audio captured)
    bool gapDetection = false;
    std::vector<std::pair<uint64_t, uint32_t>> gaps;
    std::optional<uint64_t> unreliableFrom;

    juce::var toVar() const;
    static AudioRef fromVar(const juce::var& v);
};

// Meta -- take.json's "meta" section (D12).
struct Meta
{
    std::string recordedAt;
    std::string app;
    double duration = 0.0;
    double durationBeats = 0.0;

    juce::var toVar() const;
    static Meta fromVar(const juce::var& v);
};

// LoadStats -- what Take::load reports back (D12's loader rules): unknown
// features/sections/kinds are counted and named, never silently dropped;
// a version this reader cannot read at all is refused, not half-played.
struct LoadStats
{
    bool refused = false;
    std::string refusalReason;
    std::vector<std::string> unknownFeatures;
    std::vector<std::string> unknownTopLevelSections;
    int unknownKindLanes = 0;
    std::vector<std::string> unknownKindNames;
    bool wasV1 = false;
};

// Take -- s167 D5: the DOCUMENT. A value type; lanes are the on-disk form
// (D3), not a chronological list -- Take::chronological() is a derived
// VIEW for a history list/debugging, never authoritative.
struct Take
{
    std::map<ControlPath, Lane> lanes;
    TempoMap tempo;
    PerfState checkpoint0;
    PerfState checkpointEnd;
    AudioRef audio;
    std::vector<DiscretePoint> markers;
    Meta meta;
    uint64_t nextSeq = 1;

    static constexpr int kFormatVersion = 2;
    static constexpr int kMinReader = 2;

    // Extra top-level sections this reader does not know (D12 rule 3):
    // kept opaque, re-saved verbatim, never interpreted.
    std::map<std::string, juce::var> unknownTopLevel;

    // Feature strings this reader does not recognise (still round-tripped
    // in `features` -- see toVar/fromVar).
    std::vector<std::string> unknownFeatures;

    // `folder` may be a directory (reads/writes "<folder>/take.json") or a
    // bare .json file (a fixture path) -- lets tests point straight at
    // tests/fixtures/*.json without staging a folder first.
    bool save(const juce::File& folder) const;
    static std::optional<Take> load(const juce::File& folder, LoadStats& stats);

    // Direct-from-var entry points (what save()/load() delegate to) --
    // exposed so tests can feed a fixture's parsed var without touching
    // the filesystem at all.
    juce::var toVar() const;
    static std::optional<Take> fromVar(const juce::var& root, LoadStats& stats);

    // Derived VIEW (D3): every discrete point and gesture-boundary across
    // every lane, in one chronological list, ordered by (t, seq). Never
    // authoritative -- for a history UI / debugging only.
    struct ChronoEntry { double t = 0.0; uint64_t seq = 0; ControlPath key; std::string label; };
    std::vector<ChronoEntry> chronological() const;

    // Edit ops NOW (D5); the rest (move/retime/split/merge/...) is LATER.
    void deletePoints(std::span<const uint64_t> seqs);
    void deleteLane(const ControlPath& key);

private:
    // v1 -> v2 bridge (D12 rule 5): the legacy SessionRecorder JSON shape
    // (G1 -- {"version":1,"events":[{"t","type",...}]}), read into a fresh
    // v2 Take. "No real v1 files exist beyond clip-trigger-only sessions"
    // (the July spec) -- ClipTrigger/ColumnTrigger get a faithful
    // conversion; the other five event types get a best-effort one.
    static std::optional<Take> fromV1Var(const juce::var& root, LoadStats& stats);
};
