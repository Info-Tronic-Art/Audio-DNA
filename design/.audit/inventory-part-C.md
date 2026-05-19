# FEATURE_INVENTORY.md — Part C (Domains 4, 8, 9)

---

## Domain 4: Composition & Performance

---

### 4.1 Deck/Layer/Clip Hierarchy

**Level:** PRIMARY
**Current UI:** Deck Grid (center area), Deck Tabs (bottom of grid), Layer Strips (left 250px), Clip Cells (90x96px grid)
**Sub-features:**
  - Composition container [ALWAYS-VISIBLE] — top-level entity owning all decks, global effects, global settings, genre-deck assignments, composition transform
  - Deck management (N decks) [ALWAYS-VISIBLE] — New / Insert Before / Insert After / Duplicate / Rename / Close / Clear Clips / Remove via Deck menu; tabs along bottom of grid for switching
  - Layer management (M layers per deck) [ALWAYS-VISIBLE] — New / Insert Above / Insert Below / Duplicate / Rename / Copy Effects / Paste Effects / Clear Clips / Remove / Ignore Column Trigger / Lock Content / Fold / Move Up / Move Down via Layer menu
  - Column management (K columns per deck) [ALWAYS-VISIBLE] — New / Insert Before / Insert After / Duplicate / Clear Clips / Remove / Remove All Before / Remove All After via Column menu
  - Clip management [ALWAYS-VISIBLE] — Select All / Cut / Copy / Paste / Copy Effects / Paste Effects / Rename / Clear / Show in Finder / New Source / New Effect / Replace Content / Lock Content via Clip menu
  - Column trigger buttons [ALWAYS-VISIBLE] — numbered 1..K across top of grid, click fires all clips in column simultaneously (respects per-layer Ignore Column Trigger flag)
  - Deck tabs [ALWAYS-VISIBLE] — one tab per deck, click to switch active deck; active deck highlighted
  - Active deck rendering [ALWAYS-VISIBLE] — only active deck renders; persistent layers from non-active decks also composite
  - Default grid: 3 layers x 12 columns per deck
**Parameters:**
  - Composition.activeDeckIndex (int, 0..N-1)
  - Deck.numColumns (int, default 12)
  - Deck.layers.size() (int, default 3)
**Bindings:** Switch Deck (keyboard/MIDI), Trigger Column (keyboard/MIDI)
**Programming mode:** Full grid visible with all controls
**Presentation mode:** VESTIGIAL — current ProgrammingMode only toggles signal bar expansion, hides all panels. No graduated presentation mode exists.
**Dependencies:** Render Pipeline (compositing order), Autopilot (clip advancement), Undo/Redo (all structural changes)

---

### 4.2 Layer Types

**Level:** PRIMARY
**Current UI:** Layer Inspector > Video section (implicit via layer type) and Layer Strip
**Sub-features:**
  - Opaque [ALWAYS-VISIBLE] — one clip at a time, replaces everything below. Default for first layer in deck.
  - Transparent [ALWAYS-VISIBLE] — composited over layers below with blend mode + keying. Default for layers 2+ in deck.
  - FXOnly [ALWAYS-VISIBLE] — no media; applies effects to the composited accumulator. Has dedicated Dry/Wet control.
  - ThreeD [ALWAYS-VISIBLE] — 3D model/surface rendering with rotation X/Y/Z, rotation speed, and scale controls.
  - Mask [ALWAYS-VISIBLE] — content used as luminance alpha mask for layers below.
**Parameters:**
  - Layer.type (enum: Opaque=0, Transparent=1, FXOnly=2, ThreeD=3, Mask=4)
  - FXOnly: Layer.dryWetMix (float, 0-1, default 1.0)
  - ThreeD: Layer.rotationX/Y/Z (float, degrees), Layer.rotationSpeed (float), Layer.scale3D (float, default 1.0)
**Bindings:** None directly (type set via inspector)
**Programming mode:** Full type selection and type-specific controls visible
**Presentation mode:** HIDDEN — type is set during programming, not changed during performance
**Dependencies:** Render Pipeline (compositing behavior differs by type), Keying (Transparent only), Effect Chain Architecture (FXOnly applies to accumulator)

---

### 4.3 Layer Controls

**Level:** PRIMARY
**Current UI:** Layer Strip (250px, left side of Deck Grid) + Layer Inspector

#### 4.3.1 Blend Modes (25)

**Level:** PRIMARY
**Current UI:** Layer Strip V dropdown, Layer Inspector > Video > Blend Mode dropdown
**Sub-features:**
  - Normal [ALWAYS-VISIBLE] — standard alpha compositing
  - Additive [ALWAYS-VISIBLE] — pixel values summed (default blend mode)
  - Screen [ALWAYS-VISIBLE] — inverted multiply
  - Multiply [ALWAYS-VISIBLE] — darkening blend
  - Overlay [ALWAYS-VISIBLE] — combines Multiply and Screen
  - SoftLight [ALWAYS-VISIBLE] — gentle contrast adjustment
  - HardLight [ALWAYS-VISIBLE] — strong contrast adjustment
  - VividLight [ALWAYS-VISIBLE] — extreme burn/dodge combo
  - LinearLight [ALWAYS-VISIBLE] — linear burn/dodge combo
  - PinLight [ALWAYS-VISIBLE] — conditional replacement
  - HardMix [ALWAYS-VISIBLE] — threshold posterization
  - Darken [ALWAYS-VISIBLE] — keep darker pixel
  - Lighten [ALWAYS-VISIBLE] — keep lighter pixel
  - DarkerColor [ALWAYS-VISIBLE] — keep darker color (luminance-based)
  - LighterColor [ALWAYS-VISIBLE] — keep lighter color (luminance-based)
  - ColorDodge [ALWAYS-VISIBLE] — brighten by dividing
  - ColorBurn [ALWAYS-VISIBLE] — darken by dividing
  - Difference [ALWAYS-VISIBLE] — absolute pixel difference
  - Exclusion [ALWAYS-VISIBLE] — lower-contrast version of Difference
  - Subtract [ALWAYS-VISIBLE] — subtract pixel values
  - Hue [ALWAYS-VISIBLE] — hue from layer, saturation+luminosity from below
  - Saturation [ALWAYS-VISIBLE] — saturation from layer, hue+luminosity from below
  - Color [ALWAYS-VISIBLE] — hue+saturation from layer, luminosity from below
  - Luminosity [ALWAYS-VISIBLE] — luminosity from layer, hue+saturation from below
  - Dissolve [ALWAYS-VISIBLE] — random pixel selection (dither pattern)
**Parameters:**
  - Layer.blendMode (MixMode enum, 25 blend values: Normal through Dissolve)
**Bindings:** Not directly bindable (dropdown selection)
**Programming mode:** Full dropdown with organized sections (Basic, Light, Dark/Light Compare, Dodge/Burn, Inversion, Component/HSL, Special)
**Presentation mode:** HIDDEN — set during programming
**Dependencies:** Render Pipeline (CompositorEngine blend shader)

#### 4.3.2 Transition Modes (30)

