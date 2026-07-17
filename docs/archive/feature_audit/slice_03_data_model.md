# Slice 03 — Core Data Model (Deck / Layer / Clip / Composition / Autopilot) + Undo

Files audited (line-by-line):

- `/Users/boriskarpman/Documents/RealTimeAudio/src/model/Clip.h`
- `/Users/boriskarpman/Documents/RealTimeAudio/src/model/Clip.cpp`
- `/Users/boriskarpman/Documents/RealTimeAudio/src/model/Layer.h`
- `/Users/boriskarpman/Documents/RealTimeAudio/src/model/Layer.cpp`
- `/Users/boriskarpman/Documents/RealTimeAudio/src/model/Deck.h`
- `/Users/boriskarpman/Documents/RealTimeAudio/src/model/Composition.h`
- `/Users/boriskarpman/Documents/RealTimeAudio/src/model/Autopilot.h`
- `/Users/boriskarpman/Documents/RealTimeAudio/src/model/Autopilot.cpp`
- `/Users/boriskarpman/Documents/RealTimeAudio/src/core/Command.h`
- `/Users/boriskarpman/Documents/RealTimeAudio/src/core/UndoManager.h`
- `/Users/boriskarpman/Documents/RealTimeAudio/src/core/UndoManager.cpp`

---

## Section 3: Deck / Layer / Clip Model

### 3.1 Deck — all fields

Source: `src/model/Deck.h`

| Field | Type | Default | Notes | Cite |
|-------|------|---------|-------|------|
| `name` | `std::string` | `"Deck 1"` | Display name | Deck.h:14 |
| `id` | `uint32_t` | `0` | Unique ID | Deck.h:15 |
| `layers` | `std::vector<Layer>` | empty | Rows | Deck.h:18 |
| `numColumns` | `int` | `12` | Columns in grid | Deck.h:19 |
| `kDefaultLayers` | `static constexpr int` | `3` | Default layer count on init | Deck.h:22 |
| `kDefaultColumns` | `static constexpr int` | `12` | Default column count | Deck.h:23 |
| `nextLayerId_` (private) | `uint32_t` | `100` | Assigned to new layers from `addLayer()` | Deck.h:175 |

**Methods / behaviours (Deck.h):**

- `initDefault()` — creates `kDefaultLayers` (3) layers; layer 0 is `Opaque`, others are `Transparent`; each layer has `ensureColumns(numColumns)` applied (Deck.h:26-38).
- `getLayer(int index)` / `getNumLayers()` (Deck.h:41-48).
- `addLayer(Layer::Type type = Transparent)` — assigns `id = nextLayerId_++` starting at 100 (Deck.h:50-58).
- `removeLayer(int index)` — returns false if `layers.size() <= 1` (must keep at least 1) (Deck.h:60-68).
- `moveLayer(int fromIndex, int toIndex)` — P24.13 reorder (Deck.h:71-84).
- `addColumn()` / `removeColumn(int col)` — `removeColumn` refuses if `numColumns <= 1` (Deck.h:87-105).
- `triggerColumn(int col)` — iterates all layers, skipping layers with `ignoreColumnTrigger` (Deck.h:108-116).
- `getClip(layerIndex, column)` / `setClip(layerIndex, column, clip)` — `setClip` auto-grows columns (Deck.h:119-135).
- `toVar()` / `fromVar()` serialize `name, id, numColumns, layers[]` (Deck.h:138-172).

### 3.2 Layer — all fields + each type's specifics + ALL blend modes

Source: `src/model/Layer.h`, `src/model/Layer.cpp`

**Identity / general state:**

| Field | Type | Default | Cite |
|-------|------|---------|------|
| `name` | `std::string` | `"Layer"` | Layer.h:30 |
| `id` | `uint32_t` | `0` | Layer.h:31 |
| `type` | `Type` enum | `Type::Opaque` | Layer.h:42 |
| `opacity` | `float` | `1.0f` | Layer.h:45 |
| `visible` | `bool` | `true` | Layer.h:46 |
| `bypassed` | `bool` | `false` | Layer.h:47 |
| `solo` | `bool` | `false` | Layer.h:48 |
| `muted` | `bool` | `false` (audio mute) | Layer.h:49 |
| `autopilotEnabled` | `bool` | `false` | Layer.h:50 |
| `ignoreColumnTrigger` | `bool` | `false` | Layer.h:51 |
| `persistent` | `bool` | `false` | keeps rendering when deck not active (Layer.h:52) |
| `folded` | `bool` | `false` | P24.12 collapsed row (Layer.h:53) |

**Layer type enum** (Layer.h:34-41):

| Value | Description |
|-------|------------|
| `Opaque` | One clip at a time, replaces everything below |
| `Transparent` | Composited over layers below with blend/keying |
| `FXOnly` | Effects applied to accumulator (no media) |
| `ThreeD` | 3D model/surface rendering |
| `Mask` | Content becomes alpha mask for layers below |

**Blend Mode ("MixMode") enum — unified for both layer blend AND clip transitions** (Layer.h:59-93). Total **52** values.

Persistent/standard compositing (26 values):

