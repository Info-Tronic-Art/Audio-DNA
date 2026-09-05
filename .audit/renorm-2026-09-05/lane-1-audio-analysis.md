# Lane 1 — Audio Analysis — Drift Report

Scope: `src/audio`, `src/analysis`, `src/features`. Base sha `9139dd4` (2026-07-16 norm) → HEAD.
All commands below were run from `/Users/boriskarpman/projects/RealTimeAudio`.

## 0. Scope of drift

```
$ git log --oneline 9139dd4..HEAD -- src/audio src/analysis src/features
cb4d5fa refactor(featurebus): seqlock conversion — single-writer value-copy protocol (S2: R1-R5+R7+R9)
08f7361 Fix FeatureBus TSan race: acq_rel on the triple-buffer CAS hand-off
fb271e3 Wave 1-D: five small fixes (transport, Clear-Clips bug, waveform seqlock, SR guard, tooltips)
45ae7e8 fix(xs): Wave 0 Group 2 — FeatureSnapshot defaults, 6 sources, set_bpm
368d621 refactor(cleanup): Wave 0 Group 1 — purge dead code (13 items)
```
**5 commits** touch my paths (I counted the lines myself: `wc -l` on the same command = 5).

```
$ git diff --stat 9139dd4..HEAD -- src/audio src/analysis src/features
 src/analysis/AnalysisThread.cpp |  56 ++++++++++---
 src/analysis/AnalysisThread.h   |  19 ++++-
 src/analysis/BPMTracker.cpp     |  15 ----
 src/analysis/BPMTracker.h       |   5 --
 src/analysis/FeatureSnapshot.h  |   2 +
 src/analysis/GenreSmoothing.h   |  72 -----------------
 src/analysis/OnsetDetector.cpp  |  15 ----
 src/analysis/OnsetDetector.h    |   5 --
 src/audio/AudioEngine.cpp       |   6 ++
 src/audio/AudioEngine.h         |   3 +
 src/features/FeatureBus.cpp     | 163 +++++++++++++++++++++++++-------------
 src/features/FeatureBus.h       | 171 +++++++++++++++++++++++++++++-----------
 src/features/Smoother.h         |  62 ---------------
 13 files changed, 305 insertions(+), 289 deletions(-)
```

**Important context that changes how this report is framed.** `.harmony/FEATURES.md` was itself
touched by two commits inside the 9139dd4..HEAD window that are *not* in my path list but do
rewrite my sections: `c30e393` (docs: sync living docs to Wave 0+1 reality, 2026-07-17) synced
Sections 2/3/16 to the Wave 0 (`368d621`, `45ae7e8`) and Wave 1-D (`fb271e3`) commits — and got it
right, I re-verified all of it below. But `c30e393` landed **before** `08f7361` (TSan acq_rel fix)
and `cb4d5fa` (seqlock conversion), which are the two *newest* commits in this lane
(chronological order, oldest→newest: `368d621` → `45ae7e8` → `fb271e3` → `c30e393` [doc sync] →
… → `08f7361` → `cb4d5fa`). So the doc is not stale relative to 2026-07-16 across the board — it
is stale specifically relative to the seqlock rewrite that landed *after* its own last sync. That
is the headline finding below.

I confirmed the ordering with:
```
$ git log --oneline 9139dd4..HEAD | cat -n | grep -E "cb4d5fa|08f7361|fb271e3|45ae7e8|368d621|c30e393"
     4  a4e2efd  (does not touch my sections — verified via git show --stat, only VALIDATION.md/gotchas.md/FEATURES.md §24)
    39  cb4d5fa
    48  08f7361
   117  c30e393
   118  fb271e3
   123  45ae7e8
   124  368d621
```
(`git log --oneline` lists newest first, so lower line number = more recent commit.)

---

## Findings

### FINDING 1 — CRITICAL — Section 3 "Feature Transport" describes an API and mechanism that no longer exists

**FEATURES.md §3 claims** (lines ~147–193 as of HEAD):
- "Transfers the complete FeatureSnapshot from analysis thread to render thread via lock-free
  **triple-buffer atomic swap**"
