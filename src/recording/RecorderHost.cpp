#include "recording/RecorderHost.h"
#include "recording/AudioTap.h"
#include "model/Composition.h"
#include "analysis/FeatureSnapshot.h"
#include <juce_events/juce_events.h>
#include <algorithm>
#include <cmath>

namespace
{
    // D-A9 (AudioStore.cpp): sample (take-clock) -> asset-frame, saturating
    // at 0 -- copied here (not exposed by AudioStore.h) because the
    // provisional/periodic saves need it too, not just finalize().
    uint64_t toAssetFrame(uint64_t sample, uint64_t firstSample)
    {
        return sample >= firstSample ? sample - firstSample : 0;
    }

    // R13 onset-pulse-loss fix: bound on markers emitted from a single tick's onsetCount delta
    // (see RecorderHost.h's onsetCountBaseline_ comment) -- at the real ~93.75 Hz analysis / 120 Hz
    // tick cadence, delta is 0 or 1 essentially always; this only guards a pathological tick stall.
    constexpr uint32_t kMaxOnsetMarkersPerTick = 8;

    const char* audioStatusName(AudioStore::Status s)
    {
        switch (s)
        {
            case AudioStore::Status::NoAudio:            return "NoAudio";
            case AudioStore::Status::Resolved:            return "Resolved";
            case AudioStore::Status::ResolvedUnverified:  return "ResolvedUnverified";
            case AudioStore::Status::Missing:             return "Missing";
            case AudioStore::Status::Incomplete:          return "Incomplete";
            case AudioStore::Status::Mismatch:            return "Mismatch";
            case AudioStore::Status::Legacy:              return "Legacy";
            case AudioStore::Status::MultiSegment:        return "MultiSegment";
        }
        return "Unknown";
    }
}

// s167 D5 (R7): message-thread only, same guard idiom as
// PerformanceRecorder.cpp -- lets headless tests call these directly with
// no MessageManager running, while still enforcing the invariant whenever
// one IS running.
#define RECORDER_HOST_ASSERT_MESSAGE_THREAD()                                   \
    jassert(juce::MessageManager::getInstanceWithoutCreating() == nullptr       \
            || juce::MessageManager::existsAndIsCurrentThread())

// ---- HostSink (D5): forwards Player's dispatch into `dispatch`, skips
// `audio` control points while replaying WithAudio (R-A7 -- the captured
// WAV already embodies every play/pause/stop; replaying `stop` would halt
// the very transport the Player is clocked from), and counts refusals
// (`skipped` for discrete, `continuousUnavailable` for an empty continuous
// seam) rather than ever failing silently (D2 policy 3). ----
struct RecorderHost::HostSink : Sink
{
    RecorderHost& host;
    PlayMode mode;
    HostSink(RecorderHost& h, PlayMode m) : host(h), mode(m) {}

    bool fire(const Fired& f) override
    {
        if (mode == PlayMode::WithAudio && f.key.control == "audio")
        {
            ++host.skippedCount_;
            return false;
        }
        if (!host.dispatch.fire)
        {
            ++host.skippedCount_;
            return false;
        }
        const bool ok = host.dispatch.fire(f);
        if (!ok)
            ++host.skippedCount_;
        return ok;
    }

    bool touch(const ControlPath& key, const std::string& grip) override
    {
        if (!host.dispatch.continuous.touch)
        {
            ++host.continuousUnavailableCount_;
            return false;
        }
        return host.dispatch.continuous.touch(key, grip);
    }

    bool set(const ControlPath& key, float v) override
    {
        if (!host.dispatch.continuous.set)
        {
            ++host.continuousUnavailableCount_;
            return false;
        }
        return host.dispatch.continuous.set(key, v);
    }

    void release(const ControlPath& key) override
    {
        if (host.dispatch.continuous.release)
            host.dispatch.continuous.release(key);
    }
};

RecorderHost::RecorderHost(AudioStore store) : store_(std::move(store)) {}
RecorderHost::~RecorderHost() = default;   // HostSink's complete definition is visible here (see header)

bool RecorderHost::isRecording() const { RECORDER_HOST_ASSERT_MESSAGE_THREAD(); return recording_; }
bool RecorderHost::isPlaying() const   { RECORDER_HOST_ASSERT_MESSAGE_THREAD(); return playing_; }

