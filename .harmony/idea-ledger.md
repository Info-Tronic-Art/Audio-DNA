<!-- Secondary→Primary idea-ledger (foreign-repo lane). Verbatim IDEA records (spec §4.1). The primary PULLS these at boot via config/repos.yml scan into memory/secondary-ideas-inbox.md. Append-only; never compress `raw`. -->
--- IDEA ---
id:          idea-2026-07-28-RealTimeAudio-1785289328541427496
raw:         I would like cmd x to clear a clip as well
context:     Undo v1 manual e2e sitting 2026-07-28 — asked how to delete clips from a cell; wants Cmd+X (cut) as a clip-clear gesture. Note: Clip menu lists Cut/Copy/Paste — wiring/shortcut state unverified.
why:         
intent:      
target:      
constraints: 
related:     
priority:    HIGH
repo:        RealTimeAudio     session:      date: 2026-07-28
status:      NEW
status-changed: 2026-07-28
status-note:
artifact:
history:     NEW(2026-07-28)
--- /IDEA ---

--- IDEA ---
id:          idea-2026-07-28-RealTimeAudio-17852893285415730694
raw:         I want them all loaded by default
context:     Same sitting — MilkDrop browser showed no presets (preset dir unset; libprojectM absent). Wants bundled resources/projectm_presets loaded by default. Natural bundle with the MilkDropBrowser empty-state null-deref crash fix (.ips 2026-07-28-190701).
why:         
intent:      
target:      
constraints: 
related:     
priority:    HIGH
repo:        RealTimeAudio     session:      date: 2026-07-28
status:      NEW
status-changed: 2026-07-28
status-note:
artifact:
history:     NEW(2026-07-28)
--- /IDEA ---

--- IDEA ---
id:          idea-2026-08-04-RealTimeAudio-17858604595945222831
raw:         If I drop multi images it would be great to have a toggle to select between drop on cell or drop on multi cells. Can this be visible as a selection right after drop?
context:     Boris, session 2026-08-04c, Audio-DNA. PARTIALLY addressed, NOT fully delivered. SHIPPED in 7d3a203: sequence threshold raised to 3+ (2 images now spread across 2 cells, which removes the surprise that prompted this) + a 'SEQ N' badge so sequence cells stop being visually IDENTICAL to video cells (they shared one paint branch — that was the real root cause). NOT SHIPPED: the 'visible as a selection right after drop' escape hatch.
why:         Boris dropped 2 images, they merged into one animated sequence clip, and he did not recognise a feature his own app ships and documents. He wanted both to know it happened AND to be able to opt out per-drop.
intent:      Give the user a per-drop choice between image-sequence and spread-across-cells, discoverable at the moment of the drop.
target:      RECOMMENDED HOME: a persistent 'Spread Sequence to Cells' command in the existing Clip menu (MenuBarModel.cpp:9) or as a TextButton in ClipInspector (already a panel of exactly such actions). No timer, no snapshots, correct undo depth by construction, works ten minutes after the drop, and greys out when inapplicable — which itself teaches.
constraints: An architect designed a transient post-drop chooser pill (~450-500 lines). TWO independently-dispatched blind critics BOTH returned UNSOUND/FAIL and converged against it: cells are 90px with kCellGap=0 so a legible pill is WIDER than its anchor and would occlude the trigger hit box; its dismiss mechanism (UndoManager::onHistoryChanged) is structurally BLIND to autopilot advances and non-user deck switches — the two things most likely to happen mid-set, neither of which creates a command; UndoManager::onHistoryChanged is a single std::function already assigned at MainComponent.cpp:1544 so hooking it would silently break Edit-menu undo text; and a 5s timer makes the escape hatch a race in exactly the window where the clip is most likely to have started playing.
related:     .harmony/decisions-2026-08-04c.md ; commit 7d3a203 ; .harmony/gesture-replay-results.md
priority:    medium — the surprise is already fixed by the badge + threshold; this is the remaining convenience half
repo:        RealTimeAudio     session:      date: 2026-08-04
status:      NEW
status-changed: 2026-08-04
status-note:
artifact:
history:     NEW(2026-08-04)
--- /IDEA ---