**Level:** PRIMARY
**Current UI:** Layer Strip F dropdown, Layer Inspector > Transition > Blend Mode dropdown
**Sub-features:**
  - Cut [ALWAYS-VISIBLE] — instant switch, no transition
  - WipeLeft [ALWAYS-VISIBLE] — horizontal wipe from right to left
  - WipeRight [ALWAYS-VISIBLE] — horizontal wipe from left to right
  - WipeUp [ALWAYS-VISIBLE] — vertical wipe upward
  - WipeDown [ALWAYS-VISIBLE] — vertical wipe downward
  - WipeEllipse [ALWAYS-VISIBLE] — elliptical/iris wipe
  - WipeDiagonal [ALWAYS-VISIBLE] — diagonal wipe
  - PushLeft [ALWAYS-VISIBLE] — content slides left, new enters from right
  - PushRight [ALWAYS-VISIBLE] — content slides right, new enters from left
  - PushUp [ALWAYS-VISIBLE] — content slides up, new enters from bottom
  - PushDown [ALWAYS-VISIBLE] — content slides down, new enters from top
  - ZoomIn [ALWAYS-VISIBLE] — new clip zooms in from center
  - ZoomOut [ALWAYS-VISIBLE] — new clip zooms out
  - RotateX [ALWAYS-VISIBLE] — 3D rotation around X axis
  - RotateY [ALWAYS-VISIBLE] — 3D rotation around Y axis
  - Spin [ALWAYS-VISIBLE] — 2D spin transition
  - Cube [ALWAYS-VISIBLE] — 3D cube rotation
  - Flip [ALWAYS-VISIBLE] — 3D flip card
  - Fold [ALWAYS-VISIBLE] — 3D folding paper
  - ToBlack [ALWAYS-VISIBLE] — fade through black
  - ToWhite [ALWAYS-VISIBLE] — fade through white
  - Pixelate [ALWAYS-VISIBLE] — pixelation transition
  - Blur [ALWAYS-VISIBLE] — blur transition
  - Noise [ALWAYS-VISIBLE] — noise dissolve
  - RGBSplit [ALWAYS-VISIBLE] — RGB channel offset transition
  - GlitchBlocks [ALWAYS-VISIBLE] — block glitch effect
  - Strobe [ALWAYS-VISIBLE] — strobe flash transition
  - Slide [ALWAYS-VISIBLE] — slide transition
  - Stretch [ALWAYS-VISIBLE] — stretch transition
  - Displace [ALWAYS-VISIBLE] — displacement map transition
**Parameters:**
  - Layer.transitionMode (MixMode enum, 30 transition values: Cut through Displace)
  - Layer.transitionBlendMode (MixMode, default Normal)
  - Layer.transitionSpeed (float, -1 = use global default, range 0.1-10s)
**Bindings:** Not directly bindable
**Programming mode:** Full dropdown with organized sections (Instant, Directional Wipes, Push, Zoom, 3D Rotation, Fade Through Color, Creative/VJ)
**Presentation mode:** HIDDEN — transitions are pre-configured, fire automatically on clip change
**Dependencies:** Render Pipeline (transitionFBO, crossfadeProgress per-layer)

#### 4.3.3 Keying Modes (13)

**Level:** PRIMARY
**Current UI:** Layer Inspector > Keying section (visible only for Transparent layer type)
**Sub-features:**
  - Alpha [ALWAYS-VISIBLE] — standard alpha channel transparency (default)
  - LumaKey [ALWAYS-VISIBLE] — dark areas become transparent based on luminance threshold
  - InvertedLumaKey [ALWAYS-VISIBLE] — light areas become transparent
  - LumaIsAlpha [ALWAYS-VISIBLE] — luminance value directly becomes alpha
  - InvertedLumaIsAlpha [ALWAYS-VISIBLE] — inverted luminance becomes alpha
  - ChromaKey [ALWAYS-VISIBLE] — remove specific color (green screen). Has dedicated color picker (R/G/B) and tolerance.
  - MaxRGB [ALWAYS-VISIBLE] — maximum of R/G/B channels used as key
  - SaturationKey [ALWAYS-VISIBLE] — low-saturation areas become transparent
  - EdgeDetection [ALWAYS-VISIBLE] — edge pixels visible, fill transparent
  - ThresholdMask [ALWAYS-VISIBLE] — hard binary threshold
  - ChannelR [ALWAYS-VISIBLE] — red channel as transparency
  - ChannelG [ALWAYS-VISIBLE] — green channel as transparency
  - ChannelB [ALWAYS-VISIBLE] — blue channel as transparency
**Parameters:**
  - Layer.keyingMode (KeyingMode enum, 13 values)
  - Layer.keyThreshold (float, 0-1, default 0.1)
  - Layer.keySoftness (float, 0-1, default 0.1)
  - Layer.chromaKeyR/G/B (float, 0-1, default green: 0,1,0)
  - Layer.chromaKeyTolerance (float, 0-1, default 0.2)
**Bindings:** keyThreshold and keySoftness are signal-connectable via Universal Parameter Controls
**Programming mode:** Full keying controls visible in inspector
**Presentation mode:** HIDDEN — keying is configured during setup
**Dependencies:** Render Pipeline (scratchFBO for keying shader), Layer Types (only applies to Transparent type)

#### 4.3.4 Layer Transform

**Level:** SECONDARY
**Current UI:** Layer Inspector > Transform section
**Sub-features:**
  - Position X/Y [PRESENTATION-HIDDEN] — pixel offset from center, signal-connectable
  - Scale [PRESENTATION-HIDDEN] — size multiplier (1.0 = 100%), signal-connectable
  - Rotation [PRESENTATION-HIDDEN] — degrees, signal-connectable
  - Anchor X/Y [PROGRAMMING-ONLY] — pivot point offset from center
**Parameters:**
  - Layer.positionX/Y (float, pixels)
  - Layer.layerScale (float, default 1.0)
  - Layer.layerRotation (float, degrees)
  - Layer.layerAnchorX/Y (float)
**Bindings:** All transform params are signal-connectable via Universal Parameter Controls
**Programming mode:** Full sliders with signal connect triangles
**Presentation mode:** HIDDEN — configured during programming, driven by audio mappings in performance
**Dependencies:** Render Pipeline (applied after clip compositing, before accumulator blend)

#### 4.3.5 Layer Feedback (Larsen Loop)

**Level:** SECONDARY
**Current UI:** Layer Inspector > Feedback section
**Sub-features:**
  - Enable toggle [PRESENTATION-HIDDEN] — turn feedback on/off
  - Preset dropdown [PROGRAMMING-ONLY] — Zoom In / Spiral / Drift / Kaleidoscope / Echo / Stretch (6 presets)
  - Amount slider [PRESENTATION-HIDDEN] — how much previous frame bleeds through (0-1, default 0.5)
  - Scale X/Y [PRESENTATION-HIDDEN] — per-frame zoom (default 0.98 each)
  - Rotation [PRESENTATION-HIDDEN] — per-frame rotation in degrees
  - Offset X/Y [PRESENTATION-HIDDEN] — per-frame horizontal/vertical drift (-0.5 to 0.5)
  - Luma Key [PRESENTATION-HIDDEN] — fade dark areas from feedback to prevent muddiness (0-1)
**Parameters:**
  - FeedbackConfig.enabled (bool)
  - FeedbackConfig.presetName (string)
  - FeedbackConfig.amount (float, 0-1)
  - FeedbackConfig.scaleX/Y (float, default 0.98)
  - FeedbackConfig.rotation (float, degrees)
  - FeedbackConfig.offsetX/Y (float, -0.5 to 0.5)
  - FeedbackConfig.lumaKey (float, 0-1)
**Bindings:** All feedback params signal-connectable
**Programming mode:** Full feedback controls visible
**Presentation mode:** HIDDEN — configured in programming, feedback runs automatically
**Dependencies:** Render Pipeline (FeedbackProcessor, per-layer feedback FBOs)

#### 4.3.6 Layer State Controls

