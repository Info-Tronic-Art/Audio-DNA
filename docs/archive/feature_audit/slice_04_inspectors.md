# Slice 04 — Inspectors (Feature Audit)

Files covered (all absolute paths):
- `/Users/boriskarpman/Documents/RealTimeAudio/src/ui/InspectorPanel.h`
- `/Users/boriskarpman/Documents/RealTimeAudio/src/ui/InspectorPanel.cpp`
- `/Users/boriskarpman/Documents/RealTimeAudio/src/ui/ClipInspector.h`
- `/Users/boriskarpman/Documents/RealTimeAudio/src/ui/ClipInspector.cpp`
- `/Users/boriskarpman/Documents/RealTimeAudio/src/ui/LayerInspector.h`
- `/Users/boriskarpman/Documents/RealTimeAudio/src/ui/LayerInspector.cpp`
- `/Users/boriskarpman/Documents/RealTimeAudio/src/ui/CompositionInspector.h`
- `/Users/boriskarpman/Documents/RealTimeAudio/src/ui/CompositionInspector.cpp`
- `/Users/boriskarpman/Documents/RealTimeAudio/src/ui/SignalInspector.h`
- `/Users/boriskarpman/Documents/RealTimeAudio/src/ui/SignalInspector.cpp`
- `/Users/boriskarpman/Documents/RealTimeAudio/src/ui/MappingEditor.h`
- `/Users/boriskarpman/Documents/RealTimeAudio/src/ui/MappingEditor.cpp`
- `/Users/boriskarpman/Documents/RealTimeAudio/src/ui/MacroPanel.h`
- `/Users/boriskarpman/Documents/RealTimeAudio/src/ui/MacroPanel.cpp`
- `/Users/boriskarpman/Documents/RealTimeAudio/src/ui/PresetManager.h`
- `/Users/boriskarpman/Documents/RealTimeAudio/src/ui/PresetManager.cpp`
- `/Users/boriskarpman/Documents/RealTimeAudio/src/ui/UniversalParamControl.h`
- `/Users/boriskarpman/Documents/RealTimeAudio/src/ui/UniversalParamControl.cpp`

---

## Inspector Panel Container

`InspectorPanel` (`src/ui/InspectorPanel.h:23-90`, `src/ui/InspectorPanel.cpp:1-216`)

Top-level component — 4 scrollable viewports, 4 tab buttons, 1 pin button.

- Tab buttons (at `InspectorPanel.h:62-65`, wired `InspectorPanel.cpp:15-18`):
  - `clipTabBtn_{"Clip"}` — sets active tab Clip
  - `layerTabBtn_{"Layer"}` — sets active tab Layer
  - `compTabBtn_{"Composition"}` — sets active tab Composition
  - `signalTabBtn_{"Signal"}` — sets active tab Signal
- Pin button: `pinBtn_{"Pin"}` (`InspectorPanel.h:81`, `InspectorPanel.cpp:21-26`)
  - Toggles `pinned_`. Text alternates "Pin"/"Unpin" (`InspectorPanel.cpp:212`).
  - Color change: dark green when pinned (`InspectorPanel.cpp:213-214`).
  - When pinned, auto-switch on `inspectClip()`/`inspectLayer()`/`inspectSignal()` is disabled (`InspectorPanel.cpp:136, 142, 148`).
- Viewports: `clipViewport_`, `layerViewport_`, `compViewport_`, `signalViewport_` (`InspectorPanel.h:68-71`) each wrap their inspector for scrolling (`InspectorPanel.cpp:29-43`).
- Tab auto-switch triggers: `inspectClip()` → Clip tab, `inspectLayer()` → Layer tab, `inspectSignal()` → Signal tab, `showCompositionTab()` → Composition tab (`InspectorPanel.h:38-41`).
- Tab bar painted with bright cyan accent line under active tab (`InspectorPanel.cpp:63-74`).

---

## Section 13: Inspector Tabs and Sections

### 13.1 Clip Inspector — `ClipInspector`

`src/ui/ClipInspector.h:25-161`, `src/ui/ClipInspector.cpp:1-1243`.

`ClipInspector` derives from `juce::Component` and `juce::DragAndDropTarget`.

Section order (top → bottom):

1. **Name Bar** (painted, not a component)
   - `ClipInspector.cpp:436-442` — paints `clip_->name` in bar height `kNameBarHeight = 28` (`ClipInspector.h:156`).

2. **Dashboard** — `MacroPanel macroPanel_` (`ClipInspector.h:58`)
   - 8 link knobs — see 13.6 Macro Panel.

3. **Transport** — section header painted with mode selector in the header row
   - Transport mode ComboBox `transportModeSelector_` (`ClipInspector.h:61`, `ClipInspector.cpp:8-18`, laid out at `:555`):
     - Options: `"Timeline"` (ID 1), `"BPM Sync"` (ID 2)
     - Default ID 1 (Timeline)
     - onChange: sets `clip_->transportMode` to `BPMSync` or `Timeline`
   - Playhead position readout (painted number at `ClipInspector.cpp:468-475`): `clip_->playheadPosition` as 2-decimal float
   - **Timeline bar** (interactive, painted at `:951-1048`):
     - `kTimelineHeight = 36` (`ClipInspector.h:157`)
     - Dark regions outside in/out range, active region lighter gray
     - Beat division lines (BPM Sync mode, `clip_->beatDivision` >= 2)
     - In-point bracket tab (cyan, right-pointing)
     - Out-point bracket tab (cyan, left-pointing)
     - Playhead downward triangle + vertical line (cyan)
     - Mouse interaction (`:1063-1124`):
       - Click near in-point (8px hit zone) → drag in-point (preserves `outPoint - inPoint >= 0.01`)
       - Click near out-point → drag out-point
       - Click anywhere else → scrub playhead + fire `onCuepointJump`
   - **Transport buttons row** (`ClipInspector.cpp:30-52`, laid out `:563-577`):
     - `playBackBtn_` ("◀" U+25C0) — sets `clip_->reverse = true; playing = true`
     - `pauseBtn_` ("⏸" U+23F8) — sets `clip_->playing = false`
     - `playBtn_` ("▶" U+25B6) — sets `clip_->reverse = false; playing = true`; highlighted cyan when playing forward
     - `loopDropdown_` ComboBox (`ClipInspector.h:71`, `:55-66`):
       - Options: `"Loop"` (ID 1), `"Ping Pong"` (ID 2), `"One Shot"` (ID 3)
       - Default ID 1 (Loop)
       - Writes `clip_->loopMode`
     - `triggerDropdown_` ComboBox (`ClipInspector.h:72`, `:68-72`):
       - Options: `"Restart"` (ID 1), `"Continue"` (ID 2), `"Relative"` (ID 3)
       - Default ID 1 (Restart)
       - No onChange handler wired in this file
   - **Speed row** (`ClipInspector.cpp:75-86`, `:581-591`):
     - "Speed" label (painted at `:488`)
     - `speedSlider_` ResettableSlider range 0.0..4.0 step 0.01, default 1.0, writes `clip_->speed`; no text box editable — TextBoxLeft 30×20
     - `halfSpeedBtn_` ("÷2") — multiplies speed × 0.5, min 0.01
     - `doubleSpeedBtn_` ("×2") — multiplies speed × 2.0, max 4.0
     - `reverseBtn_` ("Reverse") — toggles `clip_->reverse`; background yellow-ish `0xff4a4a2a` when on
   - **Duration/Beats row** (`ClipInspector.cpp:89-102`, `:594-603`):
     - Label painted as "Duration" (Timeline mode) or "Beats" (BPM Sync mode) (`:492-493`)
     - `durationSlider_` ResettableSlider range 0.1..300.0 step 0.1, default 8.0
     - `durHalfBtn_` ("/2")
     - `durDoubleBtn_` ("×2")
   - **Conditional extra rows (only for playable clips)** (`:607-668`):
     - **BPM Sync mode**:
       - "Beats/ Cycle" label + `beatDivisionSelector_` ComboBox (`ClipInspector.h:95`, `:210-233`):
         - Options: `"1/4 Beat"` (ID 1 → beatDivision 0.25), `"1/2 Beat"` (ID 2 → 0.5), `"1 Beat"` (ID 3 → 1), `"2 Beats"` (ID 4 → 2), `"4 Beats (1 Bar)"` (ID 5 → 4), `"8 Beats (2 Bars)"` (ID 6 → 8), `"16 Beats (4 Bars)"` (ID 7 → 16)
         - Default ID 5 (4 beats)
       - "Content Beats" label + `videoBeatsSlider_` ResettableSlider (`ClipInspector.h:99`, `:242-274`):
         - Range 1.0..64.0 step 1.0, default 4.0, skew factor 0.5
         - Snaps to {1, 2, 4, 8, 16, 32, 64} on change
         - Writes `clip_->videoBeats`
         - Editable text box (true)
       - Signal-connect triangle painted next to rows (`:498-507`) — cyan when BPM sync, grey otherwise
     - **ImageSequence (Timeline mode)**:
       - "Images/ Sec" label + `sequenceFpsSlider_` ResettableSlider (`ClipInspector.h:91`, `:183-200`):
         - Range 0.0..6.0 step 0.1, default 2.5
         - Writes `clip_->sequenceFps`

