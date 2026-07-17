# Slice 09: Bindings (Keyboard + MIDI) and MIDI I/O

**Files audited**:
- `/Users/boriskarpman/Documents/RealTimeAudio/src/binding/Binding.h` (90 lines)
- `/Users/boriskarpman/Documents/RealTimeAudio/src/binding/BindingManager.h` (73 lines)
- `/Users/boriskarpman/Documents/RealTimeAudio/src/binding/BindingManager.cpp` (306 lines)
- `/Users/boriskarpman/Documents/RealTimeAudio/src/midi/MidiHandler.h` (37 lines)
- `/Users/boriskarpman/Documents/RealTimeAudio/src/midi/MidiHandler.cpp` (102 lines)
- `/Users/boriskarpman/Documents/RealTimeAudio/src/midi/MidiOutputHandler.h` (84 lines)
- `/Users/boriskarpman/Documents/RealTimeAudio/src/midi/MidiOutputHandler.cpp` (179 lines)

---

## Section 10: Bindings

### 10.1 Binding model — all fields of `Binding` struct with defaults

Defined in `Binding.h`. Plain struct (no constructor), all members initialized inline.

| Field | Type | Default | Line | Description |
|---|---|---|---|---|
| `id` | `uint32_t` | `0` | `Binding.h:8` | Unique binding ID (assigned by `BindingManager::addBinding` via `nextId_++`) |
| `inputType` | `Binding::InputType` enum | `InputType::Keyboard` | `Binding.h:12` | Source of input: `Keyboard`, `MidiNote`, `MidiCC` (defined line 11) |
| `keyCode` | `int` | `0` | `Binding.h:14` | JUCE key code (for Keyboard input) |
| `keyModShift` | `bool` | `false` | `Binding.h:15` | Require Shift modifier |
| `keyModCmd` | `bool` | `false` | `Binding.h:16` | Require Cmd/Ctrl modifier |
| `keyModAlt` | `bool` | `false` | `Binding.h:17` | Require Alt/Option modifier |
| `midiChannel` | `int` | `0` | `Binding.h:19` | MIDI channel filter; `0` = any channel (wildcard) |
| `midiNote` | `int` | `0` | `Binding.h:20` | MIDI note number (for MidiNote inputs) |
| `midiCC` | `int` | `0` | `Binding.h:21` | MIDI CC number (for MidiCC inputs) |
| `action` | `Binding::Action` enum | `Action::TriggerClip` | `Binding.h:46` | Which action fires on match (enum at lines 24-45) |
| `triggerMode` | `Binding::TriggerMode` enum | `TriggerMode::Toggle` | `Binding.h:55` | `Toggle` or `Momentary` (piano mode); enum lines 50-54 |
| `ccMode` | `Binding::CCMode` enum | `CCMode::Absolute` | `Binding.h:64` | `Absolute` (0-127 → 0-1) or `Relative` (delta from 64); enum lines 59-63 |
| `ccStepSize` | `float` | `0.01f` | `Binding.h:65` | Step size for relative CC mode (per unit deviation from 64) |
| `targetMode` | `Binding::TargetMode` enum | `TargetMode::ByPosition` | `Binding.h:75` | `ByPosition` / `ThisItem` / `Selected`; enum lines 69-74 |
| `targetClipId` | `uint32_t` | `0` | `Binding.h:76` | Clip ID used when `targetMode == ThisItem` |
| `velocityToOpacity` | `bool` | `false` | `Binding.h:79` | If true, MIDI velocity maps to clip opacity on trigger |
| `targetLayerIndex` | `int` | `0` | `Binding.h:82` | Target layer index (for clip/layer actions) |
| `targetColumn` | `int` | `0` | `Binding.h:83` | Target column index (for clip actions) |
| `targetDeckIndex` | `int` | `0` | `Binding.h:84` | Target deck index (for SwitchDeck) |
| `targetEffectIndex` | `int` | `0` | `Binding.h:85` | Target effect index (for ToggleEffectBypass) |
| `targetMacroIndex` | `int` | `0` | `Binding.h:86` | Target macro index (for AdjustMacro) |
| `enabled` | `bool` | `true` | `Binding.h:88` | Master enable flag per binding |