**Level:** PRIMARY
**Current UI:** Layer Strip (action buttons row 1)
**Sub-features:**
  - Opacity (V slider) [ALWAYS-VISIBLE] — layer visibility (0-1, default 1.0), signal-connectable
  - Visible toggle [ALWAYS-VISIBLE] — show/hide layer
  - Bypass (B button) [ALWAYS-VISIBLE] — skip layer during rendering (dark red when active)
  - Solo (S button) [ALWAYS-VISIBLE] — render only this layer (olive when active)
  - Mute [PRESENTATION-HIDDEN] — audio mute for layer
  - Clear (X button) [ALWAYS-VISIBLE] — clear/stop active clip on this layer
  - Persistent toggle [PROGRAMMING-ONLY] — keep rendering when deck is not active
  - Ignore Column Trigger [PROGRAMMING-ONLY] — layer won't respond to column triggers
  - Fold [MINIMAL-IN-PRESENTATION] — collapse layer row to save space
  - Content Lock [PROGRAMMING-ONLY] — prevent media replacement
**Parameters:**
  - Layer.opacity (float, 0-1)
  - Layer.visible (bool)
  - Layer.bypassed (bool)
  - Layer.solo (bool)
  - Layer.muted (bool)
  - Layer.persistent (bool)
  - Layer.ignoreColumnTrigger (bool)
  - Layer.folded (bool)
**Bindings:** Toggle Layer Bypass/Solo/Mute/Visible (keyboard/MIDI), Adjust Layer Opacity (MIDI CC)
**Programming mode:** All controls visible on layer strip
**Presentation mode:** MINIMAL — Bypass/Solo/Clear buttons visible; Opacity via MIDI/signal
**Dependencies:** Render Pipeline (bypassed/solo affect compositing), Deck (column trigger respects ignoreColumnTrigger)

#### 4.3.7 Layer Transport

**Level:** PRIMARY
**Current UI:** Layer Strip (row 1 transport buttons)
**Sub-features:**
  - Play backward (<) [ALWAYS-VISIBLE] — set reverse playback
  - Pause (||) [ALWAYS-VISIBLE] — pause playback
  - Play forward (>) [ALWAYS-VISIBLE] — play forward
  - Fast forward (>|) [ALWAYS-VISIBLE] — 2x speed
  - Speed slider (S, 0-4x) [ALWAYS-VISIBLE] — playback speed multiplier (cyan colored)
  - Fade slider (F, 0-4s) [ALWAYS-VISIBLE] — transition speed between clips (blue colored)
**Parameters:**
  - Layer strip speed (float, 0-4x, default 1x)
  - Layer strip fade (float, 0-4s, default 0.3s)
**Bindings:** Layer Transport Play/Pause/Reverse (keyboard/MIDI)
**Programming mode:** Full transport buttons and sliders visible
**Presentation mode:** MINIMAL — buttons visible on layer strip
**Dependencies:** Clip Transport (per-clip transport inherits/overrides), BPM Tracker (speed synced)

#### 4.3.8 Layer Effects

**Level:** PRIMARY
**Current UI:** Layer Inspector > Layer Effects section
**Sub-features:**
  - Layer effect stack [PRESENTATION-HIDDEN] — same UI pattern as clip effects (vertical list with bypass, expand, dry/wet per effect)
  - Applied after clip compositing, before accumulator blend
**Parameters:** Per-effect: EffectSlot.effectName, EffectSlot.paramValues[], EffectSlot.dryWet, EffectSlot.enabled, EffectSlot.bypassed
**Bindings:** Toggle Effect Bypass (keyboard/MIDI), all effect params signal-connectable
**Programming mode:** Full effects stack with all controls
**Presentation mode:** HIDDEN — effects run automatically; controlled via mappings/macros
**Dependencies:** Effect Chain Architecture (per-layer scope), Render Pipeline (applied in layer compositing)

---

### 4.4 Clip Controls

**Level:** PRIMARY
**Current UI:** Clip Cell (90x96px) + Clip Inspector

#### 4.4.1 Clip Transport

**Level:** PRIMARY
**Current UI:** Clip Inspector > Transport section
**Sub-features:**
  - Transport mode dropdown [PROGRAMMING-ONLY] — Timeline or BPM Sync
  - Timeline bar [PROGRAMMING-ONLY] — draggable in/out points + playhead; beat markers in BPM Sync mode
  - Play backward / Pause / Play forward [ALWAYS-VISIBLE] — playback direction
  - Loop mode [PROGRAMMING-ONLY] — Loop / Ping Pong / One Shot
  - Trigger mode [PROGRAMMING-ONLY] — Restart / Continue / Relative
  - Speed slider (0-4x) [ALWAYS-VISIBLE] — playback rate with /2 and x2 buttons and reverse toggle
  - Duration slider [PROGRAMMING-ONLY] — playback length (0.1-300s in Timeline, 1-64 beats in BPM Sync)
  - Beats/Cycle dropdown [PROGRAMMING-ONLY] — BPM Sync only: 1/4 to 16 beats per playback cycle
  - Content Beats slider [PROGRAMMING-ONLY] — BPM Sync only: how many beats the content contains
  - Images/Sec slider [PROGRAMMING-ONLY] — image sequence only: playback FPS
  - In/Out points [PROGRAMMING-ONLY] — normalized [0,1] start/end positions, draggable on timeline
**Parameters:**
  - Clip.transportMode (enum: Timeline=0, BPMSync=1)
  - Clip.loopMode (enum: Loop=0, PingPong=1, OneShot=2)
  - Clip.speed (float, 0-4x, default 1.0)
  - Clip.reverse (bool)
  - Clip.inPoint/outPoint (float, 0-1)
  - Clip.beatDivision (float, default 4.0)
  - Clip.videoBeats (float, default 4.0)
  - Clip.sequenceFps (float, default 2.5)
**Bindings:** Trigger Clip (keyboard/MIDI with optional velocity-to-opacity)
**Programming mode:** Full transport controls visible
**Presentation mode:** HIDDEN — transport pre-configured; clips triggered via grid/bindings
**Dependencies:** BPM Tracker (beat-synced transport), Layer Transport (inherits speed/direction)

#### 4.4.2 Clip BPM Sync

**Level:** SECONDARY
**Current UI:** Clip Inspector > Transport section (when mode = BPM Sync)
**Sub-features:**
  - Beat Division [PROGRAMMING-ONLY] — how many beats to play content over (1/4 to 16)
  - Content Beats [PROGRAMMING-ONLY] — how many beats the source content contains
  - Beat markers on timeline [PROGRAMMING-ONLY] — visual beat grid overlay
**Parameters:**
  - Clip.beatDivision (float, default 4.0)
  - Clip.videoBeats (float, default 4.0)
**Bindings:** Not directly bindable
**Programming mode:** Visible when Transport Mode = BPM Sync
**Presentation mode:** HIDDEN
**Dependencies:** BPM Tracker (provides BPM for sync calculation)

#### 4.4.3 Clip Beat Snap

**Level:** SECONDARY
**Current UI:** Clip Inspector > Autopilot section > Beat Snap dropdown
**Sub-features:**
  - Snap Off [ALWAYS-VISIBLE] — trigger immediately on click
  - Snap to Beat [ALWAYS-VISIBLE] — queue trigger until next beat
  - Snap to Bar [ALWAYS-VISIBLE] — queue trigger until next bar (4 beats)
  - Snap to 2 Bar [ALWAYS-VISIBLE] — queue trigger until 2-bar boundary (8 beats)
  - Snap to 4 Bar [ALWAYS-VISIBLE] — queue trigger until 4-bar boundary (16 beats)
**Parameters:**
  - Clip.beatSnapMode (enum: Off=0, Beat=1, Bar=2, TwoBar=3, FourBar=4)
**Bindings:** Not directly bindable (set per-clip in inspector)
**Programming mode:** Dropdown visible in Autopilot section
**Presentation mode:** HIDDEN — snap behavior runs automatically when clips are triggered
**Dependencies:** BPM Tracker (beat/bar phase for snap timing), Layer.processPendingTrigger()

#### 4.4.4 Cuepoints