4. **Cuepoints** — section header painted
   - 2 rows × 8 columns; `kNumCuepoints = 8` (`ClipInspector.h:79`)
   - Row 1 — trigger buttons (numbered "1".."8") `cuepointBtns_[0..7]` (`ClipInspector.h:80`, `:277-309`):
     - Click → jump to cuepoint position, `clip_->playheadPosition = clip_->cuepoints[i]`, fires `onCuepointJump`
     - Ctrl/Cmd-click → clears the cuepoint (shifts remaining cues down, decrements `numCuepoints`)
     - Tooltip shows position value (3-decimal) + "Ctrl+click to clear" when set; "Click to set cuepoint at current position" when empty (`:1176-1183`)
     - Background color: green-ish `0xff3a4a3a` when cue is set; `kSurface` otherwise
   - Row 2 — set buttons ("Set") `cuepointSetBtns_[0..7]` (`ClipInspector.h:81`, `:311-330`):
     - Click → saves `clip_->playheadPosition` to `clip_->cuepoints[i]`; increments `numCuepoints`
     - Fires `onCuepointSet` callback

5. **Autopilot** — section header painted
   - `autopilotActionSelector_` ComboBox (`ClipInspector.h:84`, `:127-142`):
     - Options: `"Layer Determined"` (ID 1), `"Do Nothing"` (ID 2), `"Play Next"` (ID 3), `"Play Previous"` (ID 4), `"Play Random"` (ID 5), `"Play First"` (ID 6), `"Play Last"` (ID 7), `"Play Specific"` (ID 8)
     - Default ID 1
     - Writes `clip_->autopilotAction` (`Clip::AutopilotAction` enum)
   - `autopilotDurationSelector_` ComboBox (`ClipInspector.h:85`, `:144-158`):
     - Options: `"Layer Determined"` (ID 1), `"1 Beat"` (ID 2), `"2 Beats"` (ID 3), `"4 Beats"` (ID 4), `"8 Beats"` (ID 5), `"16 Beats"` (ID 6), `"32 Beats"` (ID 7)
     - Default ID 1
     - Writes `clip_->autopilotDuration` (`Clip::AutopilotDuration` enum)
   - `beatSnapSelector_` ComboBox (`ClipInspector.h:88`, `:161-174`):
     - Options: `"Snap Off"` (ID 1), `"Beat"` (ID 2), `"Bar"` (ID 3), `"2 Bar"` (ID 4), `"4 Bar"` (ID 5)
     - Writes `clip_->beatSnapMode` and sets `clip_->beatSnap` true when > 1
     - Tooltip: "Quantize clip trigger to next beat/bar boundary"

6. **Source** — (conditional: shown only if `clip_->mediaType == MediaType::Source` and `sourceParams` non-empty) `ClipInspector.cpp:519-525, 698-707`
   - Section header "Source"
   - N `UniversalParamControl` widgets built dynamically from `clip_->sourceParams` by `buildSourceParamControls()` (`ClipInspector.h:103-104`, `:764-792`):
     - Each uses `sp.name` / `sp.value` / `sp.defaultValue` from `Clip::SourceParam`
     - Full signal-connect behaviour via UniversalParamControl (see 13.7)

7. **Video** — section header painted `ClipInspector.cpp:527`
   - `clipOpacityControl_` UniversalParamControl "Opacity" default 1.0 → `clip_->clipOpacity` (`ClipInspector.h:107`, `:333-338`)
   - `clipWidthSlider_` ResettableSlider IncDecButtons, range 1..7680 step 1, default 1920 → `clip_->clipWidth` (`ClipInspector.h:108`, `:348-350`)
   - `clipHeightSlider_` ResettableSlider IncDecButtons, range 1..4320 step 1, default 1080 → `clip_->clipHeight` (`:352-354`)
   - `clipBlendModeSelector_` ComboBox (`ClipInspector.h:110`, `:356-362`):
     - Options: `"Layer Determined"` (ID 1), `"Normal"` (ID 2), `"Additive"` (ID 3), `"Screen"` (ID 4), `"Multiply"` (ID 5)
     - Default ID 1
   - `clipAlphaTypeSelector_` ComboBox (`ClipInspector.h:111`, `:364-371`):
     - Options: `"Premultiplied"` (ID 1), `"Straight"` (ID 2)
     - Default ID 1
     - Writes `clip_->alphaType` (`Clip::AlphaType`)
   - **RGBA channel toggles** (`ClipInspector.h:113-116`, `:374-388`):
     - `channelRBtn_{"R"}` ToggleButton → `clip_->channelR` (default true)
     - `channelGBtn_{"G"}` ToggleButton → `clip_->channelG` (default true)
     - `channelBBtn_{"B"}` ToggleButton → `clip_->channelB` (default true)
     - `channelABtn_{"A"}` ToggleButton → `clip_->channelA` (default true)

8. **Transform** — section header painted with "P." button, teal-tinted background (`ClipInspector.cpp:530, 914-947`)
   - `posXControl_` UniversalParamControl "Position X" default 0.5 → `clip_->positionX = (v - 0.5) * 3840` (`ClipInspector.h:119`, `:398, 404`)
   - `posYControl_` UniversalParamControl "Position Y" default 0.5 → `clip_->positionY = (v - 0.5) * 2160` (`:399, 405`)
   - `scaleControl_` UniversalParamControl "Scale" default 0.5 → `clip_->scale = pow(2, (v-0.5)*2)` (`:400, 406`)
   - `rotationControl_` UniversalParamControl "Rotation" default 0.5 → `clip_->rotation = (v - 0.5) * 720` (`:401, 407`)
   - `anchorControl_` UniversalParamControl "Anchor" default 0.5 → `clip_->anchorX = (v - 0.5) * 3840` (`:402, 408`)

9. **Effects** — section header
   - `effectStackView_` EffectStackView (`ClipInspector.h:126`, `:411`)
   - Operates on `clip_->effects` via `effectStackView_.setEffects(&clip->effects)` (`:751`)

**DragAndDropTarget** behaviour (`ClipInspector.cpp:1220-1243`):
- `isInterestedInDragSource`: only when `clip_ != nullptr` AND `details.description` starts with `"fx:"`
- On drag enter/exit: toggles `fxDropHighlight_` → cyan overlay painted in `paint()` at `:419-425`
- On drop: forwards to `effectStackView_.itemDropped(details)`