bool RecorderHost::needsTransportFrames() const
{
    RECORDER_HOST_ASSERT_MESSAGE_THREAD();
    return overdub_ || (playing_ && playMode_ == PlayMode::WithAudio);
}

AudioRef RecorderHost::liveAudioRef(AudioTap* tap) const
{
    if (overdub_ && overdubAsset_.has_value())
        return AudioStore::referencing(*overdubAsset_, 0);
    if (!tapWasStarted_ || tap == nullptr)
        return AudioRef{};   // 5.5: no audio captured -- take.audio stays default

    AudioAsset stub;
    stub.id = assetId_;
    stub.fingerprint.clear();   // R-A2: never fingerprinted before finalize() -- ResolvedUnverified territory
    stub.frames = tap->framesWritten();
    stub.rate = armedDeviceRate_;
    stub.channels = armedDeviceChannels_;
    stub.mode = audioMode_;
    stub.gapDetection = tap->gapDetectionSupported();

    const uint64_t firstSample = tap->firstSample();
    for (const auto& [sample, n] : gaps_)
        stub.gaps.emplace_back(toAssetFrame(sample, firstSample), n);
    if (auto u = tap->unreliableFrom())
        stub.unreliableFrom = toAssetFrame(*u, firstSample);

    return AudioStore::referencing(stub, firstSample);
}

RecorderHost::ArmResult RecorderHost::arm(const Composition& comp, AudioTap& tap, const ArmOptions& opts)
{
    RECORDER_HOST_ASSERT_MESSAGE_THREAD();
    ArmResult res;
    if (recording_)
    {
        res.error = "already recording";
        return res;
    }

    armedGripHoldMs_ = opts.gripHoldMs > 0.0f ? opts.gripHoldMs : 250.0f;
    // R13-C: opts.analysisRate is a deprecated no-op (see ArmOptions::analysisRate) -- never read.
    armedDeviceRate_ = opts.deviceRate;
    armedDeviceChannels_ = opts.deviceChannels;
    audioMode_ = opts.audioMode;
    onsetMarkers_ = opts.onsetMarkers;
    appVersion_ = opts.appVersion;
    takeFolder_ = opts.takeFolder;
    overdubAsset_.reset();
    overdub_ = false;
    tapWasStarted_ = false;
    tapWasRunningLastTick_ = false;
    gaps_.clear();
    markers_.clear();
    onsetCountBaseline_.reset();
    lastWriteWall_.clear();
    humanRefused_ = 0;
    skippedCount_ = 0;
    continuousUnavailableCount_ = 0;
    lastError_.clear();
    armRecordedAt_ = juce::Time::getCurrentTime().toISO8601(true).toStdString();

    // R13-C: rateChangedSinceArm always starts false at arm -- see tick()'s comment for the
    // ongoing comparison (deviceRate vs armedDeviceRate_, not vs a fixed "analysis rate": the
    // analysis thread resamples to its fixed internal rate regardless of device rate now,
    // AnalysisResampler/R13 lane A, so "rateMismatch" ("beat clock unreliable") is retired).
    rateChangedSinceArm_ = false;
    rateChangeNotified_ = false;
    lastDeviceRate_ = opts.deviceRate;

    if (opts.overdubAssetId.has_value())
    {
        // R-A6: overdub -- no tap; firstSample 0; clock = asset frame.
        auto found = store_.find(*opts.overdubAssetId);
        if (!found.has_value())
        {
            res.error = "overdub asset not found: " + *opts.overdubAssetId;
            return res;
        }
        overdubAsset_ = *found;
        overdub_ = true;
        assetId_ = found->id;
    }
    else if (opts.audio)
    {
        // R-A1: beginAsset -> tap.start(store.wavFile(id)) -> on failure abandonAsset + refuse
        // (nothing else touched).
        auto id = store_.beginAsset();
        if (!id.has_value())
        {
            res.error = "could not begin audio asset (store root unwritable?)";
            return res;
        }
        if (!tap.start(store_.wavFile(*id)))
        {
            store_.abandonAsset(*id);
            res.error = "AudioTap failed to start (disk / free-space?)";
            return res;
        }
        tapWasStarted_ = true;
        // BUGFIX (self-stop false positive): tap.start() only ARMS the tap -- AudioTap::running_
        // flips true inside push(), on the audio thread's NEXT callback, not here. Seeding this
        // true made the very first tick() after arm() (which can land on the message thread before
        // that first push()) read "was running last tick, not running now" and fire a false-positive
        // self-stop. Leaving it false means the self-stop edge in tick() only fires once a tick has
        // actually OBSERVED the tap running -- and since tick() re-sets this field from the tap's
        // real state every call (never leaves it stuck true after an observed stop), the same
        // edge-detector self-re-arms: a later genuine stop, after the tap is observed running again,
        // is reported again rather than swallowed.
        tapWasRunningLastTick_ = false;
        assetId_ = *id;
        liveGapDetection_ = tap.gapDetectionSupported();
        liveFramesWritten_ = 0;
    }
    else
    {
        // 5.5: no store, no tap, take.audio stays default.
        assetId_.clear();
    }

    recorder_.start(comp, clock_, takeFolder_);
    recording_ = true;
    lastCheckpointT_ = 0.0;

    const PerfState checkpoint0 = dispatch.capturePerfState ? dispatch.capturePerfState() : PerfState{};
    recorder_.setCheckpoint0(checkpoint0);

    // R-A1 provisional save (5.6 #1). Built from a COPY of the in-progress Take (current() is
    // read-only) plus this arm's audio/meta -- the same shape every later periodic/final save writes.
    Take provisional = recorder_.current();
    provisional.markers = markers_;
    provisional.audio = liveAudioRef(tapWasStarted_ ? &tap : nullptr);
    provisional.meta.recordedAt = armRecordedAt_;
    provisional.meta.app = appVersion_;

    res.ok = true;   // audio is running (or intentionally not requested) -- arm succeeds even if the
                      // provisional save itself fails; that failure is surfaced, not fatal (R-A1).
    res.assetId = assetId_;
    res.takeFolder = takeFolder_;

    if (!provisional.save(takeFolder_) && dispatch.notify)
        dispatch.notify("could not write provisional take.json");

    return res;
}

