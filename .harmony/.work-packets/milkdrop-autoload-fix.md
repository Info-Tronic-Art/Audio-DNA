# WORK PACKET — MilkDrop preset autoload regression

**TASK:** Restore MilkDrop preset autoloading at app startup.
**DEPARTMENT:** engineering · **PROJECT:** Audio-DNA (`~/projects/RealTimeAudio`) · **HEAD:** `3b940a0`
**SIZE:** ~40 lines · **TIER:** full (system-adjacent: touches startup wiring + a documented UAF region)

> **CONFIRM YOUR REPO FIRST.** `~/projects/RealTimeAudio copy` is a STALE DUPLICATE (HEAD
> `f128bdc`, Jul 11). Work ONLY in `~/projects/RealTimeAudio`; HEAD must descend from `7d3a203`.

---

## THE BUG (root cause already established — do NOT re-derive)
Full evidence: **`.harmony/milkdrop-autoload-rootcause.md`**. Read it first. Summary:

The MilkDrop wiring block in the **MainComponent constructor** (`MainComponent.cpp:1478`, anchor
`// === v2: MilkDrop Preset Browser Wiring ===`) is guarded by `if (pmSource)` where `pmSource`
comes from `getOrCreateSource("projectm_visualizer")`. Since `22fcedc` (2026-07-30) that returns
**nullptr unless the GL context is attached** (`Renderer.cpp:787`) — and at constructor time it
never is (`Main.cpp` constructs MainComponent before `setVisible(true)`).

So the whole block is skipped **every launch**, including `setPresetManager`, whose only call site
repo-wide is inside it (`MainComponent.cpp:1512`).

**PROVEN, not inferred:** zero `[MilkDrop]` lines in the runtime logs across all sessions, while
`[Eyes] Test server started` — 77 lines later in the SAME constructor — is present.

## PRESCRIBED FIX SHAPE — but VERIFY IT BEFORE BUILDING
Preset scanning is pure file/JSON work with **no GL dependency** and must not be gated on a
GL-thread object.

1. Hoist `ProjectMPresetManager` out of `ProjectMSource` (currently a **by-value member**,
   `src/sources/ProjectMSource.h:105`) up into `MainComponent` as an owned member.
2. Scan + wire it **unconditionally** in the ctor — remove the `if (pmSource)` dependency for the
   preset-manager path entirely. (Confirm `ProjectMPresetManager` pulls in no GL headers; the
   "pure file/JSON, no GL dependency" premise is INFERRED from headers — verify it.)
3. Renderer holds the pointer pre-attach; inject it into new `ProjectMSource`s at GL-thread source
   creation, replacing the self-wire at `ProjectMSource.cpp:19`.

### CRITICAL REFINEMENT — the SELECTOR does NOT hoist (architect-verified)
**Only the MANAGER hoists. `PresetSelector` STAYS a source member.** `PresetSelector::processFrame`
runs on the **GL thread** inside the source's render (`ProjectMSource.cpp:136`) — hoisting it would
move GL-thread work onto the message thread. Naively hoisting "the preset stuff" is the trap here.

So the ctor block's SECOND wire — `setPresetSelector(&pmSource->getPresetSelector())`
(~`MainComponent.cpp:1513`) — must go **LAZY**: a marshaled message-thread wire at source creation,
or at the existing re-resolve sites (~`:1516-1524`). The browser is already null-guarded for a
missing selector (`MilkDropBrowser.cpp:484,537`), so it degrades gracefully until a projectm clip
exists. **Preset LISTING must work at startup with no selector present** — that is the actual bug
being fixed; jukebox playback can light up later when a source exists.

Files: `src/MainComponent.{h,cpp}`, `src/render/Renderer.{h,cpp}`, `src/sources/ProjectMSource.{h,cpp}`.

### HARD CONSTRAINTS
- **DO NOT "fix" this by retry-wiring from a context-created hook.** ARCHITECT-VERIFIED wrong
  twice: it re-points the browser at a source-interior member (re-creating the documented hazard
  the moment sources are ever disposed) AND it runs browser-UI mutation on the GL thread.
