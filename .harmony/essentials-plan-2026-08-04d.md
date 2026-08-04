# ESSENTIALS PLAN — "all main surfaces working" (2026-08-04d)

Authored by a Fable-tier architect over three rounds, chaired and attacked by Harmony, with
Boris's rulings folded in. Grounded in `.harmony/surface-audit-2026-08-04d.md` (the first
systematic surface sweep) and `.harmony/milkdrop-autoload-rootcause.md`.

**HEAD `3b940a0` · 117 unpushed · NOTHING PUSHED · ctest baseline 203/203 (re-run, never inherit).**

**BORIS'S SCOPE, BINDING:** *"I want to get it working then upgrade it. Now just all main
surfaces working."* ESSENTIALS = every main surface does what it presents itself as doing.
NOT upgrades, NOT polish. Anything that improves an already-working surface is PARKED.

---

## RULINGS

**SETTLED**
1. **Output = Strategy A** — native output must show real deck content. "Declare Syphon the
   supported path" was the architect's first recommendation and was REJECTED: it fixes the
   headline surface by redefining it away. Boris: *"it was working before."*
2. **Display targeting: KEEP, simplify the UI only** (Boris: "a"). The architect twice proposed
   deleting the display-selection surface on a literal reading of "only main monitor." **Harmony
   overrode it twice.** Deleting it would remove the app's only path to a projector — invisible
   today (one display attached), catastrophic at a gig.
3. **Legacy v1 row-1 controls + 10 preset slots: DELETE** (Boris: "delete").

**OPEN**
- **Honesty batch** — hide/remove Record tab, Timing placeholder, Comp-Inspector dead blocks,
  dead param-source trio. Recommendation: yes to all, every one reversible in git.
- **Rack (`EffectsRackPanel`)** — architect and Harmony both recommend DELETE, with audio→param
  curve shaping consciously parked for re-homing into the v2 param picker.

---

## SEQUENCE

Contended-file chain (**strictly serial**, commit each lane before the next):
**L0-MD → L1 → L-OUT → L3 → L-DEL → L4 → L5 → L6 → L7 → L9**
Parallel-safe: **L2** (CompositorEngine only) anytime · Comp-Inspector hides · jukebox/prefs wires.

---

### L0-MD — MilkDrop autoload regression — WIRE — small — **FIRST**
**Gains:** MilkDrop presets list again. A feature that has been dead on every launch since Jul 30.
**Root cause:** ctor wiring block gated `if (pmSource)`; `getOrCreateSource` returns nullptr when
GL is unattached (`Renderer.cpp:787`), which at ctor time it always is. Regression `22fcedc`.
**Fix:** hoist `ProjectMPresetManager` into MainComponent, scan/wire unconditionally; Renderer
injects the pointer into new sources at GL-thread creation.
**LOAD-BEARING REFINEMENT:** the **selector does NOT hoist** — `PresetSelector::processFrame` runs
on the GL thread (`ProjectMSource.cpp:136`). It stays a source member; `setPresetSelector` goes
LAZY. Browser is null-guarded, so listing works with no selector. *Naively hoisting "the preset
stuff" is the trap.*
**Verified risks:** hoist STRENGTHENS the documented UAF (manager outlives browser + renderer) —
BUT the do-not-clear rule at `Renderer.cpp:701-702` stays load-bearing **for the selector pointer**.
Retry-wiring from a context hook is wrong twice (re-creates the hazard + mutates UI on the GL thread).
**Fail-first:** zero `[MilkDrop]` log lines at launch (already captured). **Gate:** log line with
non-zero count + populated browser + preset click loads + ctest 203/203.
**Packet:** `.harmony/.work-packets/milkdrop-autoload-fix.md` — build-ready.

### L1 — Media-leak family — WIRE — small-medium
**Gains:** `Clip > Clear` stops permanently leaking an FFmpeg decoder + GL texture + decode thread.
Unbounded today; accumulates exactly in proportion to how much you churn clips across a set.
**Three traps the "one call" sizing missed:**
(a) **Double-apply** — the handler pre-mutates, then `UndoManager::perform` re-applies
(`UndoManager.cpp:12-15`); by execute time the cell is already empty, so any dispose keyed off live
cell state fires never. Dispose must key off the command's `before_`/`after_` **snapshots**.
(b) **GL-thread destroy** — `~VideoPlayer` → `glDeleteTextures` with no context current. Needs a
mutex-guarded retire list drained on the GL thread (precedent `Renderer.cpp:699-725`).
(c) **Family coverage** — `ClearLayerClipsCmd`/`RemoveColumnCmd` vacate identically.
**Undo is already safe** — `ClipMediaHook` was built for exactly this.
**Fail-first:** `lsof` shows the handle still open after Clear today; closed after, including
clear→undo(plays again)→redo(closed).
**Risk:** disposing still-referenced media. Guarded by a liveness scan + the unique-clip-id
invariant (no clipboard, 6 minting sites, atomic swap) — **future-fragile, needs a load-bearing
comment: a clipboard feature would break the scan.**

