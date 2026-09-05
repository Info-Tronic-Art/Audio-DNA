# Lane 6 — io-api — Re-norm findings (2026-09-05)

Scope: `src/api`, `src/output`, `src/recording`, `src/core`, `src/Main.cpp`
Baseline: `9139dd4` (2026-07-16 norm) → `HEAD`

## 0. Scope re-derivation

```
git log --oneline 9139dd4..HEAD -- src/api src/output src/recording src/core src/Main.cpp | wc -l
```
→ **22 commits.** (Listed below; all 22 read.)

```
git diff --stat 9139dd4..HEAD -- src/api src/output src/recording src/core src/Main.cpp
```
→ 21 files changed, 1947 insertions, 617 deletions. Breakdown:
- `src/api/ApiServer.{h,cpp}` — heavily rewritten (497 lines touched in .cpp)
- `src/core/*` — 11 files, ~10 of them **entirely new** (Undo v1 command classes)
- `src/output/*` — **net removal**: `NdiInput.h`, `NdiOutput.h`, `SpoutOutput.h`, `SyphonInput.h/.mm` deleted outright (all-negative diffs); `SyphonOutput.{h,mm}` grew (+REST toggle support)
- `src/recording/*` — **0 commits, 0 diff** in this window (confirmed by separate `git log -- src/recording` → 0 hits)
- `src/Main.cpp` — **0 commits** in this window (confirmed separately, 99 lines, untouched since norm)

Commits (newest first): cb4d5fa def0efc 7d39d25 00e15b6 43ff194 ca068e4 bacda0d 2d1744d 8077af7 f6b208f 8bd09ba 0a1c882 4ee2dac d90e953 316a2bf 7f87094 6d2def4 daa9361 7921572 7c8d286 45ae7e8 368d621

## 1. THE PREMISE ERROR — task brief claim checked and falsified

The task brief states: *"src/core and src/output (Syphon/NDI/Spout) are both NEW since the last norm."*

**This is false.** Verified two ways:
```
git ls-tree -r --name-only 9139dd4 -- src/output
  → NdiInput.h NdiOutput.h SpoutOutput.h SyphonInput.h SyphonInput.mm SyphonOutput.h SyphonOutput.mm
git ls-tree -r --name-only 9139dd4 -- src/core
  → Command.h UndoManager.cpp UndoManager.h
git log --oneline 9139dd4 -- src/output | wc -l   → 1  (327fa6b, "P22: Output & Integration", pre-dates 9139dd4)
git log --oneline 9139dd4 -- src/core   | wc -l   → 1  (9c8cdba, "P3: Architecture foundation", Mar 17)
```
Both directories existed at the 9139dd4 norm point and are already covered by FEATURES.md §12/§13 (output) and §8 (core/UndoManager). **What actually changed** in `src/output` since the norm is the opposite of "new": NDI, Spout, and SyphonInput were **removed** (all-deletion diffs, matching "Wave 0" removals already noted in FEATURES.md's synced-header — that part of the doc is correct and not new drift). What's genuinely new in `src/core` is the Undo v1 command-class family — see §4 below, which is the real, large, undocumented addition the brief's "NEW" claim was gesturing at but mis-attributed to the whole directory.

## 2. REST endpoint census — re-derived

FEATURES.md's normalize header (line 5) states the 9139dd4 baseline count: **"22 REST endpoints."** Re-derived independently both at 9139dd4 and at HEAD by grepping `server_.(Get|Post)(` registrations in `setupRoutes()`:

```
git show 9139dd4:src/api/ApiServer.cpp | grep -nE '\.(Get|Post)\('   → 22 matches   ✓ matches doc
grep -nE '\.(Get|Post)\(' src/api/ApiServer.cpp (HEAD)                → 24 matches
```

**Current HEAD count: 24 registered routes**, not 22 and not the "20+" vague figure FEATURES.md §14 currently uses. The 2 new routes are `/api/syphon` (GET, status) and `/api/set_syphon` (POST, toggle) — added by commit `7d39d25` ("Add REST status/toggle for Syphon output"). Neither is mentioned anywhere in FEATURES.md §14 (External Control) or §13 (Output & Display).

**Functional vs. registered — re-derive "21 functional" too:** one route, `/api/inject_features`, is registered **conditionally**:
```cpp
// src/api/ApiServer.cpp, setupRoutes()
if (allowFeatureInjection_)
{
    server_.Post("/api/inject_features", ...);
}
```
`allowFeatureInjection_` is a ctor flag; `MainComponent` passes `testMode_` (commit `def0efc`). In a production run this is `false`, so the route is **never registered** → real 404, not merely gated behind a check. So:
- **Test-mode build: 24 functional endpoints.**
- **Production build: 23 functional endpoints** (24 registered − inject_features not registered).

This gate (`def0efc`, Aug 2) post-dates the 9139dd4 norm, so at norm-time `inject_features` WAS unconditionally registered — meaning the "22 endpoints, 21 functional" figure the task brief cites as the norm's claim does not match what I can find written in FEATURES.md itself (the doc's actual text is "22 REST endpoints" with no functional/non-functional split, and separately "20+ endpoints" in §14 prose). **I could not locate a "21 functional" claim anywhere in FEATURES.md** — grepped the whole file for "functional" and "21" near "endpoint"; no hit. Flagging this as a brief/doc mismatch rather than asserting a number I can't source.