**Callbacks**:
- `onSourceParamsChanged(Clip*)` — fired when any source param value changes (`ClipInspector.h:44`)
- `onCuepointJump(Clip*, double)` — fired on cue trigger + timeline scrub (`ClipInspector.h:47`)
- `onCuepointSet(Clip*, int)` — fired when user sets a new cue (`ClipInspector.h:50`)

---

### 13.2 Layer Inspector — `LayerInspector`

`src/ui/LayerInspector.h:28-140`, `src/ui/LayerInspector.cpp:1-1000`.

`LayerInspector` derives from `juce::Component` and `juce::DragAndDropTarget`.

Section order:

1. **Name Bar** — editable `juce::Label nameLabel_` (`LayerInspector.h:56`, `:6-26`)
   - Font: bold 12pt
   - Editable on click (`setEditable(false, true, false)`) + `mouseDown` triggers `showEditor()` when click hits name bar (`:726-735`)
   - Tooltip: "Click to rename layer"
   - Fires `onLayerNameChanged` callback (`LayerInspector.h:49`)

2. **Dashboard** — `MacroPanel macroPanel_` (`LayerInspector.h:58`)

3. **Autopilot** — section header
   - **Direction buttons row** (`LayerInspector.h:61-64`, `:39-71`):
     - `apRewindBtn_` ("◀◀" U+25C0×2) — enables autopilot + sets `defaultAutopilotAction = PlayPrevious`; tooltip "Autopilot: play previous clip on beat"
     - `apOffBtn_{"OFF"}` — disables `autopilotEnabled`; tooltip "Autopilot: disabled"
     - `apForwardBtn_` ("▶▶" U+25B6×2) — enables + `PlayNext`; tooltip "Autopilot: play next clip on beat"
     - `apRandomBtn_` ("🔀" U+1F500) — enables + `PlayRandom`; tooltip "Autopilot: play random clip on beat"
   - `apTriggerModeSelector_` ComboBox (`LayerInspector.h:65`, `:74-102`):
     - Options: `"End of Video"` (ID 1), `"On Beat"` (ID 2)
     - Default ID 2 (On Beat)
     - Tooltip: "When to advance to the next clip"
     - Writes `layer_->autopilotEndOfVideo` (true for ID 1, false for ID 2)
   - `apBeatCountSelector_` ComboBox — visible only in "On Beat" mode (`LayerInspector.h:66`, `:105-123`):
     - Options: `"1 Beat"` (ID 1), `"2 Beats"` (ID 2), `"4 Beats"` (ID 3), `"8 Beats"` (ID 4), `"16 Beats"` (ID 5), `"32 Beats"` (ID 6)
     - Default ID 3 (4 Beats)
     - Tooltip: "Number of beats before advancing"
     - Maps to `Clip::AutopilotDuration::Beat1..Beat32`
   - "Loops:" label (`apLoopsLabel_`, `LayerInspector.h:67`)
   - `apLoopsSlider_` ResettableSlider IncDecButtons range 1..99 step 1, default 1 (`LayerInspector.h:68`, `:131-142`):
     - Tooltip: "Number of loops before advancing to next clip"
     - Writes `layer_->autopilotLoops`

4. **Layer (Master)** — section header
   - `masterControl_` UniversalParamControl "Master" default 1.0 → `layer_->opacity` (`LayerInspector.h:71`, `:145-152`)
   - `persistentToggle_{"Persistent"}` ToggleButton (`LayerInspector.h:72`, `:154-161`):
     - Tooltip: "Keep this layer rendering when switching to another deck"
     - Writes `layer_->persistent`
   - `ignoreColumnToggle_{"Ignore Column Trigger"}` ToggleButton (`LayerInspector.h:73`, `:164-170`):
     - Tooltip: "This layer ignores column trigger buttons"
     - Writes `layer_->ignoreColumnTrigger`

5. **Video** — section header
   - `blendModeSelector_` ComboBox populated by `populateBlendModes()` (`LayerInspector.h:76`, `:946-961`):
     - 25 options (IDs 1-25): `"Normal"`, `"Additive"`, `"Screen"`, `"Multiply"`, `"Overlay"`, `"Soft Light"`, `"Hard Light"`, `"Vivid Light"`, `"Linear Light"`, `"Pin Light"`, `"Hard Mix"`, `"Darken"`, `"Lighten"`, `"Darker Color"`, `"Lighter Color"`, `"Color Dodge"`, `"Color Burn"`, `"Difference"`, `"Exclusion"`, `"Subtract"`, `"Hue"`, `"Saturation"`, `"Color"`, `"Luminosity"`, `"Dissolve"`
     - Default ID 2 (Additive)
     - Writes `layer_->blendMode` (`Layer::MixMode`)
   - `opacityControl_` UniversalParamControl "Opacity" default 1.0 → `layer_->opacity` (`LayerInspector.h:77`, `:182-189`)
   - `widthSlider_` ResettableSlider IncDecButtons range 1..7680 step 1, default 1920 → `layer_->layerWidth` (`LayerInspector.h:78`, `:200-204`)
   - `heightSlider_` ResettableSlider IncDecButtons range 1..4320 step 1, default 1080 → `layer_->layerHeight` (`LayerInspector.h:79`, `:206-210`)
   - `autoSizeSelector_` ComboBox (`LayerInspector.h:80`, `:213-222`):
     - Options: `"Off"` (ID 1), `"Fill"` (ID 2), `"Fit"` (ID 3), `"Stretch"` (ID 4), `"Original"` (ID 5)
     - Default ID 1
     - Writes `layer_->autoSize` (`Layer::AutoSizeMode`)

6. **Transition** — section header
   - `transitionBlendSelector_` ComboBox with extensive list organized by section headings (`LayerInspector.h:83`, `:225-317`):
     - Section "Compositing": Alpha, Add, Screen, Multiply, Overlay
     - Section "Light": Soft Light, Hard Light, Vivid Light, Linear Light, Pin Light, Hard Mix
     - Section "Compare": Darken, Lighten, Darker Color, Lighter Color
     - Section "Dodge / Burn": Color Dodge, Color Burn
     - Section "Inversion": Difference, Exclusion, Subtract
     - Section "Component": Hue, Saturation, Color, Luminosity
     - Section "Special": Dissolve, Cut
     - Section "Wipe": Wipe Left, Wipe Right, Wipe Up, Wipe Down, Wipe Ellipse, Wipe Diagonal
     - Section "Push": Push Left, Push Right, Push Up, Push Down
     - Section "Zoom": Zoom In, Zoom Out
     - Section "3D": Rotate X, Rotate Y, Spin, Cube, Flip, Fold
     - Section "Color Fade": To Black, To White
     - Section "Creative": Pixelate, Blur, Noise, RGB Split, Glitch Blocks, Strobe, Slide, Stretch, Displace
     - Default: Dissolve (ID derived from `1 + Layer::MixMode::Dissolve`)
     - Writes `layer_->transitionMode`
   - `transitionDurationSlider_` ResettableSlider LinearHorizontal range 0.0..5.0 step 0.01, default 0.0 → `layer_->transitionSpeed` (`LayerInspector.h:84`, `:330-334`)

7. **Keying** (conditional: only if `layer_->type == Layer::Type::Transparent`)
   - `keyingModeSelector_` ComboBox populated by `populateKeyingModes()` (`LayerInspector.h:87`, `:963-975`):
     - Options (IDs 1-13): `"Alpha"`, `"Luma Key"`, `"Inverted Luma Key"`, `"Luma is Alpha"`, `"Inverted Luma is Alpha"`, `"Chroma Key"`, `"Max RGB"`, `"Saturation Key"`, `"Edge Detection"`, `"Threshold Mask"`, `"Channel Red"`, `"Channel Green"`, `"Channel Blue"`
     - Default ID 1
     - Writes `layer_->keyingMode` (`Layer::KeyingMode`)
   - `keyThresholdSlider_` ResettableSlider range 0..1 step 0.01, default 0.1 → `layer_->keyThreshold` (`LayerInspector.h:88`, `:345-349`)
   - `keySoftnessSlider_` ResettableSlider range 0..1 step 0.01, default 0.1 → `layer_->keySoftness` (`LayerInspector.h:89`, `:351-355`)