### 10.2 `Binding::Action` — every action value

Enum at `Binding.h:24-45`, underlying type `uint8_t`. 19 actions total.

| # | Value | Line | Description |
|---|---|---|---|
| 0 | `TriggerClip` | `Binding.h:26` | Trigger a specific clip (layerIndex + column) |
| 1 | `TriggerColumn` | `Binding.h:27` | Trigger a column across all layers |
| 2 | `ToggleLayerBypass` | `Binding.h:28` | Toggle layer bypass flag |
| 3 | `ToggleLayerSolo` | `Binding.h:29` | Toggle layer solo |
| 4 | `ToggleLayerMute` | `Binding.h:30` | Toggle layer mute |
| 5 | `ToggleLayerAutopilot` | `Binding.h:31` | Toggle layer autopilot |
| 6 | `ToggleLayerVisible` | `Binding.h:32` | Toggle layer visible flag |
| 7 | `LayerTransport` | `Binding.h:33` | Layer play/pause/reverse |
| 8 | `ToggleEffectBypass` | `Binding.h:34` | Bypass a specific effect by index |
| 9 | `AdjustMacro` | `Binding.h:35` | Continuous control of a macro knob |
| 10 | `AdjustLayerOpacity` | `Binding.h:36` | Continuous control of layer opacity |
| 11 | `SwitchDeck` | `Binding.h:37` | Switch active deck (uses `targetDeckIndex`) |
| 12 | `TapTempo` | `Binding.h:38` | Tap tempo |
| 13 | `Resync` | `Binding.h:39` | Global resync |
| 14 | `GlobalPlayPause` | `Binding.h:40` | Global play/pause |
| 15 | `GlobalStop` | `Binding.h:41` | Global stop |
| 16 | `MasterOpacity` | `Binding.h:42` | Continuous control of master opacity |
| 17 | `Snapshot` | `Binding.h:43` | Take a PNG screenshot (P22.7) |
| 18 | `ToggleRecording` | `Binding.h:44` | Start/stop video recording (P22.6) |

### 10.3 Trigger modes (`Binding::TriggerMode`)

Enum at `Binding.h:50-54`, `uint8_t`.

| Value | Line | Semantics |
|---|---|---|
| `Toggle` | `Binding.h:52` | Press = activate, press again = deactivate. Default. Key-up / note-off ignored. |
| `Momentary` | `Binding.h:53` | Press = activate with value `1.0f`, release = deactivate with value `0.0f` (piano mode). |

Implemented in dispatch:
- `BindingManager::processKeyUp` skips any binding whose `triggerMode != Momentary` (`BindingManager.cpp:89`).
- `BindingManager::processMidiNoteOff` same gate (`BindingManager.cpp:140`).
- Key-down / note-on always fire regardless of mode (value = `1.0f` for key, normalized velocity for MIDI note).

### 10.4 CC modes (`Binding::CCMode`) + step size

Enum at `Binding.h:59-63`, `uint8_t`. Step size field at `Binding.h:65` (`ccStepSize`, default `0.01f`).

| Mode | Line | Semantics |
|---|---|---|
| `Absolute` | `Binding.h:61` | `outputValue = value / 127.0f` → [0.0, 1.0]. Default. (`BindingManager.cpp:192`) |
| `Relative` | `Binding.h:62` | Signed delta around 64. `value > 64` → `delta = (value - 64) * ccStepSize` (positive); `value < 64` → `delta = (value - 64) * ccStepSize` (negative); `value == 64` → no change. Accumulated state clamped to `[0, 1]`. (`BindingManager.cpp:178-187`) |

Relative state store:
- `relativeCCValues_` `std::unordered_map<int, float>` (`BindingManager.h:72`).
- Key = `(channel << 8) | cc` (`BindingManager.cpp:172`, `205`).
- Initial value when key absent: `0.5f` (middle) (`BindingManager.cpp:174`, `209`).
- Public accessor: `getRelativeCCValue(channel, cc)` (`BindingManager.h:56`, `.cpp:203-210`).
- Cleared on `clearAll()` (`BindingManager.cpp:49`) and on `fromVar()` load (`BindingManager.cpp:252`).