- **The UAF claim is now ARCHITECT-VERIFIED — with a caveat you must honor.** Hoisting *strengthens*
  the situation: the browser's manager pointer targets MainComponent-owned storage that outlives
  both browser and renderer, so the documented dangling class dies structurally **for the manager**.
  **CAVEAT: the do-not-clear rule at `Renderer.cpp:701-702` REMAINS LOAD-BEARING FOR THE SELECTOR
  POINTER. This lane must not relax it.** Read that comment before you touch anything nearby.
- **Map every internal use of `presetManager_` inside `ProjectMSource` BEFORE removing the member.**
  Leaving it unused-but-present is the safe intermediate if any use is unclear — say so in your
  report rather than guessing.
- Keep a startup log line reporting the wired preset **count** (the existing `[MilkDrop]` print at
  ~`MainComponent.cpp:1535`). It is the behavioral gate's anchor and the regression's only cheap
  future guard. Do not remove it.
- **Line numbers drift in this repo — RE-GREP BY ANCHOR TEXT, never trust an offset above.**
- Conform to existing patterns at HEAD. Do not reformat surrounding code.

### OUT OF SCOPE (do not touch)
- The dead **Preferences > Video MilkDrop folder picker** (`setPresetDirectories`/`rescan()` zero
  callers). Orthogonal defect, separate lane.
- The `/tmp/milkdrop-presets` scan and the CWD dev fallback. Both ruled out as causes.
- Jukebox Pool/Mode/Blend dead handlers. Separate lane.
- Any GL-threading change to `getOrCreateSource` itself — `22fcedc`'s hardening is CORRECT and
  stays. We are fixing the caller, not reverting the guard.

## SUCCESS CRITERIA
1. On a normal launch, the MilkDrop browser lists presets: **mood headers with counts**
   (Energetic 9 per today's manifest), clickable rows — not "No presets loaded."
2. The `[MilkDrop]` startup line prints with a non-zero count.
3. Jukebox **Play** actually starts (today it flips to red "Stop" and does nothing, because
   `presetSelector_` is also null).
4. `ctest` **203/203** still green (current baseline — re-run it, never inherit it).
5. Release build: 0 errors, no new warnings.

## FAIL-FIRST (capture BEFORE the fix — this is required, not optional)
Record against **unmodified** code, so the gate proves a real flip:
- `grep -ic milkdrop <stderr log>` → **0**
- MilkDrop tab shows "No presets loaded." on **all four sub-tabs including Favorites** (the
  discriminating signature — a merely-empty scan shows *different* strings on Favorites/Recent).

## VERIFICATION MODEL — READ THIS
**You build. You do NOT verify.** Harmony runs the behavioral gate herself (forced rebuild,
independent ctest, live-app launch + log assertions) because she did not build it. An independent
Reviewer reads your source. **Do not report DONE on the strength of your own testing, and do not
claim "self-verified."** Report what you changed, what you verified structurally, and anything you
are unsure about — especially the UAF question above.

## STANDING RULES
- **DO NOT PUSH.** 117 unpushed commits stay local.
- **SCREEN-SAFETY LAW:** do NOT open the output window. Do NOT `pkill` Audio-DNA — an instance
  (pid 18045) is running and the owner is using it. If you need the app, ASK; do not launch a
  second instance (port collision on 8080).
- FORCED REBUILD before any ctest claim (stale-binary false-green is a documented trap here).
- App needs `--test-mode` or 8080 never binds — 7070 binds anyway, which is a FALSE GREEN.
- `.harmony/` is gitignored but many files are force-tracked; `git add` prints an "ignored"
  warning and still stages tracked files — that warning breaks `&&` chains but is not a failure.

## REPORT FORMAT
`STATUS` (DONE / DONE_WITH_CONCERNS / BLOCKED) · `FILES CHANGED` · `WHAT I DID` ·
`THE UAF VERDICT` (confirmed/refuted, with evidence) · `WHAT I COULD NOT VERIFY` ·
`ANYTHING THAT SURPRISED ME`.