8. **Dry / Wet** (conditional: only if `layer_->type == Layer::Type::FXOnly`)
   - `dryWetSlider_` ResettableSlider range 0..1 step 0.01, default 1.0 → `layer_->dryWetMix` (`LayerInspector.h:92`, `:358-362`)

9. **3D Controls** (conditional: only if `layer_->type == Layer::Type::ThreeD`)
   - `rotXSlider_` range -180..180 step 0.01, default 0 → `layer_->rotationX` (`LayerInspector.h:95`, `:365-367`)
   - `rotYSlider_` range -180..180, default 0 → `layer_->rotationY` (`:369-371`)
   - `rotZSlider_` range -180..180, default 0 → `layer_->rotationZ` (`:373-375`)
   - `rotSpeedSlider_` range 0..10, default 0 → `layer_->rotationSpeed` (`:377-379`)
   - `scale3DSlider_` range 0.1..5.0, default 1.0 → `layer_->scale3D` (`:381-383`)

10. **Transform** — section header with "P." marker, teal-tinted background
    - Identical to ClipInspector's Transform section
    - `posXControl_` UniversalParamControl "Position X" default 0.5 → `layer_->positionX = (v - 0.5) * 3840` (`LayerInspector.h:100`, `:393, 399`)
    - `posYControl_` "Position Y" → `layer_->positionY = (v - 0.5) * 2160`
    - `scaleControl_` "Scale" → `layer_->layerScale = pow(2, (v-0.5)*2)`
    - `rotationControl_` "Rotation" → `layer_->layerRotation = (v - 0.5) * 720`
    - `anchorControl_` "Anchor" → `layer_->layerAnchorX = (v - 0.5) * 3840`

11. **Feedback** — section header
    - `feedbackEnableBtn_{"Enable"}` ToggleButton (`LayerInspector.h:107`, `:406-414`):
      - Tooltip: "Enable feedback loop (Larsen effect)"
      - Writes `layer_->feedback.enabled`
    - `feedbackPresetSelector_` ComboBox (`LayerInspector.h:108`, `:416-439`):
      - First item: `"Custom"` (ID 1)
      - Additional items from `FeedbackProcessor::getPresets()` starting at ID 2
      - Tooltip: "Feedback preset"
      - On change > ID 1: applies preset config to `layer_->feedback`, enables feedback, syncs
    - `feedbackAmountSlider_` range 0..1, default 0.5 → `layer_->feedback.amount`, tooltip "Feedback amount (0 = none, 1 = full)" (`LayerInspector.h:109`, `:455-458`)
    - `feedbackScaleXSlider_` range 0.5..1.5, default 0.98 → `.scaleX`, tooltip "Feedback horizontal scale" (`LayerInspector.h:110`, `:460-463`)
    - `feedbackScaleYSlider_` range 0.5..1.5, default 0.98 → `.scaleY`, tooltip "Feedback vertical scale" (`LayerInspector.h:111`, `:465-468`)
    - `feedbackRotationSlider_` range -15..15, default 0.0 → `.rotation`, tooltip "Feedback rotation per frame (degrees)" (`LayerInspector.h:112`, `:470-473`)
    - `feedbackOffsetXSlider_` range -0.1..0.1, default 0.0 → `.offsetX`, tooltip "Feedback horizontal offset per frame" (`LayerInspector.h:113`, `:475-478`)
    - `feedbackOffsetYSlider_` range -0.1..0.1, default 0.0 → `.offsetY`, tooltip "Feedback vertical offset per frame" (`LayerInspector.h:114`, `:480-483`)
    - `feedbackLumaKeySlider_` range 0..1, default 0.0 → `.lumaKey`, tooltip "Luma key: fade dark areas from feedback" (`LayerInspector.h:115`, `:485-488`)
    - Any manual slider change clears `layer_->feedback.presetName` and resets preset selector to "Custom"

12. **Layer Effects** — section header
    - `effectStackView_` EffectStackView (`LayerInspector.h:118`, `:491`)
    - Bound to `&layer_->layerEffects`

**DragAndDropTarget** (`LayerInspector.cpp:977-1000`): identical pattern to ClipInspector for "fx:" sources, forwards to `effectStackView_.itemDropped`.

---

### 13.3 Composition Inspector — `CompositionInspector`

`src/ui/CompositionInspector.h:23-96`, `src/ui/CompositionInspector.cpp:1-524`.

Section order:

1. **Name + Resolution Bar** (painted, not a component, `CompositionInspector.cpp:209-219`)
   - Shows `composition_->name + " (" + outputWidth + " x " + outputHeight + ")"`.

2. **Dashboard** — `MacroPanel macroPanel_` (`CompositionInspector.h:46`)

3. **Autopilot** — section header
   - **Direction buttons** (`CompositionInspector.h:49-52`, `:8-35`):
     - `apRewindBtn_` ("◀◀") → `Composition::AutopilotDirection::Rewind`
     - `apOffBtn_{"OFF"}` → `Off` (default, colored cyan)
     - `apForwardBtn_` ("▶▶") → `Forward`
     - `apRandomBtn_` ("🔀") → `Random`
   - `apDurationSelector_` ComboBox (`CompositionInspector.h:53`, `:38-47`):
     - Options: `"Longest Clip"` (ID 1), `"Clip Transport"` (ID 2), `"Custom"` (ID 3)
     - Default ID 1
     - Writes `composition_->autopilotDurationMode` (`Composition::AutopilotDurationMode`)
   - `apClipLoopsSlider_` ResettableSlider IncDecButtons range 1..99 step 1, default 1 → `composition_->autopilotClipLoops` (`CompositionInspector.h:54`, `:50-62`)
   - `apLoopToggle_{"Loop"}` ToggleButton → `composition_->autopilotLoop` (`CompositionInspector.h:55`, `:65-70`)
   - `apMasterLayerSelector_` ComboBox (`CompositionInspector.h:56`, `:73-81`):
     - Options: `"Off"` (ID 1), `"Layer 1"` (ID 2), `"Layer 2"` (ID 3), ..., `"Layer 8"` (ID 9)
     - Default ID 1 (Off = -1)
     - Writes `composition_->autopilotMasterLayer` (ID - 2)

4. **Per-Type Autopilot** — section header painted with labels "Opaque", "Transparent", "Effect" (`CompositionInspector.cpp:228-239`)
   - `perTypeEnabledToggle_{"Per-Type"}` ToggleButton (`CompositionInspector.h:59`, `:84-89`):
     - Writes `composition_->perTypeAutopilot.perTypeEnabled`
   - `opaqueCycleSlider_` ResettableSlider IncDecButtons range 1..64, default 16 → `perTypeAutopilot.opaqueCycleBeats` (`CompositionInspector.h:60`, `:103-106`)
   - `transparentCycleSlider_` IncDec 1..64, default 8 → `.transparentCycleBeats` (`CompositionInspector.h:61`, `:108-111`)
   - `effectCycleSlider_` IncDec 1..64, default 4 → `.effectCycleBeats` (`CompositionInspector.h:62`, `:113-116`)
   - `transparentRandomToggle_{"Randomize"}` ToggleButton, default on → `.transparentRandomize` (`CompositionInspector.h:63`, `:118-124`)
   - `effectRandomToggle_{"Randomize"}` ToggleButton, default on → `.effectRandomize` (`CompositionInspector.h:64`, `:126-132`)

