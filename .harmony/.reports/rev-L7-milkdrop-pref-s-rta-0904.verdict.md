# Reviewer Verdict — L7 milkdrop pref wire (RTA)
STATUS: DONE
VERDICT: PASS

FILES: src/ui/PreferencesDialog.{cpp,h}, src/render/Renderer.{cpp,h}, src/MainComponent.{cpp,h}
(MainComponent.h also changed, ~24 lines, not listed in the packet's "four
files" but in scope — declares milkDropBaseDirs_, milkDropPresetDir_,
setMilkDropPresetDir/load/saveMilkDropPresetDirSetting. Reviewed.)

## 1. THREADING — PASS, no race

VERDICT: no unguarded race. The guard is correct and reuses an existing
pattern rather than inventing one.

OBSERVED: `ProjectMPresetManager::presets_` (src/sources/ProjectMPresetManager.h:82)
has no lock. `rescan()` (ProjectMPresetManager.cpp:42-46) does
`presets_.clear()` then repopulates via `scanDirectory()` — a real mutation
of the container. `PresetSelector::processFrame` (PresetSelector.cpp:6-71)
reads it every call via `manager_->getPresetCount()`, `randomPreset()`,
`randomPresetInMood()` — no lock on the read side either. `processFrame` is
called unconditionally (unless `presetLocked_`) from
`ProjectMSource::render()` (ProjectMSource.cpp:135-136), which does raw GL
calls (`glBindFramebuffer` etc.) — confirms it runs on the GL thread. So
before this diff, `presets_` was populated ONCE at ctor and never mutated
again — safe by construction (read-only after startup) even without a lock.
This diff is the FIRST path that mutates it post-startup (Preferences), so
it is also the first path that actually needed synchronization — and it
added it.

The fix: `Renderer::rescanMilkDropPresets()` (Renderer.cpp:868-901) checks
`juce::OpenGLContext::getCurrentContext() != &glContext_` and, when true
(the message-thread case — the only real caller, see #6), marshals via
`glContext_.executeOnGLThread(..., /*blockUntilFinished*/ true)`. This is
the SAME idiom already used by `getOrCreateSource()` (Renderer.cpp:785-828)
for `activeSources_`, which is also GL-thread-owned/lock-free. It is NOT the
`juce::MessageManager::callAsync` idiom the packet asked me to check
against (`onProjectMSourceCreated_`/`onAutopilotAdvanced_`) — and that's
correct, not a gap: `callAsync` marshals GL-thread→message-thread (the
opposite direction of what's needed here); `executeOnGLThread` is the
established mechanism in this exact file for message-thread→GL-thread
mutation. So: no new invented mechanism, and the one borrowed is the
direction-correct one, not the direction-mismatched one named in the ask.

`blockUntilFinished=true` matters: it means `setMilkDropPresetDir()`
(MainComponent.cpp:1841-1856) doesn't call `browserPanel_->...refresh()`
(a message-thread read of the same `presets_`) until the GL-thread mutation
has fully completed and returned — so the message-thread read and GL-thread
mutation never overlap either.

Two other correctness details, both fine:
- `!glContext_.isAttached()` (line 883) mutates inline when the context is
  detached — correct, since no GL thread is running to race against.
  Mirrors the `isAttached()` defense-in-depth check in `getOrCreateSource`.
- Check order differs from `getOrCreateSource` (isAttached checked before
  the current-thread check here, vs. after there). INFERRED this is
  harmless: if somehow `isAttached()` is stale/false while genuinely
  executing on the GL thread, the code takes the "inline" branch instead of
  the "already on GL thread" branch — but both branches do the same thing
  (mutate inline), so the outcome is identical either way. Not a defect,
  just worth flagging that I reasoned through it rather than treating the
  reordering as obviously safe.
- Startup: the ctor's initial `presetManager_.setPresetDirectories()` +
  `.rescan()` (MainComponent.cpp:1526-1527) is called DIRECTLY, not through
  `Renderer::rescanMilkDropPresets()`. Confirmed safe: at this point in the
  ctor no `ProjectMSource`/`PresetSelector` exists yet (created lazily by
  `getOrCreateSource`, called later), so there is no reader to race.
  Matches the code's own comment.

Single call site confirmed for `rescanMilkDropPresets`
(grep: `rescanMilkDropPresets` under src/ → only definition in Renderer.cpp
and one call in MainComponent.cpp:1855), and that call is only reachable
from a JUCE UI callback chain (Preferences → `onMilkDropDirChanged` →
`setMilkDropPresetDir`), i.e. always the message thread. No other caller
exists to violate that assumption.

## 2. Renderer.{h,cpp} touch — necessary and minimal

Only `Renderer::rescanMilkDropPresets()` was added (36 lines .cpp, 12 lines
.h incl. comments), nothing else in either file changed. It's necessary
because `glContext_` and `executeOnGLThread` are private to `Renderer` —
`MainComponent` has no other way to safely marshal onto the GL thread. Not
scope creep; this is the minimum surface needed to make the mutation safe.

## 3. Reachability — PASS, ordinary tab visibility, not hidden-always

OBSERVED: `milkDropDirLabel_.setVisible(false)` (PreferencesDialog.cpp:206)
is inside `showActiveTab()`, which unconditionally hides ALL tabs' controls
first, then shows only the active tab's (switch on `activeTab_`,
lines 214-229) — General/Video/About are all treated identically this way,
it isn't special-cased for MilkDrop. `showActiveTab()` runs once in the
constructor (line 136, `activeTab_` defaults to `Tab::General`) and again
from `setActiveTab()` (line 181), which is presumably wired to the Video
tab button's `onClick` (pre-existing code, not part of this diff). So the
control is reachable by clicking the Video tab — this is standard
tab-switching, not an unreachable dead control.

## 4. Persistence — PASS, matches existing idiom, applied before initial scan

OBSERVED: `saveMilkDropPresetDirSetting()`/`loadMilkDropPresetDirSetting()`
(MainComponent.cpp:1817-1836) use `new juce::DynamicObject()` +
`obj->setProperty(...)` + `juce::JSON::toString(juce::var(obj))`, written to
`userApplicationDataDirectory/Audio-DNA/settings.json`. This is the same
JSON+DynamicObject pattern used by `kViewSaveLayout`/`kViewLoadLayout`
(MainComponent.cpp:4749-4755) — confirmed by direct comparison, not just
the comment's claim. `settings.json` itself is new (grep for
`settings.json`/`ApplicationProperties`/`PropertiesFile` pre-diff-adjacent
code found nothing) — there was no pre-existing generic app-settings store
to reuse, so this isn't inventing a parallel mechanism where one already
existed, it's filling a real gap. One thing worth noting for later, not a
blocker: `saveMilkDropPresetDirSetting` overwrites the whole file with only
this one key — fine today since it's the only setting, but if a second
setting is ever added to this file without a read-merge-write it will
clobber. Not this diff's problem to solve.

