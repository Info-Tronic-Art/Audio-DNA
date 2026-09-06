# L7-JUKE work packet — the remaining small wires from lane L7

## VERDICT: BUILDABLE
All three items are buildable from source alone, no blocked dependencies. But
the plan's framing of "small wires" is only accurate for two of the three —
see SIZE. One item (jukebox Pool/Mode/Blend) is hiding a fourth, bigger,
previously-unrecorded defect that must be fixed first or the other two do
nothing visible.

## SIZE
- **Ableton Link toggle**: trivial. ~10 new lines, 2 files (TopBar.h/.cpp), 1
  wiring line in MainComponent.cpp. Consumer already fully proven.
- **MIDI-out device open**: small. ~40-60 new lines across PreferencesDialog.h/.cpp
  + 2 call-site edits in MainComponent.cpp. Consumer already fully proven.
- **Jukebox Pool/Mode/Blend**: medium, NOT small, and split unevenly:
  - Prerequisite fix (onAutoSwitch dead callback): trivial, ~3 lines, MASSIVE
    value — see ROOT CAUSE. Do this first regardless of what else ships.
  - Blend: small, ~15 lines (PresetSelector.h/.cpp + ProjectMSource.cpp),
    consumer is a projectM API call that already exists but is never invoked.
  - Pool: medium, ~40-60 new lines in PresetSelector.h/.cpp — no consumer
    exists anywhere; this is new selection logic, not a wire.
  - Mode: medium, ~30-50 new lines in PresetSelector.h/.cpp — no consumer
    exists anywhere; this is new cycle-algorithm logic, not a wire. A
    reusable precedent exists in Renderer.cpp (see THE CHANGE / A3).

Total for all three items if fully done: roughly 10 files touched, ~200-250
lines. Zero files in tests/ reference any of the classes involved (see
CALL-SITE ENUMERATION) — no existing test will catch a regression here, and
no test call sites need updating for the one signature change proposed
(PreferencesDialog::show).

## CURRENT BEHAVIOUR (verified) — what happens today, traced end to end

### Item A — Jukebox Pool/Mode/Blend
`src/ui/MilkDropBrowser.h` declares four Jukebox-mode controls (line 105-109):
`jukeboxPoolSelector_` (All/Curated/Favorites), `jukeboxModeSelector_`
(Bag/Random/Sequential), `jukeboxTimingSelector_` (4/8/16/32 beats,
30/60 sec), `jukeboxBlendSlider_` (0.5-5.0s). These are DISTINCT from the
similarly-named Playlist-mode controls (`playlistCycleSelector_`,
`playlistTimingSelector_`, `playlistBlendSlider_`, header line 113-116) —
easy to conflate, don't.

In `MilkDropBrowser.cpp`'s constructor (lines 514-560), only
`jukeboxTimingSelector_` gets an `.onChange` (line 536-542, calls
`presetSelector_->setTransitionBars(bars[idx])`). `jukeboxPoolSelector_`,
`jukeboxModeSelector_` and `jukeboxBlendSlider_` are populated with items and
made visible but have no `.onChange`/`.onValueChange` at all — VERIFIED, this
is the literal "onChange" gap the plan names.

Pressing jukebox "Play" (`toggleJukeboxPlay()`, MilkDropBrowser.cpp:897-920)
does two things: `presetSelector_->setEnabled(true)`, and ONE manual
`presetManager_->randomPreset()` + `firePresetSelected(path)` to seed the
first visual. `firePresetSelected` → `onPresetSelected` → MainComponent
(line ~1561) → `src->loadPreset(path, true)`, which actually queues the
preset into projectM. This first preset DOES load correctly.

After that, `PresetSelector::processFrame()` (`src/sources/PresetSelector.cpp`)
runs every render frame (called from `ProjectMSource::render()` line 138,
guarded by `!presetLocked_`). On a structural-transition or
timing-interval trigger it computes a `newPreset` via
`manager_->randomPresetInMood(...)` or `manager_->randomPreset()` (lines
33-59, 67-74 in PresetSelector.cpp) **and calls `onAutoSwitch(newPreset->path)`
if `onAutoSwitch` is set**. `onAutoSwitch` is declared
(`PresetSelector.h:40`) and called (`PresetSelector.cpp:61-62,72-73`) but is
**never assigned anywhere in `src/` or `tests/`** — see CALL-SITE
ENUMERATION. `randomPreset()`/`randomPresetInMood()` DO mutate
`manager_->currentIndex_` (ProjectMPresetManager.cpp:270,292), but nothing
reads that mutation to trigger a `loadPreset()` call — `ProjectMSource`'s
only two callers of `getCurrentPreset()` are the one-time initial load in
`initGL()` (line 53) and a display-name getter (line 230), neither of which
runs per-frame.