5. **Composition** — section header
   - `masterControl_` UniversalParamControl "Master" default 1.0 → `composition_->masterOpacity` (`CompositionInspector.h:67`, `:135-141`)
   - `speedControl_` UniversalParamControl "Speed" default 1.0 → `composition_->masterSpeed = val * 4.0` (mapping [0,1] → [0,4]) (`CompositionInspector.h:68`, `:143-149`)

6. **Video** — section header
   - `opacityControl_` UniversalParamControl "Opacity" default 1.0 → `composition_->compOpacity` (`CompositionInspector.h:71`, `:152-158`)

7. **Transform** — section header with "P." marker, teal-tinted
   - `posXControl_` "Position X" default 0.5 → `compPositionX = (v - 0.5) * 3840` (`CompositionInspector.h:74`, `:167, 173`)
   - `posYControl_` "Position Y" → `compPositionY = (v - 0.5) * 2160`
   - `scaleControl_` "Scale" default 0.5 → `compScale = v * 2` (0..200%)
   - `rotationControl_` "Rotation" → `compRotation = (v - 0.5) * 720`
   - `anchorControl_` "Anchor" → `compAnchorX = (v - 0.5) * 3840`; `compAnchorY = 0.0`

8. **Global Effects** — section header
   - `effectStackView_` bound to `&comp->globalEffects` (`CompositionInspector.h:81`, `:380`)

9. **Output Settings** — section header
   - `resolutionSelector_` ComboBox (`CompositionInspector.h:84`, `:185-200`):
     - Options: `"1920x1080"` (ID 1), `"1280x720"` (ID 2), `"2560x1440"` (ID 3), `"3840x2160"` (ID 4)
     - Default ID 1
     - Writes `composition_->outputWidth` / `outputHeight`

Note: No `setParamName/setDefaultValue` calls on the UniversalParamControls in CompositionInspector for the Transform section — default values are set at `setParamValue` only (`:167-171`); `setDefaultValue` is NOT called in this inspector, so right-click reset won't restore 0.5 unless the UPC slider already has that as default.

---

### 13.4 Signal Inspector — `SignalInspector`

`src/ui/SignalInspector.h:15-69`, `src/ui/SignalInspector.cpp:1-422`.

Header (painted): signal name (bold 14pt) + type badge ("AUDIO", "OSC", or "ENV") in cyan-tinted rectangle (`SignalInspector.cpp:152-172`).

Controls shown depend on signal type (`SignalInspector.cpp:198-237`):

**Audio signal controls** (visible when `signal_->getType() == Signal::Type::Audio`):
- Section header: "THRESHOLD / GAIN / FALLOFF"
- `thresholdSlider_` ResettableSlider LinearHorizontal range 0..1 step 0.01, default 0.0 (`SignalInspector.h:33`, `:19`)
- `gainSlider_` range 0..4 step 0.01, default 1.0, suffix "x" (`SignalInspector.h:34`, `:20`)
- `falloffSlider_` range 0..1 step 0.01, default 0.1, suffix "s" (`SignalInspector.h:35`, `:21`)

**Oscillator controls** (`Signal::Type::Oscillator`):
- Section header: "OSCILLATOR SETTINGS"
- `waveShapeSelector_` ComboBox (`SignalInspector.h:38`, `:27-39`):
  - Options: `"Sine"` (ID 1), `"Saw Up"` (ID 2), `"Saw Down"` (ID 3), `"Triangle"` (ID 4), `"Square"` (ID 5)
  - Default ID 1
  - Writes `OscillatorSignal::setShape(WaveShape(id - 1))`
- `beatDurationSelector_` ComboBox (`SignalInspector.h:39`, `:41-56`):
  - Options: `"1/4 Beat"` (ID 1 → 0.25), `"1/2 Beat"` (ID 2 → 0.5), `"1 Beat"` (ID 3 → 1.0), `"2 Beats"` (ID 4 → 2.0), `"4 Beats"` (ID 5 → 4.0), `"8 Beats"` (ID 6 → 8.0)
  - Default ID 3 (1 Beat)
- `amplitudeSlider_` ResettableSlider range 0..1, default 1.0 → `OscillatorSignal::setAmplitude` (`SignalInspector.h:40`, `:58-64`)
- `phaseOffsetSlider_` ResettableSlider range 0..1, default 0.0 → `OscillatorSignal::setPhaseOffset` (`SignalInspector.h:41`, `:66-72`)

**Envelope controls** (`Signal::Type::Envelope`):
- Section header: "ENVELOPE SETTINGS"
- **Curve editor** (painted at `:365-422`):
  - `kCurveEditorHeight = 100` (`SignalInspector.h:66`)
  - Dark background + grid lines (4 vertical, 4 horizontal)
  - Curve path drawn from `EnvelopeSignal::getPoints()` (`EnvelopePoint` = position + value)
  - Control points drawn as cyan circles with dark centers
- `curveTypeSelector_` ComboBox (`SignalInspector.h:44`, `:75-85`):
  - Options: `"Linear"` (ID 1), `"Exponential"` (ID 2), `"S-Curve"` (ID 3)
  - Default ID 1
  - Writes `EnvelopeSignal::setCurveType`
- `envBeatDurationSelector_` ComboBox (`SignalInspector.h:45`, `:87-101`):
  - Options: `"1 Beat"` (ID 1 → 1.0), `"2 Beats"` (ID 2 → 2.0), `"4 Beats"` (ID 3 → 4.0), `"8 Beats"` (ID 4 → 8.0), `"16 Beats"` (ID 5 → 16.0)
  - Default ID 3 (4 Beats)
- `envAmplitudeSlider_` ResettableSlider range 0..1, default 1.0 → `EnvelopeSignal::setAmplitude` (`SignalInspector.h:46`, `:103-109`)
- `envPhaseSlider_` ResettableSlider range 0..1, default 0.0 → `setPhaseOffset` (`SignalInspector.h:47`, `:111-117`)
- `loopingToggle_{"Looping"}` ToggleButton → `EnvelopeSignal::setLooping` (`SignalInspector.h:48`, `:119-126`)
- `oneShotToggle_{"One Shot"}` ToggleButton → `EnvelopeSignal::setOneShot` (`SignalInspector.h:49`, `:128-135`)

**Note**: The file-level comment mentions a "Routes list: where this signal connects" (`SignalInspector.h:13`), but there is NO routes list / route-display component implemented in the file — no UI for showing which parameters this signal connects to.

---

### 13.5 Mapping Editor — `MappingEditor`

`src/ui/MappingEditor.h:18-79`, `src/ui/MappingEditor.cpp:1-322`.

Modal/popup editor component. Not tab-based — has its own Listener interface (`MappingEditor.h:21-28`): `mappingEditorChanged`, `mappingEditorDeleteRequested`, `mappingEditorCloseRequested`.

Controls:

- Title row (`:221-223`):
  - `titleLabel_` "Mapping Editor" (bold 14pt, cyan)
  - `closeButton_{"X"}` → fires `mappingEditorCloseRequested`