Startup-ordering — confirmed correct: `milkDropPresetDir_ =
loadMilkDropPresetDirSetting();` (line 1515) runs, and the loaded dir is
appended to `initialDirs` BEFORE `presetManager_.setPresetDirectories(initialDirs);
presetManager_.rescan();` (lines 1516-1527) — so a previously-saved
directory's presets are present at the very first `rescan()`, i.e. on first
paint, not only after the user reopens Preferences. This directly answers
the packet's concern and it is handled correctly.

Aside (pre-existing, NOT introduced by this diff, not a defect to charge to
this change): `tooltipsEnabled_` (MainComponent.h:322) has no
save/load-setting pair at all — it always resets to `true` on restart. This
is inconsistent with the new MilkDrop pref persisting, but it's a
pre-existing gap unrelated to this lane; flagging only as a "you found the
pattern gap while building the pattern" observation, not a blocking finding
here.

## 5. commitMilkDropDir() — PASS, guards both empty and unchanged

OBSERVED (PreferencesDialog.cpp:260-268): guards the invalid-typed-path case
(`text.isNotEmpty() && !juce::File(text).isDirectory()` → return without
firing). Empty text passes through and fires `onMilkDropDirChanged("")`.
The "unchanged" no-op case is guarded one layer down, in
`MainComponent::setMilkDropPresetDir()` (MainComponent.cpp:1842-1843):
`if (dir == milkDropPresetDir_) return;` before any write-to-disk or
`rescanMilkDropPresets()` call. So an incidental focus-lost with the field's
text unmodified does NOT trigger a filesystem rescan or a settings write —
it does still do one cheap `juce::File(text).isDirectory()` stat call in
`commitMilkDropDir` itself before the no-op check kicks in, but that's an
`lstat`, not a directory rescan; not the "real performance defect" the
packet was worried about. Not a blocking finding.

## 6. Signature changes / call sites — PASS, both real, no placeholders

Pattern 1 — `grep -rn "PreferencesDialog::show(\|PreferencesDialog(" --include="*.cpp" --include="*.h" .` (excl. build/):
only 2 call sites, both in MainComponent.cpp (kPreferences:4023,
kAbout:4029), both already updated to pass 5 args:
`PreferencesDialog::show(this, tooltipsEnabled_, [this](bool enabled){...},
milkDropPresetDir_, [this](juce::String dir){ setMilkDropPresetDir(dir); });`
— a real member and a real lambda calling a real method, not `nullptr` or
`std::function<>{}`.

Pattern 2 — `grep -n "rescanMilkDropPresets" --include="*.cpp" --include="*.h" -r .`:
single definition (Renderer.cpp:868) + single call
(MainComponent.cpp:1855, `previewPanel_.getRenderer().rescanMilkDropPresets(allDirs)`)
— confirms the callback chain actually reaches the renderer that owns the
live `projectMPresetManager_` pointer (cross-checked: the same
`previewPanel_.getRenderer()` instance also receives
`setProjectMPresetManager(&presetManager_)` at MainComponent.cpp:1546 — same
Renderer, same manager, no split-brain).

## DEFECTS
None blocking. Two non-blocking notes (both already folded into #4/#5
above, not repeating as separate line items): (a) `settings.json` is a
whole-file overwrite with no read-merge-write, fine today, worth a comment
if a second setting is ever added to that file; (b) `tooltipsEnabled_` has
no persistence at all — pre-existing gap, unrelated to this diff, not
charged against it.

## WHAT I COULD NOT DETERMINE FROM SOURCE ALONE
- Whether `videoBtn_`'s `onClick` is actually wired to
  `setActiveTab(Tab::Video)` — I did not chase this because it's pre-existing
  code untouched by the diff, and `showActiveTab()`'s uniform
  hide-all/show-active-tab structure combined with all three tabs
  (General/Video/About) being handled identically makes it very unlikely
  Video is somehow the one tab whose button doesn't route to
  `setActiveTab`. Flagging only as "traced from static text, not from a
  running UI" per the packet's "do not launch the app" constraint.
- Actual runtime confirmation that the GL-thread marshal path is exercised
  at all (i.e., that a real rescan while the context IS attached and a
  `ProjectMSource` exists was hit) — this was source-only review; I did not
  build or run. Builder/tester already reported clean build + 205/205
  ctest per the packet; I have no reason to doubt it but did not
  independently reproduce.

METADATA: reviewer=rev-L7-milkdrop, repo=RealTimeAudio, date=2026-09-05