- Entry point: `FeatureBus` class — "**triple-buffer transport**"
- Implementation chain: "Analysis thread calls `FeatureBus::write(snapshot)`" / "Render thread
  calls `FeatureBus::read()` — reads latest snapshot via **atomic index read**"
- Gotcha: "Triple buffer means analysis can write every 10.7ms while render reads every 16.67ms"
- Gotcha: "`getLatestRead()` returns the LAST consumed snapshot, not the latest available —
  different from `read()`."
- Test coverage: "single write-read, multiple writes, latest-read, **slot independence**, writer
  doesn't clobber reader" / "Missing: concurrent stress tests (actual multi-thread contention)"

**Source says (current HEAD):** None of this is true any more. `cb4d5fa` replaced the triple
buffer entirely with a **seqlock, single-writer value-copy protocol**. Verified:

```
$ grep -n "buffers_\[3\]\|kNumBuffers\|std::array<FeatureSnapshot" src/features/FeatureBus.h src/features/FeatureBus.cpp
(no output — no triple buffer, no per-slot array, anywhere in the file)
```

Current mechanism (`src/features/FeatureBus.h:39` onward, comment block at top of file):
- `seq_`: a `std::atomic<uint64_t>` generation counter (odd = publish in progress).
- `words_`: the 320-byte `FeatureSnapshot` payload reinterpreted as 80 `std::atomic<uint32_t>`
  words (`static_assert(sizeof(FeatureSnapshot) == 320)`).
- `staging_`: a single writer-private `FeatureSnapshot` — not three buffers, not an index.
- Writing goes through a **move-only `FeatureBus::Writer` handle** obtained via
  `createWriter()`. A second concurrent claim deterministically returns an invalid handle
  (`isValid() == false`). The bus itself "exposes no mutating API" (comment, FeatureBus.h:38).
- `FeatureBus::write(snapshot)` **does not exist** — confirmed:
  ```
  $ grep -n "void write\|::write(" src/features/FeatureBus.h src/features/FeatureBus.cpp
  (no match)
  ```
  The writer path is `Writer::acquireWrite()` → fill `snap` → `Writer::publishWrite()`.
- `getLatestRead()` **does not exist** — confirmed:
  ```
  $ grep -n "getLatestRead" src/features/FeatureBus.h src/features/FeatureBus.cpp
  (no match)
  ```
  Reader API is `read() const` and `readIfNewer(FeatureSnapshot&, uint64_t& lastSeq) const`,
  both bounded-retry seqlock reads (`kMaxReadAttempts = 4`).
- Every call site outside FeatureBus was updated to match — I checked all consumers:
  ```
  $ grep -rn "FeatureBus::write\|\.write(snapshot)\|createWriter" src/ | grep -v FeatureBus.h | grep -v FeatureBus.cpp
  src/MainComponent.cpp:1613: (comment referencing createWriter/Writer handle)
  src/MainComponent.cpp:1617:    analysisThread_.setFeatureBusWriter(analysisThread_.getFeatureBus().createWriter());
  src/MainComponent.cpp:1626:            analysisThread_.getFeatureBus().createWriter(),
  ```
  ```
  $ grep -n "\.read()" src/MainComponent.cpp
  2482, 2927, 3091, 3225:  const FeatureSnapshot snap = analysisThread_.getFeatureBus().read();
  ```
  So the real, live chain is: `AnalysisThread::run()` calls
  `featureBusWriter_.acquireWrite()` / `...publishWrite()` (AnalysisThread.cpp:103, 316) →
  4 call sites in `MainComponent.cpp` call `.read()` → one of them
  (`Renderer.cpp:446 compositor_.setLatestSnapshot(snap)`) feeds
  `CompositorEngine::uploadAudioUniforms()`. The **protected chain still works end to end** —
  I am not proposing any change to it — but its transport mechanism and its public API are both
  described falsely in §3.
