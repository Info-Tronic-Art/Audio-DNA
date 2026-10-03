# Harmony rulings for the bf9b merge (s-rta-1002b, 2026-10-02 22:53:13) — for the next session's merge / rebase lane
R-S2 probe-canvas f2_deck_transition (RED on the bf9b arm): RETIRED by Boris's ruling, not re-thresholded. The deck-to-deck
  fade no longer exists: a deck switch changes nothing on screen (BORIS_DECISIONS.md "Decks are boxes of clips", verbatim:
  "when I switch between decks, do not change the clips playing in the layers or how they are playing"; consequence default
  3 "the fade between decks is removed"). K1 / K1t replace it. Also update probe-canvas.json:20 "_why" (no compositeDeck).
R-S3 rebase: the lane's "Pitfall NN" becomes 67 (64 mkvidx, 65 ui, 66 bf10; bf2 gets 68 at its merge). main already has
  ui's TEST-ONLY /api/debug/undo route: keep ONE route (main's), make the lane's tests use it; no duplicate registration.
R-S1 H1 (m9b_deck_switch_live, 20 decks with MilkDrop on deck 0 L0) is a REQUIRED gate row at the bf9b merge (bf10 has
  merged, so probe-milkdrop.py exists on main): add it in the rebase lane, RED on pre-bf9b main.
R-N1 probe safety: probe-canvas.sh (and any probe that quits Audio-DNA by NAME via osascript / pkill) must quit only the pid
  it launched (the bf10 attach-refusal pattern); fix it in the rebase lane before any live gate runs. Boris uses this app.
R-N3 two BLOCKED bars that print GREEN (k5_queue_link_on — no Link driver; k7 save half): the probe must print BLOCKED in its
  final verdict line, never GREEN, while any pre-registered bar is BLOCKED.
