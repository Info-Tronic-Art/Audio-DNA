# Critic ui-ux (s-rta-1002b)
STATUS: DONE
VERDICT: PASS_WITH_NITS (0 MUST, 5 SHOULD, 6 NIT)
IMAGES READ: C1 C2 C3 C4 C5 C6 C6-crop C7 C7-crop C8 C9 C10 C11 C12 C13 C14 (+ zoomed crops of C2/C3/C5 tab rows)
SHOULD
- C2/C3: rename box barely reads as editable; only the select-all highlight (C2) tells it apart from a tab. C3 "Intro" box = dimmed navy tab, no border, no caret visible.
- C8/C9/C14: info rows tiny + low-contrast grey, crammed against DASHBOARD (~18 px gap) -- the codec is what Boris asked for.
- C7/C7-crop: tooltip wraps "30 frames per / second" (orphan word); inspector shows codec and size on two rows, tooltip jams them in one comma line.
- C8 vs C10 vs C13: info rows change inspector height; dashboard knobs jump ~21 px (video 746, picture/missing 725, source 705) when clicking between clips.
- C5: only the end-of-row case shown; a 100-px box over a 60-px MIDDLE tab covers the next tab(s) -- "exactly over the tab" is not true there. Unshot.
NIT
- C5: Deck 1 stays green while the box is on Deck 27; unclear which tab is "showing" (green?) -- if green = showing, box is on a non-showing tab.
- C10: "File missing" tiny red, Show in Finder looks enabled (opens parent folder or label message); no path, no hint.
- C11: picture shows kind only (no size).
- C14: long name truncated mid-phrase "Show i..." ~11 px from button; full name only in tooltip. 23.976 frames per second reads long.
- C6/C7 (different capture mode): white rectangle where the macro strip should be -- capture artifact?
- Non-showing tab: double-click renames only the showing tab (kRenameOnlyTheShowingTab); Boris said "each deck"; tooltip omits hint there.
- "Video file not loaded" state (ClipMediaText.h:68) was not captured; may show for clips on non-showing decks (INFERRED).