**Level:** SECONDARY
**Current UI:** Clip Inspector > Cuepoints section
**Sub-features:**
  - 8 trigger buttons [ALWAYS-VISIBLE] — numbered 1-8, click to jump to cuepoint position
  - 8 set buttons [PROGRAMMING-ONLY] — click to set cuepoint at current playhead
  - Right-click/Ctrl-click to clear [PROGRAMMING-ONLY]
**Parameters:**
  - Clip.cuepoints[8] (float array, normalized [0,1] positions)
  - Clip.numCuepoints (int, 0-8)
**Bindings:** Not directly bindable (cuepoint jump could be mapped via REST API)
**Programming mode:** Full cuepoint grid with set/clear/trigger
**Presentation mode:** MINIMAL — trigger buttons accessible, set/clear hidden
**Dependencies:** Clip Transport (playhead jumps to cuepoint position)

#### 4.4.5 Clip Transform

**Level:** SECONDARY
**Current UI:** Clip Inspector > Transform section
**Sub-features:**
  - Position X/Y [PRESENTATION-HIDDEN] — pixel offset, signal-connectable
  - Scale [PRESENTATION-HIDDEN] — size multiplier (1.0 = 100%), signal-connectable
  - Rotation [PRESENTATION-HIDDEN] — degrees, signal-connectable
  - Anchor X/Y [PROGRAMMING-ONLY] — pivot point offset
**Parameters:**
  - Clip.positionX/Y (float, pixels)
  - Clip.scale (float, default 1.0)
  - Clip.rotation (float, degrees)
  - Clip.anchorX/Y (float)
**Bindings:** All signal-connectable via Universal Parameter Controls
**Programming mode:** Full sliders visible
**Presentation mode:** HIDDEN — driven by audio mappings
**Dependencies:** Render Pipeline (applied before layer compositing)

#### 4.4.6 Clip Video Properties

**Level:** SECONDARY
**Current UI:** Clip Inspector > Video section
**Sub-features:**
  - Opacity slider [ALWAYS-VISIBLE] — per-clip transparency (0-1), signal-connectable
  - Width / Height [PROGRAMMING-ONLY] — pixel dimensions with inc/dec buttons
  - Blend Mode Override dropdown [PROGRAMMING-ONLY] — Layer Determined or Override (Normal/Additive/Screen/Multiply)
  - Alpha Type dropdown [PROGRAMMING-ONLY] — Premultiplied / Straight
  - R G B A toggles [PROGRAMMING-ONLY] — channel visibility toggles
**Parameters:**
  - Clip.clipOpacity (float, 0-1, default 1.0)
  - Clip.clipWidth/clipHeight (int, default 1920x1080)
  - Clip.blendOverride (enum: LayerDetermined=0, Override=1)
  - Clip.alphaType (enum: Premultiplied=0, Straight=1)
  - Clip.channelR/G/B/A (bool, all default true)
**Bindings:** Opacity signal-connectable, MIDI velocity maps to opacity on trigger
**Programming mode:** Full controls visible
**Presentation mode:** HIDDEN except opacity
**Dependencies:** Layer blend mode (overridden when blendOverride = Override)

#### 4.4.7 Clip Effects Stack

**Level:** PRIMARY
**Current UI:** Clip Inspector > Effects Stack section
**Sub-features:**
  - Effect list [PRESENTATION-HIDDEN] — vertical list of applied effects
  - Per-effect bypass toggle (B) [PRESENTATION-HIDDEN] — enable/disable individual effect
  - Per-effect dry/wet slider [PRESENTATION-HIDDEN] — blend original with effected (0-1)
  - Per-effect parameters [PRESENTATION-HIDDEN] — signal-connectable sliders, right-click to reset
  - Drag-drop from FX Browser [PROGRAMMING-ONLY] — add effect to chain
  - Effect ordering [PROGRAMMING-ONLY] — drag to reorder within stack
**Parameters:** Per-effect: EffectSlot.effectName, EffectSlot.paramValues[], EffectSlot.dryWet, EffectSlot.enabled, EffectSlot.bypassed
**Bindings:** Toggle Effect Bypass (keyboard/MIDI), all params signal-connectable
**Programming mode:** Full stack with all controls, add/remove/reorder
**Presentation mode:** HIDDEN — effects run automatically; controlled via mappings/macros
**Dependencies:** Effect Chain Architecture (per-clip scope, applied before keying/blend), FX Browser (source for adding effects)

---

### 4.5 Autopilot System

**Level:** PRIMARY
**Current UI:** Clip Inspector > Autopilot, Layer Inspector > Autopilot, Composition Inspector > Autopilot + Per-Type Autopilot

#### 4.5.1 Per-Clip Autopilot

**Level:** PRIMARY
**Current UI:** Clip Inspector > Autopilot section
**Sub-features:**
  - Action dropdown [PROGRAMMING-ONLY] — determines what happens when duration expires
    - LayerDetermined (default) — inherit from layer settings
    - DoNothing — no auto-advance
    - PlayNext — advance to next clip in layer
    - PlayPrevious — go to previous clip
    - PlayRandom — random clip selection
    - PlayFirst — jump to first clip
    - PlayLast — jump to last clip
    - PlaySpecific — jump to a specific column
  - Duration dropdown [PROGRAMMING-ONLY] — how long before advancing
    - LayerDetermined (default) — inherit from layer
    - 1/4, 1/2, 1, 2, 4, 8, 16, 32 beats
    - Custom (integer beat count)
  - Beat Snap dropdown [PROGRAMMING-ONLY] — timing quantization for clip launch (Off / Beat / Bar / 2 Bar / 4 Bar)
**Parameters:**
  - Clip.autopilotAction (enum, 8 values)
  - Clip.autopilotSpecificCol (int, for PlaySpecific)
  - Clip.autopilotDuration (enum, 9 values including LayerDetermined and Custom)
  - Clip.autopilotCustomBeats (int, default 4)
  - Clip.beatSnapMode (enum, 5 values)
**Bindings:** Not directly bindable
**Programming mode:** Full controls visible in Clip Inspector
**Presentation mode:** HIDDEN — runs automatically
**Dependencies:** BPM Tracker (beat timing), Layer Autopilot (LayerDetermined fallback)

#### 4.5.2 Per-Layer Autopilot

**Level:** PRIMARY
**Current UI:** Layer Inspector > Autopilot section
**Sub-features:**
  - Direction buttons [PROGRAMMING-ONLY] — Rewind / Off / Forward / Random
  - Trigger mode [PROGRAMMING-ONLY] — End of Video or On Beat
  - Beat count [PROGRAMMING-ONLY] — 1/2/4/8/16/32 beats (On Beat mode only)
  - Loops slider (1-99) [PROGRAMMING-ONLY] — number of clip loops before advancing
  - Autopilot enable toggle [ALWAYS-VISIBLE] — master on/off per layer
**Parameters:**
  - Layer.autopilotEnabled (bool)
  - Layer.defaultAutopilotAction (AutopilotAction enum)
  - Layer.defaultAutopilotDuration (AutopilotDuration enum)
  - Layer.defaultAutopilotCustomBeats (int, default 4)
  - Layer.autopilotLoops (int, default 1)
  - Layer.autopilotEndOfVideo (bool)
**Bindings:** Toggle Layer Autopilot (keyboard/MIDI)
**Programming mode:** Full autopilot controls visible
**Presentation mode:** HIDDEN — autopilot state controlled via binding toggle
**Dependencies:** BPM Tracker, Clip Autopilot (per-clip overrides layer defaults)

#### 4.5.3 Per-Composition Autopilot

**Level:** SECONDARY
**Current UI:** Composition Inspector > Autopilot section
**Sub-features:**
  - Direction buttons [PROGRAMMING-ONLY] — Rewind / Off / Forward / Random
  - Duration mode [PROGRAMMING-ONLY] — Longest Clip / Clip Transport / Custom
  - Clip Loops slider (1-99) [PROGRAMMING-ONLY] — loops before advancing
  - Loop toggle [PROGRAMMING-ONLY] — restart sequence when complete
  - Master Layer dropdown [PROGRAMMING-ONLY] — which layer drives autopilot timing