**Corrected census to write into FEATURES.md:** 24 routes registered in source at HEAD (22 at norm + 2 new Syphon routes), of which 23 are reachable in a production build and all 24 in a test-mode build (`allowFeatureInjection_`/`testMode_`).

## 3. THE OUTPUT STORY — verified exactly as briefed, plus one location error

**Constructor signature — verified from source, `src/ui/OutputWindow.h`:**
```cpp
class OutputRenderer : public juce::OpenGLRenderer
{
public:
    OutputRenderer(const FeatureBus& featureBus,
                   MappingEngine& mappingEngine,
                   EffectChain& effectChain);
    ...
    void loadImage(const juce::File& imageFile);        // content input #1
    void queueCameraFrame(const juce::Image& frame);     // content input #2
private:
    const FeatureBus& featureBus_;
    MappingEngine& mappingEngine_;
    EffectChain& effectChain_;
    ...
};
```
No `Compositor`/`CompositorEngine`, `Deck`, or `Composition` reference anywhere in the class — confirmed both by reading the full header and by `grep -n "Compositor|Deck|Composition" src/ui/OutputWindow.h src/ui/OutputWindow.cpp` (zero hits). Content can only reach the output window via `loadImage()`/`queueCameraFrame()`, both called from `MainComponent` only on static-image clip activation (`MainComponent.cpp:657-658`, `1413-1414`, `2191-2192`, `2473`) — never with deck/compositor output. **Confirmed: the output window has never rendered the live composited deck.**

**Timeline — verified from git log, all three commits read in full:**
| Commit | Date | What |
|---|---|---|
| `005935d` | 2026-03-14 | "Add fullscreen output window..." — adds `OutputWindow`/`OutputRenderer`, explicitly described in its own commit message as sharing "FeatureBus/MappingEngine/EffectChain with primary renderer" |
| `3ab7cc4` | 2026-03-15 | "M7: CompositorEngine — multi-layer key compositing..." |
| `9c8cdba` | 2026-03-17 | "P3: Architecture foundation..." — adds Composition/Deck/Layer/Clip data model |

Timeline confirmed exactly as briefed: output window (Mar 14) predates CompositorEngine (Mar 15) by one day and the Deck model (Mar 17) by three days. This is architecturally a "never wired," not a regression — there is no commit between `9c8cdba` and HEAD that adds a Compositor/Deck reference to `OutputRenderer`.

