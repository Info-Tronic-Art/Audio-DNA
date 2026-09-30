# attack-gopcache-vj -- blind seat, working-VJ temperament (plan-gopcache.md, main d88d2ea)
Read: the plan in full; VideoRing.h:60-147; VideoPlayer.cpp:320-440, 664-770; VideoPlayer.h:175-180; Renderer.cpp:1788-1800.
Verified = read in source; inferred = my reasoning.

## MUST
M1. reverseStep step 2 has the wrong sign / no catch-up (plan 3.5 lines 399-401). In reverse the writer is AHEAD when
    servedPts_ < want (it published frames below the clock). `servedPts_ - want > 3 fd` is true when the writer LAGS (served above
    the clock, e.g. the demand target, or any hitch), and the plan sends that case to idleWork() instead of publishing.
    Separately `next = servedPts_ - fd` never jumps to the clock: at speed 2x/4x, after a hold, or after a BPMSync speed spike
    the writer publishes one stale frame per step while the clock falls faster -- it never catches up (forward has the drop rule
    VideoPlayer.cpp:740 / shouldPublish; reverse has no equivalent). VJ feel: reverse at 2x+ is permanent lag.
    Fix: next = min(servedPts_ - fd, round(want/fd)*fd); lag test = `want - servedPts_ < -X` mirrored; add a ctest driving
    decodeStep at speed 2 and 4 reverse (uploads/s vs clock, late <= 3) and after a 300 ms stall.
M2. Cap enforcement is tied to store decisions (R-4: "evicts down to the cap at its next store decision"), but a fully resident
    reverser makes NO store decisions (planPrefetch returns none while unservedAhead >= prefetchAt; hits publish only).
    Player A holds 933 MB / whole file; player B joins, gets floor 8 frames; A never shrinks; B thrashes at G250 (~4.5 fps, today's
    number) for as long as A reverses. u8 misses this (four start together). Fix: per-step cheap check `bytes > capBytes()` ->
    evictOne (farthest-next-use), independent of stores; gate: ctest start A alone, then B, assert both under cap within N steps.