**Parameters:**
  - Composition.autopilotDirection (enum: Rewind=0, Off=1, Forward=2, Random=3)
  - Composition.autopilotDurationMode (enum: LongestClip=0, ClipTransport=1, Custom=2)
  - Composition.autopilotClipLoops (int, default 1)
  - Composition.autopilotLoop (bool)
  - Composition.autopilotMasterLayer (int, -1 = Off)
**Bindings:** Not directly bindable
**Programming mode:** Full controls in Composition Inspector
**Presentation mode:** HIDDEN
**Dependencies:** Layer Autopilot (composition-level overrides), BPM Tracker

#### 4.5.4 Per-Type Autopilot

**Level:** SECONDARY
**Current UI:** Composition Inspector > Per-Type Autopilot section
**Sub-features:**
  - Enable toggle [PROGRAMMING-ONLY] — activate per-type cycle timers (when off, uses per-layer autopilot)
  - Opaque Beats (1-64) [PROGRAMMING-ONLY] — beat count for opaque layer cycling
  - Opaque Play Until End toggle [PROGRAMMING-ONLY] — wait for video end before advancing
  - Transparent Beats (1-64) [PROGRAMMING-ONLY] — beat count for transparent layer cycling
  - Transparent Max Layers [PROGRAMMING-ONLY] — max simultaneous transparent layers
  - Transparent Randomize toggle [PROGRAMMING-ONLY] — random vs sequential clip advance
  - Effect Beats (1-64) [PROGRAMMING-ONLY] — beat count for FX layer cycling
  - Effect Max Layers [PROGRAMMING-ONLY] — max simultaneous FX layers
  - Effect Randomize toggle [PROGRAMMING-ONLY] — random vs sequential
  - Global Randomize [PROGRAMMING-ONLY] — override all types to random
  - Loop Autopilot [PROGRAMMING-ONLY] — restart when complete
**Parameters:**
  - PerTypeAutopilotConfig.perTypeEnabled (bool, default false)
  - PerTypeAutopilotConfig.opaqueCycleBeats (int, default 16)
  - PerTypeAutopilotConfig.opaquePlayUntilEnd (bool)
  - PerTypeAutopilotConfig.transparentCycleBeats (int, default 8)
  - PerTypeAutopilotConfig.transparentMaxLayers (int, default 2)
  - PerTypeAutopilotConfig.transparentRandomize (bool, default true)
  - PerTypeAutopilotConfig.effectCycleBeats (int, default 4)
  - PerTypeAutopilotConfig.effectMaxLayers (int, default 2)
  - PerTypeAutopilotConfig.effectRandomize (bool, default true)
  - PerTypeAutopilotConfig.globalRandomize (bool)
  - PerTypeAutopilotConfig.loopAutopilot (bool, default true)
**Bindings:** Not directly bindable
**Programming mode:** Full controls in Composition Inspector
**Presentation mode:** HIDDEN — runs automatically once enabled
**Dependencies:** Layer Types (different beat counts per type), BPM Tracker

---

### 4.6 Genre Detection & Smart Features

**Level:** SECONDARY
**Current UI:** Minimal — data exposed in FeatureSnapshot, genre/energy state accessible via Signal Bar; automation config in Composition Inspector
**Sub-features:**
  - 8-genre classifier [PRESENTATION-HIDDEN] — real-time genre detection from multi-feature scoring
    - House (0) — steady 4-on-floor kick, 120-130 BPM, warm bass
    - Techno (1) — driving, 125-145 BPM, high transient density, spectral flux
    - DnB (2) — fast breakbeats, 160-180 BPM, heavy bass, syncopation
    - Hip-Hop (3) — slower groove, 80-100 BPM, strong bass + mids
    - Ambient (4) — sparse, low transient density, spectral flatness, sustained tones
    - Rock (5) — full spectrum, high peak levels, guitar frequency presence
    - Pop/Electronic (6) — varied, mid-range BPM, bright, high spectral centroid (default)
    - Jazz/Other (7) — complex harmony, variable BPM, chromatic complexity
  - 3 energy states [PRESENTATION-HIDDEN] — Low(0), Medium(1), High(2); EMA-smoothed
  - Structural detection (4 states) [PRESENTATION-HIDDEN] — multi-scale EMA (100ms/1s/4s/16s) state machine
    - Normal (0) — standard playback
    - Buildup (1) — rising energy/tension
    - Drop (2) — high-energy peak
    - Breakdown (3) — low-energy respite
  - Auto-preset on genre change [PROGRAMMING-ONLY] — auto-switch decks when genre changes
  - Genre-deck assignment [PROGRAMMING-ONLY] — map each genre to a specific deck index (-1 = no switch)
  - Genre effect presets [PROGRAMMING-ONLY] — named FX preset to load per genre
  - Smart random autopilot [PROGRAMMING-ONLY] — energy-aware clip selection (lower column = calmer, higher = intense)
  - Structural scene triggering [PROGRAMMING-ONLY] — auto-switch decks on structural transitions (drop, breakdown)
  - AI mapping suggestions [NO UI — code only] — MappingSuggester provides genre-aware source-to-param recommendations with scored confidence (ghost feature)
**Parameters:**
  - Composition.autoPresetOnGenre (bool, default false)
  - Composition.smartAutopilotEnabled (bool, default false)
  - Composition.structuralSceneEnabled (bool, default false)
  - Composition.genreDeckAssignment[8] (int array, default all -1)
  - Composition.genrePresetNames[8] (string array)
  - GenreDetector smoothing: ~2s EMA + ~3s hysteresis
  - Per-genre EMA alpha: Techno/DnB = 0.50-0.55 (fast attack), Ambient = 0.12 (slow)
**Bindings:** Not directly bindable (genre/energy exposed as signals for routing)
**Programming mode:** Genre automation config visible in Composition Inspector
**Presentation mode:** HIDDEN — genre detection and smart features run automatically
**Dependencies:** Audio Analysis Pipeline (provides features to GenreDetector), Autopilot (smart random uses energy state), Structural Detector (provides 4 structural states)

---

### 4.7 Crossfader

**EXCLUDED** — Boris decided NOT to have this feature in the design overhaul.

Note: Crossfader code EXISTS in the Composition model (crossfaderPhase, crossfaderBlendMode, crossfaderBehaviour, crossfaderCurve fields) but has no UI and is not wired to rendering. It is listed in GAP_REPORT.md as a ghost feature. Do NOT include in the design.

---

## Domain 8: UI Framework

---

### 8.1 Top Bar