**File location error found while verifying:** FEATURES.md §13 cites `OutputWindow class (src/output/OutputWindow.h)`. **The file is actually `src/ui/OutputWindow.h`**, not under `src/output/` at all — `src/output/` today contains only `SyphonOutput.h`/`.mm`. Verified: `find . -iname "*OutputWindow*"` → only hits under `src/ui/`. This is a real, checkable path error (MINOR — the class name and behavior described are otherwise correct, only the path is wrong).

**Syphon — verified genuinely wired from the MAIN renderer, not OutputRenderer:**
```cpp
// src/render/Renderer.cpp, renderOpenGL(), ~line 689-693
if (syphonOutput_ != nullptr && syphonOutput_->isEnabled() && syphonOutput_->isInitialized())
    publishSyphonFrame(static_cast<GLuint>(defaultFBO), vpX, vpY, vpW, vpH);
```
`Renderer` (the main preview/compositor renderer, `src/render/Renderer.h`) owns `CompositorEngine compositor_` and a `Composition* composition_`, and `publishSyphonFrame()` blits the already-composited default-FBO region into a dedicated texture and calls `syphonOutput_->publishTexture(...)`. So Syphon output genuinely does carry the finished composited deck frame — it rides on the MAIN renderer's pipeline, completely independent of the broken `OutputWindow`/`OutputRenderer` path. **This means the fullscreen output window and Syphon output are on two different code paths with different capabilities: Syphon shows the real deck; the on-screen "Output" window does not.** FEATURES.md §13 does not draw this distinction anywhere — worth calling out explicitly when the section is rewritten, since a reader could otherwise assume "Output & Display" is one coherent, working subsystem.

## 4. CRITICAL — undo/redo is fully implemented; FEATURES.md's synced-header says the opposite

FEATURES.md's top normalize note (line 7) reads: *"...Undo/redo remains a no-op (Wave 2)."*

**This is false at HEAD.** 9 of the 22 in-scope commits are a complete Undo v1 rollout, entirely new files under `src/core/`:

| File | Lines added | What |
|---|---|---|
| `ClipCommands.h` | 215 | clip-level undo commands + `ClipLayerResolver` |
| `Command.h` | +10 | base `Command` interface extended |
| `CompositeCommand.h` | 61 | multi-command grouping |
| `DeckCommands.h` | 759 | `SetColumnCountCmd`, `RemoveColumnCmd`, `ClearLayerClipsCmd`, `ClearActiveClipCmd`, `ToggleLayerFlagCmd`, `AddLayerCmd`, `RemoveLayerCmd`, `MoveLayerCmd`, `AddDeckCmd`, `RemoveDeckCmd`, `SwitchDeckCmd` |
| `EffectCommands.h` | 123 | effect-stack undo commands |
| `EffectScope.h` | 31 | scope helper for effect commands |
| `MediaReconnect.h` | 17 | `needsVideoReopen()` — video-player-reopen decision for undo/redo of media swaps |
| `TriggerCommands.h` | 120 | `TriggerClipCmd` (column trigger = `CompositeCommand` of these) |
| `UndoManager.{h,cpp}` | +24/rewritten | existing class, extended |
| `UndoService.{h,cpp}` | 92+97 | new: coordinate re-resolution (no dangling pointers across vector reallocation), `syncAfterModelChange`, `withDeckDetached` GL fence |

