#pragma once
#include "recording/PerformanceRecorder.h"
#include "recording/RecorderClock.h"
#include "recording/AudioStore.h"
#include "recording/Player.h"
#include "recording/Program.h"
#include <juce_core/juce_core.h>
#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <utility>

struct Composition;
struct FeatureSnapshot;
class AudioTap;   // N13: forward-declared -- RecorderHost.cpp includes recording/AudioTap.h; this
                   // header stays juce_core-only (does not pull juce_audio_formats/juce_audio_devices).

// RecorderHost -- s-rta-0923 step 3 / s-rta-0924 Lane S3-A. Message-thread
// only (same jassert idiom as PerformanceRecorder.cpp). Owns the clock,
// recorder, store, and player and the whole arm -> tick -> disarm
// lifecycle; talks to the app ONLY through `dispatch`, a struct of
// std::functions MainComponent (Lane S3-B) fills in. Never includes
// MainComponent.h, anything under src/ui/, src/connect/, or src/render/.
//
// AMENDMENTS applied here (s-rta-0924 critic, .harmony/.reports/s-rta-0924/
// step3-critic.md): A2 -- the continuous capture hooks are onHumanTouch/
// onHumanWrite/onHumanRelease/noteHumanRefused with a string grip
// ("held"|"decaying", Lane.h's Gesture::grip vocabulary); onHumanWrite OPENS
// a gesture when none is open (every OSC/MIDI/REST manual-write site is a
// single manualWrite call with no preceding manualTouch -- see
// PerformanceRecorder::set's "stray set()" comment). No `GripKind` enum
// lives here (N1) -- the recorder side has always spoken strings
// (Sink::touch, PerformanceRecorder::touch, Gesture::grip); MainComponent's
// own `GripKind` (ParamConnection::Grip::Kind) is a different enum with a
// third (None) value and would only shadow confusingly if duplicated here.
// A5/R13-C -- ArmOptions carries `gripHoldMs` (`analysisRate` is a deprecated
// no-op, kept only so MainComponent.cpp's pre-lane-D `armOpts.analysisRate =
// ...` call keeps compiling -- arm()/tick() never read it); Status carries
// `deviceRate`/`rateChangedSinceArm`/`humanRefused` (`rateMismatch` is a
// deprecated mirror of `rateChangedSinceArm`, kept for the same reason),
// published by tick(). A7 -- AudioTap is forward-declared, not included.
class RecorderHost
{
public:
    // ---- what the app plugs in (MainComponent, Lane S3-B); every function runs on the message thread ----
    struct Dispatch
    {
        // Discrete replay. The host only ever dispatches Origin::Replay; MainComponent's handler
        // re-resolves f.target by coordinate (G7) and calls the SAME handler the human path uses.
        // Returns accepted (false = refused/unresolvable at fire time -> counted as `skipped`).
        std::function<bool(const Fired& f)> fire;

        // Continuous replay -- filled by Lane S3-B by wrapping MainComponent's shipped
        // manualTouch/manualWrite/manualRelease funnel (R8: the connection lane owns it,
        // src/MainComponent.h manualWrite/manualTouch/manualRelease). Left EMPTY only in a
        // headless test/FakeDispatch: every touch/set then returns false, release is a no-op,
        // and Status::continuousUnavailable counts every refusal -- never a stub of manualWrite.
        struct Continuous
        {
            std::function<bool(const ControlPath&, const std::string& grip)> touch;
            std::function<bool(const ControlPath&, float v)> set;      // v normalised [0,1]
            std::function<void(const ControlPath&)> release;
        } continuous;

        // checkpoint0 / checkpointEnd (D4) from the live model -- implemented by MainComponent via
        // PerfStateCapture (3.2). Called at arm and at stop.
        std::function<PerfState()> capturePerfState;

        // One-line human-readable notices (status line / log). Never a modal.
        std::function<void(const std::string&)> notify;
    };
    Dispatch dispatch;