**Level:** PRIMARY
**Current UI:** Top Bar — 34px height, full width, always visible
**Sub-features:**
  - Audio source dropdown [PROGRAMMING-ONLY] — Mic Input or Audio File selection
  - Gain slider (0-4x) [ALWAYS-VISIBLE] — input level amplification; right-click resets to 1.0
  - Play button [ALWAYS-VISIBLE] — start global playback
  - Pause button [ALWAYS-VISIBLE] — pause global playback
  - Stop button [ALWAYS-VISIBLE] — halt all playback
  - Beat wheel [ALWAYS-VISIBLE] — 4-segment circle, visual beat position indicator (beats 1-4)
  - Bar/Phrase display [ALWAYS-VISIBLE] — text showing "Bar N" and "Phr 0.XX"
  - BPM display (18px) [ALWAYS-VISIBLE] — current tempo, color-coded: gray=searching, yellow=locking, green=locked
  - Tracker state label [ALWAYS-VISIBLE] — "SEARCHING", "LOCKING", or "LOCKED"
  - Tap button [ALWAYS-VISIBLE] — tap tempo (accumulates 8 taps, computes BPM)
  - Resync button [ALWAYS-VISIBLE] — reset beat phase to align with current audio
  - Manual toggle [ALWAYS-VISIBLE] — freeze BPM detection, enter manual value
  - BPM edit field [MINIMAL-IN-PRESENTATION] — manual BPM entry (visible only in manual mode)
  - BPM multiplier buttons (/4, /2, x1, x2, x4) [ALWAYS-VISIBLE] — 5 buttons for tempo scaling
  - Quantize dropdown [ALWAYS-VISIBLE] — Off / Next Beat / Next Downbeat for clip launch timing
  - Fade slider (0-5s) [ALWAYS-VISIBLE] — global transition speed between clips
  - Master slider (0-1) [ALWAYS-VISIBLE] — master output brightness/opacity
  - Output dropdown [ALWAYS-VISIBLE] — select output display (Off / Fullscreen / Windowed)
  - FPS label [MINIMAL-IN-PRESENTATION] — real-time frame rate counter
  - DSP label [MINIMAL-IN-PRESENTATION] — CPU load percentage
**Parameters:**
  - Audio source mode (int: Mic=1, File=2)
  - Input gain (float, 0-4x, default 1.0)
  - Global transport state (play/pause/stop)
  - BPM (float, from tracker or manual)
  - BPM multiplier (int: -4, -2, 1, 2, 4)
  - Manual BPM mode (bool)
  - Quantize mode (enum: Off=0, NextBeat=1, NextDownbeat=2)
  - Global transition speed (float, 0-5s, default 0.3s)
  - Master opacity (float, 0-1, default 1.0)
  - Output display selection (int, -1=off, 0+=display index)
**Bindings:** Tap Tempo / Resync (keyboard/MIDI), Global Play/Pause/Stop (keyboard/MIDI), Master Opacity (MIDI CC)
**Programming mode:** All 19 controls visible
**Presentation mode:** MINIMAL — BPM display, beat wheel, master slider, output dropdown always needed; audio source, gain, quantize could be hidden
**Dependencies:** Audio I/O (source selection, gain), BPM Tracker (tempo display/control), Render Pipeline (master opacity, FPS/DSP stats), Output (display selection)

---

### 8.2 Signal Bar

**Level:** PRIMARY
**Current UI:** Signal Bar — horizontal strip below Top Bar, collapsible
**Sub-features:**
  - Minimized mode (~20px) [ALWAYS-VISIBLE] — tiny meter bars only
  - Normal mode (~80px) [ALWAYS-VISIBLE] — label + meter bar + peak hold + numeric value per signal
  - Expanded mode (fill) [ALWAYS-VISIBLE] — full oscilloscope / histogram per signal; used by ProgrammingMode toggle
  - Add signal button [+] [PROGRAMMING-ONLY] — add new signal (oscillator, envelope, etc.)
  - Shrink/Grow buttons [-] / [+] [ALWAYS-VISIBLE] — resize all strips
  - Click strip to open Signal Inspector [PROGRAMMING-ONLY]
  - Default signals (12 visible): RMS, Peak, Bass, Mid, High, Beat Phase, Bar Phase, BPM, Onset Strength, Dominant Pitch, Musical Key, Structural State
  - Hidden signals (25): advanced audio features (P25 — sidechain pump, swing ratio, formant, resonance, reese, etc.) registered but not visible in default signal bar
  - Modulation signals (3): oscillators, envelopes, clip position
**Parameters:**
  - Display size (enum: Minimized=0, Normal=1, Expanded=2)
  - Per-signal: threshold (0-1), gain (0-4x), falloff (0-1s) — for audio signals
  - Per-oscillator: wave shape, beat duration, amplitude, phase offset
  - Per-envelope: curve type, beat duration, amplitude, phase, looping, one-shot
**Bindings:** Not directly bindable (signals themselves are binding sources)
**Programming mode:** Normal or Expanded; click strips to configure in Signal Inspector
**Presentation mode:** MINIMIZED (26px) or HIDDEN — meters provide visual feedback but take screen space
**Dependencies:** Audio Analysis Pipeline (all audio signals), Signal Routing Engine (signal evaluation), BPM Tracker (beat/bar phase)

---

### 8.3 Deck Grid

**Level:** PRIMARY
**Current UI:** Center area — the main performance workspace
**Sub-features:**
  - Layer strips (left 250px) [ALWAYS-VISIBLE] — dense Resolume-style per-layer headers with action buttons, transport, sliders, dropdowns
  - Clip cells (90x96px each) [ALWAYS-VISIBLE] — thumbnail area (76px) + name bar (20px); visual states: empty, loaded, active (cyan border), selected (white border), missing (red border + "!"), locked (orange "L"), has-effects (FX count badge)
  - Column trigger buttons (22px, top) [ALWAYS-VISIBLE] — numbered 1..K, click fires all clips in column; active column highlighted cyan
  - Deck tabs (24px, bottom) [ALWAYS-VISIBLE] — one per deck, click to switch active deck
  - Horizontal scrolling [ALWAYS-VISIBLE] — for columns exceeding visible area
  - Vertical scrolling [ALWAYS-VISIBLE] — for layers exceeding visible area
  - Layer display order [ALWAYS-VISIBLE] — highest layer at top (like Photoshop/Resolume)
  - Clip cell interactions [ALWAYS-VISIBLE]:
    - Click thumbnail: trigger/retrigger clip
    - Click name bar: select for inspector editing (Cmd/Shift for multi-select)
    - Drag name bar: move clip to another cell
    - Drag file from OS: load image, video, or image sequence
    - Drag from FX Browser: add effect to clip
    - Drag from Sources Browser: create procedural source clip
    - Drag from MilkDrop Browser: create MilkDrop preset clip
  - Default grid: 3 layers x 12 columns
**Parameters:**
  - Deck.numColumns (int, default 12)
  - Deck.layers.size() (int, default 3)
  - Scroll position (runtime, not persisted)
**Bindings:** Trigger Clip at position (keyboard/MIDI), Trigger Column (keyboard/MIDI)
**Programming mode:** Full grid visible with all interactions
**Presentation mode:** VESTIGIAL — current ProgrammingMode hides entire grid when signal bar is expanded. No graduated visibility exists.
**Dependencies:** Layer Controls (layer strip content), Clip Controls (cell content), Autopilot (active clip highlighting), Drag-Drop system (media/effects loading)

---

### 8.4 Inspector Panel

**Level:** PRIMARY
**Current UI:** Bottom panels area (center-right, ~25%), 4 tabs
**Sub-features:**
  - Clip tab [PROGRAMMING-ONLY] — 9 sections: Name, Dashboard (8 Link Knobs), Transport, Cuepoints, Autopilot, Source Parameters, Video, Transform, Effects Stack
  - Layer tab [PROGRAMMING-ONLY] — 12 sections: Name, Dashboard (8 Link Knobs), Autopilot, Layer Master, Video, Transition, Keying, Dry/Wet, 3D Controls, Transform, Feedback, Layer Effects
  - Composition tab [PROGRAMMING-ONLY] — 9 sections: Name+Resolution, Dashboard (8 Link Knobs), Autopilot, Per-Type Autopilot, Composition Master, Video, Transform, Global Effects, Output Settings
  - Signal tab [PROGRAMMING-ONLY] — shows Signal Inspector for selected signal (Audio/Oscillator/Envelope controls)
  - Pin button [ALWAYS-VISIBLE] — prevent auto-tab-switching during performance
  - Auto-tab-switching [ALWAYS-VISIBLE] — click clip = Clip tab, click layer = Layer tab, click signal = Signal tab
