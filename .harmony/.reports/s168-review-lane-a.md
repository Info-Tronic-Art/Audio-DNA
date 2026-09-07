# Reviewer Verdict — s168-review-lane-a
STATUS: PARTIAL

VERDICT: APPROVE WITH REQUIRED FIXES

UNDECLARED DEVIATIONS FOUND:
1. v1 bridge TransportChange is NOT unresolvable (contradicts builder's DEVIATION 6
   claim). `Take::fromV1Var` (Take.cpp, `case V1Type::TransportChange`) sets
   `key.scope = ControlPath::Scope::Comp`. `Program::compile`'s `resolveKey`
   (Program.cpp) unconditionally returns `LevelOutcome::ExactMatch` for
   `Scope::Comp` ("one Composition -- always addressable"), before looking at
   any other field. So a v1 TransportChange point (real action+value, scaled
   int) resolves, is counted in `resolvedCount`, and is pushed into
   `program->discrete` -- it is NOT reported unresolved and, once step-3 wires
   a `Sink`, it WILL reach `applyAudioTransport` dispatch with legacy
   value*1000-scaled data. MacroChange (Scope::Macro, explicit Missing) and
   ParameterChange/EffectToggle/CuepointJump (Scope::Clip, deck left at -1 so
   Missing) are correctly unresolvable as claimed -- only TransportChange
   breaks it, because it's the one of the five given Comp scope like the
   (intentionally-resolvable) ColumnTrigger. Untested: `test_take.cpp`'s v1
   fixture only exercises ClipTrigger/ColumnTrigger, never TransportChange.
2. `Program::compile`'s `convertBeatX` throws away exact per-breakpoint data.
   `Gesture::stamps[i]` (Lane.h) carries the EXACT `{t,sample}` reading taken
   at capture for breakpoint `i` (`stamps.size()==curve.pts.size()`, enforced
   by `PerformanceRecorder::set/release`). `Program.cpp`'s compile loop
   (`for (const auto& bp : g.curve.pts) ... converted.x = convertBeatX(bp.x)`)
   never reads `g.stamps` at all (grep confirms: 0 references to `.stamps` in
   Program.cpp/Player.cpp) -- it reconstructs Wall/Sample-domain x purely from
   the take's TempoMap (`tAt`/`sampleAt`), discarding the exact stamp that was
   already sitting right next to it. This is strictly worse than necessary
   for row 1 (no edits exist yet, so the exact stamp is always correct) and is
   what actually turns the tempo-map sparsity gap (below) from a documented
   "later-editor nicety" into a live compile-time correctness cost today.

TEMPO-MAP SPARSITY VERDICT: worse than the builder's own note suggests.
DEVIATION 4 (no periodic "every 8 bars" anchor) is real (MEASURED:
`RecorderClock::tick`/RecorderClock.cpp has exactly three anchor triggers --
bpm>0.05 change, resync "reset", lock/unlock edge -- no periodic one). On a
40-minute unbroken locked-tempo take this leaves ONE anchor (plus maybe one
"lock" anchor at the very start) for the whole span; `TempoMap::beatAt/tAt/
sampleAt` (TempoMap.cpp) then do a single-segment LINEAR extrapolation from
that one anchor's stored bpm for the entire 40 minutes. If the live tracker's
`snap.bpm` ever drifts persistently below the 0.05 anchor threshold (plausible
jitter in a continuous beat tracker), the reconstructed t/beat/sample values
accumulate real, unbounded drift over the segment -- e.g. a steady 0.03 BPM
bias at 128 BPM over 40 min is ~1.2 beats of drift, roughly a third of a bar.
The per-point LIVE values (`p.beat`, `p.s.t`, `p.s.sample`, and discrete
`Fired.at`) are unaffected -- they come from live phase integration / stamps,
not from the map. The map/reconstruction IS load-bearing today, though,
specifically for continuous-gesture x-conversion in `Program::compile` (see
UNDECLARED #2 above) for any Wall- or Sample-clock replay (Wall is D1's
stated default) of a take recorded during a long, tempo-stable stretch. Net:
not a row-1 crash, but a real, silent accuracy hole in exactly the replay path
row 1 ships, compounded by (not limited to) the missing periodic anchor.