    explicit RecorderHost(AudioStore store);   // MainComponent passes AudioStore(AudioStore::defaultRoot())
    // Declared (not defaulted here) and defined in the .cpp: `player_`/`sink_` are
    // std::unique_ptr<...> to a type (HostSink) only forward-declared in this header --
    // the implicit inline destructor would need HostSink's complete definition at every
    // call site that constructs a RecorderHost (every TU including this header), which
    // is exactly what the forward declaration exists to avoid.
    ~RecorderHost();

    // ---- recording lifecycle (v2 spec 5.1 / 5.2 / 5.5 / 5.6) ----
    struct ArmOptions
    {
        juce::File takeFolder;                 // ~/Documents/Audio-DNA/Takes/<name>.adna-take (MainComponent picks)
        bool audio = true;                     // ruling 19's switch; default ON
        std::string audioMode;                 // "input" | "file" -- AudioEngine::getSourceMode() at arm
        double deviceRate = 0.0;               // AudioEngine::getCurrentSampleRate() at arm (CaptureFacts.rate)
        int deviceChannels = 0;                // active output channels at arm (CaptureFacts.channels)
        std::string appVersion;                // "0.1.0" (Main.cpp:8)
        bool onsetMarkers = false;             // T2 enabler: take.markers point per snap.onsetDetected (origin Engine)
        std::optional<std::string> overdubAssetId;   // 5.2: record a FRESH take against a stored asset; NO tap; clock = asset frame

        // DEPRECATED (R13-C): retired. The analysis thread now resamples the
        // device stream to its fixed internal rate regardless of the device
        // rate (AnalysisResampler, R13 lane A), so comparing deviceRate
        // against an "analysis rate" no longer means anything. This field is
        // kept ONLY so MainComponent.cpp's existing `armOpts.analysisRate =
        // ...` call (removed by lane D) keeps compiling -- arm() never reads
        // it. See `deviceRate` above / `Status::rateChangedSinceArm` below
        // for the rate hazard that survives R13.
        double analysisRate = 48000.0;
        // A5/N7: composition_.gripHoldMs -- the synthesized end of a
        // Decaying gesture with no release event uses THIS, not a hard
        // constant, so it matches the engine's own expiry
        // (ConnectionEngine::Context::gripHoldMs).
        float gripHoldMs = 250.0f;
    };
    struct ArmResult { bool ok = false; std::string error; std::string assetId; juce::File takeFolder; };
    // Sequence (5.1): beginAsset -> tap.start(store.wavFile(id)) [refuse + abandonAsset on failure] ->
    // recorder.start -> checkpoint0 = dispatch.capturePerfState() -> PROVISIONAL take.json (5.6 #1) with
    // audio = referencing(stub{id, fingerprint "", frames 0, rate, ch, mode, gapDetection = tap.gapDetectionSupported()}, 0).
    // With audio == false: 5.5 (no store, no tap, take.audio default). With overdubAssetId: 5.2
    // (store.find -> refuse with reason if absent; take.audio = referencing(*asset, 0); no tap).
    ArmResult arm(const Composition& comp, AudioTap& tap, const ArmOptions& opts);

    struct StopResult
    {
        bool ok = false; std::string error;            // error = finalize/save problems, NEVER blanks the reference (D-A11)
        juce::File takeFolder; std::string assetId; uint64_t frames = 0; bool gapDetection = false;
        int gaps = 0; int lanes = 0; int points = 0; double duration = 0.0;
    };
    // Sequence (5.1): endedEarly = tapWasStarted && !tap.isRunning(); tap.stop(); facts{...} incl. the
    // per-tick-drained gaps; if endedEarly: unreliableFrom = min(existing, firstSample+framesWritten);
    // fin = store.finalize(id, facts); take = recorder.stop(comp); take.checkpointEnd =
    // dispatch.capturePerfState(); take.audio = referencing(fin.asset, tap.firstSample()) [EVERY branch];
    // meta{recordedAt, app, duration, durationBeats}; take.save(folder); surface fin.error via notify.
    StopResult disarm(const Composition& comp, AudioTap& tap);
    bool isRecording() const;