M3. Fixed 2 GiB budget, ASSUMED for a 32 GiB machine (R-4/Q1), is not graceful on a 8/16 GiB Mac: cache + 3 IOSurface slots per
    player + SeqVram 1 GiB + decoder pools + canvas FBOs; failure mode is swap / jetsam mid-set, no counter fires. Also no reaction
    to memory pressure at all. Fix: budget = min(2 GiB, hw.memsize/16) (Q1 answer), plus a DISPATCH_SOURCE_TYPE_MEMORYPRESSURE
    hook (message thread) that lowers the cap atomically (decode threads evict lazily via M2's check). Gate: u8 with a
    test-mode env/API cap of 256 MB must stay >= 20 uploads/s, late <= 40, bytes <= cap + floors.
M4. Forward->reverse flip on a Loop/OneShot clip is the VJ's most common gesture (hit Reverse on a playing clip) and the plan
    gives it a cold DEMAND run: forward Loop stores nothing (R-9), so the BEHIND pool (which only helps reverse->forward) is
    empty. Hold = keyframe seek + catch-up, avg half GOP: g250 1080p ~250 ms, 4K ~850 ms, worst 500 ms / 1.7 s (plan's own
    numbers, 3.7); the plan calls that "today's cost" -- true, but the lane's stated goal is "as smooth as forward" and no gate
    covers it. The reversing layer freezes on every flip during the show. Fix: (a) gate it: u7 scene "flip_reverse" (trigger,
    2 s forward, setReverse via REST, 3 s) with max inter-upload gap <= 150 ms at 1080p g30; (b) cheap mitigation: while a
    Loop/OneShot clip plays FORWARD, keep the last kBehindFrames (16, ~50 MB at 1080p) of published frames -- instant reverse
    cover while the DEMAND run decodes (this is R-9's retention with a small fixed size, all modes).
M5. Deck switch = cache dropped (R-10: trim at 1 s off screen; Rule 15 decks keep time). Return to a deck with a reversing
    clip (or a column of four) = cold demand run x4 plus a fill burst (4 x 2 FFmpeg threads, plan risk 4) at the exact moment
    of the switch, on the render thread's cores. 16 decks: only drawn players hold cache, OK for memory, but the return hitch
    is unbounded by any gate (u9 checks only bytes==0 and late <= 10 AFTER settle). Fix: gate the switch-back hitch (max
    inter-upload gap on the returned layer) and stagger fill runs across players (one PREFETCH burst at a time via the shared
    Budget struct; DEMAND runs unrestricted), so a column return is not 4 simultaneous laps.

## SHOULD
S1. Gate blindness: late <= 10 per 5 s (u7 turn scenes, 4.2) allows a ~330 ms freeze at the turn and still PASS; uploads >=
    28.5/s over 5 s tolerates a 250 ms hold. The plan's own prediction is late 0-4. Add a max-gap gate (largest inter-upload
    interval <= 100 ms, per turn) via a per-upload timestamp ring in state, and cut late to <= 4 on turn scenes.
S2. Plan text says "kSlots 8" (2, tradeoffs) but VideoPlayer.h:175 is kSlots = 3, lookahead 2, and the shown slot is held
    (retire_): the writer effectively has 2 slots. All amortization math ("~6 frames per re-seek", "2 wasted ring frames per
    flip") must be re-derived at N=3; at N=3 a flip that bumps the gen can waste BOTH writable slots plus the ring is empty
    for the round trip. Add a ctest for the turn at N=3 (time-to-first-published-frame after a gen bump, in decodeStep counts).
S3. R-9 PingPong retention is sized from decodeMsEma * GOP * 2 but capped at cap/2 and by the floor 16; at 4K g250 ~1.27 GB of
    forward-play memory for a clip that may never turn (top turn 8+ s away). Retain only when the clock is within a horizon of the
    top (frames-to-top * frameMs < retain window), else nothing. Forward PingPong of long clips otherwise pays memory + a 1 ms
    copy per frame for nothing.
S4. Mid-reverse clip switch/set load: plan says retire -> thread exits -> freeFfmpeg frees cache. The thread may be in a run
    (up to one GOP decode of 6.8 ms frames) or in a 2 s pending state; Pitfall 58's staged load + drainRetiredMedia's gate wait for
    thread exit -- the destructor now also waits for the cache free, and budget give() is only at freeFfmpeg, so the incoming
    reverser's cap is short until then. Gate: u9-style scene "switch clip mid-reverse, bytes -> 0 within 1 s, incoming clip's
    first upload gap".
S5. Speed 0 -> resume, and in-point/out-point in PingPong (Renderer.cpp:1836: a jump, not a reflection): each jump is a gen
    bump; with cache eviction distances modelled as a bounce, an in/out-point clip evicts the wrong frames (plan concedes
    "efficiency only") -- but for a VJ using in/out (the norm) the window is exactly the in..out range: cap the trajectory to it.
S6. Decode threads at fill: 4 players x (2 codec threads + 1) at full tilt with no QoS; the plan names the lever (thread_.wait(1))
    "off by default" -- ship it ON for PREFETCH runs when the render thread's frame time > 12 ms (already measured). 
S7. u9 (a) bytes >= 25 MB is a near-vacuous lower bound; use frames >= min(cap frames, 0.9 * file frames).

## NIT
N1. `next < -0.5 fd -> idleWork` at the wrap: the gen bump is one GL frame away; fine but state it produces at most one late frame.
N2. `kFar` (1<<30) sums in distance for OneShot may overflow int in key comparisons if added to cur; use int64 or saturate.
N3. VFR rounding (Q3): add a synthetic VFR ctest fixture now (cheap with -vsync vfr); a miss = a silent demand run per frame.
N4. Pitfall/CLAUDE.md text claims "never a stall or a black frame": true for the reader, but 4K x4 = ~20 fps reverse is a
    stall in VJ terms; document as a measured limit.

## Strongest counterargument to my own position
The plan already isolates the risky refactor (c1b), measures diagnosis first, and keeps the reader's hold path, so no MUST
below M1 is a correctness bug; M3/M4/M5 are product-quality asks that could be deferred (VideoToolbox is the real 4K fix, and
today's flip cost is identical). I still hold them MUST because the lane's own success sentence is "as smooth as forward" and
its gates (5 s averages, late <= 10) are structurally unable to fail on a single 300-800 ms hitch, while M1/M2 are defects in the
plan's pseudo-code that would ship a lane that passes u7 (single player, cold start) and fails on stage (speed, second reverser).
