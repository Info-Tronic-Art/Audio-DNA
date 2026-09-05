# Lane 3 — sources-media — re-norm findings (2026-09-05)

Repo: /Users/boriskarpman/projects/RealTimeAudio
Paths owned: `src/sources`, `src/media`
Baseline: `9139dd4` (2026-07-16) → HEAD

## 1. Scope of drift

```
git log --oneline 9139dd4..HEAD -- src/sources src/media
```
→ **3 commits** touch these paths (I counted the log lines myself):

- `123555e` fix(milkdrop): L0-MD — restore preset autoload, dead on every launch since 22fcedc
- `daa9361` feat(undo): Undo v1 step 3 — SwapClipsCmd
- `7921572` feat(undo): Undo v1 step 2 — SetClipCmd + single-cell sites, replace-undo media fix

`git diff --stat 9139dd4..HEAD -- src/sources src/media`:
```
src/media/VideoPlayer.cpp      |  1 +
src/media/VideoPlayer.h        | 10 ++
src/sources/ProjectMSource.cpp | 19 +++++++++++-------
src/sources/ProjectMSource.h   | 19 +++++++++++++-----
4 files changed, 37 insertions(+), 12 deletions(-)
```

This is a **small, quiet lane** — SourceRegistry.cpp, ProceduralSource.{h,cpp}, PresetSelector.{h,cpp}, ProjectMPresetManager.{h,cpp}, and ImageSequence.{h,cpp} are untouched since the last normalize. The two files that did change (ProjectMSource, VideoPlayer) changed for reasons unrelated to the counts/inventory FEATURES.md documents for this area. Nothing here contradicts the "127 commits landed, ~116 feature-affecting" scale claimed for the repo as a whole — this lane just wasn't where most of that landed.

## 2. Procedural source count: 108 / 18 / 759 — RE-VERIFIED TRUE, no drift

FEATURES.md §9 claims **108 sources, 18 categories, 759 params**, and already carries a note explaining the 628-raw / 754-subtotal / 759-grand-total arithmetic. I re-derived all three numbers directly from `src/sources/SourceRegistry.cpp` and `src/sources/ProjectMSource.cpp`, independent of the doc's own note, and they hold:

**Source count (108):**
- `grep -c 'registerSource(' src/sources/SourceRegistry.cpp` → 103 textual call sites.
- Of those, 102 are literal-id calls (`registerSource("id", ...)`) and one (SourceRegistry.cpp:283) is inside the `registerWireframe` helper lambda, whose body contains one `registerSource(id, ...)` call with a *variable* id — that lambda is itself invoked 7 times (`registerWireframe("wire_sphere",...)` through `"wire_wolf"`, lines ~295-301).
- Net registrations at runtime = 101 literal-string `registerSource` calls (`grep -oE 'registerSource\("[a-z0-9_]+"'` → 101 unique) + 7 wireframe expansions = **108**. Matches the doc exactly.