### L2 — Layer Solo — WIRE — one-liner ×2 — **PARALLEL-SAFE**
Solo toggles, undoes and repaints today; the compositor never reads it. Add an `anySolo` skip at
both layer loops (`CompositorEngine.cpp:650,:680`) — same read class as `visible`/`bypassed`.
**Gate:** snapshot pixel check.

### L-OUT — Native output + Esc + one selector — WIRE/DELETE — medium — after L1
**Gains:** the output window finally shows your actual deck. Esc actually exits. Projector gets
native resolution.
**Transport (recommended, cheapest sound option):** consume the finished frame that
`publishSyphonFrame` (`Renderer.cpp:1850`, called `:693`) already proves capturable — `glReadPixels`
at that same site into the output window's **existing thread-safe latest-wins CPU queue** (the
proven camera-frame transport). Mirror frames **bypass the window's own FX pass** (frame already
carries global FX; re-applying = double-FX).
- *Rejected:* giving OutputRenderer its own compositor — no GL context sharing, and `VideoPlayer`
  owns ONE context-bound texture, so it forces per-context surgery through every media class and
  re-enters the just-hardened C1/C2/C3 arc.
- *Escalation ladder if readback stalls:* sync read → double-buffered PBO → IOSurface zero-copy.
  **The capture point and queue survive all three, so a bad measurement means more work, never a
  wrong architecture.**
**Native resolution — IN-LANE, not parked.** The compositor already renders at *render* res, not
window res (`Renderer.cpp:447-449`); the window blit is a letterboxed downscale. Call the existing
`setLockedResolution(displayW, displayH)` on open, restore on dismiss. **1:1 on the projector.**
*(This corrects a Harmony error — see CORRECTIONS below.)*
**Esc / dismissal (verified defect):** both paths only `setVisible(false)`
(`OutputWindow.cpp:316-319, 358-365`) — window + GL context survive and persisted `outputDisplay`
never resets, so **a deck saved after an in-window Esc records output ON**. Fix: `onDismiss` →
existing `closeOutput()` (real destroy) + `outputDisplay=-1` + UI sync + `grabKeyboardFocus()` so
Esc lands in-window regardless of app focus.
**One selector survives — TopBar's** (v2 performer strip; MainComponent's duplicate lives in the
legacy row-1 block and dies in L-DEL). `openOutputOnDisplay(int)` and display enumeration UNTOUCHED.
**Stale-display list gets a real fix** (it no longer vanishes by deletion): repopulate at
popup time + validate-at-open with fallback to primary. Closes the plug-in-at-the-gig failure class.
**Unverified seam:** if the global-FX pass draws straight into the default framebuffer, capturing
post-FX at full render res needs one intermediate FBO. Small, standard — **builder must confirm.**
**Gates are OWNER-ATTENDED per the SCREEN-SAFETY LAW. Flagged, not worked around.**

### L3 — Composition persistence — WIRE — medium-large — depends L1
**Gains:** a composition can actually be loaded. Today Open/Save/SaveAs operate on the **v1 FX
preset**, and `Composition::loadFromFile` has zero callers — so comps save (only as a side effect
of Collect Media) and never load.
**This is ONE root cause behind three surfaces:** broken Open/Save, the dead Comp/Decks browser,
and the entire stranded genre/P23 config family.
Sub-steps: `loadComposition` fenced per the kCompNew template · **bump `s_nextClipId` past the max
loaded id** (ids are serialized — without it, future drops collide in the media maps) · close-all
media via L1's mechanism · reconnect every playable cell · `undoManager_.clear()` (stale
index-based resolvers must not survive a model swap) · real Save/SaveAs · assign the three dead
browser callbacks. Genre auto-switch then activates for free (handler is correct; only the flag was
unreachable).
**Declared honestly, still parked:** structural-scene handler body is a bare log (behavior absent,
not merely gated); `smartAutopilotEnabled` has zero readers.