**Verified fully wired, not just infrastructure** — `MainComponent.cpp` calls `undoManager_.undo()`/`.redo()` from the Edit menu (lines 2331-2337, 3938-3945), exposes dynamic `undoDescription()`/`redoDescription()`/`canUndo()`/`canRedo()` for the menu labels (lines 1576-1583), and has 20+ call sites routing structural edits through `undoService_.withDeckDetached(...)`. `TriggerClipCmd` is constructed from real user actions (`MainComponent.cpp:3335,3383`). A dedicated test file exists: `tests/test_undo_commands.cpp` (not mentioned anywhere in FEATURES.md §8's "Test coverage" list, which cites only `test_composition.cpp`/`test_compositor.cpp`).

**Severity: CRITICAL.** This is exactly the class of error the audit brief warns about (a stale absolute claim — "no-op" — about behavior a user directly reaches via Edit → Undo/Redo or Cmd-Z), and it sits at the top of the document as a synced-header assertion, so it is likely to be trusted and propagated without a second check.

## 5. CRITICAL — the documented DATA RACE in ApiServer has been fixed since norm

FEATURES.md §14 Gotchas currently reads:

> **DATA RACE: `set_layer_opacity` and `set_param` write DIRECTLY from HTTP background thread** — no mutex, no message-thread dispatch. Race with render thread and message thread. Other mutating endpoints (trigger_clip, switch_deck) correctly use `callAsync`.

**Verified false at HEAD.** Two commits in scope fix exactly this:
- `f6b208f` "Marshal ApiServer set_param/set_layer_opacity writes to the message thread" (Jul 30) — wraps `handleSetParam`'s clip-effect branch and all of `handleSetLayerOpacity` in `juce::MessageManager::callAsync`.
- `8077af7` "Marshal remaining ApiServer effect-chain writes to the message thread" (Jul 30) — does the same for `handleSetParam`'s global-effect-chain branch, `handleSetEffect`, `handleSetEffectChain`, and `handleReset`'s effect-disable loop.

Read both handlers at HEAD to confirm — `handleSetLayerOpacity` now wraps its entire deck/layer lookup+write in `callAsync`; `handleSetParam`'s clip branch does the same. Counted all marshal sites: `grep -c callAsync src/api/ApiServer.cpp` → 10 sites, matching commit `bacda0d`'s own count ("Extend this-capture safety note to all 10 callAsync sites"). **Every mutating endpoint now dispatches to the message thread; the specific race the doc calls out no longer exists.**

**Severity: CRITICAL** — same reasoning as §4: a specific, falsifiable claim about present-tense unsynchronized behavior, reachable by anyone hitting the two named endpoints, and now simply wrong.

Note: `handleReset`'s `clearImage()`/`clearActiveSource()` calls and `handleLoadImage`/`handleLoadSource`'s `renderer_` calls are explicitly called out in `8077af7`'s own commit message as **still** untouched/still on the HTTP thread ("need a renderer-internal thread-safety design pass first") — so the race isn't fully eliminated everywhere, just at the two specific call sites FEATURES.md names. A rewritten Gotcha should say the named race is fixed but flag the renderer_-call class as the new residual (I did not audit `Renderer`'s thread-safety itself — out of my lane paths — so I can't independently confirm whether that residual is real; I'm relying on the commit's own description).

## 6. MAJOR — production API bind address changed to loopback-only; undocumented

FEATURES.md §14 Config says only "REST API: port 7070, always-on" — no mention of bind address.

Verified (`def0efc`, Aug 2): `ApiServer::start()` now binds `127.0.0.1` by default instead of `0.0.0.0`, overridable via `AUDIODNA_API_BIND` env var. This is a genuine, user-reachable behavior change (remote/other-machine control of the REST API is now off by default) that postdates the norm and isn't reflected anywhere in FEATURES.md.

## 7. MINOR — undocumented new REST surface (Syphon status/toggle)

`/api/syphon` (GET) and `/api/set_syphon` (POST) exist at HEAD (`7d39d25`, Aug 2) and are not mentioned in §13 or §14's endpoint/entry-point lists at all — not even in the "20+" prose count.

## 8. MINOR — stale line-number citations (confirmed drift, not re-flagged as content errors)

Per the audit's own warning that line numbers drift constantly, I checked citations in my lane's sections and confirmed drift (content claims otherwise still correct):
- FEATURES.md §25: `recordClipTrigger()` cited at `MainComponent.cpp:2472` → actual current call site is `MainComponent.cpp:3215` (content claim — "only 1 of 7 event types wired" — re-verified true, see §9 below).
- FEATURES.md §13: `SyphonOutput class (src/output/SyphonOutput.h:21/59)` → actual current class definitions at lines 24 (macOS impl) and 68 (stub); file grew when the new REST toggle comments were added.
- FEATURES.md §14: `ApiServer class (src/api/ApiServer.h:31)` → actual line 32. (Trivial, noting for completeness only.)

## 9. Session Recorder (§25) — re-verified, still accurate

Not in the brief's ask, but in my path (`src/recording`) and cross-referenced by §14, so re-checked:
- `src/recording/*` has 0 commits since norm (confirmed twice, §0 above) — content should be unchanged.
- Re-ran the "only `recordClipTrigger` is wired" claim: `grep -rn "record(ClipTrigger|ParameterChange|ColumnTrigger|MacroChange|TransportChange|EffectToggle|CuepointJump)" src --include='*.cpp' --include='*.h'` → only `recordClipTrigger` has a caller outside `SessionRecorder.cpp` itself (`MainComponent.cpp:3215`). Also checked whether the new Undo v1 commands call any `SessionRecorder` method (plausible integration point for MacroChange/TransportChange/EffectToggle) — zero references to `sessionRecorder`/`SessionRecorder` anywhere in `src/core/`. **Still true: 1 of 7 event types wired.** No drift in substance, only the one line-number citation noted in §8.
- `ApiServer` "holds a reference but exposes no REST endpoints for session recording" — re-verified: `grep -n "sessionRecorder_\." src/api/ApiServer.cpp` → zero hits. Still true.

## 10. Dead surfaces traced

- `MediaReconnect.h`'s `needsVideoReopen()` — traced to `MainComponent.cpp:3524`, called. **Not dead.**
- `TriggerCommands.h`'s `TriggerClipCmd` — traced to `MainComponent.cpp:3335,3383`, constructed and pushed into command history. **Not dead.**
- `DeckCommands.h`'s command classes (`AddLayerCmd`, `RemoveLayerCmd`, `MoveLayerCmd`, `AddDeckCmd`, `RemoveDeckCmd`, `SwitchDeckCmd`, etc.) — spot-checked `withDeckDetached` call sites in `MainComponent.cpp` (20+ sites) confirm these are constructed from real menu/UI actions, not merely declared. **Not dead**, though I did not individually trace every one of the ~11 command classes to a call site — see "could not determine."
- `/api/inject_features` — registered only when `allowFeatureInjection_` (= `testMode_`) is true. Not "dead" exactly (it's the intended test-only surface, and it does have a real consumer — the test harness) but production callers will get a 404 for a route FEATURES.md currently documents as if always present. Flagged in §2/§6.
- The output window / `OutputRenderer` deck-blindness itself (§3) is the pre-existing, already-briefed dead surface — reconfirmed, not new.

## Could not determine

- The task brief's exact "21 functional" figure — I could not find this phrase or an equivalent split anywhere in FEATURES.md's current text to verify or refute against; flagged as a mismatch in §2 rather than guessed at.
- Whether `handleReset`/`handleLoadImage`/`handleLoadSource`'s remaining HTTP-thread `renderer_` calls (explicitly called out as unfixed in commit `8077af7`) constitute a live, reachable race today — that requires auditing `Renderer`'s thread-safety contract, which lives in `src/render/` (outside my lane paths per the task's path list). Reporting the commit's own claim, not independently verified.
- Whether all ~11 `DeckCommands.h` command classes individually have a call site (I confirmed a representative sample via `withDeckDetached` grep, not each class by name).
- CORS preflight (OPTIONS) handling — re-checked, no `.Options(` route exists and `set_post_routing_handler` is unchanged since norm; I did not verify httplib's actual runtime behavior for unmatched-route OPTIONS requests (would require running the server), so I'm treating the doc's existing claim as unchanged/plausible rather than re-confirming it experimentally (task instructions explicitly forbid building/running).