RecorderHost::StopResult RecorderHost::disarm(const Composition& comp, AudioTap& tap)
{
    RECORDER_HOST_ASSERT_MESSAGE_THREAD();
    StopResult res;
    if (!recording_)
    {
        res.error = "not recording";
        return res;
    }

    AudioRef finalRef;
    std::string finalizeError;

    if (overdub_ && overdubAsset_.has_value())
    {
        // R-A6: no tap; firstSample 0.
        finalRef = AudioStore::referencing(*overdubAsset_, 0);
    }
    else if (tapWasStarted_)
    {
        // R-A5: isRunning() read BEFORE stop().
        const bool endedEarly = !tap.isRunning();
        tap.stop();

        std::pair<uint64_t, uint32_t> gap;
        while (tap.popGap(gap))
            gaps_.push_back(gap);

        AudioStore::CaptureFacts facts;
        facts.mode = audioMode_;
        facts.gapDetection = tap.gapDetectionSupported();
        facts.firstSample = tap.firstSample();
        facts.framesWritten = tap.framesWritten();
        facts.rate = armedDeviceRate_;
        facts.channels = armedDeviceChannels_;
        facts.gapsInTakeClock = gaps_;
        if (endedEarly)
            facts.unreliableFromInTakeClock = tap.firstSample() + tap.framesWritten();
        else if (auto u = tap.unreliableFrom())
            facts.unreliableFromInTakeClock = u;
        facts.app = appVersion_;

        const auto fin = store_.finalize(assetId_, facts);
        if (!fin.error.empty())
        {
            finalizeError = fin.error;
            if (dispatch.notify)
                dispatch.notify(fin.error);
        }
        finalRef = AudioStore::referencing(fin.asset, tap.firstSample());
    }
    // else: 5.5, no audio -- finalRef stays default.

    Take take = recorder_.stop(comp);
    take.markers = markers_;
    take.audio = finalRef;   // R-A2: every save writes it via AudioStore::referencing, never blank
    take.meta.recordedAt = armRecordedAt_;
    take.meta.app = appVersion_;
    take.meta.duration = clock_.now().t;
    take.meta.durationBeats = clock_.now().beat;

    if (dispatch.capturePerfState)
        take.checkpointEnd = dispatch.capturePerfState();

    const bool saved = take.save(takeFolder_);
    if (!saved && dispatch.notify)
        dispatch.notify("could not save take.json at stop");

    res.ok = saved;                 // D-A11: `error` names finalize/save problems but NEVER blanks the reference
    res.error = finalizeError;
    res.takeFolder = takeFolder_;
    res.assetId = finalRef.segments.empty() ? assetId_ : finalRef.segments[0].id;
    res.frames = finalRef.segments.empty() ? 0 : finalRef.segments[0].frames;
    res.gapDetection = finalRef.gapDetection;
    res.gaps = static_cast<int>(finalRef.gaps.size());
    res.lanes = static_cast<int>(take.lanes.size());
    int points = 0;
    for (const auto& [key, lane] : take.lanes)
        points += lane.kind == Lane::Kind::Discrete ? static_cast<int>(lane.points.size())
                                                      : static_cast<int>(lane.gestures.size());
    res.points = points;
    res.duration = take.meta.duration;

    recording_ = false;
    overdub_ = false;
    tapWasStarted_ = false;
    lastCheckpointT_ = 0.0;
    gaps_.clear();
    markers_.clear();
    onsetCountBaseline_.reset();
    lastWriteWall_.clear();

    publishStatus();
    return res;
}