**Net effect (verified end to end): once Jukebox Play is pressed, the visual
preset changes exactly once (the manual seed) and then never again on its
own, regardless of Timing/Pool/Mode/Blend settings.** The plan's premise —
that only Pool/Mode/Blend's onChange are missing — undercounts the defect:
Timing is *already* wired and *still* doesn't do anything, because the whole
auto-switch pipeline dead-ends at the unassigned callback.

For Pool and Mode specifically, there is a second, independent problem: even
once `onAutoSwitch` is wired, there is **no backing selection logic** for
either control anywhere in `PresetSelector`:
- `jukeboxPoolSelector_`'s options are All/Curated/Favorites. "Curated" is
  defined only as a private UI-local predicate,
  `MilkDropBrowser::getCuratedPresets()` (MilkDropBrowser.cpp:1011-1020:
  `if (p.energy > 0.1f) result.push_back(&p);`), invisible to
  `PresetSelector`/`ProjectMPresetManager`. "Favorites" has a real manager
  method (`ProjectMPresetManager::getFavorites()`) but `PresetSelector` never
  calls it — `PresetSelector` only ever calls `manager_->randomPreset()` (any
  preset) or `manager_->randomPresetInMood(mood)` (a *mood* tag like
  "Energetic", unrelated to Pool).
- `jukeboxModeSelector_`'s options are Bag/Random/Sequential.
  `PresetSelector::processFrame` has no concept of any of these — it is
  unconditionally "random, optionally mood-weighted." There is no
  Sequential path (which would need `manager_->nextPreset()`) and no Bag
  path (which would need no-repeat-until-exhausted tracking) anywhere in
  `PresetSelector`.

