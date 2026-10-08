# check-milkdrop (s-rta-1007)
Verdict: SOUND_WITH_CORRECTIONS. Document mended in place: .harmony/.reports/s-rta-1007/milkdrop-current.md (see its section "RE-CHECK (s-rta-1007)" for the one-line list of 17 changes and what was found right).

## Defects found (all mended)
- MUST: Sources-tab "double-click" does not exist (onSourceActivated assigned at MainComponent.cpp:1541, declared SourcesBrowser.h:20, never called; only a drag works). A Sources-tab MilkDrop cell is named "projectm_visualizer" (MainComponent.cpp:1123).
- MUST: "Speed" was UNKNOWN; engine source (projectm-03aa8a7 tarball) shows PresetSwitchRequestedEvent is an empty core function, the app sets no callback; duration only feeds the `progress` variable; 0 of 30 bundled presets read it. Hard cuts off by default.
- MUST: missing/broken preset file = engine keeps previous preset / idle preset, silently (ProjectM.cpp:56-67, 127-130).
- SHOULD: playlist beat counter and list position never reset on fire (Clip.h:363-364 only).
- SHOULD: Jukebox UI boxes changed before the source exists are dropped (MilkDropBrowser.cpp:532,550,571,595) while the selector keeps defaults; Play/Lock desync; Favorites-empty pool and Sequential details; Play's first jump ignores pool.
- SHOULD: 6 bundled presets use textures the app never supplies (grep sampler_; TextureManager placeholder).
- SHOULD: manifest is matched by name for any scanned preset (doc said user-folder never); userPreset/style unused; milkdrop_userdata.json uses unverified writer (Pitfall 68); libprojectM not bundled in .app (INFERRED).
- SHOULD: BD:675 quote was mangled (Harmony text inside quote marks); corrected to file wording.
- SHOULD: test-coverage claim made exact (no test of browser, Jukebox, playlist timer).

## Checked and right
See the document's RE-CHECK section: all PART 1 controls followed from on-screen words (MilkDropBrowser.h/.cpp, PreferencesDialog, SourcesBrowser, ClipCell) to their calls; about 50 PART 2 file:line items opened (list in the document).

## Open (kept in PART 3, not resolvable by reading)
Bar-crossing robustness (backward bar-phase jump counts as a bar, INFERRED, BPMTracker not traced); look of the 6 texture presets; soft fade when not drawn; Preferences-change star loss (one run); performance at 4K; Curated intent (Boris's wish).

## Not done
Nothing run, built or launched. Engine tarball unpacked read-only in the scratchpad. Not opened: other tests that only mention the words; BPMTracker.cpp barPhase code.