- **Source row** (`:227-229`):
  - `sourceLabel_` "Source:"
  - `sourceCombo_` ComboBox populated by `populateSourceCombo()` (`MappingEditor.h:62`, `:302-308`)
    - Contains all `MappingSource` enum values as formatted names via `getSourceName()` (`MappingEditor.cpp:5-62`):
      - `"RMS"`, `"Peak"`, `"RMS (dB)"`, `"LUFS"`, `"Dynamic Range"`, `"Transient Density"`
      - `"Spectral Centroid"`, `"Spectral Flux"`, `"Spectral Flatness"`, `"Spectral Rolloff"`
      - `"Band: Sub"`, `"Band: Bass"`, `"Band: Low Mid"`, `"Band: Mid"`, `"Band: High Mid"`, `"Band: Presence"`, `"Band: Brilliance"`
      - `"Onset Strength"`, `"Beat Phase"`, `"BPM"`, `"Structural State"`
      - `"Dominant Pitch"`, `"Pitch Confidence"`, `"Detected Key"`, `"Harmonic Change"`
      - `"MFCC 0"`, `"MFCC 1"`, ..., `"MFCC 12"`
      - `"Chroma C"`, `"Chroma C#"`, `"Chroma D"`, `"Chroma D#"`, `"Chroma E"`, `"Chroma F"`, `"Chroma F#"`, `"Chroma G"`, `"Chroma G#"`, `"Chroma A"`, `"Chroma A#"`, `"Chroma B"`
    - Default ID 1 (RMS)
- **Curve row** (`:233-235`):
  - `curveLabel_` "Curve:"
  - `curveCombo_` populated by `populateCurveCombo()` (`MappingEditor.h:63`, `:310-316`). Options via `getCurveName()`:
    - `"Linear"`, `"Exponential"`, `"Logarithmic"`, `"S-Curve"`, `"Stepped"`, `"Circular In"`, `"Circular Out"`, `"Circular In/Out"`, `"Back In"`, `"Back Out"`, `"Back In/Out"`, `"Elastic In"`, `"Elastic Out"`, `"Elastic In/Out"`, `"Bounce In"`, `"Bounce Out"`, `"Bounce In/Out"`, `"Cubic In"`, `"Cubic Out"`, `"Cubic In/Out"`, `"Sine In"`, `"Sine Out"`, `"Sine In/Out"`, `"Hold"`
    - Default ID 1 (Linear)
- **Slider rows** (all ResettableSlider LinearHorizontal with TextBoxRight 50×20, step 0.001) (`MappingEditor.h:65-69`, `:145-149`):
  - `inputMinSlider_` range -1000..1000, default 0.0, label "In Min:"
  - `inputMaxSlider_` range -1000..1000, default 1.0, label "In Max:"
  - `outputMinSlider_` range 0..1, default 0.0, label "Out Min:"
  - `outputMaxSlider_` range 0..1, default 1.0, label "Out Max:"
  - `smoothingSlider_` range 0.01..1.0, default 0.15, label "Smooth:"
- **Bottom row** (`:254-259`):
  - `enableToggle_{"Enabled"}` ToggleButton, default true (`MappingEditor.h:71`, `:152-156`)
  - `randomButton_{"Randomize"}` TextButton (`MappingEditor.h:74`, `:174-202`):
    - Randomizes source (first 21 "useful" sources, skipping MFCC and Chroma)
    - Randomizes curve (any `MappingCurve`)
    - Randomizes input/output ranges (min/max pairs preserved min < max)
    - Randomizes smoothing (biased low: `rng.nextFloat() * 0.5`)
  - `deleteButton_{"Delete Mapping"}` TextButton (`MappingEditor.h:72`, `:159-163`):
    - Fires `mappingEditorDeleteRequested`

All change actions fire `notifyChanged()` → `Listener::mappingEditorChanged`.

---

### 13.6 Macro Panel — `MacroPanel`

`src/ui/MacroPanel.h:15-53`, `src/ui/MacroPanel.cpp:1-153`.

Used as the "Dashboard" section inside ClipInspector, LayerInspector, and CompositionInspector. `kPreferredHeight = 110` (`MacroPanel.h:33`).

Structure: 1 row × `MacroBank::kNumMacros` columns (number of macros defined by `MacroBank`).

Each slot (`MacroPanel::MacroSlot`, `MacroPanel.h:40-44`) contains:

- `knob` — `Knob` (see ui/Knob) labelled `"Link 1"` .. `"Link 8"` (`MacroPanel.cpp:9`):
  - Default value 0.5 (`:10-11`)
  - On value change: writes `macroBank_->getMacro(i).manualValue` AND fires `onMacroValueChanged(i, val)` callback (`:14-20`)
- `sourceBtn` — `juce::TextButton` initially labelled `"Manual"` (`:23-32`):
  - Click → `showSourcePicker(macroIndex)` opens a PopupMenu (`:109-152`):
    - Item ID 1: `"Manual"` — resets `macro.sourceSignalId = 0`
    - Submenu `"Signals"` — lists all signals from `signalRegistry_` (item IDs 100 + index)
  - After selection: sets `macro.sourceSignalId` and fires `onMacroSourceChanged(macroIndex, signalId)`

Section header painted: "DASHBOARD" text (10pt secondary-text color, `MacroPanel.cpp:38-43`).

When a macro is connected to a signal (`!macro.isManual()`), the knob displays the signal-computed value (not the manual value), the source button label becomes the signal name, and a mapping indicator is shown on the knob (`MacroPanel.cpp:85-106`).

Callbacks:
- `onMacroValueChanged(int index, float value)` (`MacroPanel.h:36`)
- `onMacroSourceChanged(int index, uint32_t signalId)` (`MacroPanel.h:37`)

---

### 13.7 UniversalParamControl — all modes / features / right-click options

`src/ui/UniversalParamControl.h:44-142`, `src/ui/UniversalParamControl.cpp:1-554`.

Standard parameter widget used across all inspectors. Has a collapsed and expanded form.

**Nested class `ResettableSlider`** (`UniversalParamControl.h:21-42`):
- Subclass of `juce::Slider`. Overrides `mouseDown`:
  - **Right-click** → resets to `defaultVal_` (if `hasDefault_`) via `setValue(defaultVal_, sendNotificationSync)`
  - Normal click → forwards to `juce::Slider::mouseDown`
- `setDefaultValue(double)` stores default and marks `hasDefault_ = true`

**Collapsed view** (`kCollapsedHeight = 24`):
- Signal-connect triangle — small right-pointing triangle in the first `kTriangleSize = 14` px (`UniversalParamControl.h:87`, `:96-118`):
  - Grey (kTextSecondary × 0.5 alpha) when unconnected (`SourceMode::Manual`)
  - Cyan (kAccentCyan) when any other `SourceMode`
  - Click → `showSourcePickerAtTriangle()` (`:311-327`)
- Parameter name label (painted, 72px wide, 11pt)
- Value text (painted, 2-decimal float, 36px wide, right-aligned)
- `decrementBtn_{"-"}` — 20px wide, decrements by 0.01 (`UniversalParamControl.h:129`, `:23-32`)
- `incrementBtn_{"+"}` — 20px wide, increments by 0.01 (`UniversalParamControl.h:130`, `:34-43`)
- `valueSlider_` ResettableSlider LinearHorizontal NoTextBox range 0..1 step 0.001, scroll wheel DISABLED (`:6-20`)

**Expanded view** (toggled by clicking name/value area, `:266-271`):
- `sourceBtn_{"Manual"}` TextButton (120px wide) — opens source picker (`UniversalParamControl.h:133`, `:46-51`)
- `invertToggle_{"Invert"}` ToggleButton (70px wide) → fires `onInvertChanged(bool)` (`UniversalParamControl.h:134`, `:54-60`)
- "Range" label + two range sliders (`UniversalParamControl.h:135-137`, `:62-93`):
  - `rangeMinSlider_` LinearHorizontal TextBoxRight 35×18, range 0..1 step 0.01, scroll disabled
  - `rangeMaxSlider_` same config
  - Both fire `onRangeChanged(min, max)`
- Mini meter bar painted when connected (cyan background + magenta fill at `sourceValue_`, `:146-165`)
- Source name hint painted when connected but collapsed (`:167-178`)