1. `Normal` (Layer.h:62)
2. `Additive`
3. `Screen`
4. `Multiply`
5. `Overlay` *(section "Basic")*
6. `SoftLight` (Layer.h:64, section "Light")
7. `HardLight`
8. `VividLight`
9. `LinearLight`
10. `PinLight`
11. `HardMix`
12. `Darken` (Layer.h:66, section "Dark/Light compare")
13. `Lighten`
14. `DarkerColor`
15. `LighterColor`
16. `ColorDodge` (Layer.h:68, section "Dodge/Burn")
17. `ColorBurn`
18. `Difference` (Layer.h:70, section "Inversion")
19. `Exclusion`
20. `Subtract`
21. `Hue` (Layer.h:72, section "Component (HSL)")
22. `Saturation`
23. `Color`
24. `Luminosity`
25. `Dissolve` (Layer.h:74, "Special blend")

Transition modes (also usable as blend) (27 values):

26. `Cut` (Layer.h:79, "Instant")
27. `WipeLeft` (Layer.h:81, "Directional wipes")
28. `WipeRight`
29. `WipeUp`
30. `WipeDown`
31. `WipeEllipse`
32. `WipeDiagonal`
33. `PushLeft` (Layer.h:83, "Push")
34. `PushRight`
35. `PushUp`
36. `PushDown`
37. `ZoomIn` (Layer.h:85, "Zoom")
38. `ZoomOut`
39. `RotateX` (Layer.h:87, "3D rotation")
40. `RotateY`
41. `Spin`
42. `Cube`
43. `Flip`
44. `Fold`
45. `ToBlack` (Layer.h:89, "Fade through color")
46. `ToWhite`
47. `Pixelate` (Layer.h:91, "Creative / VJ")
48. `Blur`
49. `Noise`
50. `RGBSplit`
51. `GlitchBlocks`
52. `Strobe`
53. `Slide` (Layer.h:92)
54. `Stretch`
55. `Displace`

**Total MixMode enum values: 55** (re-counted directly from Layer.h:59-93: 25 "standard" + 30 "transition" entries, where `Dissolve` is listed as standard but also bridges transitions). Default: `blendMode = MixMode::Additive` (Layer.h:94).

**Keying Mode enum** (Layer.h:97-102) — for `Transparent` type. Default: `KeyingMode::Alpha` (Layer.h:103). Total **13** values:

1. `Alpha`
2. `LumaKey`
3. `InvertedLumaKey`
4. `LumaIsAlpha`
5. `InvertedLumaIsAlpha`
6. `ChromaKey`
7. `MaxRGB`
8. `SaturationKey`
9. `EdgeDetection`
10. `ThresholdMask`
11. `ChannelR`
12. `ChannelG`
13. `ChannelB`

**Keying parameters:**

| Field | Type | Default | Cite |
|-------|------|---------|------|
| `keyThreshold` | `float` | `0.1f` | Layer.h:104 |
| `keySoftness` | `float` | `0.1f` | Layer.h:105 |
| `chromaKeyR` | `float` | `0.0f` | Layer.h:106 |
| `chromaKeyG` | `float` | `1.0f` | Layer.h:106 |
| `chromaKeyB` | `float` | `0.0f` | Layer.h:106 |
| `chromaKeyTolerance` | `float` | `0.2f` | Layer.h:107 |

**FX Only–specific:**

| Field | Type | Default | Cite |
|-------|------|---------|------|
| `dryWetMix` | `float` | `1.0f` | Layer.h:110 |

**3D Layer–specific:**

| Field | Type | Default | Cite |
|-------|------|---------|------|
| `rotationX` | `float` | `0.0f` | Layer.h:113 |
| `rotationY` | `float` | `0.0f` | Layer.h:113 |
| `rotationZ` | `float` | `0.0f` | Layer.h:113 |
| `rotationSpeed` | `float` | `0.0f` | Layer.h:114 |
| `scale3D` | `float` | `1.0f` | Layer.h:115 |

**Per-layer video / dimensions:**

| Field | Type | Default | Cite |
|-------|------|---------|------|
| `layerWidth` | `int` | `1920` | Layer.h:118 |
| `layerHeight` | `int` | `1080` | Layer.h:119 |
| `autoSize` | `AutoSizeMode` | `AutoSizeMode::Off` | Layer.h:120-121 |

**AutoSizeMode enum values** (Layer.h:120). **5** values:

1. `Off`
2. `Fill`
3. `Fit`
4. `Stretch`
5. `Original`

**Transition fields:**

| Field | Type | Default | Cite |
|-------|------|---------|------|
| `transitionMode` | `MixMode` | `MixMode::Dissolve` | Layer.h:124 (F dropdown — momentary clip change style) |
| `transitionBlendMode` | `MixMode` | `MixMode::Normal` | Layer.h:125 (transition blend method) |
| `transitionSpeed` | `float` | `-1.0f` | Layer.h:126 (-1 = use global default) |

**Per-layer transform** (applied after clip compositing):

| Field | Type | Default | Cite |
|-------|------|---------|------|
| `positionX` | `float` | `0.0f` | Layer.h:129 |
| `positionY` | `float` | `0.0f` | Layer.h:130 |
| `layerScale` | `float` | `1.0f` | Layer.h:131 |
| `layerRotation` | `float` | `0.0f` (degrees) | Layer.h:132 |
| `layerAnchorX` | `float` | `0.0f` | Layer.h:133 |
| `layerAnchorY` | `float` | `0.0f` | Layer.h:134 |

**Feedback (Larsen loop):**

| Field | Type | Default | Cite |
|-------|------|---------|------|
| `feedback` | `FeedbackConfig` | see 3.8 | Layer.h:137 |

**Per-layer effects & autopilot defaults:**