DELETION BEHAVIOUR-NEUTRAL?: NO for RecordPanel; YES for MainComponent/
ApiServer wiring. `git diff HEAD` on MainComponent.{h,cpp} and ApiServer.{h,
cpp} confirms pure removal (ctor arg/member/one call site each); `git show
HEAD:src/api/ApiServer.cpp | grep sessionRecorder_\.` returns 0 hits pre-
change, confirming ApiServer never called into it (matches G4). RecordPanel
is a real regression, not "already dead UI" (G25 only established that
`refresh()` had no caller and the "Video (Future)" combo item was inert --
it said nothing about Record/Stop/Save/Load). BEFORE this change, Record->
[trigger clips]->Stop->Save was a genuinely working feature: `recorder_->
startRecording()` really captured `MainComponent::handleClipTrigger`'s
clip-trigger events (G4's one wired capture site) and Save/Load really
read/wrote JSON via `SessionRecorder::saveToFile/loadFromFile`. AFTER: Record
button still flips to "Recording..." (red) and Stop still flips to "Stopped"
-- but nothing is actually being captured anymore (no recorder exists) --
and Save/Load (`RecordPanel.cpp`) have NO `onClick` handler at all post-
change (confirmed by reading the file): they are visible, enabled (no
`setEnabled(false)`), and clicking them does precisely nothing -- no status
text, no tooltip, no disabled look. This is the exact "lying/buried dead
control" the owner's standing ruling forbids, and it is materially different
from the Format selector's honest "Video (Future)" label pattern the spec
itself calls for at step 4. `onStartRecording/onStopRecording/onPlayRecording`
are confirmed never assigned anywhere in MainComponent.cpp (grep, 0 hits)
both before and after, so that part was already a no-op; the regression is
specifically the loss of real capture + the silent Save/Load no-ops.

ControlPath COLLISION REACHABLE?: YES, by construction, and it is the
intended Resolume-style "cell follows position" semantics (D2 policy), not a
bug -- but reachable. `operator<`/`operator==` (ControlPath.h `asTuple()`)
key on `{scope, deck, deckRelative, layer, col, fx, control, param, scalar,
macroScope, macro}` only -- `deckName/layerName/clipName/fxName/paramKey` are
excluded, confirmed by reading `asTuple()` directly. Two controls that share
every positional field but differ only in name (e.g. two different params at
the same effect-slot index across two different effects at the same fx slot
index, or the same slider position after a composition edit swapped what's
there) collide onto one `map<ControlPath,Lane>` entry -- whichever capture
happens second overwrites/merges with the first in `PerformanceRecorder`'s
`take_.lanes[key]`. This is the exact scenario the builder's own test
(fixed in `test_take.cpp`'s `Program::compile` test, comment: "each case
below MUST use a distinct index, or two of these would collide") ran into
while writing tests. Given the spec explicitly wants position-following
semantics (D2 rejected "names only" for the same reason), this is a correct
design tradeoff, not a defect -- but it means a live take recorded across a
composition edit that changes what's at a given position can silently
blend two different real controls' history into one lane. Not tested for at
the `PerformanceRecorder` level (only at `Program::compile`'s report level).

COALESCING COMPLETE?: YES for the two things asked. Traced
`PerformanceRecorder::set/release` directly: the fix's `haveMidPoint =
curve.pts.size() >= 2` gate means the first TWO `set()` calls after `touch()`
always push distinct points regardless of elapsed time (so `pts[0]` -- begin
-- is never the one being overwritten, and only `pts.back()` at index >= 1
is ever coalesced-into). `release()` (and `stop()`'s synthesized release)
unconditionally APPENDS a fresh exact breakpoint whenever `curve.pts` is
non-empty, regardless of the coalescing window -- so end is never merged
away, including a fast touch-set-release inside one 50ms window (traced:
touch->1 set->release within 50ms yields 2 exact points, begin and end, no
loss). Minor, non-blocking nit: this makes the coalescing bound technically
"<= 2 points in the first ~0ms, then <=1/50ms after," slightly looser than
D3's stated "<=1 point/50ms," but it's the safe direction (extra precision,
never dropped data). Separately (UNDECLARED, moderate): `touch()`
unconditionally does `openGestures_[key] = std::move(og)`, so a stray SECOND
`touch()` on the same key before a `release()` silently discards the whole
in-progress gesture (already-captured breakpoints and all) with no synthesized
release and no trace -- the report/ASSUMPTIONS section covers stray
set()/release() without touch(), but not this reverse case (double-touch).
Plausible today given the recorder's own "held-set" stands in for the real
grip engine per R9's open-risk note.

PLAYER ADVERSARIAL-TIME HOLES: none of the specific ones asked (double-fire /
skip / held-forever) reproduce on read-through; one real contract gap.
- Discrete: `nextDiscrete_` only ever advances forward and fires with `<=`,
  so under a stall (large dt) every due event fires exactly once, in order,
  each at a `pos` >= its `at` -- confirmed by the shipped 400ms-stall test
  and by tracing the `while` loop by hand. Zero dt: no refire (idempotent).
- Continuous: the `while` loop (not `if`) in `advanceTo` correctly opens AND
  closes one or more short gestures inside a single big-dt call (touch+
  release both fire, never silently skipped) -- traced by hand for the "dt
  jumps clean over a whole short gesture" case.
- `swap` mid-gesture: `heldKeys` snapshot is taken BEFORE re-seating, orphans
  (held before, not covered by the new program at all) get exactly one
  `release()`; still-covered+still-held lanes are re-seated without a
  spurious re-touch. Traced and matches the shipped swap test.
- `stop()` releases every lane with `inGesture && !displaced` -- no lane can
  be left "held forever" by a stopped Player (R9) as long as `stop()` is
  actually called.
- REAL GAP: `advanceTo`/`start` never validate `pos` is non-decreasing (the
  header comment states it as a caller CONTRACT, not enforced). A genuine
  backwards seek (without an intervening `start()`/`swap()`) silently and
  permanently skips any discrete event or gesture the cursors have already
  passed -- no counter, no report field, no assert. Given D10.2 says
  playback-with-audio drives `pos` from the audio player's sample position,
  and audio position CAN jump backward on a loop/seek/device restart, this
  contract is exactly the kind of thing that gets violated by an integration
  bug in step 3, with a silent-skip outcome and no diagnostic.
- `Player::Override::Latch` is accepted by `setOverride()` and stored, but
  `override_` is READ NOWHERE in Player.cpp (grep-confirmed: the member is
  written once, never read) -- Latch has zero behavioural effect, silently
  identical to Touch, with no assert/warning if a future caller requests it.
  Declared as unimplemented in the builder report, but the API itself does
  nothing to stop a future step-3/4 caller from wiring a "Latch" UI toggle
  to a mode that quietly does nothing different -- worth a `jassert(o ==
  Override::Touch)` or a log line until it's real.

v1 BRIDGE HONEST?: NO, not fully -- see UNDECLARED #1 above (TransportChange
resolves and is dispatchable, contradicting the claimed "always compiles out
as unresolved"). ClipTrigger/ColumnTrigger (declared faithful) and
ParameterChange/MacroChange/EffectToggle/CuepointJump (declared unresolvable)
all behave exactly as claimed on direct trace.

AutomationCurve ADDITIVE?: YES, confirmed by `git diff HEAD -- src/connect/
AutomationCurve.h`: pure insertion of `Breakpoint::{toVar,fromVar}` and
`AutomationCurve::{toVar,fromVar}` plus one `#include`; `eval`/`xMin`/`xMax`/
the struct's existing members are byte-identical to HEAD. No fork.

THREADING: R7 is asserted (`jassert`) consistently across `PerformanceRecorder`
only (Player/RecorderClock/Take/Program carry no thread-confinement of their
own, matching the spec's stated ownership -- they're meant to be driven only
from the message thread by construction, same as the rest of `src/recording`
per the threading-contract table). The `jassert`'s own guard
(`getInstanceWithoutCreating() == nullptr || existsAndIsCurrentThread()`)
correctly no-ops in the headless ctest binary (no MessageManager instance) and
enforces in the real app -- matches the existing UndoManager.cpp idiom, not a
new pattern. In a live Release show build this is a no-op (assert stripped),
so nothing stops an OSC/MIDI callback that hasn't been marshalled to the
message thread from calling into `PerformanceRecorder` directly and racing
`RecorderClock`/`take_` (a `std::map`, not thread-safe) -- this is explicitly
disclosed as "the project's established posture, not a new gap," which is
accurate: I found no NEW path in this packet that calls off-thread (REST/OSC/
MIDI capture sites are all pre-marshalled today per G12/the threading table),
so this is a pre-existing, accepted-risk posture rather than a defect
introduced here.

REQUIRED FIXES (numbered, file + symbol each):
1. `src/recording/Take.cpp`, `Take::fromV1Var` `case V1Type::TransportChange`
   -- give it a scope that Program::compile's resolveKey can actually reject
   (e.g. route it through the same unresolvable pattern as ParameterChange/
   EffectToggle, or add an explicit non-Comp guard), OR fix `Program.cpp`'s
   `resolveKey` so Comp-scope v1-bridge markers with a bogus/legacy `control`
   string aren't blanket-ExactMatch. Add a v1 fixture line exercising
   TransportChange to close the test gap.
2. `src/ui/RecordPanel.cpp` -- Save/Load buttons must not be silently inert.
   Either wire them to a real "not yet available" state (`setEnabled(false)`
   + tooltip, matching the "Render... (coming)" pattern D14 already
   specifies for the format selector) or restore a minimal save-what-exists
   path. At minimum, Record/Stop must not present "Recording.../Stopped" as
   if capture is happening when nothing is being captured.
3. `src/recording/Program.cpp`, `compile()`'s continuous-gesture conversion
   loop -- use `g.stamps[i].t`/`g.stamps[i].sample` directly for Wall/Sample
   DriveClock instead of reconstructing via `tempo.tAt/sampleAt`, at least
   for row-1 (unedited) takes; this removes the tempo-map-sparsity accuracy
   hole entirely rather than just documenting it.
4. `src/recording/RecorderClock.cpp` -- add the spec's "at least every 8
   bars" periodic anchor (declared DEVIATION 4); even with fix #3 this keeps
   the tempo map itself useful for LATER edit/lint operations that do need it.
5. (Suggestion, non-blocking) `src/recording/PerformanceRecorder.cpp`,
   `touch()` -- guard against a double-touch silently discarding an
   in-progress gesture (e.g. synthesize a release for the old one first, or
   assert/log).
6. (Suggestion, non-blocking) `src/recording/Player.h/.cpp` -- assert or log
   if `Override::Latch` is ever requested, until it's wired, so a future
   caller gets a loud signal rather than silent Touch behaviour.

WHAT I COULD NOT CHECK WITHOUT BUILDING: the actual ctest pass count (285->
296) and the claimed two bugs-caught-during-the-run; whether `FeatureSnapshot`,
`Composition::initDefault`, `Clip::EffectSlot` etc. match the field names/
shapes this code assumes (only static-read, no compiler); real behaviour of
`juce::var`/`DynamicObject` edge cases (e.g. int64 round-tripping) beyond what
the shipped tests already assert; whether `EffectLibrary.cpp`/`Effect.cpp`
actually link cleanly into `test_take` as the CMakeLists diff claims.