**Source-picker popup** (`buildSourcePickerMenu`, `:341-458`):
- Item ID 1: `"Manual"` (default/root)
- Separator
- Submenu `"Audio"` — for each `Signal::Type::Audio` in `signalRegistry_`, item ID `100 + idx`
- Submenu `"BPM Sync"` with 4 shape submenus — for each shape × 6 divisions, item ID `300 + shape*6 + div`:
  - Shapes: `"Sine"`, `"Saw"`, `"Triangle"`, `"Square"`
  - Divisions: `"1/4 Beat"`, `"1/2 Beat"`, `"1 Beat"`, `"2 Beats"`, `"4 Beats"`, `"8 Beats"`
- Submenu `"Oscillator"` — for each `Signal::Type::Oscillator` in registry, item ID `400 + idx`
- Submenu `"Envelope"` — for each `Signal::Type::Envelope` in registry, item ID `500 + idx`
- Item ID 2: `"Clip Position"`
- Item ID 3: `"Timeline"`
- Separator
- Submenu `"Macro"` — 8 items (`"Macro 1"` .. `"Macro 8"`), item IDs 200..207

**SourceMode enum** (`UniversalParamControl.h:61-70`): `Manual`, `Signal`, `BPMSync`, `Oscillator`, `Envelope`, `ClipPosition`, `Timeline`, `Macro`.

**Mouse handling** (`:246-273`):
- Right-click anywhere → reset to `defaultValue_` (writes slider + fires `onValueChanged`)
- Click in triangle area (x < 14, y < 24) → source picker
- Click in name/value area (x < 14+72+36 = 122, y < 24) → toggle expanded
- Otherwise → forwards to `Component::mouseDown`

**Callbacks** (`UniversalParamControl.h:90-94`):
- `onValueChanged(float)` — fired on slider change, right-click reset, +/- buttons
- `onExpandToggled()` — fired when collapsed/expanded toggles
- `onSourceChanged(SourceMode, String name)` — fired from source picker selection
- `onRangeChanged(float min, float max)` — fired from range sliders
- `onInvertChanged(bool)` — fired from invert toggle

---

## Section 2 partial: Signal Connect Triangle Popup

`UniversalParamControl::showSourcePickerAtTriangle()` (`UniversalParamControl.cpp:311-327`) builds menu via `buildSourcePickerMenu()` (`:341-458`).

Full option tree (`SourceMode` values selected by these):

| Menu path | Item ID | Selects |
|---|---|---|
| `Manual` | 1 | `SourceMode::Manual`, clears name |
| `Audio › <signal name>` | 100+N | `SourceMode::Signal` + signal name |
| `BPM Sync › Sine › 1/4 Beat` | 300 | `SourceMode::BPMSync` + "Sine 1/4 Beat" |
| `BPM Sync › Sine › 1/2 Beat` | 301 | "Sine 1/2 Beat" |
| `BPM Sync › Sine › 1 Beat` | 302 | "Sine 1 Beat" |
| `BPM Sync › Sine › 2 Beats` | 303 | "Sine 2 Beats" |
| `BPM Sync › Sine › 4 Beats` | 304 | "Sine 4 Beats" |
| `BPM Sync › Sine › 8 Beats` | 305 | "Sine 8 Beats" |
| `BPM Sync › Saw › <six divs>` | 306-311 | `BPMSync` + "Saw {div}" |
| `BPM Sync › Triangle › <six divs>` | 312-317 | `BPMSync` + "Triangle {div}" |
| `BPM Sync › Square › <six divs>` | 318-323 | `BPMSync` + "Square {div}" |
| `Oscillator › <signal name>` | 400+N | `SourceMode::Oscillator` + signal name |
| `Envelope › <signal name>` | 500+N | `SourceMode::Envelope` + signal name |
| `Clip Position` | 2 | `SourceMode::ClipPosition` |
| `Timeline` | 3 | `SourceMode::Timeline` |
| `Macro › Macro 1` .. `Macro 8` | 200-207 | `SourceMode::Macro` + "Macro N" |

Expanded sub-controls (visible only when expanded):
- **Manual**: slider + ± buttons only (no source meter)
- **Signal / Oscillator / Envelope / BPM Sync / Clip Position / Timeline / Macro**: 14px mini meter row painted (showing `sourceValue_`) + Source button + Invert toggle + Range min/max sliders

---

## Section 16 partial: Dialogs invoked from inspectors

**No dialogs invoked from inspectors directly in these files.**

Reviewing all reads:
- No `juce::FileChooser` invoked from any inspector file.
- No `juce::AlertWindow` invoked from any inspector file.
- No `juce::DialogWindow` use in these files.

PresetManager (`src/ui/PresetManager.h:22-87`, `.cpp:1-379`) is a **static utility class** — no dialog UI. It just has static `savePreset`, `loadPreset`, `saveDeck`, `loadDeck` methods that take `juce::File` args directly. Directory helpers `getPresetsDirectory()`, `getFxSaveDirectory()`, `getDeckDirectory()` create subdirs in `userApplicationDataDirectory/AudioDNA/{Presets|FX Saves|Decks}`. Dialogs that call PresetManager live in MenuBar or MainComponent, not in inspector files.

**Popups** (not dialogs — PopupMenus) invoked from inspectors:
- MacroPanel source picker popup (`MacroPanel.cpp:109-152`) — see 13.6
- UniversalParamControl source picker popup (`UniversalParamControl.cpp:311-458`) — see Section 2 above

---

## Section 17 partial: Right-click menus in inspectors

Right-click behaviors across inspectors are **limited to reset-to-default**, not context menus.

- `ResettableSlider::mouseDown` (`UniversalParamControl.h:28-37`):
  - Right-click → `setValue(defaultVal_, sendNotificationSync)` then returns (no menu)
- `UniversalParamControl::mouseDown` (`UniversalParamControl.cpp:249-255`):
  - Right-click → resets to `defaultValue_` + fires `onValueChanged(defaultValue_)` then returns

No right-click context menus (copy/paste/clear-binding/etc.) exist in ClipInspector, LayerInspector, CompositionInspector, SignalInspector, or MappingEditor.

**Special modifier behaviors (not right-click)**:
- ClipInspector cuepoint buttons (`ClipInspector.cpp:288-300`): **Ctrl/Cmd-click** on a numbered cuepoint button clears it (shifts remaining down, decrements `numCuepoints`). Tooltip displays "Ctrl+click to clear" when cue is set.

---

## Section 20 partial: Cross-references

- **Inspector host**: `InspectorPanel` (`src/ui/InspectorPanel.h`) contains 4 inspectors and manages tab switching + pin state.
- **`MacroPanel` embedded** in:
  - `ClipInspector::macroPanel_` (`src/ui/ClipInspector.h:58`)
  - `LayerInspector::macroPanel_` (`src/ui/LayerInspector.h:58`)
  - `CompositionInspector::macroPanel_` (`src/ui/CompositionInspector.h:46`)
- **`EffectStackView` embedded** in (via `ui/EffectStackView.h`, not in this slice):
  - `ClipInspector::effectStackView_` (`ClipInspector.h:126`) — bound to `clip_->effects`
  - `LayerInspector::effectStackView_` (`LayerInspector.h:118`) — bound to `layer_->layerEffects`
  - `CompositionInspector::effectStackView_` (`CompositionInspector.h:81`) — bound to `comp->globalEffects`
- **`UniversalParamControl` used as**:
  - ClipInspector: `clipOpacityControl_`, `posXControl_`, `posYControl_`, `scaleControl_`, `rotationControl_`, `anchorControl_`, plus N dynamic source-param controls (`sourceParamControls_`)
  - LayerInspector: `masterControl_`, `opacityControl_`, `posXControl_`, `posYControl_`, `scaleControl_`, `rotationControl_`, `anchorControl_`
  - CompositionInspector: `masterControl_`, `speedControl_`, `opacityControl_`, `posXControl_`, `posYControl_`, `scaleControl_`, `rotationControl_`, `anchorControl_`