By contrast, `jukeboxBlendSlider_` (0.5-5.0s crossfade) DOES have a ready,
unused consumer: projectM's own
`projectm_set_soft_cut_duration(projectm_handle, double seconds)`
(`~/.local/include/projectM-4/parameters.h:129`, third-party header, not in
this repo's `src/`). VERIFIED via grep: this function is never called
anywhere in `src/`. `ProjectMSource::applyParams()` (ProjectMSource.cpp:239-256)
already calls the *sibling* function `projectm_set_preset_duration` for an
unrelated per-frame "Speed" param at line 250 — same family of API, proving
the pattern is already in use in this file, just not for soft-cut duration.

**Prior-session claim check (per your instruction #2):** a prior session's
"the three playlist knobs (cycle/timing/blend) now actually change what
lands" is TRUE but is about a *different* control set — `playlistCycleSelector_`
/`playlistTimingSelector_`/`playlistBlendSlider_`, read via
`getPlaylistCycleModeId()`/`getPlaylistTriggerBeats()`/`getPlaylistBlendSeconds()`
at drag-drop time (`MilkDropBrowser.cpp:924-940`), consumed at
`MainComponent.cpp:1296-1303` into `Clip::playlistCycleMode` /
`playlistTriggerBeats` / `playlistBlendSeconds`, and actually driven every
frame by `Renderer.cpp:296-357` (a beat-synced advance loop that DOES call
`pmSource->loadPreset(entry.presetPath, true)` — this one really works,
VERIFIED end to end). **This is a wholly separate, already-working system.
It does NOT mean Jukebox Pool/Mode/Blend is done — confirmed still
undone.** The naming similarity (cycle/mode, timing, blend) is the trap the
task brief warned about.

### Item B — Ableton Link toggle
`src/sync/LinkSync.h` is a complete, thread-safe wrapper (atomics for all
cross-thread state) with `setEnabled(bool)`/`isEnabled()`. MainComponent owns
one instance, `linkSync_` (`MainComponent.h:320`). The consumer loop is real
and live: `MainComponent.cpp:2586-2597` — `if (linkSync_.isEnabled()) {
linkSync_.update(); ... tracker->setManualBPM(linkBPM); }`, run every frame
from the main timer callback. **VERIFIED: `linkSync_.setEnabled(...)` is
never called anywhere in `src/` or `tests/`.** `enabled_` defaults to `false`
(LinkSync.h:50), so the gate at line 2587 can never open. There is no
Ableton/Link-named UI control anywhere in `src/ui/` (the six `grep -i link`
hits in ui/ files are all false positives — "Macro Link knobs," unrelated).
**This is exactly what the plan says: consumer fully proven and live, only
the UI toggle is missing — nothing else is wrong.**

### Item C — MIDI-out device open
`src/midi/MidiOutputHandler.h/.cpp` is complete: `openDevice(id)` looks up
the device by identifier via `juce::MidiOutput::getAvailableDevices()` and
opens it (MidiOutputHandler.cpp:18-35), `isOpen()` reflects it, `closeDevice()`
is called at shutdown (`MainComponent.cpp:1905`). The consumer loop is real
and live: `MainComponent.cpp:2601-2603` — `if (uiUpdateCounter_ == 0 &&
midiOutputHandler_.isOpen()) midiOutputHandler_.updateFromDeck(...)`, ~6Hz
per the adjacent comment ("P22.10"). **VERIFIED: `openDevice(` is never
called anywhere in `src/` or `tests/`.** `outputDevice_` is always null, so
`isOpen()` is always false and the pad-feedback loop never runs. There is no
MIDI-output device picker anywhere in `src/ui/` — `PreferencesDialog.h:6`'s
own header comment says "Tabs: General, Video, About. (Only wired controls
are exposed.)" — confirming this is a deliberate repo convention the control
was never added under, not an oversight buried in a hidden tab.
**This also matches the plan exactly: consumer fully proven and live, only
the device-picker UI is missing.**

(Adjacent, out-of-scope observation: `MidiHandler::enableDevice()` — the
MIDI *input*-side per-device toggle — is similarly never called, but this is
NOT a bug: `MidiHandler::start()` (MidiHandler.cpp:13-25) already
auto-enables every available MIDI input device by default, so the input
side needs no picker. Output has no such auto-enable path in JUCE — a human
choice of device is required. Not fixing this, just flagging why it's not
the same shape as item C.)

## ROOT CAUSE (verified)

1. **Item A, primary defect (blocks everything else in item A):**
   `PresetSelector::onAutoSwitch` (`src/sources/PresetSelector.h:40`) is
   declared and invoked but never assigned. `ProjectMSource` never wires it
   to its own `loadPreset()`. Jukebox autopilot silently changes internal
   manager bookkeeping (`currentIndex_`) but never pushes a new preset to
   the render engine after the initial manual seed.
2. **Item A, secondary defect (Pool):** no code anywhere restricts
   auto-switch candidate selection to a Curated or Favorites subset;
   `jukeboxPoolSelector_`'s value has no possible destination today.
3. **Item A, secondary defect (Mode):** no code anywhere implements
   Bag/Sequential cycling for jukebox autopilot; selection is always random
   (optionally mood-weighted); `jukeboxModeSelector_`'s value has no
   possible destination today.
4. **Item A, Blend — NOT a defect, an omission:** the real consumer
   (`projectm_set_soft_cut_duration`) exists in the linked library and is
   simply never called from this codebase.
5. **Item B:** `LinkSync::setEnabled` has zero callers — omission, not a
   defect. No design questions; UI just needs to exist.
6. **Item C:** `MidiOutputHandler::openDevice` has zero callers — omission,
   not a defect. UI just needs to exist, and per the repo's own stated
   convention it should go in `PreferencesDialog`.

## FILES TOUCHED — exhaustive list, each with why

### Item A (in build order — do 1 before 2-4; 2 is worth doing even alone)
1. `src/sources/ProjectMSource.cpp` — constructor: wire
   `presetSelector_.onAutoSwitch = [this](const std::string& path) { loadPreset(path, true); };`
   This is the prerequisite fix. Self-contained; `presetSelector_` is an
   owned value member (`ProjectMSource.h:115`), not a pointer, so this needs
   no external plumbing.
2. `src/sources/PresetSelector.h` + `.cpp` — add `blendSeconds_` (default
   2.0, matches `jukeboxBlendSlider_`'s current default) +
   `setBlendSeconds(float)`/`getBlendSeconds()`, mirroring the existing
   `transitionBars_` pattern exactly.
3. `src/sources/ProjectMSource.cpp` — in `render()`, near the existing
   `presetSelector_.processFrame(snapshot);` call (line 138) or inside
   `applyParams()`, add
   `projectm_set_soft_cut_duration(pm_, presetSelector_.getBlendSeconds());`
   (guarded by `#ifdef AUDIODNA_HAS_PROJECTM` / `if (pm_)`, matching the
   surrounding code).
4. `src/ui/MilkDropBrowser.cpp` — add `jukeboxBlendSlider_.onValueChange`,
   mirroring the existing `jukeboxTimingSelector_.onChange` block
   (lines 536-542) exactly in shape: null-guard `presetSelector_`, call
   `presetSelector_->setBlendSeconds(static_cast<float>(jukeboxBlendSlider_.getValue()));`
5. `src/sources/PresetSelector.h` + `.cpp` — add `poolFilter_` (enum:
   All/Curated/Favorites) + `setPoolFilter(...)`/`getPoolFilter()`; in the
   two preset-picking blocks (lines 33-59 and 67-74), filter the candidate
   list by the predicate before applying the existing random/mood logic.
   "Curated" should reuse the exact predicate already in
   `MilkDropBrowser::getCuratedPresets()` (`p.energy > 0.1f`) — either
   duplicate that one-line predicate locally (smallest footprint, my
   recommendation) or promote it to `ProjectMPresetManager` as a shared
   method (cleaner, touches 2 more files: `ProjectMPresetManager.h`/`.cpp`,
   optional, see OUT OF SCOPE). "Favorites" can call
   `manager_->getFavorites()` directly — it already exists.
6. `src/ui/MilkDropBrowser.cpp` — add `jukeboxPoolSelector_.onChange`,
   same shape as above, calling `presetSelector_->setPoolFilter(...)`.
7. `src/sources/PresetSelector.h` + `.cpp` — add `cycleMode_` (enum:
   Bag/Random/Sequential) + `setCycleMode(...)`/`getCycleMode()`; in the
   picking blocks, branch: Sequential → `manager_->nextPreset()` instead of
   `randomPreset()`; Random/Bag → existing logic. A reusable precedent for
   this exact 3-way switch already exists at `src/render/Renderer.cpp:325-347`
   (the Playlist cycle-mode advance) — mirror its shape, not its file.
8. `src/ui/MilkDropBrowser.cpp` — add `jukeboxModeSelector_.onChange`, same
   shape, calling `presetSelector_->setCycleMode(...)`.

### Item B
1. `src/ui/TopBar.h` — add `juce::ToggleButton linkToggleBtn_{"Link"};` next
   to `manualModeBtn_` (TopBar.h line ~104-106, Tempo Section) and
   `std::function<void(bool)> onLinkToggled;` alongside the other
   `std::function` callbacks (TopBar.h line ~21-29).
2. `src/ui/TopBar.cpp` — construct/wire it exactly like `manualModeBtn_`
   (TopBar.cpp:91-97: `addAndMakeVisible`, tooltip, colour, `onStateChange`
   lambda firing `onLinkToggled` with the new toggle state), and add its
   bounds in `resized()` next to `manualModeBtn_.setBounds(...)`
   (TopBar.cpp:475).
3. `src/MainComponent.cpp` — add
   `topBar_->onLinkToggled = [this](bool enabled) { linkSync_.setEnabled(enabled); };`
   right after the existing `topBar_->onTapTempo = ...` block (anchor:
   `topBar_->onTapTempo = [this](float tappedBPM) {`, line 516).

### Item C
1. `src/ui/PreferencesDialog.h` — add a `Midi` value to the `Tab` enum
   (`enum class Tab : int { General = 0, Video, About }`, line ~39), a
   `midiOutBtn_` tab button, a `juce::ComboBox midiOutputSelector_`, and
   extend both the constructor and `show()` signatures with
   `const juce::String& midiOutputDeviceId` (seed) and
   `std::function<void(juce::String)> onMidiOutputDeviceChanged` (fires on
   selection) — same shape as the existing `milkDropDir`/`onMilkDropDirChanged`
   pair.
2. `src/ui/PreferencesDialog.cpp` — populate `midiOutputSelector_` from
   `MidiOutputHandler::getAvailableDevices()` (needs `#include "midi/MidiOutputHandler.h"`),
   add a `layoutMidiTab(...)` mirroring `layoutVideoTab(...)`, wire
   `midiOutputSelector_.onChange` to fire `onMidiOutputDeviceChanged` with
   the selected device's identifier.
3. `src/MainComponent.cpp` — update BOTH `PreferencesDialog::show(...)`
   call sites (lines 4021-4025 and 4027-4031, both inside the same
   `switch (commandId)`) to pass the current device id and a callback:
   `[this](juce::String id) { midiOutputHandler_.openDevice(id); }`. Both
   sites currently have identical argument lists — keep them identical
   after the edit too.

## CALL-SITE ENUMERATION — grep commands run + raw output

```
$ grep -n "jukeboxPoolSelector_\|jukeboxModeSelector_\|jukeboxTimingSelector_\|jukeboxBlendSlider_\|jukeboxPlayBtn_" src/ui/MilkDropBrowser.cpp
```
→ 30 hits; only `jukeboxTimingSelector_` (line 536) and `jukeboxPlayBtn_`
(line 507) have an `onChange`/`onClick` assignment. `jukeboxPoolSelector_`
and `jukeboxModeSelector_` appear only in `addItem`/`setSelectedId`/
`setupCombo`/`addAndMakeVisible`/`setVisible`/`setBounds` calls.
`jukeboxBlendSlider_` appears only in range/value/colour/visibility calls.
(Full output reproduced in CURRENT BEHAVIOUR above; omitted here for length.)

```
$ grep -rn "onAutoSwitch" src/ tests/
src/sources/PresetSelector.h:40:    std::function<void(const std::string& presetPath)> onAutoSwitch;
src/sources/PresetSelector.cpp:61:        if (newPreset && onAutoSwitch)
src/sources/PresetSelector.cpp:62:            onAutoSwitch(newPreset->path);
src/sources/PresetSelector.cpp:72:        if (preset && onAutoSwitch)
src/sources/PresetSelector.cpp:73:            onAutoSwitch(preset->path);

$ grep -rnE "onAutoSwitch[[:space:]]*=[^=]" src/ tests/
(no output)
```
Two agreeing patterns (plain substring, and an assignment-shaped regex):
`onAutoSwitch` is declared + called but never assigned. VERIFIED negative.

```
$ grep -rn "linkSync_\.setEnabled" src/
(no output)
$ grep -rn "\.setEnabled(" src/ | grep -i link
(no output)
$ grep -rn "linkSync_" src/
src/MainComponent.h:320:    LinkSync linkSync_;
src/MainComponent.cpp:2587:    if (linkSync_.isEnabled())
src/MainComponent.cpp:2589:        linkSync_.update();
src/MainComponent.cpp:2590:        double linkBPM = linkSync_.getBPM();
```
Two agreeing patterns (direct method-call substring, and a broader
`.setEnabled(` + case-insensitive "link" filter): zero assignments found.
Third pattern shows every use of `linkSync_` that DOES exist, confirming
the consumer loop and ruling out an assignment hiding under a different
accessor name.

```
$ grep -rn "\.openDevice(\|->openDevice(" src/
(no output)
$ grep -rn "midiOutputHandler_" src/MainComponent.cpp src/MainComponent.h
src/MainComponent.h:429:    MidiOutputHandler midiOutputHandler_;
src/MainComponent.cpp:1905:    midiOutputHandler_.closeDevice();
src/MainComponent.cpp:2602:    if (uiUpdateCounter_ == 0 && midiOutputHandler_.isOpen())
src/MainComponent.cpp:2603:        midiOutputHandler_.updateFromDeck(composition_.getActiveDeck());
```
Two agreeing patterns (method-call substring across all of `src/`, and every
use of the member itself): `openDevice(` has zero callers; `closeDevice()`
and the `isOpen()`-gated consumer loop both exist and are live.

```
$ grep -rn "Link" src/ui/*.{h,cpp} -i    [ran per-file, 9 files matched "Link" case-insensitively]
```
All 9 hits are "Macro Link knobs" / "relink missing media" / unrelated —
zero real Ableton Link UI. (Full per-file output in CURRENT BEHAVIOUR.)

```
$ grep -rln "PresetSelector" tests/     → (no output)
$ grep -rln "MilkDropBrowser" tests/    → (no output)
$ grep -rln "ProjectMSource" tests/     → (no output)
$ grep -rln "LinkSync" tests/           → (no output)
$ grep -rln "MidiOutputHandler" tests/  → (no output)
$ grep -rln "TopBar" tests/             → (no output)
$ grep -rln "PreferencesDialog" tests/  → (no output)
```
Zero test files reference any of the seven classes touched by this packet.
(`tests/test_preset_manager.cpp` exists but its content — checked directly —
is entirely about effect-parameter/mapping retargeting, not
`ProjectMPresetManager`; misleading filename, confirmed not a match.)

```
$ grep -rn "PreferencesDialog(" src/ tests/
src/ui/PreferencesDialog.cpp:7:PreferencesDialog::PreferencesDialog(bool tooltipsEnabled,
src/ui/PreferencesDialog.h:11:    PreferencesDialog(bool tooltipsEnabled, std::function<void(bool)> onTooltipToggled,
$ grep -rn "PreferencesDialog::show\|PreferencesDialog\b" src/MainComponent.cpp
src/MainComponent.cpp:2:#include "ui/PreferencesDialog.h"
src/MainComponent.cpp:4023:            PreferencesDialog::show(this, tooltipsEnabled_, ...)
src/MainComponent.cpp:4029:            PreferencesDialog::show(this, tooltipsEnabled_, ...)
```
Only signature/constructor change proposed in this packet
(`PreferencesDialog::show`/ctor, for item C). Exactly 2 call sites, both in
`src/MainComponent.cpp`, both inside the same `switch (commandId)` block.
Zero call sites in `tests/` (confirmed above) — no test updates needed for
this change.

## THE CHANGE
See FILES TOUCHED above — it is already sequenced step by step per item.
Recommended overall order: B (Link) → A steps 1-4 (onAutoSwitch fix + Blend)
→ C (MIDI-out) → A steps 5-8 (Pool + Mode), per the ranking below.

## FENCE — every file this lane will WRITE
- `src/sources/PresetSelector.h`
- `src/sources/PresetSelector.cpp`
- `src/sources/ProjectMSource.cpp`
- `src/ui/MilkDropBrowser.cpp`
- `src/ui/TopBar.h`
- `src/ui/TopBar.cpp`
- `src/ui/PreferencesDialog.h`
- `src/ui/PreferencesDialog.cpp`
- `src/MainComponent.cpp`

Optional (only if the builder takes the "promote getCuratedPresets() to the
manager" path for Pool — see OUT OF SCOPE):
- `src/sources/ProjectMPresetManager.h`
- `src/sources/ProjectMPresetManager.cpp`
- `src/ui/MilkDropBrowser.h` (only if `getCuratedPresets()` is removed from
  here; not needed if it's just left in place and duplicated)

Not touched, not needed: `src/ui/MilkDropBrowser.h` (for the base plan —
all three new onChange handlers are lambdas assigned in the existing
constructor body in the .cpp, exactly like the existing `jukeboxTimingSelector_`
one; no new private members or method declarations required in the header).

## TRAPS — what will bite the builder

1. **The settings.json overwrite trap (real, waiting to happen).**
   `MainComponent::saveMilkDropPresetDirSetting()` (MainComponent.cpp:1828-1836)
   does `auto* obj = new juce::DynamicObject(); obj->setProperty("milkDropPresetDir", dir);`
   then `settingsDir.getChildFile("settings.json").replaceWithText(...)` —
   it REPLACES the whole file with a single-key object; it does not read the
   existing file and merge first. If item C's builder persists the chosen
   MIDI-out device to the same `settings.json` the same naive way (a very
   natural thing to copy), the two settings will clobber each other on
   every save — whichever setting is changed last wins, silently deleting
   the other. If persistence is added, it MUST parse the existing file
   first (`juce::JSON::parse(file.loadFileAsString())`), merge the new key
   into that `DynamicObject`, then write it back — mirroring nothing
   currently in this file, because nothing here does it correctly yet.
   Currently `milkDropPresetDir` is the ONLY key ever written there, so this
   trap is latent, not yet triggered — item C would be the first to trigger
   it. (Persisting the MIDI device at all is optional — see THE CHANGE /
   OUT OF SCOPE.)
2. **`presetSelector_` can be null.** `MilkDropBrowser::presetSelector_` is
   only set once a `ProjectMSource` exists (lazy, GL-thread creation via
   `setOnProjectMSourceCreated`, MainComponent.cpp:1553-1560). Every new
   `onChange` added for Pool/Mode/Blend must null-guard it, exactly like
   the existing `jukeboxTimingSelector_.onChange` does (`if (!presetSelector_) return;`).
   Jukebox is the DEFAULT play mode (`PlayMode activePlayMode_ = PlayMode::Jukebox;`,
   MilkDropBrowser.h:77) so these controls are visible and interactive
   before that pointer is ever set — the guard is not optional.
3. **`onAutoSwitch` runs on the GL thread.** `PresetSelector::processFrame`
   is called from `ProjectMSource::render()`, which is GL-thread code
   (see the existing thread-safety comments throughout ProjectMSource.cpp
   around FBOs/GL state). The lambda you assign to `onAutoSwitch` will
   therefore also run on the GL thread. `ProjectMSource::loadPreset()`
   already handles this correctly — it just takes a mutex and queues
   `pendingPresetPath_` for the render loop to pick up next frame
   (ProjectMSource.cpp:195-201) — so calling it directly from `onAutoSwitch`
   is safe. Do NOT call anything else from that lambda that assumes the
   message thread (e.g. don't touch `MilkDropBrowser`'s UI state from it).
4. **projectM's OWN internal auto-advance timer is a separate, unrelated
   mechanism.** `ProjectMSource::applyParams()` already calls
   `projectm_set_preset_duration(pm_, ...)` (line 250) from the unrelated
   "Speed" clip param. `projectm_set_preset_duration` and
   `projectm_set_soft_cut_duration` are different projectM knobs (preset
   *dwell* time vs. crossfade *duration*) — don't confuse the two, and
   don't assume `preset_duration` drives any visible auto-advance here: this
   app calls `projectm_load_preset_file` directly rather than using
   projectM's own playlist/auto-advance API, so `preset_duration` should be
   inert in this app's architecture. Worth a sanity check by the builder but
   not expected to interact with this packet's changes.
5. **"Bag" in the existing Playlist precedent (Renderer.cpp:339-347) is
   actually implemented as plain random-with-replacement, same as
   `RandomOther`/`PingPong`** — not true no-repeat-until-exhausted bag
   semantics. If you mirror that precedent's shape for Jukebox Mode's
   "Bag" (recommended, for consistency and lowest risk), you inherit that
   same simplification. Implementing a *true* bag would need new
   state (a shuffled remaining-indices queue) not present in either
   codepath. This is a product-scope question, not a code trap — flagged
   as an OPEN QUESTION below, not fixed here.
6. **Two identical `PreferencesDialog::show(...)` call sites.** They are
   currently byte-identical in argument shape (MainComponent.cpp:4021-4025
   and 4027-4031) — one is for `kPreferences`, one for `kAbout` (with a
   `// TODO: auto-switch to About tab` comment already there, unaddressed).
   Update both identically for the new MIDI param, or the About-menu path
   will silently regress relative to the Preferences-menu path.
7. **projectM build flag.** All of `ProjectMSource`'s projectM calls are
   guarded by `#ifdef AUDIODNA_HAS_PROJECTM`. Any new call
   (`projectm_set_soft_cut_duration`) must go inside that same guard, in the
   same style as its neighbors (see ProjectMSource.cpp:117-165 for the
   guarded block shape).

## HOW TO PROVE IT WORKS

**Jukebox (machine-gatable, no hardware/peer needed):**
- Compile-level: confirm the new `onAutoSwitch` lambda captures `this` by
  value-safety (ProjectMSource outlives the lambda's lifetime trivially,
  it's a same-object member wiring — no dangling risk to verify further).
- Behavioral, machine-observable: with jukebox Play pressed and a structural
  transition or timing interval elapsed, `currentPresetPath_`
  (ProjectMSource, private but exposed via `getCurrentPresetName()`) should
  change without any further manual preset click — this is the single
  clearest proof the dead-callback fix landed, and can be asserted from a
  test harness or logged and eyeballed from stderr (`std::cerr` traces
  already exist in `MidiOutputHandler` as a style precedent — projectM's own
  preset-switch callback, `projectm_preset_switch_requested_event`
  (`callbacks.h:41`), is a lower-risk instrumentation point than adding new
  logging to this packet's own changed files).
- Pool: with Pool set to "Favorites" and at least one preset starred,
  auto-switches over a few minutes should never land on a non-favorite —
  checkable from logged preset paths against `getFavorites()`'s path list,
  no human needed.
- Mode "Sequential": consecutive auto-switches should walk
  `getAllPresets()` in index order — also machine-checkable from logged
  paths.
- Blend: `projectm_get_soft_cut_duration(pm_)` should read back whatever the
  slider was last set to — machine-checkable if the builder adds a
  temporary getter/log, no human needed for THIS check. But whether a
  0.5s vs 5.0s crossfade actually *looks* smoother is a human/Boris call —
  the plan text didn't gate this one on a peer or hardware, so it's the one
  piece of item A that plausibly wants a quick human glance even though
  nothing forces it.

**Ableton Link (needs Boris at the machine for full proof):**
- Machine-gatable: toggle compiles, `linkToggleBtn_.getToggleState()`
  flips, `linkSync_.isEnabled()` reflects it, `linkSync_.update()` starts
  being called every frame (all directly observable/loggable, no peer
  needed).
- Needs Boris + a peer: confirming BPM actually converges with another
  Link-enabled app (Ableton Live, another Link-capable tool) on the same
  network — the plan's own framing ("needs a Link peer → attended") is
  correct and this recon found nothing to change that.

**MIDI-out (needs Boris + IAC loopback or hardware for full proof):**
- Machine-gatable: the new Preferences tab lists devices from
  `MidiOutputHandler::getAvailableDevices()` (whatever the CI/dev box has,
  even zero — an empty list should render without crashing, worth an
  explicit check), selecting one calls `openDevice(id)`, `isOpen()` flips
  true/false correctly, and — if the sandbox/CI box has ANY MIDI output
  (including a virtual one) — `getDeviceName()` should match what was
  picked. All loggable, no hardware strictly required to prove the wiring.
- Needs Boris + IAC loopback (macOS) or real hardware (Launchpad/APC40):
  confirming actual MIDI note messages arrive and pad colors update per
  `MidiOutputHandler`'s documented velocity map (header lines 12-17) — this
  requires a receiving device/loopback a human sets up, exactly as the plan
  says.

## OUT OF SCOPE — what you deliberately are NOT doing and why
- **Not** fixing `MidiHandler::enableDevice()`'s zero-callers state (MIDI
  input per-device toggle) — confirmed not a bug, input auto-enables all
  devices by default (`MidiHandler::start()`), so there is nothing to wire.
- **Not** implementing true no-repeat "Bag" semantics for Jukebox Mode —
  the existing Playlist precedent doesn't do this either (see TRAPS #5);
  matching that precedent's simplification keeps behavior consistent
  across the app and is a smaller/lower-risk change. Flagged as an open
  product question, not silently decided.
- **Not** promoting `MilkDropBrowser::getCuratedPresets()`'s
  `energy > 0.1f` predicate to `ProjectMPresetManager` as a shared method —
  duplicating the one-line predicate inside `PresetSelector.cpp` is smaller
  and lower-risk for this packet's scope; the shared-method refactor is a
  clean follow-up but touches two more files for no behavioral gain.
- **Not** adding settings.json persistence for the chosen MIDI-out device —
  the plan only asks that opening the device work; persisting the choice
  across restarts is a nice-to-have that, if added carelessly, triggers the
  overwrite trap in TRAPS #1. Left as an explicit builder decision.
- **Not** touching `projectm_set_preset_duration`/the "Speed" param — noted
  only as a trap to avoid confusing with soft-cut duration, not something
  this lane needs to change.
- **Not** running cmake/ctest or launching the app — recon is static per
  the task's hard rules; all "how to prove it" items above are for the
  builder/Boris to execute.

## OPEN QUESTIONS — anything you could not settle from source
1. Should Jukebox "Bag" mode get true no-repeat semantics, or is
   consistency with the (arguably also-simplified) Playlist precedent
   acceptable? Product call, not answerable from source (see TRAPS #5).
2. Should the chosen MIDI-out device persist across restarts? If yes, the
   settings.json merge-not-replace fix (TRAPS #1) becomes mandatory scope,
   not optional — this changes item C's size from small to small-medium.
3. Exact visual placement/copy for the Link toggle: this packet recommends
   mirroring `manualModeBtn_` in TopBar's Tempo Section (closest existing
   precedent, same section conceptually owns tempo-sync state), but Boris
   may prefer a different location (e.g. near the audio source selector,
   or a menu item instead of a persistent button). Not settled from source
   since there's no existing precedent to defer to.
4. Whether the builder should use `projectm_get_soft_cut_duration` for a
   real behavioral test/log during development, or whether that's
   considered "running the app" and thus off-limits for whoever picks this
   packet up next (this recon session did not run anything, per its own
   hard rules, so this wasn't testable either way).

## RANKING — value delivered / risk, and machine vs. Boris gating

1. **Ableton Link toggle — best ratio.** Tiny, isolated change (one boolean
   flip through an already-thread-safe class); consumer is fully proven
   end-to-end already; zero new business logic; zero design ambiguity
   beyond button placement. Machine can verify the wiring compiles and
   flips state correctly; Boris + a Link peer needed only for the final
   "BPM actually converges" proof — exactly as the plan assumed, confirmed.

2. **Jukebox onAutoSwitch fix + Blend (item A, steps 1-4 only) — best
   *value*, still low risk.** The onAutoSwitch fix alone is 3 lines and
   turns Jukebox autopilot from "loads one preset and sits static forever"
   into "actually auto-cycles" for the first time — this is arguably the
   single highest-value fix in this entire packet, because it's a marquee,
   currently-100%-broken feature, not a cosmetic gap. Blend adds a real,
   ready projectM API call on top for ~15 more lines. No hardware/peer
   needed — fully machine-gatable per HOW TO PROVE IT WORKS.

3. **MIDI-out device open — good ratio, more surface.** Consumer fully
   proven; the gap is a real (if small) new UI surface — a Preferences tab,
   a `show()`/ctor signature change with 2 call sites to update. Machine
   can verify all wiring and even exercise `openDevice()`/`isOpen()`
   against whatever MIDI output the build box happens to expose (including
   none); Boris + IAC loopback or real hardware needed only for the final
   "pads actually light up correctly" proof — as the plan assumed,
   confirmed.

4. **Jukebox Pool + Mode (item A, steps 5-8) — worst ratio of the five
   pieces above, worth attending to LAST or deferring.** Both need new
   selection/cycling logic that does not exist anywhere yet (not "wires" —
   a category error in the plan's framing), both are the biggest single
   chunks of new code in this packet (~40-60 lines each), and both are
   lower-stakes than the onAutoSwitch fix they depend on. No hardware/peer
   needed to test (machine-gatable), but the largest design surface of
   anything here (see OPEN QUESTIONS #1). Recommend shipping items B, A(1-4),
   C first, and treating Pool/Mode as a follow-on if Boris wants full
   parity with the plan's original three-control ask.