void RecorderHost::tick(const FeatureSnapshot& snap, double wallNow, uint64_t deliveredSamples,
                         AudioTap& tap, std::optional<int64_t> transportFrames, double deviceRate)
{
    RECORDER_HOST_ASSERT_MESSAGE_THREAD();

    lastDeviceRate_ = deviceRate;

    // R13-C: a Status fact, current every tick (recording or not) -- true whenever the device
    // rate no longer matches the rate armed with. The analysis thread resamples to its fixed
    // internal rate regardless of device rate now (AnalysisResampler, R13 lane A), so this is
    // no longer about beat-clock correctness -- it is the only rate hazard that survives R13:
    // sample stamps before/after a mid-take change are in different domains. rateChangeNotified_
    // is separate from rateChangedSinceArm_ so a flapping rate (change -> back to the armed rate
    // -> change again) still notifies exactly ONCE per arm, even though rateChangedSinceArm_
    // itself can flip back to false if the device recovers to the armed rate.
    rateChangedSinceArm_ = deviceRate > 0.0 && armedDeviceRate_ > 0.0
        && std::abs(deviceRate - armedDeviceRate_) > 0.5;

    if (recording_ && rateChangedSinceArm_ && !rateChangeNotified_)
    {
        lastError_ = "device sample rate changed mid-take: "
                     + std::to_string(static_cast<int>(std::lround(armedDeviceRate_))) + " -> "
                     + std::to_string(static_cast<int>(std::lround(deviceRate)))
                     + " Hz; sample stamps after this point are mixed-domain (audio tap stopped if it was running)";
        rateChangeNotified_ = true;
        if (dispatch.notify)
            dispatch.notify(lastError_);
    }

    // (1) sample domain for the clock: normally the absolute delivered-sample counter; while
    // overdubbing, the asset-frame conversion (5.2 formula) so the recorded lane's `sample` stamps
    // land in the SAME domain the loaded asset is addressed in.
    uint64_t sampleForClock = deliveredSamples;
    if (overdub_ && transportFrames.has_value() && overdubAsset_.has_value() && deviceRate > 0.0)
    {
        sampleForClock = static_cast<uint64_t>(std::llround(
            static_cast<double>(*transportFrames) * overdubAsset_->rate / deviceRate));
    }
    clock_.tick(snap, wallNow, sampleForClock);

    // (2) recording-side bookkeeping.
    if (recording_)
    {
        if (tapWasStarted_)
        {
            // Rising-to-falling edge on the tap's OWN observed running state (never seeded true at
            // arm -- see arm()'s comment): fires once per genuine running -> stopped transition. Since
            // `tapWasRunningLastTick_` is re-set from the tap's real state every tick below (including
            // after a fire), the edge self-re-arms -- if the tap is later observed running again (a
            // device reconnect), a subsequent genuine stop is reported again, not swallowed.
            // `lastError_` is NOT cleared on recovery; it simply gets overwritten with the (same-shaped)
            // message the next time this branch fires, so status().lastError always reflects the MOST
            // RECENT self-stop, not a stale first one.
            const bool runningNow = tap.isRunning();
            if (tapWasRunningLastTick_ && !runningNow)
            {
                lastError_ = "audio tap self-stopped mid-take (device rate/channel change?) -- "
                              "sample stamps after this point are unreliable";
                if (dispatch.notify)
                    dispatch.notify(lastError_);
            }
            tapWasRunningLastTick_ = runningNow;

            std::pair<uint64_t, uint32_t> gap;
            while (tap.popGap(gap))   // R-A4: drained EVERY tick while recording
                gaps_.push_back(gap);

            liveGapDetection_ = tap.gapDetectionSupported();
            liveFramesWritten_ = tap.framesWritten();
        }

        // T2, R13 onset-pulse-loss fix: one marker per onset EVENT, tracked via the monotonic
        // FeatureSnapshot::onsetCount delta rather than onsetDetected/timestamp -- see
        // onsetCountBaseline_'s comment in the header for the full design.
        if (onsetMarkers_)
        {
            if (!onsetCountBaseline_.has_value())
            {
                // First snapshot observed after arm -- establishes the baseline. Never emits for
                // onsets that happened before this arm.
                onsetCountBaseline_ = snap.onsetCount;
            }
            else
            {
                const uint32_t delta = snap.onsetCount - *onsetCountBaseline_;  // unsigned, wrap-safe
                if (delta > 0)
                {
                    const uint32_t n = std::min(delta, kMaxOnsetMarkersPerTick);
                    for (uint32_t i = 0; i < n; ++i)
                        marker("onset");
                    onsetCountBaseline_ = *onsetCountBaseline_ + n;   // advance by n only -- any
                                                                       // excess (delta > n) is picked
                                                                       // up on a later tick, never lost
                }
            }
        }

        synthesizeIdleDecayingEnds(wallNow);

        const auto now = clock_.now();
        if (now.t - lastCheckpointT_ >= kCheckpointSeconds)
        {
            // R-A3: recorder_.current() copied, plus audio/meta/checkpoint0 (checkpoint0 already
            // lives inside current() -- setCheckpoint0() wrote it straight into the recorder's
            // in-progress take_ at arm time).
            Take snapshot = recorder_.current();
            snapshot.markers = markers_;
            snapshot.audio = liveAudioRef(tapWasStarted_ ? &tap : nullptr);
            snapshot.meta.recordedAt = armRecordedAt_;
            snapshot.meta.app = appVersion_;
            snapshot.meta.duration = now.t;
            snapshot.meta.durationBeats = now.beat;
            if (!snapshot.save(takeFolder_) && dispatch.notify)
                dispatch.notify("periodic take save failed");
            lastCheckpointT_ = now.t;
        }
    }

    // (3) playback.
    if (playing_ && player_ && sink_)
    {
        double pos;
        if (playMode_ == PlayMode::WithAudio && transportFrames.has_value())
            pos = static_cast<double>(playFirstSample_) + static_cast<double>(*transportFrames);
        else
            pos = wallNow - playStartWall_;
        player_->advanceTo(pos, *sink_);
    }

    // (4) publish.
    publishStatus();
}