- Test coverage claim is also stale. Actual `tests/test_feature_bus.cpp` (10 `TEST_CASE`s,
  confirmed by `grep -c "TEST_CASE"`):
  `FeatureBus initial state`, `single write-read cycle`, `read returns the last published
  snapshot repeatedly`, `multiple writes before read`, `writer does not clobber a reader's copy`,
  `staging buffer is stable and writer-private`, `createWriter enforces the single-writer claim`,
  **`concurrent write-read stress test`**, **`multi-reader coherence under concurrent publish`**,
  and `FeatureSnapshot clear restores struct-default genre/energy`. There is no "slot
  independence" test (there are no slots left to be independent), and the doc's own "Missing:
  concurrent stress tests" line is now **false** — two such tests exist and were added by this
  exact refactor (`git diff --stat` shows `tests/test_feature_bus.cpp` grew by ~336/-125 lines
  in this window).

**Why CRITICAL, not MAJOR:** §3 is the doc's description of a subsystem the task brief itself
calls out as VERIFIED WORKING and asks me to "document accurately." Every mechanism noun in the
current text (triple-buffer, index swap, `write()`, `getLatestRead()`) is provably absent from
source, and the doc's own stated test gap has been closed without the doc noticing. A reader of
§3 today would misdescribe the mechanism, cite two non-existent methods, and repeat a coverage
gap that's already fixed.