### 10.5 Targeting modes (`Binding::TargetMode`)

Enum at `Binding.h:69-74`, `uint8_t`. Used by the action dispatcher (not by `BindingManager` itself, which only routes the binding struct through the callback).

| Mode | Line | Semantics |
|---|---|---|
| `ByPosition` | `Binding.h:71` | Targets the clip/layer at the specified index; survives reorder (targets the slot, not the content). Default. |
| `ThisItem` | `Binding.h:72` | Targets a specific clip by `targetClipId`; follows the clip if moved. |
| `Selected` | `Binding.h:73` | Targets whatever is currently selected in the UI. |

`BindingManager` only stores/serializes the mode — resolution is in the consumer's `ActionCallback` (see cross-references below).

### 10.6 Velocity-to-opacity

- Field: `Binding::velocityToOpacity`, `bool`, default `false` (`Binding.h:79`).
- Source: incoming MIDI note velocity normalized in `processMidiNoteOn` as `normalizedVelocity = velocity / 127.0f` (`BindingManager.cpp:113`).
- Dispatch always passes `normalizedVelocity` as the second argument to the `ActionCallback` (`BindingManager.cpp:122`). The consumer is responsible for reading `velocityToOpacity` to decide whether to apply the value as clip opacity.
- Not applied by `BindingManager` internally.

### 10.7 Keyboard learn flow

Capture-based "binding mode":
- `bindingMode_` flag (`BindingManager.h:67`), getter/setter `isBindingMode()` / `setBindingMode(bool)` (`BindingManager.h:47-48`).
- `BindingCaptureCallback` signature: `void(InputType type, int keyOrNote, int cc, bool shift, bool cmd, bool alt)` (`BindingManager.h:51-52`).
- Setter: `setBindingCaptureCallback` (`BindingManager.h:53`).
- In `processKeyDown`: when `bindingMode_` is true, invokes `captureCallback_(Keyboard, keyCode, 0, shift, cmd, alt)` and returns true without matching any existing binding (`BindingManager.cpp:54-59`).
- `processKeyUp` returns false immediately if in binding mode (`BindingManager.cpp:80`) — only key-down captures the shortcut.
- The UI is expected to flip `bindingMode_` off and then call `addBinding()` with the captured info.

### 10.8 MIDI learn flow

Same capture mechanism, for note and CC:
- `processMidiNoteOn` in binding mode: `captureCallback_(MidiNote, note, 0, false, false, false)` (`BindingManager.cpp:108-110`). Modifier booleans always `false` for MIDI.
- `processMidiNoteOff` returns false in binding mode (`BindingManager.cpp:131`) — only note-on captures.
- `processMidiCC` in binding mode: `captureCallback_(MidiCC, 0, cc, false, false, false)` (`BindingManager.cpp:156-158`). Note field is `0`; CC number goes in the third arg.

Full routing path (incoming MIDI → binding manager):
- `MidiHandler::handleIncomingMidiMessage` (MIDI thread) → posts lambda via `juce::MessageManager::callAsync` → message thread → `BindingManager::processMidi*` (`MidiHandler.cpp:70-101`).
- Channel extracted via `message.getChannel()` (`MidiHandler.cpp:74`); passed verbatim to `BindingManager` (1-16 from JUCE).
- Velocity extracted via `message.getVelocity()` (`MidiHandler.cpp:79`).
- CC number via `message.getControllerNumber()` (`MidiHandler.cpp:95`), value via `message.getControllerValue()` (`MidiHandler.cpp:96`).

### 10.9 Binding presets (import/export)

P24.10 serialization (`BindingManager.cpp:212-306`). JSON format, version-stamped.

**Persistence API** (`BindingManager.h:59-62`):
- `juce::var toVar() const`
- `void fromVar(const juce::var& v)`
- `bool saveToFile(const juce::File& file) const`
- `bool loadFromFile(const juce::File& file)`

**JSON schema (`toVar()` at `.cpp:214-247`)**:

Root:
```
{
  "version": 1,
  "bindings": [ ... ]
}
```