void RecorderHost::synthesizeIdleDecayingEnds(double wallNow)
{
    const double holdSeconds = static_cast<double>(armedGripHoldMs_) / 1000.0;
    std::vector<ControlPath> expired;
    for (const auto& [key, info] : lastWriteWall_)
    {
        const auto& [lastWall, grip] = info;
        if (grip != "decaying")
            continue;
        if (wallNow - lastWall >= holdSeconds)
            expired.push_back(key);
    }
    for (const auto& key : expired)
    {
        recorder_.release(key);   // finishGesture appends the exact end breakpoint (PerformanceRecorder.cpp)
        lastWriteWall_.erase(key);
    }
}

void RecorderHost::capture(const ControlPath& key, DiscretePoint&& p)
{
    RECORDER_HOST_ASSERT_MESSAGE_THREAD();
    if (!recording_ || p.origin == Origin::Replay)   // D6 origin rule; R5 overdub feedback closed by construction
        return;
    recorder_.discrete(key, std::move(p));
}

uint64_t RecorderHost::nextGroupId()
{
    RECORDER_HOST_ASSERT_MESSAGE_THREAD();
    return nextGroupId_++;
}

void RecorderHost::onHumanTouch(const ControlPath& key, const std::string& grip)
{
    RECORDER_HOST_ASSERT_MESSAGE_THREAD();
    if (!recording_)
        return;
    recorder_.touch(key, grip);
    lastWriteWall_[key] = { juce::Time::getMillisecondCounterHiRes() / 1000.0, grip };
}

