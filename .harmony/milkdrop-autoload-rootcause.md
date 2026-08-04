# MILKDROP PRESETS NEVER AUTOLOAD — ROOT CAUSE (2026-08-04d)

**Status: root cause VERIFIED. Not yet fixed.** Regression, not a race.

Boris: *"we need to figure out why the milkdrop presets are not autoloading every time."*

---

## VERDICT

**"Not every time" means not every BUILD, not every launch.** Since `22fcedc` (2026-07-30)
the MilkDrop preset wiring is skipped on **every single launch**. Before that commit it ran on
every launch. Boris was comparing across builds — which is why it read as intermittent and
sent the investigation looking for a race that does not exist.

## THE CHAIN (every link VERIFIED; Harmony re-derived all of it independently)

1. The MilkDrop wiring block lives in the **MainComponent CONSTRUCTOR**
   (`src/MainComponent.cpp:1478`, anchor `// === v2: MilkDrop Preset Browser Wiring ===`).
2. It is guarded by `if (pmSource)`, where `pmSource` comes from
   `getOrCreateSource("projectm_visualizer")` (`MainComponent.cpp:1481-1483`).
3. `Renderer::getOrCreateSource` returns **nullptr unless the GL context is attached**
   (`src/render/Renderer.cpp:787`).
4. JUCE attaches a context only when it has a peer and non-zero size (`canBeAttached` requires
   `isShowingOrMinimised` && w>0 && h>0). At constructor time none of that is true —
   `src/Main.cpp` constructs MainComponent **before** `setContentOwned` / `setVisible(true)`.
5. => `pmSource == nullptr` **always**. The whole block is skipped: no `scanDirectory`, no
   `loadManifest`, no `setPresetSelector`, no `onPresetSelected`, and critically no
   **`setPresetManager`** — whose ONLY call site repo-wide is inside that block
   (`MainComponent.cpp:1512`).

### THE DECIDING EVIDENCE — from the live app, no relaunch required
- `grep -ic milkdrop /tmp/adna-err.log` → **0**, across all logged sessions.
- `[Eyes] Test server started` — **77 lines LATER in the same constructor**
  (`MainComponent.cpp:1612`) — **IS present**, twice.
- The constructor therefore demonstrably ran *past* the MilkDrop block without printing.
  `pmSource` was null. **This is proof, not inference.**

### THE REGRESSION COMMIT
`22fcedc` **2026-07-30** — *"Harden activeSources_: confine mutation to the GL thread."*
`git show 22fcedc^:src/render/Renderer.cpp | grep isAttached` returns **EMPTY** — the GL gate
did not exist before it. VERIFIED.

**A correct thread-safety fix that silently severed a feature on its way past.** Its own commit
message names "MilkDrop preset-manager wiring in MainComponent.cpp" as a caller it was fixing.
It severed it instead. Nothing caught this because nothing tests it.

## WHAT THE USER SEES (per-path, and they discriminate)
The empty-state string is shared by the null-manager and zero-presets cases, so that text alone
does NOT discriminate. These do:
- **This bug (null manager):** "No presets loaded…" on **ALL FOUR sub-tabs including Favorites
  and Recent** — `paint()` early-returns before the sub-tab switch. **No mood headers at all.**
  Jukebox **Play** flips to "Stop" and turns red but nothing happens (`toggleJukeboxPlay` wraps
  its body in `if (presetSelector_)`, also null). Status label never shows "MilkDrop: …".
- **A merely-empty scan:** Favorites shows *"No favorites yet. Right-click to star."* and Recent
  shows *"No recently used presets."* — different strings, different code paths.

**THE ONE QUESTION: click the Favorites sub-tab.** Still "No presets loaded" ⇒ this bug.

## RULED OUT
- **Not missing content.** 31 entries in `Contents/Resources/projectm_presets` in the running
  Release bundle. VERIFIED.
- **Not a race, not lazy-load, not a latched scan guard.** No threading and no
  `loaded_`/`hasScanned_` guard anywhere in this path; `MilkDropBrowser::refresh()` only
  repaints and every read is live off `presetManager_`. VERIFIED.
- **Not the CWD dev fallback** (`MainComponent.cpp:1494-1496`) — masked by this bug and only
  reachable for a non-bundled/Debug binary.
- **Not `/tmp/milkdrop-presets`** (`:1507-1509`) — absent; would change the COUNT, never
  empty-vs-full.
- **The dead Preferences folder picker is ORTHOGONAL** — separate defect, fix separately.

## FIX SHAPE (assess before building — not yet architect-ratified)
Preset scanning is pure file/JSON work with **no GL dependency** and should never have been
gated on a GL-thread object. Hoist `ProjectMPresetManager` out of `ProjectMSource` (currently a
by-value member, `src/sources/ProjectMSource.h:105`) into `MainComponent`; scan and wire it
**unconditionally** in the ctor with no `if (pmSource)` guard; give `ProjectMSource` a
non-owning pointer set when the GL thread later creates it; re-point
`presetSelector_.setPresetManager(...)` at the hoisted instance. **~40 lines** across
`MainComponent.{h,cpp}` + `ProjectMSource.{h,cpp}`.

**RISK TO VERIFY BEFORE BUILDING:** claimed to *strengthen* rather than reopen the UAF
documented at `Renderer.cpp:699-712` (the manager would outlive every source). And explicitly:
do **NOT** "fix" this by retry-wiring from a context-created hook — that re-creates exactly the
dangling-interior-pointer hazard that comment describes. Both claims are INFERRED and must be
confirmed by the builder/reviewer.

## CORRECTS A PRIOR SESSION
Gesture-list row **e** was closed on the inference that Boris "almost certainly never navigated
to the MilkDrop tab." **He had. It was broken.** The tab genuinely showed no presets. The prior
session's own unasked question ("did you see mood headers, or the empty-state text?") would have
caught this. **Closing a row on inference instead of asking cost 5 days.**
