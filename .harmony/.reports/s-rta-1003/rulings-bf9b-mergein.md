# Harmony rulings for the bf9b merge-in lane (s-rta-1003, 2026-10-03 13:36:16) — on top of .harmony/.reports/s-rta-1002b/rulings-bf9b-merge.md (R-S1 / R-S2 / R-S3 / R-N1 / R-N3 stay binding)
H-1 MERGE-IN, not a history rewrite: main is merged INTO lane/bf9b (one merge commit). The 49 reviewed lane commits keep their
    SHAs (the r1 / r2 reviews stay pinned to real commits); Harmony's final merge to main is then conflict-free.
H-2 Stage order in the one worktree: M1 merge-in + R-S3 -> M2 by-name-quit sweep (R-N1) -> M3 MilkDrop deck-switch row (R-S1 / H1-H3)
    + post-merge probe re-runs. The round-2 review items that need a design (use-after-free read left by the M1 fix, Undo of
    Add / Load / Duplicate Deck, the untested wiring line, K5 Link-on, the S2b 4.B omissions, the grid NITs) and Boris's BF14
    (strip badge removed) go through plan -> blind council -> ruling (.harmony/.reports/s-rta-1003/plan-bf9b-merge.md,
    ruling-bf9b-merge.md) and are built in a FIX stage after M3. One pinned review round covers a7491d4..final head.
H-3 state-r2 NIT 6 (probe-quit-ours: "only OURPID running" then quit by name): NO CHANGE. Reason: with exactly one Audio-DNA
    process alive and it is ours, a by-name Apple-event quit can only reach that process; a second instance cannot appear in
    the window by a normal launch (same bundle id activates the running one). A SIGTERM would skip the app's own teardown.