**Parameters:**
  - Active tab (int, 0-3)
  - Pinned state (bool)
**Bindings:** Not directly bindable
**Programming mode:** Full inspector with all tabs and sections
**Presentation mode:** HIDDEN — inspector is for configuration, not performance. Pin button allows selective access.
**Dependencies:** All Composition/Layer/Clip controls (inspector surfaces their parameters), Signal Routing (Signal tab), Universal Parameter Control (all sliders use UPC widget)

---

### 8.5 Browser Panel

**Level:** PRIMARY
**Current UI:** Bottom panels area (right, ~25%), 6 tabs
**Sub-features:**
  - Files tab [PROGRAMMING-ONLY] — grid/list view with 64px thumbnails, path bar, search, favorites, drag-to-deck
  - FX tab [PROGRAMMING-ONLY] — 135 effects across 11 categories (Warp 28, Color 31, Glitch 15, Blur/Post 10, 3D/Depth 10, Pattern 15, Animation 4, Time 6, Audio 7, Blend 5, Composite 3); search field, multi-select, drag to cells/inspectors/stacks
  - Sources tab [PROGRAMMING-ONLY] — 101 procedural sources across 15+ categories; search, multi-select, drag to deck cells
  - Comp-Decks tab [PROGRAMMING-ONLY] — two sections (Compositions and Decks) with Save/Load/Delete for each
  - Record tab [PROGRAMMING-ONLY] — Record/Stop buttons, Play button, Save/Load as JSON, Browse Output Folder, Status/Event count display
  - MilkDrop tab [PROGRAMMING-ONLY] — ~9,800 presets via projectM; 4 sub-tabs (Curated 30 / Favorites / Recent 20 / All searchable); 3 play modes (Jukebox, VJ Clip, Playlist); Jukebox controls: Play/Stop, Pool dropdown, Timing (4/8/16/32 beats or 10/30/60 seconds), Blend slider; Navigation: < / > / ? (random) / Lock buttons
**Parameters:**
  - Active tab (int, 0-5)
  - Files: current path, view mode (grid/list), search query
  - FX: search query, expanded categories
  - Sources: search query, expanded categories
  - MilkDrop: active sub-tab, Jukebox mode/timing/pool/blend, lock state
**Bindings:** Not directly bindable (content accessed via drag-drop)
**Programming mode:** Full browser with all tabs
**Presentation mode:** HIDDEN — browser is for content loading during programming
**Dependencies:** Effect Library (FX tab content), Source Registry (Sources tab content), PresetManager (Comp-Decks save/load), SessionRecorder (Record tab), MilkDrop/projectM (MilkDrop tab)

---

### 8.6 Timing Window

**Level:** UTILITY
**Current UI:** Bottom panels area (center-left, ~28%), 3 tabs
**Sub-features:**
  - BPM tab [PLACEHOLDER] — intended for BPM visualization, content is placeholder
  - Routing tab [PLACEHOLDER] — intended for signal routing display, content is placeholder
  - Oscillators tab [PLACEHOLDER] — intended for oscillator waveform display, content is placeholder
**Parameters:** None functional
**Bindings:** None
**Programming mode:** Tabs visible but empty
**Presentation mode:** HIDDEN — no functional content
**Dependencies:** BPM Tracker (BPM tab would display), Signal Routing (Routing tab would display), Oscillator signals (Oscillators tab would display)

---

### 8.7 Programming Mode

**Level:** UTILITY
**Current UI:** View menu > Programming Mode toggle
**Sub-features:**
  - Toggle signal bar expansion [ALWAYS-VISIBLE] — switches SignalBar between Normal (84px) and Expanded (fills everything)
  - Panel hiding [ALWAYS-VISIBLE] — when expanded, all other panels (deck, inspector, browser, preview, timing) are setVisible(false)
**Parameters:**
  - Programming mode active (bool)
**Bindings:** Not directly bindable (menu toggle only)
**Programming mode:** Toggle via View menu
**Presentation mode:** VESTIGIAL — the ProgrammingMode component itself is always setVisible(false) in MainComponent::resized(). The dual-mode system that would provide graduated control visibility DOES NOT EXIST YET. Current implementation is binary: see everything OR see only expanded signal bar.
**Dependencies:** Signal Bar (expansion target), MainComponent (panel visibility management)

---

### 8.8 Preferences

**Level:** UTILITY
**Current UI:** Audio-DNA menu > Preferences (dialog window), 8 tabs
**Sub-features:**
  - General tab [PROGRAMMING-ONLY] — Confirm on quit, Show tooltips
  - Audio tab [PROGRAMMING-ONLY] — Sample rate, Buffer size, BPM detection range
  - Video tab [PROGRAMMING-ONLY] — FPS target, Render resolution, MilkDrop preset directory
  - MIDI tab [PLACEHOLDER] — placeholder, no functional content
  - Recording tab [PLACEHOLDER] — placeholder, no functional content
  - Defaults tab [PLACEHOLDER] — placeholder, no functional content
  - Feedback tab [PLACEHOLDER] — placeholder, no functional content
  - About tab [PROGRAMMING-ONLY] — Version, credits
**Parameters:**
  - General: confirmOnQuit (bool), showTooltips (bool)
  - Audio: sampleRate (int), bufferSize (int), bpmDetectionRange (range)
  - Video: fpsTarget (int), renderResolution (WxH), milkDropDir (path)
**Bindings:** None
**Programming mode:** Full preferences dialog
**Presentation mode:** HIDDEN — settings only changed outside performance
**Dependencies:** Audio I/O (audio settings), Render Pipeline (video settings), MilkDrop (preset directory)

---

### 8.9 Hidden v1 Components

**Level:** UTILITY
**Current UI:** NO UI — code exists but setVisible(false) in v2 layout

#### AudioReadoutPanel [HIDDEN-V1]
- Full audio feature readout panel from v1 layout
- Shows all audio analysis features as text/meters
- Hidden in v2 layout, superseded by Signal Bar

#### SpectrumDisplay [HIDDEN-V1]
- 7-band energy visualization from v1 layout
- Bar graph of sub/bass/lowmid/mid/highmid/presence/brilliance
- Hidden in v2 layout, partially replaced by Signal Bar band meters

#### EffectsRackPanel [HIDDEN-V1]
- v1 effects panel with knobs and mapping UI
- Superseded by EffectStackView in v2 inspector
- Hidden in v2 layout

#### v1 Controls in MainComponent [HIDDEN-V1]
- audioSourceSelector_, inputGainSlider_, masterLevelSlider_, displaySelector_
- resolutionSelector_, randomLabel_, beatRandomToggle_, beatCountSelector_
- syncButton_, fpsLabel_, cpuLabel_
- openImageButton_, fileLabel_, imageSequenceButton_
- v1 preset slots (10 buttons + dropdowns)
- v1 file-loading controls
- All setVisible(false) in v2 layout but code remains

**Parameters:** Various (all from v1 architecture)
**Bindings:** None in v2
**Programming mode:** Invisible
**Presentation mode:** Invisible
**Dependencies:** None in v2 (orphaned code)

---

## Domain 9: Data & Persistence

---

### 9.1 Preset Management

**Level:** PRIMARY
**Current UI:** Browser Panel > Comp-Decks tab (Save/Load/Delete for compositions and decks) + Composition menu (New/Open/Save/Save As)
**Sub-features:**
  - Save composition [ALWAYS-VISIBLE] — Cmd+S, full hierarchy serialized to JSON: all decks, layers, clips, effects, mappings, global settings, genre automation, transforms
  - Open composition [ALWAYS-VISIBLE] — Cmd+O, deserializes JSON back to full app state
  - Save As [PROGRAMMING-ONLY] — save to new file path
  - New composition [PROGRAMMING-ONLY] — reset to default state (1 deck, 3 layers, 12 columns)
  - FX preset save/load [PROGRAMMING-ONLY] — save/load effect chain + mappings as separate JSON (PresetManager)
  - Deck template save/load [PROGRAMMING-ONLY] — save/load deck state including audio path, image path, FX, mappings, slot assignments, keyboard layout (PresetManager.DeckState)
  - Collect Media [PROGRAMMING-ONLY] — package composition + all referenced media files into one folder
  - Relocate Files [PROGRAMMING-ONLY] — find and relink missing media file references