### L-DEL — Legacy v1 deletion — DELETE — small-medium — **HARD DEPS: after L-OUT and L3**
**Sequencing is load-bearing:** the 10 `outputWindow_->loadImage(...)` sites are output's ONLY
content source until the mirror exists. Deleting them first strands output with nothing.
Scope: row-1 controls, 10 preset slots, the 10 v1 feed sites. Rack rides here as its own committed
step if ruled DELETE.
**CARVE-OUT:** `resolutionSelector_` sits visually in row-1 but is **NOT v1 debris** — it drives
the live renderer lock L-OUT depends on. Must be excluded and re-homed.
**PresetManager engine STAYS** (genre auto-preset consumes presets by name).
**A deletion's fail-first is different in kind** — the check IS absence + non-breakage: build ·
ctest 203/203 · grep-zero on deleted symbols · attended walk of shared-path surfaces.

### L4 — Honesty batch — DELETE/hide — small — **needs Ruling 2**
Comp-Inspector Autopilot + globalFX blocks · Timing placeholder (reclaims a permanent quarter of
the bottom row) · signal-header "routes list" promise · param-source dead trio (they mark a param
"connected" and then kill its own slider) · Record tab hide.

### L5 — Quantize — WIRE — small-medium
Offers Off/Next Beat/Next Downbeat and lies. Build the missing consumer: pending-trigger queue
consumed on beat crossing. **Survey-first** — find the single choke point all trigger forms route
through; if none exists, the lane grows and says so (MIDI/OSC bypass risk).

### L6 — BPM multiplier — WIRE — small
UI writes `composition_.bpmMultiplier`; zero consumers. One multiplicative factor at the analysis
publish point. **Touches THE protected asset (the 28-uniform audio pipeline) — smallest possible
diff, hard stop-and-report if it isn't a clean single factor.**

### L7 — Small wires — WIRE — each small
MilkDrop folder pref (**depends L0-MD**) · jukebox Pool/Mode/Blend onChange · Ableton Link toggle
(consumer loop already live; full gate needs a Link peer → attended) · MIDI-out device open
(consumer live; IAC loopback gate).

### L9 — Modulation freeze — VERIFY-FIRST — last
Audit severity is INFERRED. Confirm at runtime first; if real, relocate the drive out of
`EffectStackView::refresh()` into the `tickFeaturePipeline()` seam the arc named for this class.
If not confirmed: close, zero code. **The one item straddling essential/upgrade** — a "connected"
param that stops modulating when unwatched lies during a live set.

---

## PARKED FOR THE UPGRADE PASS (nothing silently dropped)
**Beat-authoring the ~44 hollow `u_beatPhase` shaders** — ruled UPGRADE, not essential: no control
presents "beat response" and fails, and the app does not ignore the beat (it drives autopilot,
playlist advance, and ~12 shaders). Coverage is thin, not absent. Shape when its turn comes: 8-10
flagship shaders, one repeatable recipe (beat-decay envelope from `u_beatPhase` gated by
`u_onsetStrength`), **not all 44**.
Also: native-res render beyond the lock · OutputRenderer FX-machinery removal · IOSurface transport
(unless promoted by the FPS gate) · RoutingEngine UI · **curve-shaping re-home into
`UniversalParamControl`** (covers the consciously-dropped MappingEditor AND the dead
BPMSync/ClipPosition/Timeline sources) · structural-scene body · smartAutopilot consumer ·
comp-globalEffects plumbing · Record build-out · Timing window content · `u_energyState` dead
freight · half-hollow `u_spectralCentroid` shaders · Prefs About default.

---

## CORRECTIONS MADE DURING PLANNING (the record matters more than the tidy version)
1. **Harmony error, retracted:** "no resolution lock exists anywhere in the repo" — FALSE. The grep
   pattern was `lockResolution`; the symbol is `setLockedResolution`. A negative from a grep is only
   as strong as its pattern. Consequence was material: native-res projector output moved from
   "parked blind" to "cheap, in-lane."
2. **Architect's display-picker deletion: overridden twice** by Harmony, then ruled by Boris.
3. **Architect's first recommendation (Syphon-first) rejected** — it redefined away the surface the
   owner asked to have fixed. Its own risk section named that as the strongest counter to itself.
4. **`u_beatPhase` inherited claim corrected:** handoffs said "3 shaders declare it, ZERO use it."
   Actual: **56 declared, ~12 used.** Off by ~18×, and "zero" was wrong.
5. **"One call" sizing of the leak fix was wrong in three ways** — see L1.

## VERIFICATION MODEL (every lane)
Builders build. **The party that builds NEVER verifies.** Harmony runs the behavioral gate herself
(forced rebuild, independent ctest, app-level checks) because she did not build. An independent
Reviewer reads source. Full-tier gets both halves. Output-window gates are **owner-attended only**
per the SCREEN-SAFETY LAW.