H-4 gates-r2 NIT 6 (record_ourpid after the 60 s health loop) and NIT 4 (k1b_duplicate not a registered row): FIX in M2.
H-5 RED arm for every post-merge row = the PRE-MERGE main app (main 5abdf01's build, read-only), not the old STAGE_P copy.
    A row whose RED pattern differs from the lane report's STAGE_P table is listed with the reason, never re-thresholded.
H-6 (2026-10-03 13:54:41; M1 stop item) .harmony/probe-ui-files-rename.sh rows R3, R4a, R9a expect "builds +1" (a tab-row rebuild) on a
    deck switch. That clause encodes the PRE-bf9b switch. Boris (verbatim): "when I switch between decks, do not change the
    clips playing in the layers or how they are playing" -- on the lane a switch between same-shape decks is showDeck ->
    refresh, no rebuild. M3: (1) run the probe UNCHANGED on the lane app first and paste the raw lines (expected: only the
    build-count clause of R3 / R4a / R9a differs; anything else that fails is a real finding -> STOP item); (2) change ONLY
    the build-count clause of those rows to "+0" for same-shape decks, keep every other clause (active deck, editor closed /
    open, the box stays on its deck over its tab), put Boris's quote at each changed row; (3) ADD a positive control so the
    counter cannot be dead: one row where an action that legitimately rebuilds the tab row reads "+1" (name the driver that
    exists: a rename commit, add / duplicate / remove deck); (4) re-run GREEN on the lane app, and show the changed rows
    RED on the pre-merge main app (it reads +1). A switch between decks with DIFFERENT column counts still rebuilds the
    grid (live-r2 NIT-1): add no row for it -- it is under plan review (P6).
H-7 (2026-10-03 14:44:55; M2 stop item 1) The 15 ARCHIVED evidence scripts under .harmony/.reports/** that still quit or kill Audio-DNA by
    name (21 lines; pkill / killall / unguarded osascript quit; the lock.sh copies of s-rta-0927 .. s-rta-0930): they are
    records, but handoffs point at those folders as templates, so a copy-and-run would quit Boris's app. NEUTRALISE, do not
    rewrite: each such script gets ONE inserted line right after its first line that prints "ARCHIVED RECORD (R-N1,
    s-rta-1003): this script quits Audio-DNA by name -- never run or source it; use .harmony/probe-quit-ours.sh" to stderr
    and stops (exit 64 for a run script; return 64 2>/dev/null || exit 64 for a sourced helper). Nothing else in them
    changes. NOT neutralised: a helper whose by-name quit sits under the only-ours check (the s-rta-1002b and s-rta-1003
    wf/lock.sh, in use today). Built in FIX-4; proof = the by-name grep over .harmony/.reports lists every remaining hit
    with either the neutraliser above it or the only-ours guard.
H-8 (2026-10-03 14:44:55; M2 stop item 2) CONFIRMED: quit_ours keeps its by-name branch untouched; the added "no pid recorded ... nothing is
    quit" line and the new ask_ours_to_quit (a second by-name line under the same only-ours condition) are within H-3 and
    within gate G-1's allowed class. The gates reviewer reads both blocks.
H-9 (2026-10-03 14:44:55; M2 stop item 3) CONFIRMED: probe-ui-files-rename.sh UIFR_ATTACH=1 refuses unless UIFR_ATTACH_PID, or LOCK_LIB +
    LANE, names the running test-mode app. Harmony's gate runs pass one.
H-10 (2026-10-03 15:32:01; FIX-1 stop 1) CONFIRMED: lint B4f gains ONE new pin { "ui/InspectorRepoint.h", 1 } (AM-2's own setClip(nullptr)
     matches the lint's token); the 12 existing pins are unchanged. The ruling's "B4f counts unchanged" reads "existing
     counts unchanged".
H-11 (2026-10-03 15:32:01; FIX-1 stop 2) CONFIRMED: the every-fenced-edit check (adoption item 2) is the else branch of the hand-over
     ("stack moved -> the hook function; else -> onFencedEdit"): the owned-or-clear check runs exactly once per fenced edit
     and MU2 keeps its teeth. FILED (SF-11, ui lane): after Add / Remove Column the Clip tab goes EMPTY when its clip moved
     in memory (memory-safe, but the same clip could be re-pointed by its id).
H-12 (2026-10-03 15:32:01; FIX-2 stops 2, 4) CONFIRMED: AM-18's comment lives in src/api/ApiServer.cpp (comment lines only). T6f / T6g / T6j
     under the asan label may go RED by their own REQUIRE under MU6 (an assertion, not an "ERROR: AddressSanitizer" line):
     acceptable for B3b, whose RED arm with heap-use-after-free reports is FIX-1 commit 1 (AS0 / AS5 / AS6 / AS7).
H-13 (2026-10-03 15:32:01; FIX-2 stop 1) T6h (Undo of a Load Deck that added layers stops that deck's clip) stays pinned in THIS lane
     (adoption item 8); lane BF31 (Undo never changes what is live) re-registers it.
H-14 (2026-10-03 15:58:55; FIX-3 stops 2, 3) CONFIRMED: the Undo Remove button and the 'Removed deck "<name>"' file-label line were on main and
     are removed on Boris's words (adoption items 9-11). Gate B2's RETIRED set = the 8 names in the lane report's FIX-3
     section (AM-12's five + three from adoption 9-11, one of them a test that exists on main) + 1 rename (M-c); any other
     missing name = RED.
H-15 (2026-10-03 15:58:55; FIX-3 stop 1) There is NO "Edit" menu in the app; Undo is the first item of the "Composition" menu and reads "Undo
     Remove Deck" after a Remove Deck (pinned by a test). Not changed in this lane. Put to Boris (he said "the top edit
     menu"): add a standard Edit menu (Undo / Redo) — default yes, in the next UI pass.
H-16 (2026-10-03 15:58:55; FIX-3 stops 4, 6, 7) ACCEPTED: lint B4j scans whole lines incl. comments (a comment naming the removed widgets
     fails it — intended). Composition::routineLoadNote has no reader and no log line -> FILED to the notices lane (rule
     there: every removed text keeps or gains ONE log line). After Remove Layer of a selected non-top layer the strip that
     takes that index is highlighted (before: none) -> ACCEPTED as list-selection behaviour; the screen reviewer reads it.
H-17 (2026-10-03 17:09:46; FIX-4 stop 1 + memory-review SHOULD 1) SF-12: GET /api/composition reads the deck list on the http thread unlocked
     while the message thread appends a deck (ASan container-overflow once on the UNMUTATED ASan app; the same handler is on
     main). Not the lane's fix and not fixed here: FILED to tsan-r5 with this evidence (its scope already names "the httplib
     reader"). Gate note: a PROBE-ASAN-LIVE RED at L4 / L5 whose report is "container-overflow ... ApiServer::handleComposition"
     is SF-12 -> re-run once; any other report is a lane failure.
H-18 (2026-10-03 17:09:46; memory + gates SHOULD 2, FIX-4 stop 4) probe-asan-live waits on three of main's event texts ("Loaded deck:",
     "Duplicated deck:", "Loaded:"). The notices lane (BF32) must first move those waits to a model fact (a counter in
     /api/debug/ui_text) and only then remove the texts: written into that lane's brief (board row 3c).
H-19 (2026-10-03 17:09:46; gates SHOULD 1) probe-boxes-perf.sh exits 0 / prints DONE whatever B6(ii) says: Harmony's gate reads the B6(...)
     lines themselves (section 5 strings), never the rc. FILED: a distinct last line + non-zero rc (next probe touch).
H-20 (2026-10-03 17:09:46; screen SHOULD 1) BORIS_DECISIONS.md:358-359 (the lane's "Built (bf9b ...)" sentence: strip badge, tab dots, a
     yellow note) is stale: Harmony rewrites that sentence on main right after the merge (doc-only).
H-21 (2026-10-03 17:09:46; FIX-4 stops 2, 3, 5, 6) FILED: a Clip-inspector step for probe-asan-live (POST /api/debug/inspect_clip exists
     after the merge-in). CONFIRMED: 14 archived scripts neutralised (the 15th is the guarded s-rta-1002b lock.sh); the
     B6(ii) test order and the quiet rule's numbers as built; FM-6's teeth proof with 20 s runs (+37 ms against THR 1 ms).
H-22 (2026-10-03 18:01:10; B7 visual gate, wf_95420269-4c3: 128 captures + manifest, 5 critics) RESULT: visual-design YES, UX YES, logic YES,
     interaction-logic YES to its question but verdict FAIL on a MUST, graphic-design NO. Machine checks in ctest PASS.
     Numerics: the "strip unchanged" values d = 0.72 (Remove Deck), 1.20 / 1.03 (4 <-> 8 column switch), 0.78 (Load Deck +
     Undo) are WITHIN the pre-registered floor max(1.5, 4 x noise) = 1.5 (ruling-bf9b rig text :468-469; the capture builder
     compared with the raw noise 0.0000); the difference is the thumbnail squares only and the pre-merge app shows it too.
     RULINGS: (a) interaction-logic's MUST ("Loaded deck: five-rows" in the second toolbar row) is a text that was on main
     before this lane (FIX-3's found_not_fixed list; facts-notices.md class A): OUT OF THIS LANE by adoption item 11 -> first
     item of the notices lane (BF32). Not a lane MUST. (b) graphic-design's NO (about 185 pt of empty bar between Quantize
     and Master Signal where "Fade:" was): the bar "five yes" is NOT MET on this seat and is reported so. Harmony's decision:
     no change in this lane -- Fade was the last item of the left run, so what grew is the flexible space between the left run
     and the right-aligned group; closing it means a top-bar re-layout, which the adopted sync-dial plan does (Gain 140 px,
     SYNC button). On the Boris page as a look item. (c) UX SHOULD + the capture builder's finding: with a layer selected
     and the Layer tab open, Remove Deck leaves the Layer tab BROKEN (stale title "Layer 3", cut-off knobs, "No layer
     selected" across them; Harmony looked at P1-...-inspector.png): a fault on the lane's own model (layers are shared; a
     deck is a box) and on Boris's own check path -> RETURNED TO A FIX STAGE (FIX-5), not shipped. (d) visual-design SHOULD
     (S7-a "a clipped second X B S strip"): Harmony looked at the capture: it is the FOLDED layer row (the fold design:
     compact X B S, small faders, tiny thumbnail), not a leftover -> no action. (e) FILED to ui-polish: a 20-character deck
     name is cut on its tab without an ellipsis; the strip shows the media file's name, not the clip's name (same on the
     pre-merge app); a deck loaded from a file takes the file's name.
H-23 (2026-10-03 18:23:20; FIX-5 at fc51063, review PASS_WITH_NITS 0 MUST / 0 SHOULD, critics UX YES + visual-design YES) ACCEPTED: (1) the
     headless case AS8 has no failing arm (the fault sat in MainComponent::removeDeck, which no headless test reaches); its
     teeth are lint B4k (RED at 4137f60, RED under the mutant) and the live rows (4 FAIL of 10 on the 4137f60 app, 0 at the
     fix) -- Harmony re-runs the live rows herself. (2) removeDeck no longer calls selectLayer(-1) when the shown deck is
     removed (needed for the highlight and for Undo). (3) The no-layer paint fault was PRE-EXISTING on main; fixed in 5
     lines with a headless case. (4) The Clip tab is still emptied by every Remove Deck (as before): FILED to the ui lane
     (keep the clip when the removed deck is another deck; needs the effect scope's deck index re-pointed). (5) B6 perf is
     NOT re-run on the final head: the delta 4137f60..fc51063 changes message-thread inspector code only (file list in
     gateA2.out); the B6 results at 4137f60 stand. (6) FILED: the performance-controls.md sentence overclaims "Undo / Redo
     leave the Clip tab emptied" (they re-point it by the selected cell).