**Parameters:**
  - Composition.filePath (juce::File — current save location)
  - Composition.name (string)
**Bindings:** Not directly bindable (menu actions)
**Programming mode:** Full file management via menu and Comp-Decks browser tab
**Presentation mode:** HIDDEN — save operations happen between performances. Cmd+S always available.
**Dependencies:** Composition Serialization (JSON format), Effect Chain (FX preset content), Mapping Engine (mapping preset content)

---

### 9.2 Composition Serialization

**Level:** PRIMARY
**Current UI:** NO UI — code only (Composition::toVar/fromVar, Deck::toVar/fromVar, Layer::toVar/fromVar, Clip::toVar/fromVar)
**Sub-features:**
  - Composition to JSON [ALWAYS-VISIBLE] — Composition.toVar() serializes name, activeDeckIndex, masterOpacity, globalTransitionSpeed, bpmMultiplier, quantizeMode, outputWidth/Height/Display, all decks, global effects
  - Deck to JSON [ALWAYS-VISIBLE] — Deck.toVar() serializes name, id, numColumns, all layers
  - Layer to JSON [ALWAYS-VISIBLE] — Layer.toVar() serializes name, id, type, all layer fields (opacity, blend mode, keying, transition, transform, feedback, autopilot, effects, clips)
  - Clip to JSON [ALWAYS-VISIBLE] — Clip.toVar() serializes name, id, mediaType, mediaFile, sourceType, sourceParams, effects, transport, in/out points, beat snap, cuepoints, autopilot, video properties, transform, MilkDrop playlist, content lock
  - JSON parse/write [ALWAYS-VISIBLE] — via JUCE var/DynamicObject + JSON::toString/parse
  - File I/O [ALWAYS-VISIBLE] — Composition.saveToFile / loadFromFile (replaceWithText / loadFileAsString)
**Parameters:** All composition state serialized (see Data Hierarchy in source doc section 21)
**Bindings:** None (serialization is infrastructure)
**Programming mode:** Transparent — happens on save/load
**Presentation mode:** Transparent
**Dependencies:** JUCE (var, DynamicObject, JSON, File), all model types (Composition, Deck, Layer, Clip, EffectSlot)

**Known gaps:**
  - Composition.toVar() does NOT serialize: masterSpeed, compOpacity, crossfader fields, composition transform, autopilot config, per-type autopilot, genre automation config, genreDeckAssignment, genrePresetNames. These fields exist in the model but are missing from the serialization code.
  - PresetManager kSourceNames[] missing P25 advanced sources (SidechainPump, SwingRatio, etc.) — presets saved with P25 mappings fail to round-trip.

---

### 9.3 Layout Management

**Level:** UTILITY
**Current UI:** View menu > Save Layout / Load Layout / Reset Layout
**Sub-features:**
  - Save Layout [PROGRAMMING-ONLY] — save current workspace panel proportions and visibility to persistent storage
  - Load Layout [PROGRAMMING-ONLY] — restore saved panel arrangement
  - Reset Layout [PROGRAMMING-ONLY] — return to default 4-panel proportions (22% preview / 28% timing / 25% inspector / 25% browser)
  - Panel show/hide toggles [PROGRAMMING-ONLY] — View menu: Signal Bar / Deck / Preview / Inspector / Browser / Timing Window — toggle individual panel visibility
  - FPS Stats toggle [PROGRAMMING-ONLY] — View menu: toggle performance overlay
**Parameters:**
  - Panel visibility states (bool per panel: signal bar, deck, preview, inspector, browser, timing window)
  - Panel proportions (float, 4 panels with 3 dividers)
  - FPS stats overlay (bool)
**Bindings:** None
**Programming mode:** Full layout management via View menu
**Presentation mode:** HIDDEN — layout is configured during setup
**Dependencies:** MainComponent (panel visibility), Bottom panels (divider proportions)

---

### 9.4 Undo/Redo

**Level:** PRIMARY
**Current UI:** Composition menu > Undo (Cmd+Z) / Redo (Cmd+Shift+Z)
**Sub-features:**
  - Undo [ALWAYS-VISIBLE] — Cmd+Z, reverses most recent command
  - Redo [ALWAYS-VISIBLE] — Cmd+Shift+Z, re-applies most recently undone command
  - History stack [PRESENTATION-HIDDEN] — 500-deep linear history using Command pattern
  - Undo/Redo descriptions [PRESENTATION-HIDDEN] — human-readable descriptions for each command
  - History change callback [PRESENTATION-HIDDEN] — fires onHistoryChanged for menu state updates (enable/disable Undo/Redo items)
  - Clear history [PROGRAMMING-ONLY] — wipe all history (e.g., on new composition)
  - Oldest commands discarded [PRESENTATION-HIDDEN] — when stack exceeds 500, oldest commands are removed
**Parameters:**
  - UndoManager.history_ (vector of Command, max 500)
  - UndoManager.currentIndex_ (int, points to next write slot)
**Bindings:** Cmd+Z / Cmd+Shift+Z (hardcoded keyboard shortcuts)
**Programming mode:** Undo/Redo always available via menu and shortcuts
**Presentation mode:** MINIMAL — keyboard shortcuts still work, menu items may be hidden
**Dependencies:** Command pattern (all state-changing operations must create Command objects), Composition model (state to undo/redo)

---

### 9.5 Binding Import/Export

**Level:** UTILITY
**Current UI:** Shortcuts menu > Export Bindings / Import Bindings
**Sub-features:**
  - Export Bindings [PROGRAMMING-ONLY] — save all keyboard and MIDI bindings to JSON file
  - Import Bindings [PROGRAMMING-ONLY] — load bindings from JSON file, replacing current bindings
  - Copy Effects / Paste Effects [PROGRAMMING-ONLY] — via Composition menu (clip-to-clip) and Layer menu (layer-to-layer effect chain transfer)
**Parameters:**
  - Binding data: per-binding keyCode or MIDI note/CC, action, targetMode, triggerMode, modifiers
**Bindings:** None (this IS the binding management)
**Programming mode:** Menu actions for import/export
**Presentation mode:** HIDDEN — bindings configured before performance
**Dependencies:** BindingManager (owns all binding data), MidiHandler (MIDI binding source), JSON serialization

---

### 9.6 Session Recording & Playback

**Level:** SECONDARY
**Current UI:** Browser Panel > Record tab
**Sub-features:**
  - Record button [ALWAYS-VISIBLE] — start capturing all parameter changes, clip triggers, transport changes as timestamped JSON events
  - Stop button [ALWAYS-VISIBLE] — end recording session
  - Play button [PROGRAMMING-ONLY] — playback recorded session (recreates the entire performance)
  - Save / Load [PROGRAMMING-ONLY] — export/import session recording as JSON
  - Browse Output Folder [PROGRAMMING-ONLY] — choose save directory
  - Status / Event count display [ALWAYS-VISIBLE] — recording state and number of events captured
**Parameters:**
  - Recording state (bool)
  - Output directory (path)
  - Event count (int, runtime)
**Bindings:** Toggle Recording (keyboard/MIDI binding available)
**Programming mode:** Full record panel with all controls
**Presentation mode:** MINIMAL — record/stop buttons and status needed; save/load/browse hidden
**Dependencies:** All model changes (events captured), JSON serialization (event format)