    // Cheap check for B1's tick hunk (S3-B): whether tick() needs a
    // transportFrames reading this call (avoids a status()-under-mutex copy
    // every 120 Hz tick just to answer this).
    bool needsTransportFrames() const;

    // ---- the 120 Hz tick (FIRST thing in tickFeaturePipeline -- 3.3) ----
    // deliveredSamples: AudioEngine::getDeliveredSamples(). transportFrames: AudioTransportSource::
    // getNextReadPosition() when the audio transport is the clock (play-with-audio, overdub), else
    // nullopt. deviceRate: for the 5.2 asset-frame conversion AND A5's rate-mismatch publish. Does,
    // in order: (1) clock.tick(snap, wallNow, sampleForClock) where sampleForClock = deliveredSamples
    // normally, or the asset frame (llround(transportFrames * asset.rate / deviceRate)) while
    // overdubbing (5.2 formula); (2) while recording: drain tap.popGap into facts (5.6 #2), onset
    // marker if armed and snap.onsetDetected, synthesize an exact end for any Decaying gesture idle
    // longer than the armed gripHoldMs (N7), periodic save every kCheckpointSeconds of clock t
    // (5.6 #1); (3) while playing: pos = wall or (assetFrame + firstSample) per DriveClock;
    // player.advanceTo(pos, sink) -- backward pos is Player's own seek (T21); (4) publish Status
    // under the mutex.
    void tick(const FeatureSnapshot& snap, double wallNow, uint64_t deliveredSamples,
              AudioTap& tap, std::optional<int64_t> transportFrames, double deviceRate);

    // ---- capture (called by MainComponent's choke points; message thread) ----
    // Discrete: the caller fills v/action/retrigger/origin/group; stamps are minted here from the
    // clock (PerformanceRecorder::discrete). Ignored unless recording AND origin != Replay (D6 origin
    // rule; R5 overdub feedback is closed by construction, not by the caller remembering).
    void capture(const ControlPath& key, DiscretePoint&& p);
    uint64_t nextGroupId();                              // shared `group` for a column trigger's points

    // Continuous (critic A2/B2): MainComponent's onManualWrite/onManualTouch/onManualRelease
    // subscribers call these (origin Human, accepted only). `grip` is "held" | "decaying"
    // (Lane.h Gesture::grip vocabulary). onHumanWrite OPENS the gesture if none is open on `key`.
    void onHumanTouch(const ControlPath& key, const std::string& grip);
    void onHumanWrite(const ControlPath& key, float v, const std::string& grip);
    void onHumanRelease(const ControlPath& key);
    void noteHumanRefused(const ControlPath& key);        // Status.humanRefused++ (N12: first refusal diagnostic)
    void marker(const std::string& action);               // take.markers, origin Engine (T2)

    // ---- playback (5.4) ----
    struct LoadResult { bool ok = false; std::string error; LoadStats stats; AudioStore::Resolution audio; };
    LoadResult load(const juce::File& takeFolder);       // Take::load + store.resolve; refusal reasons verbatim (spec 6)
    enum class PlayMode { WallClock, WithAudio };
    struct PlayResult
    {
        bool ok = false; std::string error;
        // WithAudio: MainComponent must loadFile(wav) in File mode and play() BEFORE the first tick with
        // transportFrames; the host compiles DriveClock::Sample and expects assetFrame from tick().
        juce::File wav; double assetRate = 0.0; uint64_t firstSample = 0;
        CompileReport report;
    };
    PlayResult play(PlayMode mode, const Composition& comp);   // compile + Player::start(0)
    void stopPlay();                                           // Player::stop(sink) -> every touch released (R9)
    bool isPlaying() const;
    // Repair (5.4 Incomplete): store.finalize(id, *CaptureFacts::fromAudioRef(loaded.audio, app))
    std::string repairLoadedAudio(const std::string& appVersion);

