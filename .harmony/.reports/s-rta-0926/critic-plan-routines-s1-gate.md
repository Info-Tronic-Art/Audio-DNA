Blind Critic -- plan-routines-s1.md -- lens: SCOPE and GATEABILITY

Verdict: SOUND_WITH_FIXES. The plan's individual engineering claims check out well against
current disk state (I re-verified the load-bearing ones independently, see below); scope is
justifiably large for a first-class new engine, but "buildable in one session by 1-2 lanes" is
optimistic given the size and the criticality of one shared refactor, and one product-ruling
interpretation is presented as settled with more confidence than the cited text alone supports.
No vacuous-pass defect found in the live probe design -- it is the strongest part of the plan.

Verified claims (re-derived from disk, not from the plan's prose)

- src/recording/Player.h and Player.cpp really do expose start/advanceTo/seek/firePreamble/
  stop/swap with the exact semantics the plan leans on (Player.h:29-118, Player.cpp:1-90).
  start() unconditionally resets pos_, running_, and cursors_ (Player.cpp:11-30), so the plan's
  loop mechanism (stop() then start(0) then advanceTo(pos-lengthBeats) in the same tick, plan
  section 4.2 step 3) is mechanically sound against the ACTUAL code, not just the header
  comment. This is the plan's own strongest claim (R1: "the engine is ~250 lines around a Player
  that already exists") and it holds up.
- Program.cpp:503-529 (VERIFIED): today stampMismatches increments for ANY clock when
  g.stamps.size() != g.curve.pts.size(), even though for DriveClock::Beat,
  convertBeatX(beatX) == beatX (Program.cpp:450) makes the fallback path bit-identical to the
  stamped path. The plan's "load-bearing" stamp-rule fix (section 3.4) is therefore a pure
  reporting fix, not a dispatch-behavior fix -- a routine's stampless gestures were never going
  to misfire, only mislabel themselves invalid. Good: this de-risks what the plan calls its
  riskiest touch to shared code (R6) -- it is smaller than R6 makes it sound.
- ApiServer.cpp:367-379 (VERIFIED): handleComposition's clip block has no per-effect-param
  readback today (only scalars via addLiveBlock). The plan's added effects[] block (section 5.1)
  is a real, correctly-scoped gap-fill, not padding.
- Binding.h:24-46 / BindingManager.cpp:228 (VERIFIED): enum truly ends at MasterSignal,
  serialized as int. Appending TriggerRoutine after it is a genuine back-compat-safe append,
  as claimed (R10).
- RecorderClock.cpp:11-98 (VERIFIED): nothing in tick() requires "being armed for a recording"
  -- it self-initializes on first call and is otherwise a pure function of
  (snap, wallNow, deliveredSamples). The plan's design of a SECOND, independent, always-ticking
  RecorderClock instance owned by RoutineEngine (bypassing the documented "ticks only while
  recording" limitation of RecorderHost's instance, RecorderHost.cpp:425-429) is architecturally
  sound and does not race the recording clock (separate state, no shared globals).
- Layer.h:279-315 (VERIFIED) confirms the plan's quantize arithmetic is isomorphic to
  processPendingTrigger's, just keyed on totalBarCount's rising edge instead of a
  beatInBar==0 level read -- which is actually a MORE robust choice per the codebase's own
  Pitfall 32, not a deviation to worry about.

Findings

SHOULD -- "one session, one serial lane" undersells the risk concentration in step 5

plan-routines-s1.md:521-546 (Lane 1, 12 steps, one commit each, live probe deferred to the very
end / Lane 2). Step 5 is a refactor of Program.cpp:459-541 (compileLanes) -- the function EVERY
existing replay path (probe-step3.sh, 92/1 at close per this session's own state block) depends
on. The plan's own R6 names this as the risk that "breaks every replay" and its only stated
mitigation is "all existing tests stay green" -- checked by the SAME builder that just wrote the
refactor, with the live probe (probe-step3.sh) not run until Lane 2, after 7 more steps
(~1400 more lines: RoutineEngine, ~10 MainComponent wiring sites, REST, OSC, Binding) have been
piled on top. This session's own dispatch packet states outright: "Reviews said PASS three times
on live-broken work this session -- the live gate is the verdict." Given that lesson was learned
THIS session, deferring the one live-gateable check on the plan's riskiest shared-code touch to
the very end of a ~1900-line single pass reproduces the exact failure mode just diagnosed, rather
than catching a compileLanes regression cheaply, right after step 5, before building 1400 more
lines on top of it.
Fix: insert an intermediate gate after step 5: rebuild + ctest + one live run of probe-step3.sh
(it costs nothing extra since Lane 2 will run it anyway) before proceeding to step 6. If it
regresses, the cost of unwinding is one step, not eleven.

SHOULD -- the plan's "one session" framing for Lane 1 is not itself gated by anything

plan-routines-s1.md:521 estimates ~900 source + ~600 test + ~350 probe lines across a brand-new
model file, a Composition schema addition, a Take/RecorderHost format addition, a new slicer, a
refactor of shared compile code, a brand-new scheduler engine, ~10 MainComponent call sites, 6
REST routes + a composition schema change, an OSC route, and a Binding enum + 3 UI-overlay
touch-points -- for ONE serial builder lane, reviewed once at the end. Nothing in the plan
commits to a checkpoint at which Harmony would decide "this is not landing this session, split
it." The plan's own philosophy elsewhere (surgical slices, "the smallest Boris-visible slice")
argues for a narrower first landing; the plan does defer real content (D9's routine-in-take lane,
editor UI, re-target table) but keeps the ENTIRE runtime + REST + OSC + Binding surface in one
lane. Given this session's stated context (a pre-existing live-visible blank-frame bug still
unfixed, a still-open step3 probe row, doc-count drift) is already on the plate, loading a
~1900-line new subsystem into the same session on top of those is a scope risk to the SESSION,
even where each individual piece of the design is sound.
Fix: name an explicit fallback line in the plan itself -- e.g. "if step 5 is not green with a
live probe-step3.sh pass by [checkpoint], stop after step 5/6 (model + slice + compileRoutine, no
REST/OSC/Binding/engine) and hand off the rest as slice 1b" -- so "one session" has a defined
cheaper landing spot instead of being all-or-nothing.

NICE -- ruling 26 ("record time") interpretation is presented as more settled than the cited text
alone establishes, though the fallback citation (D4) does resolve it

plan-routines-s1.md:207-214 rules "the state it was recorded in" = the state at the SLICE's own
start (not the take's checkpoint 0), citing D4 step 2 (spec:265-270, VERIFIED: "Preamble = the
state at beatFrom of every selected lane's control"). Ruling 26 itself
(binding-decisions.md:427-430, VERIFIED) says only "restore" / "the way they were at record time"
and "per the recommendation he accepted" -- the recommendation being D4's architecture mechanism.
Since Boris said "per the recommendation he accepted" this is legitimately settled, not
ambiguous -- I checked for a genuine ambiguity and did not find one strong enough to call this a
MUST or SHOULD; noting it only because it is the one place in the plan where a Boris ruling is
interpreted rather than quoted directly, and a future reader skimming just ruling 26 (without the
D4 cross-reference) could reasonably read it the other way (checkpoint-0-of-the-whole-take). Cost
of confirming with one sentence in the next status update to Boris is near zero.

NICE -- live-probe design is the strongest artifact in the plan against the "vacuous pass" risk

Checked specifically for the failure modes named in my brief (restore/replay as no-ops, a value
that was already there, a skipped row counted as pass). Found none: row 4 independently verifies
the PERTURBATION itself is pixel-visible (mad(ref,pert) greater than 0.5) before row 6 checks the
restore undoes it (mad(pert,rest) greater than 0.5 AND mad(ref,rest) less-or-equal 2.0) -- so a
no-op restore would fail row 6 outright (pert and rest would be identical, mad near 0). Row 5
additionally requires preambleFired greater-or-equal 5 from the engine's own counters, but that
count is not the ONLY evidence -- rows 6-7 independently re-derive the same outcome from
/api/composition state reads and pixels, so a bug in the counters alone could not produce a false
GREEN. This is well-designed; my only addition is procedural (see the SHOULD above about running
the live gate earlier, not about the probe's own design, which is fine).

What I could not check (say so, don't guess)

- Whether wave-1 lane D's TopBar/Master-Signal critic-panel fix touches MainComponent.cpp (which
  would collide with Lane 1's ~10 wiring sites there) -- not in my read set. The plan asserts
  "MainComponent.cpp is shared with nothing in wave 1" (line 514) but the fence list for lane D
  only names src/ui/CompDecksBrowser.*/ClipInspector.*/"TopBar shots"; I cannot verify from the
  inputs given whether "TopBar shots" means test artifacts only or also code edits. Mitigated
  regardless by "wave 1 merges first" ordering.
- Whether the Composition.h additions actually compile cleanly against every guarded-read site
  the plan implies beyond the region I spot-checked (Composition.h:220-335) -- I read the region
  cited, not the full ~500 lines, so I did not attempt to independently re-derive every
  guarded-read site the plan claims will need matching additions.
