# V6 Design Brief — Audio-DNA VJ Mockups

## Reference Image Analysis

These mockups MUST match the design language of the user's approved reference images (Audio Dna VJ 1-26). Key patterns observed:

### Layout Structure (most common pattern across 30 reference images)

```
┌──────────────────────────────────────────────────────────────────────┐
│ TOP BAR: Logo │ BPM: 128.00 │ TAP │ NUDGE± │ RESYNC │ ●●●● │ 037/04.1 │ Room Delay │
├──────────┬─────────────────────────────────┬─────────────────────────┤
│ SCENE    │  Intro│Verse│Build│Drop│Outro   │  SOURCES               │
│ FLOW     ├─────────────────────────────────┤  ▓ Master Audio  ~~~~  │
│          │                                 │  ♪ Kick Drum    ▄▄▄▄  │
│ Intro  T │    LIVE OUTPUT PREVIEW          │  ∿ LFO — Sine   ~~~   │
│ Verse  T │    (medium, ~500px wide)        │  ∿ LFO — Custom ~~~   │
│ Chorus T │                                 │  + Add Audio Source    │
│ Drop   T │    ▶ ‖ ■    ═══════════         │                        │
│ + Scene  │                                 │  Drag a source onto    │
│          │    GLOBAL MACROS                 │  a parameter.          │
│ Layers:  │    Hue Sat Bri Scale Speed FX   │                        │
│ L1: Clip │    (●) (●) (●) (●)  (●)  (●)   │  Layer Controls:       │
│ L2: Pat  │    1   2   3   4    5    6      │  Blend Mode  Normal ▼  │
│ L3: Wave │                                 │  H S B Spd knobs       │
├──────────┴─────────────────────────────────┴─────────────────────────┤
│ Layer 1 ▎ Layer 2 ▎ Layer 3 ▎ Layer 4 ▎ Layer 5 ▎ ...  + Add Layer │
├──────────────────────────────────────────────────────────────────────┤
│ LAYER STRIPS (one active layer expanded)                             │
│ ┌──────┐ ┌──────┐ ┌──────┐ ┌──────┐                                │
│ │VIDEO │ │IMAGE │ │SYNTH │ │VIDEO │  + New Clip                     │
│ │ClipA │ │Pat B │ │WaveC │ │ClipD │                                │
│ ├──────┤ ├──────┤ ├──────┤ ├──────┤                                │
│ │ H S B│ │ H S B│ │ H S B│ │ H S B│   ← Hue/Sat/Brightness knobs  │
│ │ Spd  │ │ Spd  │ │ Spd  │ │ Spd  │                                │
│ │══════│ │══════│ │══════│ │══════│   ← Opacity slider              │
│ │FX Mac│ │FX Mac│ │FX Mac│ │FX Mac│   ← FX/Macro buttons           │
│ │Strobe│ │Strobe│ │Strobe│ │Strobe│                                │
│ │Expand│ │Expand│ │Expand│ │Expand│                                │
│ └──────┘ └──────┘ └──────┘ └──────┘                                │
├──────────────────────────────────────────────────────────────────────┤
│ FOOTER: Master Sources ▼ │ Audio Player ▶ │ Outputs ▼ │ Save Load Help │
└──────────────────────────────────────────────────────────────────────┘
```

### Critical Design Rules (from reference images)

1. **Clip thumbnails are SQUARE** — not rectangles. Roughly 80-100px squares with rounded corners (2-4px radius)
2. **Engine type badges** — each clip shows its type: VIDEO (red/orange), IMAGE (blue), SYNTH (green/cyan) as a colored label badge
3. **Scene Flow** — left panel with structural sections (Intro/Verse/Chorus/Drop/Outro), each with Trigger button
4. **Sources panel** — right panel with audio sources (Master Audio, Kick Drum, LFO-Sine, LFO-Custom), each with mini waveform/spectrum visualization
5. **Layer Strips** — horizontal, at the bottom. Each clip within a layer has: type badge, clip name, Hue/Sat/Bri/Speed knobs (small rotary), opacity slider (horizontal), FX button, Macro button, Strobe toggle, Expand button
6. **Layer tabs** — horizontal tabs across the layer strip area (Layer 1, Layer 2... Layer 8+)
7. **Global Macros** — numbered knobs (1-7) for Hue/Sat/Bright/Scale/Speed/FX
8. **Preview** — medium-sized (not tiny, not huge), center area, ~500-600px wide. With transport controls below it (play/pause/stop + timeline scrubber)
9. **Top bar** — BPM (bold, large), TAP, NUDGE±, RESYNC, beat dots (●●●●), bar/phrase counter (037/04.1), room delay
10. **Footer** — Master Sources, Audio Player, Outputs, Save/Load/Help
11. **Background** — NOT pure flat black. Use subtle dark gradients (#1a1e22 to #151820), slight texture feel
12. **Controls** — rotary knobs (small circles with arc indicators), NOT just flat sliders
13. **Color scheme** — dark base with amber/gold accents for active states, cyan/teal for selection, green for audio meters, red for alerts
14. **Scene columns** in the clip grid (Intro, Verse, Build, Drop, Outro) — clips organized by song structure
15. **S M buttons** per layer — Solo and Mute (not B/S like current app)
16. **Knobs use colored arcs** — showing value as a partial circle arc around the knob
17. **Level meters** — green-to-red vertical bars next to audio sources
18. **The "Sources" panel uses tabs**: Sources | Layer | Clip — different contexts for the right panel

### What the Current App Has (from OurAppFeatures_1.md)

Read /Users/boriskarpman/Documents/RealTimeAudio/research/OurAppFeatures_1.md for the complete feature inventory. Key items to include:

- Top bar with BPM/transport/quantize/fade/master/output controls
- Signal bar (collapsible audio meters)
- Deck grid (layers × columns with clip cells)
- Layer strips with S/K/V/F sliders, transport, blend mode
- Inspector (Clip/Layer/Composition/Signal tabs)
- Browser (Files/FX/Sources/Comp-Decks/Record/MilkDrop)
- 135 effects, 81 sources, 80 audio features
- Signal routing with 24 curve types
- 8 dashboard macro knobs per scope
- Binding system (keyboard + MIDI)
- Session recording, video recording, Syphon output

### CSS/HTML Requirements

- 1920x1080 fixed size
- Self-contained HTML with inline CSS
- Realistic content (real effect names, source names, parameter values)
- Square clip thumbnails with CSS gradient backgrounds simulating visuals
- Rotary knob controls drawn with CSS (border-radius: 50%, arc indicators)
- Subtle background gradients, not flat black
- Must look like a REAL application screenshot, not a wireframe