    // ---- status (ANY thread; mutex-guarded copy published by tick()) ----
    struct Status
    {
        bool recording = false, playing = false, overdub = false;
        std::string takeFolder, assetId, audioMode, playMode, lastError;
        double t = 0.0, beat = 0.0; uint64_t sample = 0; float bpm = 0.0f;
        int lanes = 0, points = 0, gestures = 0, markers = 0;
        bool gapDetection = false; uint64_t framesWritten = 0; int gaps = 0;
        // playback
        double position = 0.0, length = 0.0;
        int unresolved = 0, reboundByPosition = 0, reboundByName = 0, invalid = 0;
        int skipped = 0, continuousUnavailable = 0, refusedByHand = 0;
        std::string audioStatus;   // AudioStore::Status name + reason

        // A5/R13-C: published by tick() from the `deviceRate` argument it already
        // receives (critic N3: never read the device directly off the HTTP thread).
        double deviceRate = 0.0;
        // R13-C: true when the device rate has changed since arm() (compared against
        // `armedDeviceRate_`, not a fixed "analysis rate" -- the analysis thread now
        // resamples to its fixed internal rate regardless, AnalysisResampler/R13 lane A).
        // This is the only rate hazard that survives R13: sample stamps recorded before
        // and after the change are in different domains, so a take spanning the change
        // is mixed-domain past that point. "rateMismatch" ("device != 48 kHz, beat clock
        // unreliable") is RETIRED -- the beat clock is correct at any device rate now.
        bool rateChangedSinceArm = false;
        // DEPRECATED (R13-C): mirrors `rateChangedSinceArm` (same value, not the old
        // "device != 48000" meaning). Kept ONLY so MainComponent.cpp's existing
        // `s.rateMismatch` read (renamed by lane D) keeps compiling.
        bool rateMismatch = false;
        // N12: count of Human writes MainComponent's funnel refused (a Held grip already
        // holds the control) -- the first diagnostic the funnel has ever had.
        int humanRefused = 0;
    };
    Status status() const;

    // MainComponent's destructor calls this FIRST (before the audio device closes): disarm if
    // recording (flush + finalize + save), stopPlay if playing.
    void shutdown(const Composition& comp, AudioTap& tap);

    static constexpr double kCheckpointSeconds = 60.0;   // 5.6 #1 periodic Take::save

private:
    struct HostSink;                                     // implements Sink; forwards to `dispatch`; skips `audio`
                                                           // points while PlayMode::WithAudio (nested class -- has
                                                           // access to RecorderHost's private members, no friend needed)
    void publishStatus();
    void synthesizeIdleDecayingEnds(double wallNow);
    // Un-finalized (no real fingerprint) AudioRef for the provisional/periodic saves (R-A2): built
    // fresh from the live tap counters + accumulated `gaps_` every time, via AudioStore::referencing
    // (never hand-rolled) so every save shares ONE conversion path with the finalized ref disarm()
    // builds. `tap` is nullptr for the no-audio (5.5) branch; overdub ignores `tap` entirely (R-A6).
    AudioRef liveAudioRef(AudioTap* tap) const;

    AudioStore store_;
    RecorderClock clock_;
    PerformanceRecorder recorder_;

    bool recording_ = false;
    bool overdub_ = false;
    bool tapWasStarted_ = false;
    // Edge-detector for tick()'s self-stop check (RecorderHost.cpp): the tap's own `running_` state
    // as observed on the PREVIOUS tick. Always seeded/reset false -- never true -- at arm(), because
    // tap.start() only ARMS the tap; AudioTap::running_ flips true inside push(), on the audio
    // thread's next callback, not synchronously with start(). Re-set from the tap's real state every
    // tick (see tick()'s own comment), so a stop -> recover -> stop sequence re-arms itself with no
    // extra state needed.
    bool tapWasRunningLastTick_ = false;
    juce::File takeFolder_;
    std::string assetId_;
    std::string audioMode_;
    bool onsetMarkers_ = false;
    std::string appVersion_;
    std::string armRecordedAt_;
    float armedGripHoldMs_ = 250.0f;
    double armedDeviceRate_ = 0.0;
    int armedDeviceChannels_ = 0;
    double lastCheckpointT_ = 0.0;
    std::optional<AudioAsset> overdubAsset_;
    std::string lastError_;

