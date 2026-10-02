# Critic ui / graphic design - s-rta-1002b
STATUS: DONE
VERDICT: PASS_WITH_NITS (0 MUST, 5 SHOULD, 4 NIT)
IMAGES READ: C1 C2 C3 C4 C5 C6 C6-crop C7 C7-crop C8 C9 C10 C11 C12 C13 C14 (+ zoom crops of C1/C2/C3/C5/C8)
SHOULD
- C2/C3/C5: rename box barely reads as an edit field. Navy fill, same 1px border as the tabs, selection highlight a dull teal, no visible caret; on a dark stage it looks like a tab that turned blue. Fix: 2px cyan (the app accent) border + brighter selection + steady caret.
- C8/C9/C14 vs C10/C11/C12 vs C13: the info rows push the whole inspector down by 42 px (video), 21 px (picture/sequence/missing), 0 (source). Knobs, Transport, Cuepoints jump when the clip changes. Fix: reserve a fixed info band or put codec and size on one line.
- C8: info rows are ~11 px grey (about #888 on #1c1c1c) vs a 13 px bold name; the second row is dimmer; hard to read at arm's length on stage. Fix: ~12-13 px and lighter grey (>= #b0b0b8), merge into one line "H.264 High - 64 x 64 - 30 fps" if it fits.
- C10: "File missing" is small thin red on near-black, the same size as normal info, yet Show in Finder stays full weight/enabled. The one actionable fault is the least visible. Fix: bold/larger red, optionally a red-tint row, and dim the button or make its target obvious.
- C7-crop: tooltip wraps "frames per / second" leaving an orphan word; all three lines same bold weight (name, info, action hint) so no hierarchy. Fix: "30 fps" or widen the tooltip; make the hint lighter/dimmer.
NIT
- C8: three different left edges (name x1010, info rows x1018, DASHBOARD x1013); the last info row sits 18 px above the DASHBOARD caption (C10: 19 px) - crowded.
- C14: ellipsis ends flush against the button (about 4 px gap), cut mid-phrase "Show i..."; add 8 px gutter.
- C5 crop: edit text sits ~1.5 logical px lower than neighbours' labels and is brighter/heavier than the grey tab labels.
- C6: the one new menu item is small caption-size text under a large bold title; title dominates, item looks non-clickable (no hover/separator visible). Top bar rendering white in C6/C7 looks like a capture artifact - not judged.
60-px tab / 100-px box (C5): VERIFIED box right edge sits ~15 displayed px inside the window edge, no overlap with Deck 26; legible.