| Field | Type | Default | Cite |
|-------|------|---------|------|
| `layerEffects` | `std::vector<Clip::EffectSlot>` | empty | Layer.h:140 |
| `defaultAutopilotAction` | `Clip::AutopilotAction` | `PlayNext` | Layer.h:143 |
| `defaultAutopilotDuration` | `Clip::AutopilotDuration` | `Beat4` | Layer.h:144 |
| `defaultAutopilotCustomBeats` | `int` | `4` | Layer.h:145 |
| `autopilotLoops` | `int` | `1` | Layer.h:146 (# clip loops before advancing) |
| `autopilotEndOfVideo` | `bool` | `false` | Layer.h:147 (advance when playhead reaches outPoint) |

**Clips and runtime state:**

| Field | Type | Default | Cite |
|-------|------|---------|------|
| `clips` | `std::vector<std::optional<Clip>>` | empty | Layer.h:151 |
| `activeClipColumn` | `int` | `-1` | Layer.h:154 |
| `previousClipColumn` | `int` | `-1` | Layer.h:155 (for crossfade) |
| `crossfadeProgress` | `float` | `1.0f` | Layer.h:156 |
| `pendingTriggerColumn` | `int` | `-1` | Layer.h:157 (queued beat-snap trigger) |

**Layer methods (trigger / snap):**

- `getActiveClip()` / `getActiveClip() const` (Layer.h:160-178).
- `getClipAt(int column)` (Layer.h:180-188).
- `triggerClip(int column)` — honors `beatSnap`/`beatSnapMode` and queues via `pendingTriggerColumn` (Layer.h:190-214).
- `triggerClipImmediate(int column)` — retrigger preserves playing state; new clip auto-plays only if `!hasBeenTriggered` (Layer.h:218-250).
- `processPendingTrigger(beatInBar, barCount)` — P21 granularity; dispatches based on `BeatSnapMode`:
  - `Off`/`Beat` → trigger every beat (Layer.h:269-271).
  - `Bar` → `beatInBar == 0` (Layer.h:273-274).
  - `TwoBar` → `beatInBar == 0 && (barCount % 2) == 0` (Layer.h:276-277).
  - `FourBar` → `beatInBar == 0 && (barCount % 4) == 0` (Layer.h:279-280).
- `clearActiveClip()` (Layer.h:288-298).
- `ensureColumns(count)` (Layer.h:300-304).
- Serialization: `toVar()` / `fromVar()` (Layer.cpp:3-140). Note: `folded` is read conditionally (Layer.cpp:82-83).

### 3.3 Clip — all fields + all enum values

Source: `src/model/Clip.h`, `src/model/Clip.cpp`

**Identity / media:**

| Field | Type | Default | Cite |
|-------|------|---------|------|
| `name` | `std::string` | empty | Clip.h:13 |
| `id` | `uint32_t` | `0` | Clip.h:14 |
| `mediaType` | `MediaType` | `MediaType::None` | Clip.h:17-18 |
| `mediaFile` | `juce::File` | empty | Clip.h:19 |
| `cameraDeviceIndex` | `int` | `-1` | Clip.h:20 |
| `sourceType` | `std::string` | empty | Clip.h:21 (e.g. "perlin_noise") |
| `hasAlpha` | `bool` | `false` | Clip.h:22 |

**MediaType enum** (Clip.h:17) — **6** values:

1. `None`
2. `Image`
3. `Video`
4. `Camera`
5. `Source`
6. `ImageSequence`

**Image sequence:**

| Field | Type | Default | Cite |
|-------|------|---------|------|
| `sequenceFiles` | `std::vector<juce::File>` | empty | Clip.h:25 |
| `sequenceFps` | `float` | `2.5f` | Clip.h:26 |
| `beatDivision` | `float` | `4.0f` | Clip.h:30 (beats per playback cycle) |
| `videoBeats` | `float` | `4.0f` | Clip.h:35 (content beat length) |

**Source parameters** (Clip::SourceParam, Clip.h:38-45):

| Field | Type | Default |
|-------|------|---------|
| `name` | `std::string` | empty |
| `uniformName` | `std::string` | empty |
| `value` | `float` | `0.5f` |
| `defaultValue` | `float` | `0.5f` |

Clip holds `std::vector<SourceParam> sourceParams` (Clip.h:45).

**Per-clip effect chain** — `Clip::EffectSlot` (Clip.h:48-55):

| Field | Type | Default |
|-------|------|---------|
| `effectName` | `std::string` | empty |
| `paramValues` | `std::vector<float>` | empty |
| `dryWet` | `float` | `1.0f` |
| `enabled` | `bool` | `true` |
| `bypassed` | `bool` | `false` |

Clip holds `std::vector<EffectSlot> effects` (Clip.h:56).

**Transport:**

| Field | Type | Default | Cite |
|-------|------|---------|------|
| `transportMode` | `TransportMode` | `TransportMode::Timeline` | Clip.h:60 |
| `loopMode` | `LoopMode` | `LoopMode::Loop` | Clip.h:62 |
| `speed` | `float` | `1.0f` | Clip.h:63 |
| `reverse` | `bool` | `false` | Clip.h:64 |
| `startOffset` | `float` | `0.0f` | Clip.h:65 (legacy, use inPoint) |
| `inPoint` | `float` | `0.0f` | Clip.h:68 |
| `outPoint` | `float` | `1.0f` | Clip.h:69 |

**TransportMode enum** (Clip.h:59) — **2** values:

1. `Timeline`
2. `BPMSync`

**LoopMode enum** (Clip.h:61) — **3** values:

1. `Loop`
2. `PingPong`
3. `OneShot`

**BeatSnapMode enum** (Clip.h:72-79) — **5** values; default `BeatSnapMode::Off`:

1. `Off` — No snapping, immediate trigger
2. `Beat` — Snap to next beat
3. `Bar` — Snap to next bar (4 beats)
4. `TwoBar` — Snap to next 2-bar boundary (8 beats)
5. `FourBar` — Snap to next 4-bar boundary (16 beats)

Legacy `beatSnap` bool (Clip.h:81) = `true` if `beatSnapMode != Off`. Upgrade rule in `fromVar`: if `beatSnap == true && beatSnapMode == Off`, promote to `Beat` (Clip.cpp:141-142).

**Autopilot on the Clip:**

| Field | Type | Default | Cite |
|-------|------|---------|------|
| `autopilotAction` | `AutopilotAction` | `LayerDetermined` | Clip.h:94 |
| `autopilotSpecificCol` | `int` | `-1` | Clip.h:95 |
| `autopilotDuration` | `AutopilotDuration` | `LayerDetermined` | Clip.h:101 |
| `autopilotCustomBeats` | `int` | `4` | Clip.h:102 |

**AutopilotAction enum** (Clip.h:89-93) — **8** values:

1. `LayerDetermined`
2. `DoNothing`
3. `PlayNext`
4. `PlayPrevious`
5. `PlayRandom`
6. `PlayFirst`
7. `PlayLast`
8. `PlaySpecific`

**AutopilotDuration enum** (Clip.h:97-100) — **8** values:

1. `LayerDetermined`
2. `Beat1`
3. `Beat2`
4. `Beat4`
5. `Beat8`
6. `Beat16`
7. `Beat32`
8. `Custom`

**Video properties (per-clip):**

| Field | Type | Default | Cite |
|-------|------|---------|------|
| `clipOpacity` | `float` | `1.0f` | Clip.h:105 |
| `clipWidth` | `int` | `1920` | Clip.h:106 |
| `clipHeight` | `int` | `1080` | Clip.h:107 |
| `blendOverride` | `BlendOverride` | `LayerDetermined` | Clip.h:108-109 |
| `alphaType` | `AlphaType` | `Premultiplied` | Clip.h:110-111 |
| `channelR` | `bool` | `true` | Clip.h:112 |
| `channelG` | `bool` | `true` | Clip.h:112 |
| `channelB` | `bool` | `true` | Clip.h:112 |
| `channelA` | `bool` | `true` | Clip.h:112 |

**BlendOverride enum** (Clip.h:108) — **2** values: `LayerDetermined`, `Override`.

**AlphaType enum** (Clip.h:110) — **2** values: `Premultiplied`, `Straight`.

**Per-clip transform** (applied before layer compositing):

| Field | Type | Default | Cite |
|-------|------|---------|------|
| `positionX` | `float` | `0.0f` (pixels offset from center) | Clip.h:115 |
| `positionY` | `float` | `0.0f` | Clip.h:116 |
| `scale` | `float` | `1.0f` (100%) | Clip.h:117 |
| `rotation` | `float` | `0.0f` (degrees) | Clip.h:118 |
| `anchorX` | `float` | `0.0f` | Clip.h:119 |
| `anchorY` | `float` | `0.0f` | Clip.h:120 |

**MilkDrop preset playlist (P20.5):**

`Clip::PresetEntry` (Clip.h:124-130):

| Field | Type | Default |
|-------|------|---------|
| `presetPath` | `std::string` | empty |
| `presetName` | `std::string` | empty |
| `mood` | `std::string` | empty |
| `energy` | `float` | `0.5f` |

| Field | Type | Default | Cite |
|-------|------|---------|------|
| `presetPlaylist` | `std::vector<PresetEntry>` | empty | Clip.h:131 |
| `presetPlaylistIndex` | `mutable int` | `0` | Clip.h:132 |
| `presetBeatsPlayed` | `mutable int` | `0` | Clip.h:133 |
| `playlistCycleMode` | `PlaylistCycleMode` | `RandomBag` | Clip.h:139 |
| `playlistTrigger` | `PlaylistTrigger` | `Beats` | Clip.h:145 |
| `playlistTriggerBeats` | `int` | `8` | Clip.h:146 |
| `playlistBlendSeconds` | `float` | `1.5f` | Clip.h:147 |
| `playlistEnabled` | `bool` | `false` | Clip.h:148 |
| `hasPresetPlaylist()` helper | — | — | Clip.h:150 |

**PlaylistCycleMode enum** (Clip.h:135-138) — **5** values:

1. `Sequential`
2. `Reverse`
3. `RandomOther`
4. `RandomBag`
5. `PingPong`

**PlaylistTrigger enum** (Clip.h:141-144) — **6** values:

1. `Beats`
2. `Bars`
3. `Phrase`
4. `OnDrop`
5. `OnBreakdown`
6. `Manual`

**Content lock (P24.5):**

| Field | Type | Default | Cite |
|-------|------|---------|------|
| `contentLocked` | `bool` | `false` | Clip.h:153 |

Used by `replaceContent()` (Clip.h:170-241) — refuses to overwrite media when locked; otherwise preserves effects, transport, loop, speed, reverse, in/out, beat snap, autopilot action+duration, blend override, alpha type, RGBA channels, transform, and the lock flag itself while copying in new media + source params + playlist + dimensions.

**Runtime state (not serialized):**

| Field | Type | Default | Cite |
|-------|------|---------|------|
| `playing` | `mutable bool` | `false` | Clip.h:156 (render thread updates for OneShot/PingPong stop) |
| `playheadPosition` | `mutable double` | `0.0` | Clip.h:157 (normalized [0,1]) |
| `beatsPlayed` | `int` | `0` | Clip.h:158 |
| `hasBeenTriggered` | `bool` | `false` | Clip.h:159 |
| `thumbnail` | `juce::Image` | empty | Clip.h:160 |

**Helpers (Clip.h:163-166):**

- `hasMedia()` — `mediaType != None`
- `isPlayable()` — `Video` or `ImageSequence`
- `hasEffects()` — `!effects.empty()`
- `isEmpty()` — `!hasMedia() && !hasEffects()`

**`clear()`** (Clip.h:243-280) resets all fields to defaults, including `contentLocked = false`.

### 3.4 Cuepoints

Source: Clip.h:84-86, Clip.cpp:70-73, 167-175.

- `static constexpr int kMaxCuepoints = 8` (Clip.h:84).
- `float cuepoints[kMaxCuepoints] = {}` — stored as **normalized positions [0,1]** (Clip.h:85).
- `int numCuepoints = 0` (Clip.h:86).
- Serialization stores `cuepoints` as a `juce::Array<var>` in JSON, reloaded up to `kMaxCuepoints` on read.
- No explicit `setCuepoint` / `triggerCuepoint` / `clearCuepoint` methods in `Clip.h` (managed externally). `clear()` resets `numCuepoints = 0` (Clip.h:265). `replaceContent()` does NOT preserve cuepoints (not mentioned in Clip.h:170-241 preserved list).

### 3.5 Transform (per level)

| Level | Fields | Cite |
|-------|--------|------|
| Per-clip (pre-layer) | `positionX, positionY, scale (1.0), rotation, anchorX, anchorY` | Clip.h:115-120 |
| Per-layer (post-clip composite) | `positionX, positionY, layerScale (1.0), layerRotation, layerAnchorX, layerAnchorY` | Layer.h:129-134 |
| Composition (final output) | `compPositionX, compPositionY, compScale (1.0), compRotation, compAnchorX, compAnchorY` | Composition.h:40-45 |

3D layers additionally expose `rotationX/Y/Z`, `rotationSpeed`, `scale3D` (Layer.h:113-115).

### 3.6 Video (per level)

| Level | Fields | Cite |
|-------|--------|------|
| Clip | `clipOpacity (1.0), clipWidth (1920), clipHeight (1080), blendOverride (LayerDetermined), alphaType (Premultiplied), channelR/G/B/A (all true), hasAlpha (false)` | Clip.h:22, 105-112 |
| Layer | `opacity (1.0), layerWidth (1920), layerHeight (1080), autoSize (AutoSizeMode::Off)` | Layer.h:45, 118-121 |
| Composition | `masterOpacity (1.0), masterSpeed (1.0), compOpacity (1.0), outputWidth (1920), outputHeight (1080), outputDisplay (-1)` | Composition.h:24-25, 28, 101-103 |

### 3.7 Keying options

Source: Layer.h:97-107. Default `KeyingMode::Alpha`. All 13 enum values listed above in §3.2. Associated params: `keyThreshold (0.1f)`, `keySoftness (0.1f)`, `chromaKeyR (0.0f)`, `chromaKeyG (1.0f)`, `chromaKeyB (0.0f)`, `chromaKeyTolerance (0.2f)`.

### 3.8 Feedback presets

Source: `FeedbackConfig` struct (Layer.h:12-23). Per-layer struct; `presetName` string field stores which of 6 presets is selected (CLAUDE.md says Zoom In, Spiral, Drift, Kaleidoscope, Echo, Stretch — but preset names themselves are not defined inside `FeedbackConfig`; only the `presetName` string is stored. The preset list is defined outside this slice, in `FeedbackProcessor`).

| Field | Type | Default | Cite |
|-------|------|---------|------|
| `enabled` | `bool` | `false` | Layer.h:14 |
| `amount` | `float` | `0.5f` | Layer.h:15 [0,1] how much of prev frame bleeds through |
| `scaleX` | `float` | `0.98f` | Layer.h:16 (<1 zoom in, >1 zoom out) |
| `scaleY` | `float` | `0.98f` | Layer.h:17 |
| `rotation` | `float` | `0.0f` | Layer.h:18 (degrees/frame) |
| `offsetX` | `float` | `0.0f` | Layer.h:19 (range [-0.5, 0.5]) |
| `offsetY` | `float` | `0.0f` | Layer.h:20 |
| `lumaKey` | `float` | `0.0f` | Layer.h:21 (fade dark areas, [0,1]) |
| `presetName` | `std::string` | empty | Layer.h:22 ("" = custom) |

Note: FeedbackConfig is NOT serialized in the current `Layer::toVar()` / `fromVar()` (Layer.cpp:3-140). The `feedback` field exists in memory but does not round-trip to disk.

---

## Section 7: Composition / Global

Source: `src/model/Composition.h`.

**Identity / structure:**

| Field | Type | Default | Cite |
|-------|------|---------|------|
| `name` | `std::string` | `"Untitled"` | Composition.h:13 |
| `filePath` | `juce::File` | empty | Composition.h:14 |
| `decks` | `std::vector<Deck>` | empty | Composition.h:17 |
| `activeDeckIndex` | `int` | `0` | Composition.h:18 |
| `globalEffects` | `std::vector<Clip::EffectSlot>` | empty | Composition.h:21 |

**Master:**

| Field | Type | Default | Cite |
|-------|------|---------|------|
| `masterOpacity` | `float` | `1.0f` | Composition.h:24 |
| `masterSpeed` | `float` | `1.0f` | Composition.h:25 |
| `compOpacity` | `float` | `1.0f` | Composition.h:28 |

**CrossFader:**

| Field | Type | Default | Cite |
|-------|------|---------|------|
| `crossfaderPhase` | `float` | `0.5f` | Composition.h:31 |
| `crossfaderBlendMode` | `CrossfaderBlendMode` | `Alpha` | Composition.h:33 |
| `crossfaderBehaviour` | `CrossfaderBehaviour` | `Cut` | Composition.h:35 |
| `crossfaderCurve` | `CrossfaderCurve` | `Linear` | Composition.h:37 |

**Cross-deck / CrossFader blend modes (3 values)** — `CrossfaderBlendMode` enum (Composition.h:32):

1. `Alpha` (crossfade)
2. `Add` (additive)
3. `Multiply`

**CrossfaderBehaviour enum** (Composition.h:34) — **2** values: `Cut`, `Smooth`.

**CrossfaderCurve enum** (Composition.h:36) — **3** values: `Linear`, `EaseInOut`, `SCurve`.

**Composition transform (final output):**

| Field | Type | Default | Cite |
|-------|------|---------|------|
| `compPositionX` | `float` | `0.0f` | Composition.h:40 |
| `compPositionY` | `float` | `0.0f` | Composition.h:41 |
| `compScale` | `float` | `1.0f` | Composition.h:42 |
| `compRotation` | `float` | `0.0f` | Composition.h:43 |
| `compAnchorX` | `float` | `0.0f` | Composition.h:44 |
| `compAnchorY` | `float` | `0.0f` | Composition.h:45 |

**Global tempo / quantize:**

| Field | Type | Default | Cite |
|-------|------|---------|------|
| `globalTransitionSpeed` | `float` | `0.3f` (seconds) | Composition.h:48 |
| `bpmMultiplier` | `int` | `1` | Composition.h:49 (-4, -2, 1, 2, 4) |
| `quantizeMode` | `QuantizeMode` | `Off` | Composition.h:52 |

**QuantizeMode enum** (Composition.h:51) — **3** values: `Off`, `NextBeat`, `NextDownbeat`.

**Composition-level autopilot:**

| Field | Type | Default | Cite |
|-------|------|---------|------|
| `autopilotDirection` | `AutopilotDirection` | `Off` | Composition.h:56 |
| `autopilotDurationMode` | `AutopilotDurationMode` | `LongestClip` | Composition.h:58 |
| `autopilotClipLoops` | `int` | `1` | Composition.h:59 |
| `autopilotLoop` | `bool` | `false` | Composition.h:60 |
| `autopilotMasterLayer` | `int` | `-1` (Off) | Composition.h:61 |

**AutopilotDirection enum** (Composition.h:55) — **4** values: `Rewind`, `Off`, `Forward`, `Random`.

**AutopilotDurationMode enum** (Composition.h:57) — **3** values: `LongestClip`, `ClipTransport`, `Custom`.

**Per-type autopilot** — `Composition::PerTypeAutopilotConfig` struct (Composition.h:65-85):

| Field | Type | Default | Applies to |
|-------|------|---------|-----------|
| `opaqueCycleBeats` | `int` | `16` | Opaque layers |
| `opaquePlayUntilEnd` | `bool` | `false` | Opaque layers |
| `transparentCycleBeats` | `int` | `8` | Transparent/ThreeD/Mask |
| `transparentMaxLayers` | `int` | `2` | Transparent |
| `transparentRandomize` | `bool` | `true` | Transparent |
| `effectCycleBeats` | `int` | `4` | FXOnly |
| `effectMaxLayers` | `int` | `2` | FXOnly |
| `effectRandomize` | `bool` | `true` | FXOnly |
| `perTypeEnabled` | `bool` | `false` | master enable |
| `globalRandomize` | `bool` | `false` | global override |
| `loopAutopilot` | `bool` | `true` | — |

Held as `PerTypeAutopilotConfig perTypeAutopilot` field (Composition.h:86).

**Genre-aware automation (P23):**

| Field | Type | Default | Cite |
|-------|------|---------|------|
| `autoPresetOnGenre` | `bool` | `false` | Composition.h:89 |
| `smartAutopilotEnabled` | `bool` | `false` | Composition.h:90 |
| `structuralSceneEnabled` | `bool` | `false` | Composition.h:91 |
| `genreDeckAssignment[8]` | `int[8]` | all `-1` | Composition.h:95 (genre ID → deck index, -1 = no switch) |
| `genrePresetNames[8]` | `std::string[8]` | empty | Composition.h:98 |

**Output:**

| Field | Type | Default | Cite |
|-------|------|---------|------|
| `outputWidth` | `int` | `1920` | Composition.h:101 |
| `outputHeight` | `int` | `1080` | Composition.h:102 |
| `outputDisplay` | `int` | `-1` (no external) | Composition.h:103 |

**Methods:**

- `initDefault()` — name `"Untitled"`, empty file path, creates 1 deck ("Deck 1", id 0, `initDefault`), resets master/transition/bpmMultiplier/quantize (Composition.h:106-122).
- `getActiveDeck()` (Composition.h:125-137).
- `addDeck(name = "New Deck")` — `id = nextDeckId_++` starting at 100 (Composition.h:140-147, 265).
- `removeDeck(index)` — refuses if `decks.size() <= 1` (Composition.h:149-159).
- `toVar()`/`fromVar()` serialize: `name, activeDeckIndex, masterOpacity, globalTransitionSpeed, bpmMultiplier, quantizeMode, outputWidth/Height/Display, decks[], globalEffects[]` (Composition.h:162-244). Note: `compOpacity`, `masterSpeed`, crossfader fields, comp transform fields, per-type autopilot config, genre-aware automation fields are **NOT** serialized.
- `saveToFile(juce::File)` / `loadFromFile(juce::File)` via `juce::JSON::toString` / `juce::JSON::parse` (Composition.h:247-262).

---

## Section 7 (cont.): Autopilot Logic

Source: `Autopilot.h`, `Autopilot.cpp`.

**Class `Autopilot`:**

- `processFrame(Deck& deck, const FeatureSnapshot& snapshot)` returns `true` if any clip advanced (Autopilot.h:22, Autopilot.cpp:4).
- `setSmartRandomEnabled(bool)` — P23 (Autopilot.h:25).
- `setPerTypeConfig(const Composition::PerTypeAutopilotConfig*)` — P20 (Autopilot.h:28).

**Two trigger modes inside `processFrame`:**

1. **End-of-Video mode** (Autopilot.cpp:8-41): checked every frame. If `layer.autopilotEndOfVideo == true` and playhead ≥ `outPoint - 0.01`, increments `beatsPlayed` (reused as loop counter), fires `advanceClip` / `smartAdvanceClip` when `beatsPlayed >= autopilotLoops`.

2. **Beat-based mode** (Autopilot.cpp:44-107): runs on beat crossings (`snapshot.beatPhase < lastBeatPhase_ - 0.5f`). Processes pending beat-snap triggers first via `layer.processPendingTrigger(beatInBar, barCount)` (Autopilot.cpp:50-60). Increments `clip->beatsPlayed`. Picks per-type vs per-clip timing based on `perTypeConfig_->perTypeEnabled`.

**Duration map (`getBeatsForClip`)** (Autopilot.cpp:168-178):

| Duration | Beats |
|----------|-------|
| `Beat1` | 1 × loops |
| `Beat2` | 2 × loops |
| `Beat4` | 4 × loops |
| `Beat8` | 8 × loops |
| `Beat16` | 16 × loops |
| `Beat32` | 32 × loops |
| `Custom` | `customBeats × loops` |

`loops = max(1, layer.autopilotLoops)` (Autopilot.cpp:166).

**Per-type beats (`getPerTypeBeats`)** (Autopilot.cpp:109-126):

- `Opaque` → `perTypeConfig_->opaqueCycleBeats` (default 16)
- `Transparent`, `ThreeD`, `Mask` → `transparentCycleBeats` (default 8)
- `FXOnly` → `effectCycleBeats` (default 4)
- default → `4`

**Per-type action (`getPerTypeAction`)** (Autopilot.cpp:128-152): returns `PlayRandom` if `globalRandomize` or the layer's type-specific `transparentRandomize`/`effectRandomize` flag is on; otherwise `PlayNext`.

**`advanceClip` action handling** (Autopilot.cpp:189-285):

- `PlayNext` — wraps with modulo search.
- `PlayPrevious` — wraps with modulo search.
- `PlayRandom` — picks uniform-random from non-current clip columns.
- `PlayFirst` — first non-null column.
- `PlayLast` — last non-null column.
- `PlaySpecific` — uses `clip->autopilotSpecificCol`.
- `DoNothing` / `LayerDetermined` — no-op.

**`smartAdvanceClip` (P23)** (Autopilot.cpp:287-368): scores candidates by distance to a target intensity derived from `snapshot.structuralState`:

- `structuralState == 2` (Drop) → targetIntensity `0.85`.
- `structuralState == 1` (Buildup) → targetIntensity `0.65`.
- `structuralState == 3` (Breakdown) → targetIntensity `0.15`.
- Otherwise → `targetIntensity = energyState / 2.0f`.

Adds up to `+0.2` jitter per candidate, picks the highest score. Falls back to uniform random when ≤2 candidates.

**Private state:**

- `lastBeatPhase_ = 0.0f`, `lastOnBeat_ = false`, `perTypeConfig_ = nullptr`, `smartRandomEnabled_ = false`, `lastEnergyState_ = 1` (Autopilot.h:55-63).

---

## Undo / Redo

Source: `src/core/Command.h`, `src/core/UndoManager.h`, `src/core/UndoManager.cpp`.

**`Command` base class** (Command.h:7-25):

- `virtual void execute() = 0` (Command.h:12).
- `virtual void undo() = 0` (Command.h:15).
- `virtual std::string description() const = 0` (Command.h:19).
- `virtual bool canMergeWith(const Command&) const { return false; }` (Command.h:23).
- `virtual void mergeWith(const Command&) {}` (Command.h:24).

**`UndoManager`** (UndoManager.h, UndoManager.cpp):

- `perform(std::unique_ptr<Command>)` — executes, attempts to merge with previous via `canMergeWith`/`mergeWith` (UndoManager.cpp:3-36). If merged, the new command is discarded.
- `undo()` returns bool, calls `history_[currentIndex_-1]->undo()`, decrements index (UndoManager.cpp:38-48).
- `redo()` returns bool, calls `history_[currentIndex_]->execute()`, increments index (UndoManager.cpp:50-60).
- `canUndo()` / `canRedo()` / `undoDescription()` / `redoDescription()` / `clear()` (UndoManager.h:25-31, UndoManager.cpp:62-89).
- `historySize()` / `undoIndex()` for testing (UndoManager.h:34-35).
- `onHistoryChanged` callback (UndoManager.h:38) for menu update.
- `kMaxHistory = 500` (UndoManager.h:44); on overflow, oldest entries erased (UndoManager.cpp:26-33).
- Thread safety: "all methods must be called from the message thread only" (UndoManager.h:9).

**Concrete Command subclasses**: NONE in the codebase. Grep for `: public Command` and `make_unique<...Command` in `/src` returns no results — the infrastructure is defined but no concrete commands are currently registered. Only `UndoManager::perform(std::unique_ptr<Command>)` is referenced (UndoManager.h:16, UndoManager.cpp:3).

---

## Section 20 partial: Cross-references

- `Layer::layerEffects` and `Composition::globalEffects` reuse `Clip::EffectSlot` (Layer.h:140, Composition.h:21).
- `Layer::defaultAutopilotAction` reuses `Clip::AutopilotAction`; `Layer::defaultAutopilotDuration` reuses `Clip::AutopilotDuration` (Layer.h:143-144).
- `Deck::triggerColumn` iterates all layers and honors `Layer::ignoreColumnTrigger` (Deck.h:108-116).
- `Autopilot` takes `Deck&` + `FeatureSnapshot`, mutates `Layer::pendingTriggerColumn`, `clip->beatsPlayed`, then calls `Layer::triggerClip` (Autopilot.cpp).
- Serialization: `Composition::toVar` → `Deck::toVar` → `Layer::toVar` → `Clip::toVar` (all `.cpp` files), `juce::JSON` text via `saveToFile`/`loadFromFile` (Composition.h:247-262).
- Beat-snap: `Clip::beatSnapMode` is read in `Layer::triggerClip` (Layer.h:206) and `Layer::processPendingTrigger` (Layer.h:260-282); fed beat info from `Autopilot::processFrame` which reads `snapshot.beatInBar`, `snapshot.barCount` (Autopilot.cpp:54-59).
- Persistent layers (Layer.h:52) and `ignoreColumnTrigger` (Layer.h:51) — persistence logic is NOT in this slice (documented by CLAUDE.md as in `CompositorEngine`).
- `Composition::genreDeckAssignment[8]` — crossreferenced with the `GenreDetector` / `genre` snapshot field (out of this slice).
- `replaceContent` preserves lock flag itself; `fromVar` conditionally reads `contentLocked` (Clip.cpp:178-179).

---

## Summary

**File output**: `/Users/boriskarpman/Documents/RealTimeAudio/.feature_audit/slice_03_data_model.md`.

Counts:
- **Deck fields**: 4 public + 2 static constexpr + 1 private. Methods: 12.
- **Layer fields**: 40+ public (name, id, type, 12 booleans, 4 chroma/key params, 5 3D params, 5 video/transition fields, 6 transform fields, feedback struct, layerEffects, 5 autopilot defaults, clips vector, 4 runtime fields).
- **Clip fields**: ~45 public + 9-field EffectSlot + 4-field SourceParam + 4-field PresetEntry + 8-entry cuepoint array. Transport, video props, transform, playlist, content lock, runtime state all here.
- **Composition fields**: ~30 public + 11-field PerTypeAutopilotConfig + 8-entry genreDeckAssignment + 8-entry genrePresetNames.
- **Blend/MixMode enum values**: **55** (25 standard blend + 30 transition — unified list shared by `blendMode`, `transitionMode`, `transitionBlendMode`).
- **Other enum totals**: MediaType 6, TransportMode 2, LoopMode 3, BeatSnapMode 5, AutopilotAction 8, AutopilotDuration 8, BlendOverride 2, AlphaType 2, PlaylistCycleMode 5, PlaylistTrigger 6, Layer::Type 5, KeyingMode 13, AutoSizeMode 5, CrossfaderBlendMode 3, CrossfaderBehaviour 2, CrossfaderCurve 3, QuantizeMode 3, AutopilotDirection 4, AutopilotDurationMode 3. **Grand total: ~143 enum values** across 20 distinct enums.

**Surprises / gaps**:

1. **No concrete Command subclasses exist.** The `Command`/`UndoManager` scaffolding is built but nothing in the repo currently creates or registers undo-able commands — grep for `: public Command` and `make_unique<...Command` found zero matches. Anything claiming "full undo from the start" is aspirational.
2. **`FeedbackConfig` is not serialized** by `Layer::toVar`/`fromVar` (Layer.cpp:3-140) — feedback settings do not survive save/load.
3. **Composition serialization omits many fields**: `compOpacity`, `masterSpeed`, all crossfader fields, all comp transform fields, per-type autopilot config, all genre-aware automation fields. Only `name`, `activeDeckIndex`, `masterOpacity`, `globalTransitionSpeed`, `bpmMultiplier`, `quantizeMode`, output size/display, decks, and global effects survive save/load.
4. **Clip cuepoints have no public set/trigger/clear API inside the model** — cuepoints are a raw `float[8]` + count, managed externally.
5. **Clip `replaceContent` does NOT preserve cuepoints** and does NOT preserve MilkDrop playlist position — it preserves everything else listed.
6. **`Layer::MixMode` unifies both blend modes and transitions** as a single 55-value enum used by three fields (`blendMode`, `transitionMode`, `transitionBlendMode`). The `Dissolve` value bridges both categories.
7. **Legacy upgrade**: `Clip::fromVar` promotes a legacy `beatSnap == true && beatSnapMode == Off` to `beatSnapMode = Beat` (Clip.cpp:141-142).
8. **`Clip::startOffset`** is explicitly marked legacy in the header (Clip.h:65) and is used as a fallback for `inPoint` when loading older files (Clip.cpp:133).
