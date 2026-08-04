# NEW FINDINGS — session 2026-08-04c (from the gesture replay)

---

## FINDING 0 — ROW (e) WAS NOT BLOCKED BY MISSING CONTENT. Presets ARE installed.

**Harmony-VERIFIED by direct disk check of the RUNNING bundle:**
`build/AudioDNA_artefacts/Release/Audio-DNA.app/Contents/Resources/projectm_presets` contains
**31 entries: 30 `.milk` files + `presets.json`**, copied by the CMake POST_BUILD step
(`CMakeLists.txt:534-542`, added 2026-07-30 to fix exactly this empty-browser complaint).

**Harmony-VERIFIED manifest parse:** Energetic **9**, Geometric 9, Psychedelic 9, Calm 2,
Dark 1 = 30. **"Energetic (9)" is the CORRECT label for today's content** — the count is
computed live (`MilkDropBrowser.cpp:233`), not hardcoded.

[RECON-ONLY] Most likely cause of "no presets loaded": **the browser opens on the Files tab
(`BrowserPanel.h:44 activeTab_ = Tab::Files`), and MilkDrop is the 6th and LAST tab.** Boris
probably never navigated to it.

**The one question that settles it:** did he see mood headers with counts, or the message "No
presets loaded. To add MilkDrop presets: ..."? The two states are mutually exclusive
(`MilkDropBrowser.cpp:231` skips empty sections; `:16-27` paints the empty-state text). Wrong
tab -> he saw neither.

**Row (e) is therefore NOT a code defect and NOT blocked. It is re-runnable right now.**

---

## FINDING 0b — DEAD CONTROL: the Preferences MilkDrop folder picker does NOTHING (real defect)

**Harmony-VERIFIED.** `Preferences > Video` renders a "MilkDrop Presets:" label, a text field,
and a Browse button with a real working directory chooser (`src/ui/PreferencesDialog.cpp:67-88`).
The chooser even writes the chosen path into the field (`:86`).

**And the value is never read by anything.** Grep of all `src/` for `milkDropDirEdit_` returns
only construction, styling, visibility, bounds, and that `setText` — **zero reads**. Grep for
`setPresetDirectories` / `rescan()` returns ONLY the definitions and declarations
(`ProjectMPresetManager.cpp:40,45`, `.h:29,33`) — **zero callers anywhere in the codebase.**

**Why this is worse than a merely unfinished control:** the empty-state message a user sees when
they genuinely have no presets says *"Or set a directory in Preferences > Video"*
(`MilkDropBrowser.cpp:16-27`). **The app instructs the user to use a control that cannot work.**
A user with their own `.milk` library today has no way to load it without a code change, and the
UI actively misdirects them. Small, contained, high embarrassment. Deserves its own row.


All claims below were Harmony-verified by direct grep/read at HEAD `20fc53d` after recon
reported them. Recon-only claims are marked [RECON-ONLY].

---

## FINDING 1 — CONTENT LOCK IS BYPASSED BY MULTI-IMAGE DROPS (real defect, data loss)

**`applyFileDrop` honours the content lock. `applyMultiFileDrop` does not.**

- VERIFIED: `contentLocked` is checked at `src/MainComponent.cpp:3617` — inside the SINGLE-file
  drop path: `if (existing->contentLocked) return std::nullopt;`
- VERIFIED: `applyMultiFileDrop` (body from `:3685`) snapshots the cell for undo (`:3689`) and
  goes straight to building the clip — **no `contentLocked` check anywhere in it.** The only
  other `contentLocked` hits in the file are the lock-toggle UI (`:4349`, `:4376-4378`).

**Consequence:** you lock a cell to protect it, then drop 2+ images on it, and it is overwritten
anyway. Single-file drops respect the lock; multi-image drops silently ignore it. This is a
live-performance failure mode — the lock exists precisely to stop mid-set clobbering.

Mitigation that limits severity: the overwrite IS undoable (the cell is snapshotted for undo at
`:3689` and the drop is one transaction). So it is recoverable data loss, not permanent.

**Pre-existing. Not a regression. Not caused by the gesture replay.** Discovered incidentally
while diagnosing something that turned out not to be a bug.

---

## FINDING 2 — GENRE AUTO-SWITCH IS UNREACHABLE DEAD CODE (explains row g)

Boris could not find the feature because **it cannot be reached from the app at all.**

The chain, all VERIFIED:
1. Detection works: `src/render/Renderer.cpp:259-272` fires `onGenreChanged_` when the detected
   genre changes with confidence > 0.1.
2. The handler immediately gates on a flag: `src/MainComponent.cpp:597`
   `if (!composition_.autoPresetOnGenre) return;`
3. **Nothing in the app can set that flag.** Repo-wide, `autoPresetOnGenre` appears ONLY at
   `Composition.h:89` (default `false`), `:214` (serialize out), `:333-334` (deserialize in),
   and the read at `MainComponent.cpp:597`. No UI, menu, shortcut, or API route writes it.
