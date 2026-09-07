# Reviewer Verdict — L0-MD-milkdrop-autoload-s-rta-0904
STATUS: PARTIAL
VERDICT: FAIL

FILES: RealTimeAudio/src/MainComponent.cpp, MainComponent.h, render/Renderer.cpp, render/Renderer.h, sources/ProjectMSource.cpp, sources/ProjectMSource.h

## VERDICT: FAIL

One defect, but it is exactly the question this review was commissioned to
adjudicate (#4), and the answer to "is the ordering safe" is NO, not as
written — even though nothing in today's code paths currently exploits it.
Everything else in the lane (1, 2, 3, 5, plus the ALSO-CHECK list) is clean.
Fix is a one-line member reorder, not a design failure.

## 1. HOIST BOUNDARY — PresetSelector did NOT hoist. OBSERVED, PASS.
`ProjectMSource.h` still declares `PresetSelector presetSelector_;` as a
by-value member (unchanged by the diff — grep confirms no removal/move of
that declaration). `PresetSelector::processFrame` and its `manager_` pointer
(`src/sources/PresetSelector.h:43`, `.cpp` lines 6-71) are untouched by this
diff. Only a raw pointer to the *manager* is injected into ProjectMSource
(`setPresetManager`); the selector object itself never leaves ProjectMSource,
and no GL-thread work moved to the message thread.

## 2. LAZY SELECTOR WIRE — thread-correct. OBSERVED, PASS.
Wire happens in two parts, correctly split by thread:
- `pmSource->setPresetManager(projectMPresetManager_)` — runs synchronously
  on the **GL thread** inside `Renderer::getOrCreateSourceOnGLThread`
  (Renderer.cpp:837). This only touches ProjectMSource/PresetSelector
  internals (no browser/UI object), so GL-thread execution is fine.
- The **browser-mutating** half —
  `browserPanel_->getMilkDropBrowser().setPresetSelector(&pmSrc->getPresetSelector())`
  — is the body of the `onProjectMSourceCreated_` callback, which Renderer
  fires via `juce::MessageManager::callAsync(...)` (Renderer.cpp:839-841).
  So the only line that mutates browser UI state runs on the **message
  thread**, not the GL thread. This exactly mirrors the pre-existing
  `onAutopilotAdvanced_`/`onGenreChanged_` callAsync pattern in the same file
  (Renderer.cpp:252-268), which the packet asked it to match — confirmed
  identical shape (`auto callback = onX_;` then `callAsync`).
- Confirmed `getOrCreateSourceOnGLThread` is only ever reached from the GL
  thread: either directly when already on it, or marshaled via
  `glContext_.executeOnGLThread(...)` in the caller above it
  (Renderer.cpp:798-805).

## 3. DO-NOT-CLEAR RULE — preserved, correctly narrowed. OBSERVED, PASS.
Anchor comment (`Renderer.cpp:700-702`, `do not activeSources_.clear() here`)
is unchanged, still directly above the unmodified
`for (auto& [id, src] : activeSources_) src->releaseGL();` loop
(Renderer.cpp:722) — no `activeSources_.clear()` was (re)introduced anywhere
in the diff. The new text (Renderer.cpp:704-720) keeps the original UAF
history as-is, then adds: "This rule REMAINS LOAD-BEARING for
`MilkDropBrowser::presetSelector_`... PresetSelector was deliberately NOT
hoisted... and stays a per-source member, so destroying a source here would
still dangle that pointer." That is the correct, narrower claim (selector
only, not manager) and matches the packet's CAVEAT verbatim. Not relaxed,
not moved, correctly scoped down.

## 4. THE UAF QUESTION — hoist does NOT safely outlive the browser as claimed. INFERRED risk, OBSERVED ordering → FAIL.
Declaration order in `MainComponent.h` (confirmed by direct read, lines
204-312):
```
204: PreviewPanel previewPanel_{...};      // owns Renderer -> ProjectMSource
...
301: std::unique_ptr<BrowserPanel> browserPanel_;   // owns MilkDropBrowser
309: ProjectMPresetManager presetManager_;           // the hoisted manager
```
C++ destroys members in **reverse declaration order**. `presetManager_` is
declared *after* both `previewPanel_` and `browserPanel_`, so it is
destroyed **before** both of them — the exact inversion of what the new
comment at MainComponent.h:307-308 claims: *"Outlives Renderer and every
ProjectMSource, so the browser's pointer into it never dangles."* As written,
during `~MainComponent()`: `presetManager_` is destructed first; `browserPanel_`
(holding `MilkDropBrowser::presetManager_` pointing at it) and `previewPanel_`
(owning Renderer → `projectMPresetManager_`, and every live `ProjectMSource`
→ `presetManager_`/`PresetSelector::manager_`) are both destructed *later*,
each carrying a dangling pointer for that window.

