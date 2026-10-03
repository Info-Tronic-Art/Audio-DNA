# critic-bf10-visual
STATUS: DONE
VERDICT: PASS_WITH_NITS
Images read: look_pre_m2, look_pre_m6, look_lane_m6, look_lane_m9a, look_lane_rewarm, look_ctl
- SHOULD look_lane_m6 / look_ctl: only flat colour is shown in the Preview panel, so aspect and stretch of real MilkDrop content are not visible. Fix: one real-preset Preview-panel crop at 640x360 and 1080x1920.
- SHOULD look_lane_rewarm: after a canvas-size change one frame is a white-noise flash with hard 2x2 seams (bright top-right quadrant, mean luma 178 vs 37-50). The pre-lane app flashes too. Fix: hold the last frame or fade the flash in after the rebuild. If the file is a montage, say so.
- NIT look_lane_m9a: capture goes through render_frame, not the Preview panel, so letterbox is not exercised there.
