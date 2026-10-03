# critic-bf10-live
STATUS: DONE
VERDICT: PASS_WITH_NITS
Images read: look_pre_m2.png, look_pre_m6.png, look_lane_m6.png, look_lane_m9a.png, look_lane_rewarm.png, look_ctl.png
SHOULD: look_lane_rewarm.png - after a canvas size change the first MilkDrop frames flash (one tile near-white, others dark grey static streaks; lane2.log m3_rewarm L/L0 = 4.78 at +0.1s, lane1 0.21 -> 1.63 within 0.6s). A VJ sees a white strobe / noise burst on a resize. Fix: hold the previous frame (or black) until the new FBO has warmed, or clear and fade in.
NIT: look_lane_m6.png - flat orange fill, so it proves geometry only (letter/pillar-box, no black block), not live picture content or motion.
NIT: look_ctl.png - control capture only (green flat fill; Preview letterbox and panel behave the same as the lane frames).