**Category count (18):**
- Extracted the category string (3rd ctor arg to `ProceduralSource`) per source. Direct literal-category sources group into 16 named categories (3D, Audio-Visual, Fractal, Geometric, Lighting, Lines, Math, Nature, Noise, Organic, Particle, Pattern, Routing, Simulation, Text, Utility) whose per-category source counts sum to 100 (24+9+7+11+1+11+8+6+3+1+3+8+1+3+2+2). Add **Wireframe** (7 sources, category literal is inside the `registerWireframe` lambda body and doesn't match the same regex — verified separately at SourceRegistry.cpp:284) and **MilkDrop** (1 source — `ProjectMSource`, whose ctor at ProjectMSource.cpp:8 passes category `"MilkDrop"`) → **18 categories total**, 100+7+1 = **108 sources**. Matches the doc's per-category table row-for-row (spot-checked 3D=24, Wireframe=7, Routing=1, MilkDrop=1).

**Param count (759):**
- `grep -c 'addParam(' src/sources/SourceRegistry.cpp` → **628** (matches the doc's stated raw count exactly).
- `addTorusControls` (SourceRegistry.cpp:644) is a lambda with 12 `addParam` calls, invoked 7 times (torus family) → contributes 84 at runtime vs. 12 counted once by grep (+72).
- `registerWireframe`'s body has 9 `addParam` calls, invoked 7 times → contributes 63 at runtime vs. 9 counted once by grep (+54).
- 628 − 12 − 9 + 84 + 63 = **754** shader-registry params. `ProjectMSource.cpp` adds 5 more (`grep -c 'addParam(' src/sources/ProjectMSource.cpp` → 5: Beat Sensitivity, Speed, Warp Amount, Decay, Gamma, ProjectMSource.cpp:13-17) → **754 + 5 = 759**. Matches the doc exactly, including its own stated arithmetic.

**Verdict: the 108/18/759 figures are correct as of HEAD.** The "older doc said 64" reference in this session's brief is stale relative to *this* FEATURES.md, which was already re-normalized at 9139dd4 with the right numbers — and since none of the source files that determine this count changed between 9139dd4 and HEAD, the number that was true on 2026-07-16 is still true today. This is a **re-confirmation, not a finding** — I'm recording it so the next reader doesn't have to re-derive it, and doesn't need to chase down the "64" figure as if it were live.

## 3. `Renderer::closeMediaForClip` — zero call sites CONFIRMED TRUE (build-lane premise is SOUND)

This is the item the brief flagged as needing verification before a build session commits to it. **I confirmed the zero-call-sites claim with two independent patterns, plus a third line of evidence (an in-source comment written by the codebase's own authors describing the identical defect).**

**Pattern 1 — exact symbol, whole repo, every extension:**
```
grep -rn "closeMediaForClip" . --include="*.*"
```
→ exactly 2 hits, both in `src/render/`:
- `src/render/Renderer.h:170` — declaration (`void closeMediaForClip(uint32_t clipId);`)
- `src/render/Renderer.cpp:950` — definition

No third hit anywhere in the tree (UI, commands, tests, MainComponent).

**Pattern 2 — call-syntax only (rules out the method merely being mentioned in a comment/string):**
```
grep -rn "[.\>]closeMediaForClip" .
```
→ **0 hits.** Nothing in the codebase writes `renderer.closeMediaForClip(` or `->closeMediaForClip(` anywhere.

**Corroborating evidence — the codebase already documents this as a known gap.** `MainComponent::makeClipMediaHook()` (MainComponent.cpp:3511-3516), the hook that's supposed to keep Renderer's media state in sync with clip-model mutations for undo/redo, contains this comment verbatim:
```cpp
// Risk #4 guard: video/sequence players are keyed by clip id and never
// closed, so redo reconnects for free today. Reconnect-if-missing keeps
// redo correct even if a future wave adds player disposal.
```
That hook only ever calls `renderer.openVideoForClip(...)` / `renderer.openImageSequenceForClip(...)` (reconnect-if-missing) — it never calls `closeMediaForClip`. I traced the actual `Clip > Clear` path (`MainComponent.cpp` case `C::kClipClear`, ~line 4352) end to end: it calls `deck->clearCell(...)` (model-level — resets the `Clip` struct to `nullopt`) and, if the cleared cell was active, `layer->clearActiveClip()` (runtime pointer reset). **Neither touches `Renderer::videoPlayers_`/`imageSequences_` in any way.**

I also confirmed the `.erase()` calls are exactly where the brief said: `Renderer.cpp:958` (`videoPlayers_.erase(it)`) and `:967` (`imageSequences_.erase(it)`), both **only** reachable from inside `closeMediaForClip` itself (grep of all `videoPlayers_`/`imageSequences_` usages in Renderer.cpp shows no other `.erase`/`.clear()` on either map). So the maps are genuinely insert-only from the moment a clip is cleared until process exit.

**One correction to the defect's stated shape:** the brief describes the stranded resource as "an FFmpeg decoder + GL texture + decode thread." I verified `VideoPlayer::close()` (VideoPlayer.cpp:195) does free the FFmpeg codec context (`avcodec_free_context`) and the GL texture (`glDeleteTextures`, VideoPlayer.cpp:348) — those two are real and confirmed never freed for a cleared clip. But **there is no dedicated decode thread** to leak: `grep -n "Thread\|thread" src/media/VideoPlayer.h src/media/VideoPlayer.cpp` shows decoding happens synchronously on the GL thread inside `advanceFrame()`; the only "thread" reference is `codecCtx_->thread_count = 2` (VideoPlayer.cpp:103), which is FFmpeg's *internal* slice-threading for a single decode call, not an application-level thread that outlives the call. So: **decoder + GL texture leak, confirmed; "decode thread" leak, not applicable — there isn't one to leak.**

**Verdict for the build lane: the premise is sound, proceed.** `Clip > Clear` strands a `VideoPlayer` (or `ImageSequence`) instance — with its live FFmpeg codec context and GL texture — in `Renderer::videoPlayers_`/`imageSequences_` for the remainder of the process, keyed by a clip id that the model no longer references. Repeated clear-and-reload of video clips in a single session will accumulate one live decoder+texture per clear. This is a real, wire-traceable resource leak, not a false handoff number.

## 4. FEATURES.md drift findings

| # | Section | Severity | Claim | Reality | Evidence |
|---|---------|----------|-------|---------|----------|
| 1 | §8 Clip & Layer Composition / §11 Video Playback & Media | **MAJOR** | Neither section's "Failure modes" list documents any resource-disposal gap on clip clear; §11's Failure modes only lists codec/corrupt-file/build-time cases. | `Clip > Clear` never calls `Renderer::closeMediaForClip` — see §3 above. This is a real, user-reachable behavior (repeatedly clearing video clips leaks a decoder + GL texture per clear) that belongs in Failure modes/Gotchas for both sections but appears in neither. | `grep -rn "closeMediaForClip" .` (2 hits, no callers); `MainComponent.cpp:3511-3516` comment; `MainComponent.cpp` `case C::kClipClear` (~4352) traced to `clearCell`/`clearActiveClip` only. |

I looked for, and did not find, any other drift in this lane's two FEATURES.md sections (§9 Procedural Sources, §11 Video Playback & Media, plus §17 MilkDrop where it overlaps `ProjectMSource`):
- §11's transport-only claims for `VideoPlayer`/`ImageSequence` (Loop/PingPong/OneShot only; BPM sync/in-out/cuepoints live on `Clip`, not the player classes) — verified against `VideoPlayer.h` header comment and enum (`VideoPlayer.h:53`) — accurate, unchanged since normalize.
- §11's codec list ("MP4, MOV, QuickTime (H.264/H.265/ProRes), HAP/HAP Alpha, AVI... MKV/WebM NOT in header") — matches `VideoPlayer.h:19` verbatim, unchanged since normalize.
- §17's MilkDrop preset-manager wiring: the one substantive change in this lane (`123555e`, hoisting `ProjectMPresetManager` ownership out of `ProjectMSource` into `MainComponent`, fixing a preset-autoload regression introduced by `22fcedc` on 2026-07-30 — both commits fall inside the 9139dd4..HEAD window, so the regression and its fix are both new since the last normalize and net out to zero visible drift at HEAD). I traced the new wiring end to end to make sure it's not another "declared but unconsumed" case: `MainComponent.cpp:1522` `previewPanel_.getRenderer().setProjectMPresetManager(&presetManager_)` → `Renderer::projectMPresetManager_` (Renderer.h:337) → `Renderer.cpp:837` `pmSource->setPresetManager(projectMPresetManager_)` at GL-thread source-creation time → `ProjectMSource::setPresetManager` (ProjectMSource.h:52) sets both `presetManager_` and `presetSelector_`'s pointer. **Fully wired, not a dead surface.** §17's text doesn't describe ownership location either way, so there is nothing in it to correct.

## 5. Dead surfaces checked

- `Renderer::closeMediaForClip` — **dead surface, confirmed** (see §3). This is the highest-value finding in this lane; already captured above rather than repeated here.
- `VideoPlayer::getFile()` (new in `7921572`, VideoPlayer.h:44-48) — traced its consumer: `Renderer::getVideoPlayerFile()` (Renderer.cpp:982) → `MainComponent::makeClipMediaHook()` (MainComponent.cpp:3524, `needsVideoReopen(renderer.getVideoPlayerFile(clip.id), ...)`) for id-stable content-swap detection on replace-undo/redo. **Consumed, not dead.**
- `ProjectMSource::setPresetManager` / the new raw-pointer `presetManager_` — traced above in §4, fully wired.
- Checked whether anything else calls `Renderer::getVideoPlayer`/`getImageSequence` expecting `closeMediaForClip` semantics that might implicitly free things (e.g., an overwrite-in-place path) — no; `openVideoForClip`/`openImageSequenceForClip` (Renderer.cpp:926-946) do `videoPlayers_[clipId] = std::move(player)`, which destroys any prior entry at that key via `unique_ptr` assignment — so **re-opening media on the same clip id does NOT leak** (the old player's destructor runs). The leak is specifically the "clip id goes away with no replacement" path (Clear), not the "clip id gets new content" path (Replace). This is a meaningful nuance for the build lane: the map-assignment overwrite already self-heals the replace case, so the fix only needs to hook the clear/delete paths (`clearCell`, and by extension whole-column/whole-layer/whole-deck clear, and clip-cell deletion via move/swap that leaves a source cell empty).

## 6. Recounted numbers

| What | Doc says | Source says | Changed? | How counted |
|------|----------|-------------|----------|-------------|
| Procedural source count | 108 | 108 | No | `registerSource` literal-id calls (101) + wireframe-lambda expansions (7); see §2 |
| Category count | 18 | 18 | No | Distinct category strings across all `ProceduralSource` ctors + `ProjectMSource`'s `"MilkDrop"`; see §2 |
| Total params | 759 | 759 | No | `addParam` raw grep (628) adjusted for `addTorusControls`×7 and `registerWireframe`-body×7 lambda reuse, +5 from `ProjectMSource`; see §2 |
| `closeMediaForClip` call sites (excl. its own definition) | (not stated as a count; brief's premise) | 0 | N/A — confirmed, not previously in doc | Two grep patterns, see §3 |
| Commits touching src/sources + src/media since 9139dd4 | (not previously stated) | 3 | N/A | `git log --oneline 9139dd4..HEAD -- src/sources src/media` |

## 7. Undocumented-in-source things

Nothing new to add beyond §4's single MAJOR finding — this lane's source surface (SourceRegistry, ProceduralSource, ImageSequence) is otherwise unchanged since the last normalize, so there is no fresh undocumented feature to report here. The one real gap is the media-lifetime leak already covered in §3/§4.

## 8. Could not determine

- **FFmpeg version "8.0"** (FEATURES.md §11 "Dependencies & services"): could not verify from this repo — no `CMakeLists.txt`/`vcpkg.json`/`cmake/` file in the tree pins or names an FFmpeg version (`grep -rn "ffmpeg" CMakeLists.txt` and a repo-wide `vcpkg.json` search both came back empty). This number may have been verified against the build machine's linked library at the original normalize and isn't re-derivable from source alone. Flagging rather than guessing.
- **Whether other clear/delete paths beyond `kClipClear` (whole-layer clear, whole-deck clear, column removal, drag-swap-onto-occupied-cell) also fail to call `closeMediaForClip`** — I traced `kClipClear` fully (§3/§5) and spot-checked that `RemoveColumnCmd` and friends route through `makeClipMediaHook()` (which only opens, never closes) rather than `closeMediaForClip`, so the same gap almost certainly applies repo-wide to every clip-removal path, not just single-cell Clear — but I did not individually trace `kDeckClearClips`/`kLayerClearClips`/`RemoveColumnCmd`'s full command bodies line-by-line (those live in `src/core/DeckCommands.h`, `src/core/ClipCommands.h`, mostly outside this lane's assigned paths). The build session should verify each clear/remove path independently rather than assume `kClipClear`'s finding generalizes untested.