4. **And there is no way to import a JSON that sets it:** `Composition::loadFromFile`
   (`src/model/Composition.h:393`) has ZERO callers in `src/`. (The other `loadFromFile` hits in
   the tree belong to `BindingManager` and `SessionRecorder` — different classes.) The
   Composition menu's Open/Save route to `loadPreset`/`savePreset`, which serialize the effect
   chain and mapping engine, NOT the Composition model.

**Therefore the flag is false for the entire life of every session, and the handler returns at
line 597 every single time.** The feature has never run for Boris, and cannot.

What he probably saw: genre IS displayed read-only in the Audio Readout panel
(`AudioReadoutPanel.cpp:172-174, 518-560`) and over `/api/features`. A live readout with no
control behind it. [RECON-ONLY]

Related, [RECON-ONLY]: `genrePresetNames[8]` (`Composition.h:98`) — the "genre effect preset"
half — is serialized but read by nothing outside tests. That half was never implemented.

### DOC/CODE DIVERGENCE — worth its own attention
[RECON-ONLY] `design/FEATURE_INVENTORY.md:2090,2122` claims "Genre automation config visible in
Composition Inspector". **The code has no such control.** Either the doc is aspirational or it
describes UI that never landed. This is the same disease as the handoff-inheritance problem:
a document asserting a capability the source does not have. Worth an audit pass on that file —
if it is wrong here, it is likely wrong elsewhere, and it is the file a future session would
trust when scoping work.

### LATENT BUG behind the dead feature
[RECON-ONLY, code-level] On a switch to an EMPTY deck, `refreshPreviewFromActiveClip`
(`MainComponent.cpp:3374-3415`) correctly blanks the PREVIEW (`:3406-3413`) — but that purge
branch never touches `outputWindow_`, and `OutputWindow` has `loadImage` with **no
`clearImage`** (`OutputWindow.h:32,89`). So the external OUTPUT window would retain the previous
image while the preview goes blank. **Not exercised today because the feature cannot fire** —
but it would bite immediately if anyone wires the feature up. Fix the clear path in the same
commit that enables the feature, never after.

---

## FINDING 3 — ROW (b) IS NOT A BUG: it is a SHIPPED, DOCUMENTED FEATURE

**CONFIRMED on five independent pieces of evidence** (recon, second pass): image-sequence
playback is a first-class feature, not an accident.
1. `Clip::MediaType::ImageSequence` (`src/model/Clip.h:17`); `Clip.h:164` `isPlayable()` treats
   it as playable media alongside Video.
2. Dedicated subsystem `src/media/ImageSequence.h/.cpp`, alongside VideoPlayer.
3. **A UI control for it already exists**: "Images/ Sec" in `src/ui/ClipInspector.cpp:177`,
   range 0-6 fps (`:185`), shown only for sequence clips (`:454, 610, 889, 1148`).
4. Persisted: `Clip.h:25 sequenceFiles`, reopened on deck load (`MainComponent.cpp:3225`, `:3453`).
5. **Documented in `CLAUDE.md:598`**: "Image sequence playback (multi-image drag-drop as video)
   with configurable FPS". Also `CLAUDE.md:453`, `CONTEXT.md:76`, two archived docs.

**Boris hit a feature of his own app that he did not have in mind at drop time.** The behavior
is correct; the DISCOVERABILITY is the defect. The only signal that a sequence was created is an
"Images/ Sec" slider quietly appearing in the inspector.

**One-second confirming test Boris can run himself:** select that cell and look for the
"Images/ Sec" slider in the inspector. Present = it is a sequence clip = intended path.

**Threshold is exactly 2** (`images.size() > 1` -> sequence). Whether 2 is the right threshold
is a product question, not a code fact.

### Original diagnosis (superseded but kept, since the reasoning still holds)

[RECON-ONLY, quoted code] `applyMultiFileDrop` (`MainComponent.cpp:3691-3719`) deliberately
collapses ANY multi-image drop into ONE `Clip::MediaType::ImageSequence` at
`sequenceFps = 2.5f`, holding both files in `clip.sequenceFiles`. Videos then start at the next
cell (`videoStartCol = col + (images.empty() ? 0 : 1)`, `:783-824`). The same asymmetry is
duplicated in the internal drag path (`ClipCell.cpp:462-487`).

So: **N images -> 1 sequence cell. N videos -> N cells.** Boris's "2 images on one cell" was the
code doing exactly what it specifies. No data loss, single undo transaction.

**This is now a PRODUCT question, not an engineering one:** is image-collapse-to-sequence the
behavior Boris wants? It is a normal VJ idiom (drop a frame sequence, get an animation), but he
did not expect it. Only he can rule. Changing it would touch both drop paths plus
`handleMultiFileDrop`.
