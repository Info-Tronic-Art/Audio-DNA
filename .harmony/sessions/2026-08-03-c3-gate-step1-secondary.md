# Session 2026-08-03a — OW arc C3 gate step 1 (secondary, slim)

Role: secondary · Profile: slim · Repo: ~/projects/RealTimeAudio · Commit: 0cc2b5a

## What this session was for
The inherited START-HERE was "OutputWindow arc C3, blocked on a fail-first freeze capture
that needs Boris to expand the SignalBar by hand." Two prior sessions had failed to produce
that capture. This session produced it, without Boris's hands, and finalised the one
unresolved design fork (A4) on the way.

## The actual blocker was not what the handoff said it was
The handoff diagnosed the blocker as a UI-driving problem (~15px cycler, synthetic-click
flake, app would not take frontmost focus). That was real but incomplete. Three separate
inherited errors ALSO stood in the way, any one of which would have prevented or corrupted
the capture:

1. The app was never launched in test mode. `--test-mode` is required or port 8080 never
   binds — but 7070 binds regardless, so a "is the server up" check passes while the test
   server is absent. The handoff's command block omitted the flag entirely.
2. The stated health route (`/api/status`) returns EMPTY on 8080; it is `/api/health`. And
   the server binds `::1` only, so every `127.0.0.1` probe returns empty and looks exactly
   like a dead server.
3. The stated detach oracle was inverted. The handoff said "fps collapses when the preview
   GL context detaches — use it to PROVE the state before trusting a FAIL." In fact `fps`
   is stored at exactly one site, inside `renderOpenGL()` (Renderer.cpp:181), so when the
   context detaches the value FREEZES. Measured live: **106.18 fps with the context
   provably dead.** Following the instruction would have shown a healthy reading in the
   detached state and sent this session hunting a phantom click failure — plausibly the
   exact trap the previous session fell into.

Lesson recorded: prove an oracle fires POSITIVE in the known-good state before trusting any
negative from it. The replacement oracle (`POST /api/render_frame`, 200-fast vs 500-after-5s)
was proven positive first, then used.

## What was produced
- **Fail-first evidence** for probe states 3 and 4, both FAILING against provenance-verified
  pre-C3 code. State 4 is the decisive case: the output window rendered at full rate
  throughout while the mapped param still froze, isolating the freeze to the preview
  context rather than to GL activity generally.
- **tests/visual/ax_press.py** — AX-title button driver via the Accessibility C API,
  retiring the probe's manual operator step. Built because states 3-4 must run AGAIN after
  C3 and on every regression; a permanently manual gate step is a gate that stops being run.
  AppleScript was found unable to reach JUCE's nested AX elements at all (`entire contents`
  does not recurse into AXGroups) — which is likely why a prior session concluded the AX
  tree was empty.
- **A4 finalised: DELETE.** Two independent fable seats, the second adversarial. The design's
  own conditional fallback ("if the no-op proof is falsified, callAsync marshal instead")
  turned out to be wrong: the proof IS falsified, but the marshal preserves the same wipe
  and widens it. The redteam confirmed deletion and surfaced an unasked-for finding (the
  PresetManager cross-build retarget hazard).
- **C3 work packet** written with corrected line numbers, the A6 ordering constraint, and
  working rig mechanics, so the next session dispatches rather than rediscovers.

## Deliberate non-actions
- Did NOT start the C3 build. The build is modest but the full-tier gate (ctest, 4 probe
  states, EMA parity, TSan 2-context drive, independent review) is not, and compressing the
  verification half is the trade the drain-mode long-task rule exists to prevent. Handed
  over a complete unit instead of a half-gated commit.
- Did NOT commit the work packet (`.work-packets/` is gitignored by convention).
- Did NOT touch the graphify-out churn or `.audit/` — not this lane's.