void RecorderHost::onHumanWrite(const ControlPath& key, float v, const std::string& grip)
{
    RECORDER_HOST_ASSERT_MESSAGE_THREAD();
    if (!recording_)
        return;
    // critic B2: every OSC/MIDI/REST manual-write site is a single manualWrite call with no
    // preceding manualTouch -- open the gesture here if none is open, or
    // PerformanceRecorder::set's "stray set() without a preceding touch() -- ignored" rule would
    // silently drop every one of those captures.
    if (!recorder_.hasOpenGesture(key))
        recorder_.touch(key, grip);
    recorder_.set(key, v);
    lastWriteWall_[key] = { juce::Time::getMillisecondCounterHiRes() / 1000.0, grip };
}

void RecorderHost::onHumanRelease(const ControlPath& key)
{
    RECORDER_HOST_ASSERT_MESSAGE_THREAD();
    if (!recording_)
        return;
    recorder_.release(key);
    lastWriteWall_.erase(key);
}

void RecorderHost::noteHumanRefused(const ControlPath&)
{
    RECORDER_HOST_ASSERT_MESSAGE_THREAD();
    ++humanRefused_;   // N12: first refusal diagnostic the funnel has ever had
}

void RecorderHost::marker(const std::string& action)
{
    RECORDER_HOST_ASSERT_MESSAGE_THREAD();
    if (!recording_)
        return;
    const auto stamp = clock_.now();
    DiscretePoint p;
    p.s.seq = markerSeq_++;
    p.s.t = stamp.t;
    p.s.sample = stamp.sample;
    p.beat = stamp.beat;
    p.bpm = stamp.bpm;
    p.origin = Origin::Engine;
    p.action = action;
    markers_.push_back(std::move(p));
}

RecorderHost::LoadResult RecorderHost::load(const juce::File& takeFolder)
{
    RECORDER_HOST_ASSERT_MESSAGE_THREAD();
    LoadResult res;
    LoadStats stats;
    auto loaded = Take::load(takeFolder, stats);
    res.stats = stats;
    if (!loaded.has_value())
    {
        res.ok = false;
        res.error = stats.refused ? stats.refusalReason : "could not load take from " + takeFolder.getFullPathName().toStdString();
        return res;
    }

    loadedTake_ = std::move(*loaded);
    loadStats_ = stats;
    loadedAudio_ = store_.resolve(loadedTake_->audio);
    res.audio = loadedAudio_;
    res.ok = true;
    return res;
}

RecorderHost::PlayResult RecorderHost::play(PlayMode mode, const Composition& comp)
{
    RECORDER_HOST_ASSERT_MESSAGE_THREAD();
    PlayResult res;
    if (!loadedTake_.has_value())
    {
        res.error = "no take loaded";
        return res;
    }
    if (playing_)
    {
        res.error = "already playing";
        return res;
    }
    if (mode == PlayMode::WithAudio
        && loadedAudio_.status != AudioStore::Status::Resolved
        && loadedAudio_.status != AudioStore::Status::ResolvedUnverified)
    {
        res.error = "audio not resolved: " + loadedAudio_.reason;
        return res;
    }

    const DriveClock clock = (mode == PlayMode::WithAudio) ? DriveClock::Sample : DriveClock::Wall;
    program_ = compile(*loadedTake_, comp, clock);
    sink_ = std::make_unique<HostSink>(*this, mode);
    player_ = std::make_unique<Player>(program_);

    playMode_ = mode;
    playStartWall_ = juce::Time::getMillisecondCounterHiRes() / 1000.0;
    playFirstSample_ = 0;
    playAssetRate_ = 0.0;
    skippedCount_ = 0;
    continuousUnavailableCount_ = 0;

    if (mode == PlayMode::WithAudio)
    {
        res.wav = loadedAudio_.wav;
        res.assetRate = loadedAudio_.asset.rate;
        res.firstSample = loadedAudio_.firstSample;
        playFirstSample_ = loadedAudio_.firstSample;
        playAssetRate_ = loadedAudio_.asset.rate;
    }

    player_->start(0.0);
    playing_ = true;

    res.ok = true;
    res.report = program_->report;
    publishStatus();
    return res;
}