Each binding (22 properties, `.cpp:220-241`): `inputType`, `keyCode`, `keyModShift`, `keyModCmd`, `keyModAlt`, `midiChannel`, `midiNote`, `midiCC`, `action`, `triggerMode`, `ccMode`, `ccStepSize`, `targetMode`, `targetClipId`, `velocityToOpacity`, `targetLayerIndex`, `targetColumn`, `targetDeckIndex`, `targetEffectIndex`, `targetMacroIndex`, `enabled`. (`id` is NOT serialized — regenerated on load at `.cpp:263`.)

**Save**: `saveToFile` serializes via `juce::JSON::toString(toVar())` and calls `file.replaceWithText(json)` (`.cpp:292-296`).

**Load** (`loadFromFile` at `.cpp:298-306`):
1. Read file text; return false if empty.
2. `juce::JSON::parse(json)`; return false if void/parse error.
3. Call `fromVar(parsed)`.
`fromVar` clears `bindings_` and `relativeCCValues_` first (`.cpp:251-252`), then replays each binding object, reassigning fresh IDs.

No explicit binding-preset merge/append API — load replaces all bindings.

### 10.10 Default keyboard / MIDI mappings

**None in these files.** `BindingManager` starts with an empty `bindings_` vector (`BindingManager.h:65`), and no code in this slice installs default bindings. The audited files contain no hardcoded default key/MIDI mappings — any defaults must be seeded by the caller (e.g., `MainComponent` or a preset file).

**BindingManager public API surface** (full list):
- Construction: default ctor (`.h:13`).
- `uint32_t addBinding(const Binding&)` (`.h:16`, `.cpp:4-10`).
- `bool removeBinding(uint32_t bindingId)` (`.h:19`, `.cpp:12-23`).
- `Binding* getBinding(uint32_t)` / `const Binding* getBinding(uint32_t) const` (`.h:22-23`, `.cpp:25-37`).
- `int getNumBindings() const` inline (`.h:26`).
- `Binding* getBindingAt(int index)` (`.h:27`, `.cpp:39-44`).
- `void clearAll()` (`.h:30`, `.cpp:46-50`) — clears bindings AND relative CC state.
- Input dispatch: `processKeyDown`, `processKeyUp`, `processMidiNoteOn`, `processMidiNoteOff`, `processMidiCC` (`.h:33-39`).
- Callbacks: `setActionCallback(ActionCallback)` (`.h:44`), `setBindingCaptureCallback(BindingCaptureCallback)` (`.h:53`).
- Binding mode: `isBindingMode()` / `setBindingMode(bool)` (`.h:47-48`).
- Relative CC: `getRelativeCCValue(channel, cc)` (`.h:56`).
- Serialization: `toVar`, `fromVar`, `saveToFile`, `loadFromFile` (`.h:59-62`).

---

## MIDI Output

Implemented in `MidiOutputHandler.h/cpp`.

### Device enumeration

- `static juce::Array<juce::MidiDeviceInfo> getAvailableDevices()` (`MidiOutputHandler.h:36`, `.cpp:51-54`) — thin wrapper around `juce::MidiOutput::getAvailableDevices()`.
- `bool openDevice(const juce::String& deviceIdentifier)` (`MidiOutputHandler.h:30`, `.cpp:19-39`) — closes any previous device first (`.cpp:21`), iterates available devices, matches on identifier, calls `juce::MidiOutput::openDevice(identifier)`, logs success/failure to `std::cerr`.
- `void closeDevice()` (`MidiOutputHandler.h:33`, `.cpp:41-49`) — clears all pads before destroying `outputDevice_`.
- `juce::String getDeviceName() const` (`MidiOutputHandler.h:39`, `.cpp:56-61`).
- `bool isOpen() const` inline (`MidiOutputHandler.h:40`) — true when `outputDevice_ != nullptr`.
- Destructor calls `closeDevice()` (`.cpp:14-17`), which sends note-offs for every lit pad.
- Single device open at a time (`std::unique_ptr<juce::MidiOutput> outputDevice_` at `MidiOutputHandler.h:80`). Non-copyable (deleted copy ctor/assign at `.h:73-74`).

