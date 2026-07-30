# Scout dossier — MilkDrop playlist-drop wiring (2026-07-30 PM, HEAD 972e8dd)

Trigger: Boris dragged group header "Energetic (9)" onto a cell → ONE preset
landed (gate item 8 partial). Scout: read-only Explore, delivered same day.

VERDICT: **PLAYLIST-MODEL-EXISTS-DRAG-NOT-WIRED** — model, multi-preset drag
flavor, drop handler, undo, and beat-synced playback are all complete at HEAD;
no code path turns a group-header drag into a multi-preset payload. A header
drag silently falls through to the single-preset branch (drags the
last-clicked preset).

## 1. Playlist concept — real, not aspirational (VERIFIED)
- src/model/Clip.h:122-150 — std::vector<PresetEntry> presetPlaylist,
  PlaylistCycleMode (RandomBag/Sequential/Reverse/RandomOther/PingPong),
  PlaylistTrigger (Beats/Bars/Phrase/OnDrop/OnBreakdown), playlistTriggerBeats,
  playlistBlendSeconds, playlistEnabled, hasPresetPlaylist().
- Value-copy: Clip.h:208-214; reset Clip.h:268-275; JSON round-trip
  Clip.cpp:96-112 + 252-278.
- Playback LIVE: src/render/Renderer.cpp:259-351 advances presets on beat
  crossings + OnDrop/OnBreakdown structural triggers.
- Tests: tests/test_composition.cpp:240-280 (serialization),
  tests/test_undo_commands.cpp:91 (clip equality incl. presetPlaylist).

## 2. Drag flavors — two; the header exports neither (VERIFIED)
- Single: "milkdrop:<path>" — src/ui/MilkDropBrowser.cpp:101.
- Multi: "milkdrop_playlist:<p1>|<p2>|..." — MilkDropBrowser.cpp:72-93; fires
  ONLY when isMultiSelectMode() && selectedIndices_.size() > 1 (line 70);
  isMultiSelectMode() = play mode == Playlist (MilkDropBrowser.h:153).
- Group header has NO drag support: mouseDown → presetIndexAtY returns -2
  (MilkDropBrowser.cpp:303-304) → handleSectionHeaderClick only toggles
  expand/collapse (338-370), never touches selection. mouseDrag (61-112) does
  NO hit-testing → header drag lands in `else if (lastClickedIndex_ >= 0)`
  (line 95) → drags the last-clicked SINGLE preset. (INFERRED this is what
  Boris saw — matches his report; refutes one-element-playlist alternative via
  the `> 1` guard + cell would read "MilkDrop Playlist (1)".)

## 3. Drop target — correct on both flavors, nothing broken downstream (VERIFIED)
- ClipCell.cpp:369 accepts both; :478-488 playlist branch → 
  onMilkDropPlaylistDrop; :489-495 single. DeckView.cpp:159-160 forwards.
- MainComponent.cpp:1086-1146 multi handler: names "MilkDrop Playlist (N)"
  (:1095), playlistEnabled=true, RandomBag, 8 beats (:1124-1126), undoable
  edit (:1136). Single path :1028-1083 stores one-entry playlist,
  playlistEnabled=false.

## 4. Dead declaration (VERIFIED)
- MilkDropBrowser.h:155 declares `void startDrag();` — NO definition anywhere.
  Plausible intended home for the never-written header-drag logic.

## 5. Working gesture TODAY (VERIFIED from code, NOT-CHECKED at runtime)
MilkDrop tab → "Playlist" button (bottom mode bar) → click one preset →
Cmd-click (or Shift-click; both toggle individually, NO range select —
MilkDropBrowser.cpp:797-802) more, until label reads "N selected - drag to
cell" (:672-675) → drag from any list ROW onto a grid cell. Header-drag also
works in that state (mouseDrag ignores position).

## SIDE FINDING (VERIFIED): Playlist-mode controls are DECORATIVE
playlistCycleSelector_, playlistTimingSelector_, playlistBlendSlider_ are never
read anywhere in src/; MainComponent.cpp:1125-1126 hardcodes RandomBag + 8
beats; playlistBlendSeconds appears nowhere outside Clip serialization. Even
the working gesture ignores all three controls.

## Fix surface (IF Boris wants group-drag → playlist)
- MilkDropBrowser.cpp mouseDown (~54-58): on -2, record the section under the
  cursor (new member), not just toggle expansion.
- Add sectionAtY() helper mirroring handleSectionHeaderClick's walk (338-370)
  → that section's preset vector.
- mouseDrag (:61): if press started on a header, build "milkdrop_playlist:"
  from the section's presets (reuse 73-93) — natural body for startDrag().
- Optional: MainComponent.cpp:1124-1126 read the browser's cycle/timing/blend
  controls instead of hardcoding (fixes the decorative-controls side finding).
- ClipCell / DeckView / MainComponent multi handler: NO changes.

CONFIDENCE: all file:line VERIFIED at HEAD 972e8dd; single INFERRED claim
(header drag emits last-clicked preset). NOT-CHECKED: runtime browser state in
Boris's session. Read-only; nothing edited; app not run.