    // Gaps accumulated (take-clock/absolute-sample domain, per AudioTap::popGap /
    // CombinedCallback's `deliveredBefore`) since arm(); drained into the finalized ref at disarm().
    std::vector<std::pair<uint64_t, uint32_t>> gaps_;
    bool liveGapDetection_ = true;
    uint64_t liveFramesWritten_ = 0;

    // Onset markers (T2, opt-in via ArmOptions::onsetMarkers) -- kept host-side rather than a 4th
    // PerformanceRecorder method (the packet's scope is 3 one-liners there); folded into every
    // saved Take's `markers` field at provisional/periodic/final save time. Own seq space (T2 is
    // additive and markers are never part of `take.lanes`'s k-way merge, so this never collides
    // with PerformanceRecorder's own take_.nextSeq).
    std::vector<DiscretePoint> markers_;
    uint64_t markerSeq_ = 1;
    uint64_t nextGroupId_ = 1;

    // Onset-marker dedupe (R13 onset-pulse-loss fix). FeatureSnapshot::onsetCount is a monotonic
    // per-hop counter (AnalysisThread increments it once per detected onset, independent of the
    // 120 Hz tick or FeatureBus::read()'s always-latest semantics) -- comparing consecutive counts,
    // not FeatureSnapshot::timestamp, means a hop published while no tick was looking is never
    // silently lost (the defect the old timestamp dedupe had: onsets between two reads of the same
    // snapshot were invisible; onsets published and then overwritten before any tick read them were
    // ALSO invisible, which the timestamp scheme could never detect at all). onsetCountBaseline_
    // starts unset; the FIRST snapshot observed after arm establishes the baseline with zero markers
    // emitted for it (never emit for onsets that happened before arm -- AnalysisThread's counter is
    // a process-lifetime value, unrelated to per-take state). Each tick with onsetMarkers_ enabled
    // emits min(delta, kMaxOnsetMarkersPerTick) markers, where delta = snap.onsetCount -
    // *onsetCountBaseline_ (unsigned subtraction -- stays correct across a wrap past 2^32), and
    // advances the baseline by exactly the number emitted (not to snap.onsetCount) -- so a tick that
    // hits the cap loses nothing; the excess is picked up on a later tick. A tick that emits n>1
    // markers stamps all n with that tick's clock time (marker()'s existing tick-time "late point"
    // semantics, spec D10.3 T1); the probe's grid pairing tolerates this. Reset wherever per-take
    // state resets (arm() / markers_ clear).
    std::optional<uint32_t> onsetCountBaseline_;

    // Continuous-gesture idle tracking for the synthesized Decaying end (N7): last wall-clock write
    // time + grip per key currently open on the recorder side; only "decaying" entries expire here
    // (a Held grip has no timeout -- it stays held until an explicit release).
    std::map<ControlPath, std::pair<double, std::string>> lastWriteWall_;
    int humanRefused_ = 0;

    // Loaded take (playback, 5.4)
    std::optional<Take> loadedTake_;
    LoadStats loadStats_;
    AudioStore::Resolution loadedAudio_;

    // Playback
    std::shared_ptr<const Program> program_;
    std::unique_ptr<Player> player_;
    std::unique_ptr<HostSink> sink_;
    PlayMode playMode_ = PlayMode::WallClock;
    bool playing_ = false;
    double playStartWall_ = 0.0;
    uint64_t playFirstSample_ = 0;
    double playAssetRate_ = 0.0;

    // Status bookkeeping (N3: tick() is the only writer; status() the only, mutex-guarded, reader)
    double lastDeviceRate_ = 0.0;
    // R13-C: current-vs-armed comparison (see Status::rateChangedSinceArm); can flip back to
    // false if the device recovers to the armed rate. rateChangeNotified_ is separate so a
    // flapping rate still notifies exactly ONCE per arm, even if rateChangedSinceArm_ itself
    // toggles back and forth.
    bool rateChangedSinceArm_ = false;
    bool rateChangeNotified_ = false;
    int skippedCount_ = 0;
    int continuousUnavailableCount_ = 0;

    mutable std::mutex statusMutex_;
    Status published_;
};