### Per-cell color mapping (Launchpad velocity codes)

`PadState` enum at `MidiOutputHandler.h:53-60`, `uint8_t`, 5 values:

| State | Value | Description |
|---|---|---|
| `Empty` | `0` | No clip in cell |
| `Loaded` | `1` | Clip present but stopped / inactive |
| `Playing` | `2` | Clip is the active clip and playing |
| `Triggered` | `3` | Clip is active but not playing (queued/stopped) |
| `ActiveWithFx` | `4` | Clip is playing AND has at least one non-bypassed effect |

Velocity constants (`MidiOutputHandler.h:63-67`):

| Constant | Value | State | Launchpad X/Mini MK3 color |
|---|---|---|---|
| `kVelocityEmpty` | `0` | `Empty` | Note Off (unlit) |
| `kVelocityLoaded` | `5` | `Loaded` | dim amber |
| `kVelocityPlaying` | `60` | `Playing` | green |
| `kVelocityTriggered` | `52` | `Triggered` | flashing green |
| `kVelocityActiveWithFx` | `62` | `ActiveWithFx` | bright yellow |

`int velocityForState(PadState) const` (`MidiOutputHandler.h:77`, `.cpp:153-164`) is a direct switch over the table above.

State determination (`updateFromDeck` at `.cpp:63-126`):
- `newState = Empty` unless a clip is present in `layer.clips[column]` (`.cpp:80-82`).
- If cell IS active clip column (`layer.activeClipColumn == ci`, `.cpp:84`):
  - clip playing → scan `clip->effects` for any `!bypassed` (`.cpp:89-97`); `ActiveWithFx` if any, else `Playing`.
  - clip not playing → `Triggered`.
- Cell is not active → `Loaded`.

### Poll rate / state change detection

- Not driven by a timer inside `MidiOutputHandler`. Caller must invoke `updateFromDeck(const Deck*)` periodically — per the class-level comment at `MidiOutputHandler.h:22-23`: "Called from MainComponent's timerCallback (~50ms)" → approximately 20 Hz.
- Also noted elsewhere in CLAUDE.md as ~6Hz; implementation imposes no rate itself — it's whatever the caller does.
- State change detection: `padStates_` is `std::array<std::array<PadState, kMaxColumns>, kMaxLayers>{}` (`MidiOutputHandler.h:83`), zero-initialized to `Empty`. Each cell's new state is compared to the cached state (`.cpp:112`); MIDI is only sent when the state has changed, to avoid flooding the bus.

Grid bounds (`MidiOutputHandler.h:70-71`):
- `kMaxLayers = 10`
- `kMaxColumns = 20`

Only `min(deck->layers.size(), kMaxLayers)` × `min(deck->numColumns, kMaxColumns)` cells are tracked (`.cpp:68-69`). Cells beyond the grid are ignored.

### Note mapping formula

`int noteForCell(int layer, int column) const` (`MidiOutputHandler.h:78`, `.cpp:166-179`):

```cpp
int row = layer;  // 0 = bottom
int col = column;
return (row + 1) * 10 + (col + 1);
```

Formula: **note = (layer + 1) × 10 + (column + 1)**.

- Layer 0, Column 0 → note 11
- Layer 0, Column 7 → note 18
- Layer 1, Column 0 → note 21
- Layer 7, Column 7 → note 88

This matches the **Launchpad X / Launchpad Mini MK3 grid layout** (notes 11-88, rows from bottom). For grids wider than 8 columns (up to `kMaxColumns=20`), the formula keeps adding (col+1) linearly, which overflows the Launchpad grid — the comment at `.cpp:170-171` notes this: "For grids > 8 columns, wrap or use higher note banks" (not implemented in this slice).

All MIDI output is sent on **MIDI channel 1** (hardcoded `1` at `.cpp:120`, `.cpp:122`, `.cpp:146`).

### Supported controllers

