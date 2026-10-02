# Lane ui-polish — Harmony packet (s-rta-1002b, 2026-10-02 18:42:40): fold in the visual critic panel's findings on lane ui (merged 3262fb6)
Critic papers: critic-ui-{visual-design,ux,graphic-design,interaction-logic}.md (4 x PASS_WITH_NITS, 0 MUST); captures
.harmony/.reports/s-rta-1002b/ui-shots/. Harmony's rulings (binding; execution only, no redesign):
P1 Info rows legible on a dark stage: the Clip inspector's codec / size rows use the inspector's normal secondary text
   colour and size (match the Transport section labels, not the caption grey), aligned to the clip-name left edge, with
   ~6 px more space before DASHBOARD. Missing-file row stays red.
P2 No layout jump: reserve a FIXED two-row info block for every clip kind (video 2 rows; picture / sequence / missing 1 row
   + blank; procedural source 2 blank rows), so DASHBOARD and every control below sit at the same y for all kinds. Guard:
   a unit / layout test that the DASHBOARD y is equal for H.264, PNG, missing and a source (RED on main).
P3 The rename box reads as editable: a 1-2 px outline in the app's accent (cyan) colour, a fill a step lighter than a tab,
   a visible caret colour, text baseline aligned with the tab labels. The showing tab's highlight is kept around the box.
P4 Cell tooltip: line 1 file name; line 2 the codec; line 3 size + rate (same split as the inspector); the hint line dimmed
   if JUCE allows, else unchanged; no word orphaned at the default tooltip width (wording "frames per second" stays).
P5 Rename box on a narrow tab: the box (min 100 px) is CENTRED on the tab and clamped inside the row, so on a middle 60-px
   tab it covers neither neighbour's label more than necessary; add a probe / unit row for a MIDDLE tab (e.g. tab 10 of 27).
NOT in scope: "Video file not loaded" wording (file F-ui-1 for Boris's page), picture size for PNG (Boris Q3 default no),
the cell menu title style (app-wide popup style).
GATES: unit tests RED-first for P2 + P5; full ctest; .harmony/probe-ui-files-rename.sh 53+ PASS / 0 FAIL (+ the new middle-tab
row); re-take the captures (--shots) for C2, C3, C5, C8, C10, C13, C7 and a middle-tab C5b; Harmony re-runs the critic panel.
Harmony constraint: BORIS USES THIS MACHINE AND THIS APP — never quit / kill / touch an Audio-DNA your lane did not start.
