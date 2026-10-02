# Critic ui visual-design
STATUS: DONE
VERDICT: PASS_WITH_NITS
IMAGES READ: C1 C2 C3 C4 C5 C6 C6-crop C7 C7-crop C8 C9 C10 C11 C12 C13 C14 (+ zoom crops of C1/C2/C5/C8/C10/C14)
FINDINGS (no MUST):
SHOULD C7/C7-crop: tooltip wraps "...30 frames per / second" (orphan word), 4 lines, all same weight, black box covering deck tabs + "+" button. Fix: put codec and "64 x 64, 30 fps" on separate lines (or shorter "30 fps"), widen max width.
SHOULD C8/C9/C14: info rows are small dim grey on near-black; hard to read on a dark stage. Raise to the inspector's normal secondary text colour/size.
SHOULD C8 vs C13 vs C10/C11: selecting clips shifts the whole inspector (dashboard 695 -> 715 -> 737 px) because the info block height varies (0/1/2 rows). Reserve a fixed block or fixed 2-row height.
NIT C8: info rows indented ~10 px (x~30) vs title (x~16) and DASHBOARD label (x~20): 3 left edges. Row 2 sits ~18 px above DASHBOARD while rows are ~36 px apart; cramped.
NIT C6: menu is title (large bold) + one small "Show in Finder" item: heading/item size mismatch; menu floats over the deck tab row.
NIT C2/C3/C5: rename box text sits ~2 px lower than tab labels; box is exactly over the tab (C2 matches C1 x/width) and the showing-green is lost while editing. C5 box (100 pt) stays inside the window (right edge ~3401/3456 px) - OK; the "+" tab button is not visible in C5 (not verifiable if pre-existing).
NIT C10: "Show in Finder" looks fully enabled for a missing file; no disabled/dim state.