Per `MidiOutputHandler.h:10-21` comments:
- **Launchpad X** — primary target.
- **Launchpad Mini MK3** — primary target, same velocity table.
- Generic reference to "APC40, etc." in the header comment (`.h:11`) but no APC-specific code path exists in this file — velocities are purely Launchpad's palette.
- Any MIDI output device JUCE enumerates can be opened (`openDevice` matches by identifier). The note-on/note-off messages will be sent; whether the pads light correctly depends on the target device interpreting velocities as Launchpad does.
- Raw escape hatch: `void sendMessage(const juce::MidiMessage& msg)` (`.h:47`, `.cpp:128-132`) — user can build sysex/custom messages for non-Launchpad controllers.

### Other MIDI Output APIs

- `void clearAllPads()` (`.h:50`, `.cpp:134-151`) — iterates the full `kMaxLayers × kMaxColumns` grid, sends note-off for any non-`Empty` pad, resets cache to `Empty`. Called by `closeDevice()` and destructor. Channel 1.
- Sends use `sendMessageNow()` (synchronous, no internal MIDI output queue in this file) (`.cpp:120`, `122`, `131`, `146`).

---

## MIDI Input

Implemented in `MidiHandler.h/cpp`. Subclasses `juce::MidiInputCallback`.

### Device enumeration

- `static juce::Array<juce::MidiDeviceInfo> getAvailableDevices()` (`MidiHandler.h:22`, `.cpp:60-63`) — wrapper around `juce::MidiInput::getAvailableDevices()`.
- `bool isDeviceEnabled(const juce::String& deviceIdentifier) const` (`.h:25`, `.cpp:65-68`) — checks internal `enabledDevices_` `StringArray`.
- `void start(juce::AudioDeviceManager& deviceManager)` (`.h:15`, `.cpp:13-27`):
  - Calls `stop()` to clean up previous state.
  - Stores the device manager pointer.
  - Enumerates all MIDI inputs via `juce::MidiInput::getAvailableDevices()`.
  - For each device: if not already enabled in the `AudioDeviceManager`, enables it; then calls `addMidiInputDeviceCallback(identifier, this)`.
  - Tracks identifiers in `enabledDevices_` (`StringArray`).
  - **All inputs are auto-enabled on start** — no opt-in required.
- `void stop()` (`.h:16`, `.cpp:29-38`) — removes this callback from every device in `enabledDevices_`, clears the list, nulls the device-manager pointer.
- `void enableDevice(const juce::String&, bool enabled)` (`.h:19`, `.cpp:40-58`):
  - Enable: sets device enabled in `AudioDeviceManager`, adds callback, appends to `enabledDevices_` (guards against duplicates via `contains`).
  - Disable: removes callback, removes identifier from `enabledDevices_`. Does NOT call `setMidiInputDeviceEnabled(id, false)` — only detaches this handler.
- Destructor calls `stop()` (`.cpp:8-11`).
- No hot-plug watcher — hot-plug handling is the `AudioDeviceManager`'s responsibility; this class only enumerates at `start()`.

### Channel filtering

- No channel filtering inside `MidiHandler`. All messages from all enabled devices are forwarded to `BindingManager`.
- Channel is passed through to `BindingManager::processMidi*` via `message.getChannel()` (`.cpp:74`).
- Channel filtering happens **per-binding** in `BindingManager`:
  - `processMidiNoteOn`: `b.midiChannel == 0 || b.midiChannel == channel` (`.cpp:119`). `0` acts as wildcard.
  - `processMidiNoteOff`: same predicate (`.cpp:142`).
  - `processMidiCC`: same predicate (`.cpp:165`).
- A single `BindingManager` handles all devices/channels; there is no per-device binding routing.

### Note-on, note-off, CC handling

`handleIncomingMidiMessage(juce::MidiInput*, const juce::MidiMessage&)` (`MidiHandler.cpp:70-101`).

Called on JUCE's MIDI thread — explicitly **marshaled to the message thread** via `juce::MessageManager::callAsync` before touching `BindingManager` (comment at `.cpp:73`).

