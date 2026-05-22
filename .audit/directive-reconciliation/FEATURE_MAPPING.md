# Feature Mapping by UI Screen — Audio-DNA

> For next session: evaluate features, organize outline, create mockup pages.
> Maps every directive component to codebase state. Organized by screen.
> Written 2026-05-21.

---

## Status Legend
- **EXISTS** — fully implemented
- **PARTIAL** — partially implemented
- **NOT YET** — needs building
- **GHOST** — code exists but no UI path

---

## SCREEN 1: Top Chrome (98px)

| Component | Directive | Status | Current | Target | Phase |
|-----------|-----------|--------|---------|--------|-------|
| BPM Display (48px) | 5.1, 4.2 | PARTIAL | 18pt in TopBar | 48px Plex Mono Medium, blue, dominant | 4 |
| Beat Wheel (4 squares) | 5.1, 7 | NOT YET | No component | 4 squares in row (14x14px), active=kAccent fill | 4 |
| Bar/Phrase Counter | 5.1 | EXISTS | In TopBar | Already visible | - |
| Structural State Pill | 1.9, 5.1 | NOT YET | In analysis only | Blue pill on top bar | 4 |
| Transport Strip (32px) | 5.1 | PARTIAL | In TopBar (34px) | Separate 32px strip | 4 |
| Signal Bar (22px) | 1.6, 5.1 | PARTIAL | 26px minimized | 22px, clickable overlay | 4, 7 |
| Mode Tabs (LIVE/PROG/SETUP) | 5.4 | PARTIAL | Vestigial | Blue text+underline, full mode switching | 5 |
| Layout Hotkeys | 5.5 | NOT YET | No UI | Template buttons top-right | 6 |
| Pulse AI Button | 3.1 | NOT YET | No component | Top-right icon | 13 (deferred) |
| FPS/Stats | 5.1 | EXISTS | In TopBar | No change needed | - |
| Master Slider | 5.1 | PARTIAL | Exists in model | Needs top bar UI | 4 |
| REC Indicator | 5.1 | EXISTS | Works | kDanger color | 1 |

---

## SCREEN 2: Signal Bar / Signal Drawer

| Component | Directive | Status | Current | Target | Phase |
|-----------|-----------|--------|---------|--------|-------|
| Collapsed Bar (22px) | 1.6 | PARTIAL | 26px, not clickable | 22px, click to expand overlay | 4, 7 |
| 15+ Live Values | 1.6, 1.8 | PARTIAL | 8 visible, 21 hidden | All visible in expanded | 7 |
| Full Overlay Drawer | 1.6 | NOT YET | Expanded=hides all | Overlay ON TOP of workspace | 7 |
| Signal Configuration | 1.6 | NOT YET | No config UI | Signal group editing in drawer | 7, 8 |

---

## SCREEN 3: Deck Grid

| Component | Directive | Status | Current | Target | Phase |
|-----------|-----------|--------|---------|--------|-------|
| Clip Cells | 5.9 | PARTIAL | 90x96px | 100x80px, 6px wave, 14px name | 14 |
| Mini Waveform (6px left) | 5.9 | NOT YET | No component | Per-clip audio waveform strip | 14 |
| Layer Strips | 5.10 | PARTIAL | 250px, basic controls | +Signal columns left side | 8 |
| Signal Columns on Layers | 1.4, 5.10 | NOT YET | No columns | 60px fixed width, 5 visible, h-scroll | 8 |
| Column Triggers | 2.12, 5.9 | PARTIAL | Headers exist | Clickable scene triggers | 14 |
| Deck Tabs | 5.9 | EXISTS | Works | Style update only | 1 |
| Per-Layer Sparklines | S2 ref | NOT YET | No sparklines | Signal name per layer | 8 |

---

## SCREEN 4: Inspector Panel

| Component | Directive | Status | Current | Target | Phase |
|-----------|-----------|--------|---------|--------|-------|
| Inspector Tabs | 5.8 | EXISTS | 4 tabs work | Blue+underline active styling | 1 |
| Inspector Rows | 5.6 | PARTIAL | 72px label, 36px value | 90px label, 50px value, 14px slider, P. | 3 |
| Section Headers | 5.7 | PARTIAL | Exist, no blue bar | 24px, 2px blue left bar when routed | 3 |
| Dashboard (MacroPanel) | 1.4 | PARTIAL | 8 rotary Knobs | 8 VerticalSignalColumns | 2 |
| Effect Stack | 5.6 | EXISTS | Works | Style audit only | 1 |
| Triangle Indicators | 1.3 | EXISTS | Works correctly | Already aligned | - |
| P. Button (section headers) | 5.7 | NOT YET | No P. button | Route-entire-section action | 3 |

---

## SCREEN 5: Browser Panel

| Component | Directive | Status | Current | Target | Phase |
|-----------|-----------|--------|---------|--------|-------|
| Files Tab | 5.11 | EXISTS | Complete | No change | - |
| FX Tab | 5.11 | EXISTS | Complete | No change | - |
| Sources Tab | 5.11 | EXISTS | Complete | No change | - |
| Comp/Decks Tab | 5.11 | EXISTS | Complete | No change | - |
| Record Tab | 5.11 | PARTIAL | Controls only | Add recordings browser | 14 |
| MilkDrop Tab | 5.11 | EXISTS | Complete | No change | - |
| Content Tagging | 5.12 | NOT YET | No tag system | Custom tags, search by tag | 14 |