void RecorderHost::stopPlay()
{
    RECORDER_HOST_ASSERT_MESSAGE_THREAD();
    if (!playing_ || !player_ || !sink_)
        return;
    player_->stop(*sink_);   // R9: every touch released
    playing_ = false;
    player_.reset();
    sink_.reset();
    program_.reset();
    publishStatus();
}

std::string RecorderHost::repairLoadedAudio(const std::string& appVersion)
{
    RECORDER_HOST_ASSERT_MESSAGE_THREAD();
    if (!loadedTake_.has_value())
        return "no take loaded";
    if (loadedTake_->audio.segments.empty())
        return "take has no audio segment to repair";

    auto facts = AudioStore::CaptureFacts::fromAudioRef(loadedTake_->audio, appVersion);
    if (!facts.has_value())
        return "take's audio ref is not repairable (multi-segment or malformed)";

    const std::string id = loadedTake_->audio.segments[0].id;
    const auto fin = store_.finalize(id, *facts);
    loadedAudio_ = store_.resolve(loadedTake_->audio);
    return fin.error;
}

void RecorderHost::publishStatus()
{
    Status s;
    s.recording = recording_;
    s.playing = playing_;
    s.overdub = overdub_;
    s.takeFolder = takeFolder_.getFullPathName().toStdString();
    s.assetId = assetId_;
    s.audioMode = audioMode_;
    s.playMode = playing_ ? (playMode_ == PlayMode::WithAudio ? "withAudio" : "wallClock") : "";
    s.lastError = lastError_;

    if (recording_)
    {
        const auto now = clock_.now();
        s.t = now.t;
        s.beat = now.beat;
        s.sample = now.sample;
        s.bpm = now.bpm;

        const Take& cur = recorder_.current();
        s.lanes = static_cast<int>(cur.lanes.size());
        int points = 0, gestures = 0;
        for (const auto& [key, lane] : cur.lanes)
        {
            if (lane.kind == Lane::Kind::Discrete)
                points += static_cast<int>(lane.points.size());
            else if (lane.kind == Lane::Kind::Continuous)
                gestures += static_cast<int>(lane.gestures.size());
        }
        s.points = points;
        s.gestures = gestures;
        s.markers = static_cast<int>(markers_.size());
        s.gapDetection = tapWasStarted_ ? liveGapDetection_ : false;
        s.framesWritten = tapWasStarted_ ? liveFramesWritten_ : 0;
        s.gaps = static_cast<int>(gaps_.size());
    }

    if (playing_ && player_)
    {
        s.position = player_->position();
        s.length = program_ ? program_->length : 0.0;
        if (program_)
        {
            const auto& rep = program_->report;
            s.unresolved = static_cast<int>(rep.unresolved.size());
            s.reboundByPosition = static_cast<int>(rep.reboundByPosition.size());
            s.reboundByName = static_cast<int>(rep.reboundByName.size());
            s.invalid = static_cast<int>(rep.invalid.size());
        }
    }

    if (loadedTake_.has_value())
        s.audioStatus = audioStatusName(loadedAudio_.status);

    s.skipped = skippedCount_;
    s.continuousUnavailable = continuousUnavailableCount_;
    s.refusedByHand = 0;   // reserved: not distinguished from continuousUnavailable in this lane
    s.deviceRate = lastDeviceRate_;
    s.rateChangedSinceArm = rateChangedSinceArm_;
    s.rateMismatch = rateChangedSinceArm_;   // DEPRECATED mirror -- see Status::rateMismatch comment
    s.humanRefused = humanRefused_;

    std::lock_guard<std::mutex> lock(statusMutex_);
    published_ = s;
}

RecorderHost::Status RecorderHost::status() const
{
    std::lock_guard<std::mutex> lock(statusMutex_);
    return published_;
}

void RecorderHost::shutdown(const Composition& comp, AudioTap& tap)
{
    RECORDER_HOST_ASSERT_MESSAGE_THREAD();
    if (playing_)
        stopPlay();
    if (recording_)
        disarm(comp, tap);
}