**Does this crash today?** No, and I looked for it directly:
- `ProjectMSource::~ProjectMSource()` is empty but for a comment
  (ProjectMSource.cpp:24-27) — no dereference of `presetManager_`.
- `PresetSelector::manager_` is only read from `processFrame`/`nextPreset`/
  etc (PresetSelector.cpp), none of which fire during teardown.
- `MilkDropBrowser::~MilkDropBrowser() = default;` — no explicit dereference
  of `presetManager_`/`presetSelector_` in its own or `BrowserPanel`'s
  destructor.
- `Renderer::~Renderer()` just calls `detach()`, which routes through
  `openGLContextClosing()` — that only calls `src->releaseGL()`, never
  touches the preset pointers.
So the dangling window exists but nothing in the current code walks into it.
That is a landmine, not a live bug: the comment asserts a lifetime guarantee
the declaration order does not provide, and the *entire reason this pointer
class is treated as load-bearing in this codebase* (Renderer.cpp:704-712's
own UAF history) is that a future addition — a "flush favorites on close",
a final preset-name log, a destructor-time repaint — would silently UAF
exactly where the comment tells the next engineer it's now safe. Given this
is the single question the packet flagged as the highest-risk part of the
lane (HARD CONSTRAINTS, item 2) and the review packet was explicitly built
to adjudicate it, I'm calling this a blocking defect rather than a note.

**Fix:** move the `presetManager_` declaration above `previewPanel_` (i.e.
near the top of `MainComponent`'s private members, before anything that
transitively holds a pointer into it — `previewPanel_` and `browserPanel_`
both need to be declared after it). One-line move, no logic change.

## 5. `presetManager_` INSIDE ProjectMSource — fully audited, all null-guarded. OBSERVED, PASS.
All 7 remaining uses (grep, `ProjectMSource.cpp`/`.h`) are exhaustively
guarded:
- `initGL`: `if (presetManager_ && presetManager_->getPresetCount() > 0)`
- `nextPreset`/`prevPreset`/`randomPreset`: each starts `if (!presetManager_) return;`
- `getCurrentPresetName`: ternary-guarded
No unguarded `presetManager_->` or leftover value-type `presetManager_.` use
found anywhere (two separate grep patterns, both empty). Old
`getPresetManager()` accessor has zero remaining callers repo-wide (grepped
`getPresetManager` and `->getPresetManager` separately — both empty), so the
removed API left no dangling caller.

## ALSO CHECK
- **Null-dereference / browser guards**: `MilkDropBrowser.cpp:484,537,904`
  (`if (presetSelector_)` / `if (!presetSelector_) return;`) are exactly
  where the packet said they'd be and correctly gate every use before the
  lazy wire fires. OBSERVED, PASS.
- **Init order**: `setProjectMPresetManager(&presetManager_)` runs
  synchronously in the MainComponent ctor body, and — per the root-cause doc,
  unchallenged by this diff — the GL context structurally cannot attach
  before the ctor finishes (no peer/size yet), so no ProjectMSource can be
  created before the manager pointer is wired. PASS.
- **Scope creep**: none found. `Renderer::getOrCreateSource`'s `isAttached()`
  guard (the `22fcedc` hardening) is untouched. The `onPresetSelected` lambda
  body is byte-for-byte identical to before, just de-indented one level (a
  required side effect of removing the `if (pmSource)` wrapper, not
  gratuitous reformatting). No other file/logic touched.
- **`[MilkDrop]` startup log line**: preserved verbatim
  (`std::cerr << "[MilkDrop] Loaded " << presetManager_.getPresetCount() << " presets"`),
  now unconditional instead of buried inside the always-false `if(pmSource)`.
  PASS — matches success criteria #2 exactly.
- **Style**: new callback plumbing in Renderer.h/.cpp matches the existing
  `onAutopilotAdvanced_`/`onGenreChanged_` pattern precisely, same
  `callAsync` idiom, same accessor naming (`setOnX`/`onX_`). No gratuitous
  reformatting anywhere in the diff — each hunk is scoped tightly to the
  affected block.

## MINOR (non-blocking)
- `ProjectMSource.h` still `#include "sources/ProjectMPresetManager.h"`
  though the member is now a bare pointer and could be forward-declared
  (as `PresetSelector.h` already does). Pre-existing include, not touched by
  this diff — cosmetic only, not worth a churn commit for this lane.

## WHAT I COULD NOT DETERMINE FROM SOURCE ALONE
- Whether any JUCE-internal teardown path (timers, deferred repaints still
  queued on the message loop at app-quit) could reach into the
  `presetManager_`/`presetSelector_` dangling window between member
  destructors before `~MainComponent()` finishes. I found no such path in
  this codebase's own code, but JUCE's own internals are outside this diff's
  visibility and I did not exhaustively audit them.

## SCOPE CREEP
None found.

METADATA: reviewer=L0-MD-review, builder_packet=milkdrop-autoload-fix, date=2026-09-05