- **`ResettableSlider` used as**: the "all sliders must be resettable" invariant — every slider in these inspectors is `ResettableSlider`, not `juce::Slider`.
- **Data model references**:
  - `Clip` (`src/model/Clip.h`) — fields written by ClipInspector: `transportMode`, `loopMode`, `playing`, `reverse`, `speed`, `beatSnapMode`, `beatSnap`, `sequenceFps`, `beatDivision`, `videoBeats`, `autopilotAction`, `autopilotDuration`, `clipOpacity`, `clipWidth`, `clipHeight`, `alphaType`, `channelR/G/B/A`, `positionX/Y`, `scale`, `rotation`, `anchorX`, `inPoint`, `outPoint`, `playheadPosition`, `cuepoints[]`, `numCuepoints`, `sourceParams[].value`, `effects`
  - `Layer` (`src/model/Layer.h`) — fields written by LayerInspector: `name`, `autopilotEnabled`, `defaultAutopilotAction`, `autopilotEndOfVideo`, `defaultAutopilotDuration`, `autopilotLoops`, `opacity`, `persistent`, `ignoreColumnTrigger`, `blendMode`, `layerWidth`, `layerHeight`, `autoSize`, `transitionMode`, `transitionSpeed`, `keyingMode`, `keyThreshold`, `keySoftness`, `dryWetMix`, `rotationX/Y/Z`, `rotationSpeed`, `scale3D`, `positionX/Y`, `layerScale`, `layerRotation`, `layerAnchorX`, `feedback.{enabled,amount,scaleX,scaleY,rotation,offsetX,offsetY,lumaKey,presetName}`, `layerEffects`
  - `Composition` (`src/model/Composition.h`) — fields written by CompositionInspector: `autopilotDirection`, `autopilotDurationMode`, `autopilotClipLoops`, `autopilotLoop`, `autopilotMasterLayer`, `perTypeAutopilot.{perTypeEnabled,opaqueCycleBeats,transparentCycleBeats,effectCycleBeats,transparentRandomize,effectRandomize}`, `masterOpacity`, `masterSpeed`, `compOpacity`, `compPositionX/Y`, `compScale`, `compRotation`, `compAnchorX/Y`, `outputWidth`, `outputHeight`, `globalEffects`
  - `Signal` / `OscillatorSignal` / `EnvelopeSignal` (`src/signal/*.h`) — Setters used by SignalInspector: `setShape`, `setBeatDuration`, `setAmplitude`, `setPhaseOffset`, `setCurveType`, `setLooping`, `setOneShot`
  - `Mapping` (`src/mapping/MappingTypes.h`) — read/written by MappingEditor: `source`, `curve`, `inputMin/Max`, `outputMin/Max`, `smoothing`, `enabled`, `targetEffectId`, `targetParamIndex`
  - `MacroBank` (`src/routing/MacroBank.h`) — `kNumMacros`, `getMacro(i)`, `Macro::manualValue`, `.sourceSignalId`, `.currentValue`, `.name`, `.isManual()`, `updateValues(SignalRegistry&)`
  - `SignalRegistry` (`src/signal/SignalRegistry.h`) — `getNumSignals`, `getSignal(id)`, `getSignalAt(idx)`, `getCachedValue(id)`
  - `FeedbackProcessor::getPresets()` — feeds feedback preset selector in LayerInspector
- **Shared layout constants** (per-inspector): `kSectionHeaderHeight`, `kSectionGap`, `kRowHeight`, `kNameBarHeight`, plus ClipInspector-specific `kTimelineHeight = 36`, `kInset = 6`, `kNumCuepoints = 8`.
- **Callbacks out of ClipInspector**: `onSourceParamsChanged`, `onCuepointJump`, `onCuepointSet` (`ClipInspector.h:44-50`).
- **Callback out of LayerInspector**: `onLayerNameChanged` (`LayerInspector.h:49`).
- **Callbacks out of MacroPanel**: `onMacroValueChanged`, `onMacroSourceChanged` (`MacroPanel.h:36-37`).
- **Listener interface of MappingEditor**: `mappingEditorChanged`, `mappingEditorDeleteRequested`, `mappingEditorCloseRequested` (`MappingEditor.h:21-28`).

---

## Summary counts

- **Inspector panel container**: 1 (InspectorPanel with 4 tab buttons + 1 pin button)
- **Inspectors**: 4 (Clip, Layer, Composition, Signal) + MappingEditor (standalone)
- **ClipInspector sections**: 9 (Name, Dashboard, Transport, Cuepoints, Autopilot, [Source conditional], Video, Transform, Effects)
- **LayerInspector sections**: 7 base + up to 3 conditional (Name, Dashboard, Autopilot, Layer (Master), Video, Transition, [Keying/DryWet/3D conditional], Transform, Feedback, Layer Effects) = 10–12 depending on type
- **CompositionInspector sections**: 9 (Name+Resolution, Dashboard, Autopilot, Per-Type Autopilot, Composition, Video, Transform, Global Effects, Output Settings)
- **SignalInspector sections**: 1 conditional (Audio / Oscillator / Envelope based on signal type; Envelope adds curve-editor sub-panel)
- **Total unique ComboBoxes**: 21+ — transport mode, loop, trigger, beatDivision, autopilotAction, autopilotDuration, beatSnap, clipBlendMode, clipAlphaType (Clip); apTriggerMode, apBeatCount, blendMode (25 items), transitionBlend (53 items w/ section headings), keyingMode (13 items), autoSize, feedbackPreset (Layer); apDuration, apMasterLayer, resolution (Composition); waveShape, oscBeatDuration, curveType, envBeatDuration (Signal); sourceCombo (~53 items), curveCombo (24 items) (MappingEditor)
- **Total unique Sliders** (all ResettableSlider): ~40+ across all inspectors
- **Total TextButtons**: ~25 (transport, cuepoint trigger/set, autopilot direction, duration ÷/× buttons, etc.)
- **Total ToggleButtons**: ~12 (channelR/G/B/A, persistent, ignoreColumn, feedbackEnable, enable, random per-type toggles, looping, oneShot)

### Surprises and non-obvious findings

1. **No routes list in SignalInspector** — the header comment promises "Routes list: where this signal connects" (`SignalInspector.h:13`) but no such component is implemented. The inspector only shows signal-type-specific settings.
2. **No right-click context menus anywhere** — only the reset-to-default behavior via `ResettableSlider::mouseDown` and `UniversalParamControl::mouseDown`. No copy/paste/clear-binding menus.
3. **`triggerDropdown_` in ClipInspector has no `onChange` wired** — the clip's TriggerMode (Restart/Continue/Relative) is set by the dropdown UI but the value is not written to the clip model in this file (just `setSelectedId` default without a handler). Only loop mode is wired.
4. **CompositionInspector UniversalParamControls do NOT call `setDefaultValue()`** — unlike Clip and Layer inspectors, meaning right-click reset may not work as expected for composition-level transform/master/speed/opacity params unless the ResettableSlider's default is set elsewhere.
5. **PresetManager is pure static utility** — no UI in this slice for presets (save/load dialogs live outside inspector files).
6. **LayerInspector `transitionBlendSelector_` has 53 entries** across 14 section headings — far richer than the 25-entry simple `blendModeSelector_` on the same layer.
7. **MacroPanel sourceBtn text is dynamic** — it doubles as both a "Manual" default button and a live signal-name label; clicking opens the source picker.
8. **Dual cuepoint buttons** (trigger + set), not single buttons with modifier behavior.
9. **Timeline bar drag zones**: in-point handle, out-point handle, and playhead; 8-pixel hit zone around in/out brackets before falling through to "scrub playhead".
10. **UniversalParamControl value slider has scroll wheel DISABLED** (so scrolling scrolls the containing viewport, not the slider).