**Fix needed (for the doc's next writer, not done here):** rewrite §3's "What it does",
"Implementation chain", the `write()`/`getLatestRead()` bullets, the triple-buffer gotcha, and
the test-coverage list to match the seqlock/Writer-handle design above. Section 2 step 16
("Publishes complete `FeatureSnapshot` to FeatureBus via triple-buffer swap") has the identical
problem and needs the same correction.

---

### FINDING 2 — MINOR — u_beatPhase "declared vs. used" count: refine ~12 to an exact 11

The task brief states the last two documented figures were "3 declared / 0 used" (already known
false) and "56 declared / ~12 used." I re-derived this from scratch across the whole repo, not
just my lane paths, since the consumer side (shaders) lives in `src/render`/`src/sources`.

**Method:** shaders are not on-disk `.frag`/`.vert` files — none exist
(`find . -iname "*.frag" -o -iname "*.vert" -o -iname "*.glsl"` → 0 hits). All GLSL is embedded as
C++ raw-string literals, overwhelmingly in `src/render/EmbeddedShaders.h` (12,466 lines). I parsed
every `(inline|static|static constexpr) const char* NAME = R"( ... )";` block in that file
(three different declaration styles are used in the file — confirmed by grep, e.g.
`static constexpr const char* harmonicDisplace = R"(#version 410 core`) and:
1. Counted a shader as **declaring** `u_beatPhase` iff its body contains the line
   `uniform float u_beatPhase;`.
2. Counted a shader as **using** it iff `u_beatPhase` appears on any *other* line inside that same
   shader body (a use in an expression, not the declaration line itself).

```
total shader blocks parsed: 268   (matches grep -c 'R"(' on the file exactly, so no block was missed)
declared (uniform float u_beatPhase;): 56
used (referenced beyond its own declaration line): 11
```
The 11 using shaders, each referencing it exactly once:
`sourceTwistedTorus`, `sourceLissajousWeaver`, `sourceFermatSpiral`, `sourceHyperbolicTiling`,
`sourceAstralGrid`, `sourceSacredGeometry`, `rhythmSlice`, `densityWave`, `sourceSpectralRing`,
`sourceTextWall`, `sourceTextAnimator`.

Sanity check on totals: `grep -c u_beatPhase src/render/EmbeddedShaders.h` = 68 lines total.
56 (declarations) + 11 (uses) = 67; the 68th hit is a doc comment at line 2547
(`//                  u_beatPhase, u_spectralCentroid, u_onsetStrength`) that sits outside any
shader block — not a declaration or a use, just a comment. That accounts for the discrepancy
cleanly.

I also confirmed the three non-shader occurrences of `u_beatPhase` in `.cpp` files are the C++
upload sites, not shader code: `CompositorEngine.cpp:1433`, `ProceduralSource.cpp:169`,
`EffectChain.cpp:330` — these look up the uniform location and upload `snap.beatPhase` each
frame; they are not double-counted in the 268-block parse above (that parse only covers
EmbeddedShaders.h).

**Verdict:** 56 declared / **11** used, not "~12." The "~12" hedge in the prior handoff was close
but not exact — worth pinning down since "declared but unused" is this codebase's named recurring
defect class. 45 of the 56 declaring shaders (80%) pull in a beat-phase uniform their body never
reads — that's the dead-surface shape this repo has been burned by before, just smaller than the
last time someone measured it.

---

### FINDING 3 — MINOR — `uploadAudioUniforms()` uploads 29 distinct uniforms per effect per frame, not 28

The task brief's "PROTECT THIS ASSET" note states the chain "publishes 28 uniforms per effect per
frame." FEATURES.md itself does not state this number anywhere (`grep -n "uploadAudioUniforms\|28
uniform"  .harmony/FEATURES.md` → no hit), so this is not a doc-drift finding against FEATURES.md,
but since I was asked to verify the protected asset, I counted it:

```
$ awk '/^void CompositorEngine::uploadAudioUniforms/,/^}/' src/render/CompositorEngine.cpp \
    | grep -o 'loc("[a-zA-Z_]*")' | sort -u | wc -l
29
```
The 29: `u_rms, u_bass, u_mid, u_high, u_beatPhase, u_barPhase, u_phrasePhase,
u_spectralCentroid, u_spectralFlux, u_onsetStrength, u_onsetDetected, u_dominantPitch,
u_pitchConfidence, u_detectedKey, u_keyIsMajor, u_structuralState, u_bpm, u_hcdf,
u_bandEnergies (7-float array), u_chromagram (12-float array), u_mfccs (13-float array),
u_genre, u_genreConfidence, u_energyState, u_sidechainPump, u_swingRatio, u_formantPresence,
u_resonancePeak, u_reeseBass`.

The chain itself is confirmed intact and working as described: `AnalysisThread::run()` publishes
via the writer handle → `MainComponent.cpp` reads via `FeatureBus::read()` (4 call sites) →
`Renderer.cpp:446` calls `compositor_.setLatestSnapshot(snap)` → `CompositorEngine.cpp:369` calls
`uploadAudioUniforms(program)` once per effect-slot inside the per-layer render loop. I am not
proposing any change to this — flagging only that "28" undercounts by one against current source,
in case that number is repeated elsewhere in the norm.

---

### FINDING 4 — MINOR — entry-point line number for `FeatureBus` class has drifted

§3 entry points cites `FeatureBus` class at `src/features/FeatureBus.h:21`. Current:
```
$ grep -n "^class FeatureBus" src/features/FeatureBus.h
39:class FeatureBus
```
Line drifted from 21→39 because the seqlock rewrite added an 18-line header comment block above
the class. Purely cosmetic (folded into Finding 1's rewrite, not a separate action item) but
listed separately since the task asked to anchor on text, not offsets, and this is a concrete
example of why.

---

## Re-derived numbers (recount table)

| What | Doc says | Source says | Changed? | How I counted |
|---|---|---|---|---|
| Commits touching my lane paths since 9139dd4 | (not stated) | 5 | n/a | `git log --oneline 9139dd4..HEAD -- src/audio src/analysis src/features \| wc -l` |
| Audio analysis pipeline stages (comment-numbered) | 14 | 14 | No | Counted `// --- N. ...` stage comment headers in `AnalysisThread::run()`, 1 through 14, all present |
| Profiled timing slots (stageNames array) | 13 | 13 | No | `stageNames[]` literal array in AnalysisThread.cpp has 13 string entries; loop is `for (int s=0; s<13; ++s)` |
| `kNumStages` (array size) | 14 | 14 | No | `AnalysisThread.h:149: static constexpr int kNumStages = 14;` — confirmed unused 14th slot is never written (not merely "unlogged"); `stageTimesUs_[13]` has zero write sites anywhere in AnalysisThread.cpp |
| `FeatureSnapshot` field count | 40 | 40 | No | Manually enumerated every named member in the struct (arrays counted as 1 field each, matching doc's convention): 40 exactly, confirmed after the two Wave-0 default-value lines were added (no new fields added) |
| FeatureBus transport mechanism | "triple-buffer atomic swap" / index read | Seqlock, single-writer value-copy (`seq_` + `words_[80]` + `staging_`) | **Yes — full rewrite** | Read FeatureBus.h/.cpp in full; grepped for triple-buffer array (none), `write()` (none), `getLatestRead()` (none) |
| `u_beatPhase` declared in shaders | 56 (prior handoff) | 56 | No | Parsed all 268 `R"(...)"` shader blocks in `EmbeddedShaders.h`, counted `uniform float u_beatPhase;` lines |
| `u_beatPhase` actually used | ~12 (prior handoff) | 11 | Refined | Same parse; counted shaders with a `u_beatPhase` occurrence on a line other than its own declaration |
| Uniforms uploaded per effect per frame (`uploadAudioUniforms`) | 28 (task brief, not FEATURES.md) | 29 | Refined | `awk` range extraction of the function body + `grep -o 'loc("[a-zA-Z_]*")' \| sort -u \| wc -l` |
| `FeatureBus` class line anchor | FeatureBus.h:21 | FeatureBus.h:39 | Yes (drift) | `grep -n "^class FeatureBus"` |
| `tests/test_feature_bus.cpp` TEST_CASE count | implied ~5 (named list) | 10 | Yes | `grep -c "TEST_CASE" tests/test_feature_bus.cpp` |

---

## Dead surfaces traced

- `BPMTracker::setThreshold`, `setSilence`, `setPhraseBars` and `OnsetDetector::setThreshold`,
  `setSilence`, `setMinInterOnsetMs` — all six removed in `368d621` (Wave 0 Group 1 cleanup).
  I checked whether FEATURES.md ever named these as documented, wireable controls:
  `grep -n "setThreshold\|setSilence\|setPhraseBars\|setMinInterOnset" .harmony/FEATURES.md` →
  no hits. So this is a correctly-executed dead-code removal, not a doc-drift finding — noting
  it here only because the task explicitly asks me to trace every control I encounter; these six
  had zero call sites before removal (that's *why* they were removed) and now have zero lines of
  code, consistent outcome, nothing to flag.
- `GenreSmoothing` class (deleted, 72 lines) and `OneEuroFilter` inside `Smoother.h` (deleted, 62
  lines) — both already correctly marked "removed Wave 0" in FEATURES.md §3 and §16 (synced by
  `c30e393`, which I verified predates the seqlock work but postdates these two removals — correct
  order, correct doc state). No drift.
- `FeatureBus::Writer` double-claim path: `createWriter()` called twice returns an invalid second
  handle by design (R4 in the header comment) — this is a genuine safety property, not a dead
  surface; I traced it to `MainComponent.cpp:1617/1626` where the one real writer is claimed and
  handed to `AnalysisThread`/`TestServer`, confirming single production writer as designed.

No newly-introduced dead surface (declared-but-unconsumed field/control) was found in this lane's
5 commits — the `u_beatPhase` 56-vs-11 gap (Finding 2) is not new to this window; it is a
pre-existing shape I was asked to re-measure precisely, not something these 5 commits caused or
worsened.

## Could not determine

- Whether "~30 audio features" (§2's headline count) is itself still accurate. This number is
  presented as a soft/hedged figure layered on top of the two hard counts (58 mapping sources / 40
  FeatureSnapshot fields) that I *did* verify. The 58-mapping-sources figure lives in
  `src/mapping/MappingTypes.h`, which is outside my assigned paths (`src/audio`, `src/analysis`,
  `src/features`) and had zero commits in this window per `git log --oneline 9139dd4..HEAD --
  src/mapping` — I did not audit it and defer to whichever lane owns `src/mapping`. I did confirm
  the 40-field FeatureSnapshot count directly (see recount table) and it did not change.
- Whether any consumer outside `src/` (test harnesses, `tests/test_feature_bus.cpp` stress tests)
  ever exercises `readIfNewer()` — I found the method exists and is documented in-header, but did
  not find a call site for it anywhere in `src/` or `tests/`:
  `grep -rn "readIfNewer" src/ tests/` returns only the FeatureBus.h declaration and
  FeatureBus.cpp definition. This may be a second, smaller dead-surface finding (an API with zero
  callers) but I'm listing it as "could not determine" rather than a confirmed finding because a
  header-only utility method with no current caller is a normal, low-severity shape (unlike a UI
  toggle with no consumer) and I did not have budget to check whether it's intended as
  forward-looking public API (e.g. for a future HTTP polling consumer) versus truly dead. Worth a
  one-line mention in the next norm's gotchas if the doc's author wants to flag it.