| Message type | Detection | Fields extracted | Forwarded call |
|---|---|---|---|
| Note-on | `message.isNoteOn()` (`.cpp:76`) | `getNoteNumber()`, `getVelocity()` (`.cpp:78-79`) | `bindingManager_.processMidiNoteOn(channel, note, velocity)` (`.cpp:82`) |
| Note-off | `message.isNoteOff()` (`.cpp:85`) | `getNoteNumber()` (`.cpp:87`) | `bindingManager_.processMidiNoteOff(channel, note)` (`.cpp:90`) |
| CC | `message.isController()` (`.cpp:93`) | `getControllerNumber()`, `getControllerValue()` (`.cpp:95-96`) | `bindingManager_.processMidiCC(channel, cc, value)` (`.cpp:99`) |

Other message types (pitch bend, aftertouch, program change, clock, sysex, etc.) are **silently ignored** — no other branches.

**Return value semantics for `BindingManager::processMidi*`**: bool indicating whether any binding was matched/triggered. Unused in this slice (lambda throws the return away on message thread).

No threading protection on `bindings_` or `relativeCCValues_` beyond the message-thread marshaling — `BindingManager` is treated as single-threaded (message thread).

---

## Section 16 partial: Modal surfaces

- **MIDI Learn modal / Keyboard Learn modal**: Not implemented in this slice. The `BindingManager` provides the protocol:
  - Set `bindingMode_ = true` via `setBindingMode(true)` (`BindingManager.h:48`).
  - Set a capture callback via `setBindingCaptureCallback(cb)` (`BindingManager.h:53`) with signature `void(InputType, int keyOrNote, int cc, bool shift, bool cmd, bool alt)`.
  - The next key-down / MIDI note-on / CC will invoke the capture callback and suppress normal dispatch.
  - Modifier booleans are only populated for keyboard captures — always `false` for MIDI note/CC.
  - The UI layer (a `Component` dialog somewhere outside this slice) is responsible for toggling binding mode on/off, rendering the "waiting for input" state, and calling `addBinding()` with the captured details.
- **Edit Keyboard Shortcuts / Edit MIDI Mappings dialogs**: Not implemented in this slice. `BindingManager` exposes the CRUD primitives (`addBinding`, `removeBinding`, `getBindingAt`, `getNumBindings`, `clearAll`) — UI for enumerating/editing bindings would live in a UI component elsewhere.
- **Binding preset import/export dialogs**: Not here. `BindingManager::saveToFile` / `loadFromFile` are plain file helpers — any OS file chooser or preset browser is in the UI layer.
- **MIDI device picker**: Not here. `MidiHandler::getAvailableDevices` + `enableDevice` and `MidiOutputHandler::getAvailableDevices` + `openDevice` are the backing APIs; the picker UI is outside this slice.

---

## Section 20 partial: Cross-references

- **Action consumer**: `BindingManager::ActionCallback` (`BindingManager.h:43`) is set by owning code (likely `MainComponent`) — that consumer interprets all 19 `Binding::Action` values, resolves `TargetMode::ByPosition` / `ThisItem` / `Selected` against the current `Composition` / `Deck` / `Layer` state, and applies `velocityToOpacity`. `BindingManager` itself never touches composition/deck state.
- **Relative encoder state**: `BindingManager::relativeCCValues_` is in-process only; it is NOT serialized by `toVar()`. Encoder "positions" reset on app restart or `clearAll()`.
- **Serialized action integer values**: `Binding::Action` is `uint8_t` starting at `TriggerClip=0`, `TriggerColumn=1`, ..., `ToggleRecording=18`. `toVar`/`fromVar` cast via `int`. Reordering or inserting enum values will **break existing saved presets**.
- **MIDI input → BindingManager** message-thread marshaling: `MidiHandler::handleIncomingMidiMessage` posts lambdas via `juce::MessageManager::callAsync` (`MidiHandler.cpp:80, 88, 97`). `BindingManager` is NOT thread-safe and must only be called from the message thread.
- **MIDI output** flows the other direction: `MainComponent`'s ~50ms timer polls deck state and calls `MidiOutputHandler::updateFromDeck(deck)` (per `MidiOutputHandler.h:22-23` comment). No queue inside `MidiOutputHandler` — uses `sendMessageNow`.
- **`Deck` / `Layer` / `Clip` model dependency**: `MidiOutputHandler.cpp` includes `model/Deck.h`, `model/Layer.h`, `model/Clip.h` (`.cpp:2-4`). Reads `deck->layers`, `deck->numColumns`, `layer.clips`, `layer.activeClipColumn`, `clip->playing`, `clip->effects[i].bypassed`.
- **`Binding::TargetMode::ThisItem`** relies on a persistent `Clip::id` lookup — not resolved here.
- **Persistent-layer / cross-deck actions** (P21) use the same `Binding::Action` values — the consumer is expected to route `SwitchDeck` / `ToggleLayerVisible` / etc. appropriately.
- **Snapshot and ToggleRecording** (actions 17, 18) are P22 features — consumer routes to `Renderer::takeSnapshot()` and `VideoRecorder` respectively.
- **Ableton Link** (P21): not referenced here — Link state is outside the binding/MIDI surface.
- **No APC-specific code**: although the header mentions APC40, the velocity table (`kVelocity*`) is specifically Launchpad X / Mini MK3. APC owners must tolerate approximate colors or use `sendMessage()` with their own sysex to set colors.
- **Channel wildcard = `0`**: Used across three `processMidi*` methods. `midiChannel` persists as `0` unless explicitly set — new bindings match any channel by default.