---

## SCREEN 6: Bottom Focus Bar

| Component | Directive | Status | Current | Target | Phase |
|-----------|-----------|--------|---------|--------|-------|
| Collapsed Summary | 1.7, 5.2 | NOT YET | No component | Thin bar, 1-line summary | 7 |
| Expanded Detail | 1.7, 5.2 | NOT YET | No component | Docked at bottom, ~30% screen expanded, drag-resizable, overlays panels, context-sensitive | 7 |
| Hit Inspector View | 2.11 | NOT YET | No component | Full Hit anatomy when Hit selected | 10 |
| Macro Detail View | 1.7 | NOT YET | No component | Signal group controls when macro selected | 8 |

---

## SCREEN 7: Waveform Display

| Component | Directive | Status | Current | Target | Phase |
|-----------|-----------|--------|---------|--------|-------|
| Stereo Waveform | 5.13 | EXISTS | Scrolling display | Style update only | 1 |
| Display Mode Selector | 5.13 | NOT YET | Single mode | Buttons: Condensed/DJ/Spectrum | 11 |
| DJ/CDJ View | 5.13 | NOT YET | No component | Beat grid, colored frequency bands | 11 |
| Hit Lane | 2.13 | NOT YET | No component | Hit pills below stereo pair | 10 |
| Click Interaction | 5.13 | NOT YET | Passive display | Click to place Hit, scrub | 10, 11 |
| Structural Zone Overlays | 1.9 | NOT YET | No overlays | Colored zones on waveform | 11 |

---

## SCREEN 8: Hit System UI

| Component | Directive | Status | Current | Target | Phase |
|-----------|-----------|--------|---------|--------|-------|
| Hit Data Model | 2.1-2.8 | NOT YET | No model | Hit, HitPayload, HitEnvelope, HitGroup | 9 |
| Hit Pills | 2.9 | NOT YET | No component | 8x24px, S/C/M indicators | 10 |
| Hit Creation (Capture) | 2.4 | NOT YET | No workflow | Button + snapshot + filter dialog | 10 |
| Hit Envelope System | 2.7 | NOT YET | No system | Onset/curve/release per Hit | 9, 10 |
| Hit Groups | 2.8, 2.14 | NOT YET | No groups | Reusable sequences, deploy to timeline | 10 |
| Hit Chains | 2.10 | NOT YET | No chains | Lines/arcs between pills | 10 |
| Pattern Presets | 2.15 | NOT YET | No presets | 1/2/4/8/16/32/64 bar patterns | 10 |
| Hit Inspector | 2.11 | NOT YET | No inspector | Full anatomy in bottom focus bar | 10 |

---

## SCREEN 9: Programming Mode Workspace

| Component | Directive | Status | Current | Target | Phase |
|-----------|-----------|--------|---------|--------|-------|
| Three-Panel Layout | 1.5 | NOT YET | Vestigial ProgrammingMode | Left signals, center controls, right params | 6, 8 |
| Signal Source List | 1.5 | NOT YET | No list | All 32+ signals, grouped, draggable | 8 |
| Parameter Target List | 1.5 | NOT YET | No list | All params, grouped by layer/clip | 8 |
| Visual Patch Bay | 1.5 | NOT YET | No patch bay | SVG lines connecting sources to targets | 8 |
| Signal Group Editor | 1.1 | NOT YET | No editor | Processing chain builder | 8 |

---

## SCREEN 10: Pulse AI (Deferred)

| Component | Directive | Status | Phase |
|-----------|-----------|--------|-------|
| Chat Interface | 3.1 | NOT YET | 13 |
| Content Navigation | 3.2 | NOT YET | 13 |
| Signal Suggestions | 3.2 | GHOST (MappingSuggester) | 13 |
| Settings Management | 3.2 | NOT YET | 13 |
| Inactive State (headings) | 3.3 | PARTIAL | 13 |

---

## GHOST FEATURES (Quick Wins)

| Feature | LOC | Effort | Impact |
|---------|-----|--------|--------|
| MappingSuggester | 266 | ~1 day | AI mapping suggestions |
| LinkSync requestBeatAtTime | 12 | ~1 day | Ableton Link sync button |
| LUT Loader | 111 | ~1-2 days | Color grading |
| Hidden Audio Signals (21) | 0 | ~0.5 day | Surface in signal bar |
| Session Recorder (6/7 unwired) | exists | ~2 days | Full event recording |
| Camera Input | exists | ~3-5 days | Live camera as source |

---

## SUMMARY: Screens by Readiness

| Screen | EXISTS | PARTIAL | NOT YET | Ready for Mockup? |
|--------|--------|---------|---------|-------------------|
| Top Chrome | 3 | 6 | 3 | YES — structure clear |
| Signal Bar | 0 | 2 | 2 | YES — spec complete |
| Deck Grid | 1 | 3 | 3 | YES — mockup H1 exists |
| Inspector | 3 | 2 | 1 | YES — grammar defined |
| Browser | 5 | 1 | 1 | MOSTLY DONE |
| Bottom Focus | 0 | 0 | 4 | YES — spec complete |
| Waveform | 1 | 0 | 5 | YES — mockup exists |
| Hit System | 0 | 0 | 8 | YES — data model designed |
| Programming | 0 | 0 | 5 | YES — S1 mockup exists |
| Pulse AI | 0 | 1 | 4 | NO — scope undefined |

**9 of 10 screens ready for mockup creation in next session.**