## Process notes
- Boris standing directive received: fable for planning/architecture, opus for other work.
  He switches the main session; Harmony sets subagent models per dispatch. Both architecture
  seats ran on fable; the session stayed on opus (implementation + gating).
- One self-correction worth recording: I declared the fps oracle "VERIFIED usable" after
  confirming it tracks the preview renderer, then had to retract it — I had verified half
  the claim (which renderer) and inferred the other half (behaviour on detach). The
  inferred half was wrong. Labelling the two halves separately would have caught it.
- Three direction changes on the SignalBar approach (manual → automated → manual →
  automated), each driven by a new verified fact but the middle two avoidable by testing
  AppleScript's recursion before announcing the approach.

---

## CORRECTION — the "Deliberate non-actions" section above is SUPERSEDED

That section says C3 was deliberately not built. TRUE WHEN WRITTEN, FALSE AFTERWARDS: Boris
extended the context budget mid-session ("keep working till you use 35% ctx then close"),
which removed the constraint the deferral was based on. **C3 was then built, gated, and
shipped in this same session.**

- `c51aff7` feat(mapping): message-thread mapping tick @120Hz (W5+A4+A6)
- `e76ca9f` the follow-up MappingEngine.h threading-comment fix (closes the review minor)
- The OutputWindow arc (C1 88af683 / C2 fcad6d0 / C3 c51aff7) is COMPLETE.

FULL-TIER GATE, both halves closed: Harmony's behavioral half (forced-rebuild Release,
independent ctest 193/193, probe states 1-4 ALL PASS with the detach oracle still proving
the preview context dead in states 3-4) + independent source review (PASS, 0 blocking,
1 minor, since fixed). Builder additionally ran Debug/ASan with jasserts live — 1478
assertions, none fired.

GATE GAPS, unchanged and still open: W7(iii) EMA parity not measured; TSan 2-context drive
not run. Both recorded with risk reads in `.harmony/ow-freeze-before.log` and the C3 commit.

## ADDITIONAL FINDINGS after the budget extension (neither was asked for)
- **Eyes reactivity DIAGNOSED.** u_bass is correctly banded (bandEnergies[1] = Bass
  60-250Hz, confirmed 4 independent ways) — no live rig impact. Real cause is a PRODUCT
  GAP: shaders declare audio uniforms they never consume, so GLSL strips them. u_beatPhase
  declared in 3 shaders, used in ZERO. The diagnosis predicts the pass/fail split exactly
  (u_rms IS used in all 6 sources; the RMS test is the one that passes). Boris ruling needed
  on scope — do NOT wire every declared uniform in as a "fix".
- **Preset/binding silent retarget, assessed LATENT-but-probably-already-biting.** Mappings
  restore by RAW effect index while the effect restore right above it name-matches. The
  chain is built by CATEGORY GROUPING, so appending an effect lands mid-chain and shifts
  ~100 indices — presets saved before the 3 newest effects may already mis-target. Fails
  SILENT (in-range-but-wrong dominates), no version guard, and BindingManager has the
  identical bug. One-commit fix designed in the handoff ledger.

## PROCESS LESSONS (the ones worth carrying)
- **Prove an instrument reads POSITIVE in the known-good state before trusting a negative
  from it.** This is what separated this session from the two that bounced off the same
  blocker. The inherited fps oracle reads healthy for a dead context.
- **A hardcoded count in a handoff cannot survive its own correction.** The unpushed figure
  was wrong 3 handoffs running (~49, 90, 95) for that structural reason. Replaced with the
  command to run. Fix the recurrence, not the instance.
- **Red-team a ruling you already like.** The A4 redteam confirmed the ruling AND surfaced
  the preset retarget bug nobody asked about.
- **Context self-estimates ran ~50% high.** Estimated 37% at close; the gauge said 24.3%.
  Nearly closed with a third of the usable budget unspent. Trust the gauge, not the feel.
- **A session record written mid-session rots like any other inherited fact** — this
  correction block exists because the original claimed C3 was deferred.