---

## Summary

**File paths audited**: `src/binding/Binding.h`, `src/binding/BindingManager.h/cpp`, `src/midi/MidiHandler.h/cpp`, `src/midi/MidiOutputHandler.h/cpp` (all under `/Users/boriskarpman/Documents/RealTimeAudio/`).

**Total `Binding::Action` values**: **19** — `TriggerClip`, `TriggerColumn`, `ToggleLayerBypass`, `ToggleLayerSolo`, `ToggleLayerMute`, `ToggleLayerAutopilot`, `ToggleLayerVisible`, `LayerTransport`, `ToggleEffectBypass`, `AdjustMacro`, `AdjustLayerOpacity`, `SwitchDeck`, `TapTempo`, `Resync`, `GlobalPlayPause`, `GlobalStop`, `MasterOpacity`, `Snapshot`, `ToggleRecording`.

**Supported MIDI output controllers**: Explicitly two (Launchpad X, Launchpad Mini MK3, via identical velocity table `0/5/60/52/62`). APC40 referenced in header comment but no APC-specific code path. Any MIDI output device enumerable by JUCE can be opened via `openDevice()`; non-Launchpad controllers receive note-ons on channel 1 with Launchpad velocity codes (colors will not match) or users must call `sendMessage()` with custom messages.

**Surprises / gaps**:
- `BindingManager` has **no default bindings seeded** — audited files install nothing; defaults must come from the caller or a preset file not in this slice.
- MIDI output hardcodes **channel 1** (`MidiOutputHandler.cpp:120, 122, 146`); no config for alternate channels.
- MIDI output poll rate is **caller-driven** with no internal timer — class comment says ~50ms but CLAUDE.md mentions ~6Hz, so the actual cadence is wherever `MainComponent::timerCallback` calls it.
- Note mapping formula `(layer+1)*10 + (column+1)` **overflows the Launchpad 8×8 grid** when `column >= 8` (up to `kMaxColumns=20`); the header comment acknowledges this but provides no wrap/bank logic.
- Relative encoder state (`relativeCCValues_`) is **not serialized** — encoder positions reset on app restart and on binding preset load.
- `MidiHandler::start()` **auto-enables every detected MIDI input** without user opt-in — could cause surprise routing from unrelated devices.
- `Binding::Action` enum values are serialized as raw `int` — **enum reordering silently corrupts saved presets** (no version-to-action migration).
- `enableDevice(..., false)` **removes the callback but does not call `setMidiInputDeviceEnabled(false)`** on the AudioDeviceManager, so the device stays enabled at the JUCE layer.
- Non-note, non-CC MIDI messages (pitch bend, aftertouch, sysex, clock, program change) are **silently dropped** in `MidiHandler::handleIncomingMidiMessage`.
- `TargetMode` is stored but **not resolved inside this slice** — the action callback owner is entirely responsible for `ByPosition` vs `ThisItem` vs `Selected` interpretation.
