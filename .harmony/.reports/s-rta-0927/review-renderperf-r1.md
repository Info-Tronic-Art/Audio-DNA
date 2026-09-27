# Reviewer Verdict — renderperf-r1
STATUS: PARTIAL
VERDICT: REQUEST_CHANGES
FILES: src/render/CompositorEngine.h, src/render/CompositorEngine.cpp, src/render/FrameRing.h (new),
       src/render/PixelConvert.h (new), src/render/Renderer.h, src/render/Renderer.cpp,
       tests/test_frame_ring.cpp (new), tests/test_pixel_convert.cpp (new), tests/CMakeLists.txt,
       .harmony/probe-render-state.py, .harmony/probe-render-state.json, .harmony/HANDOFF.md,
       .harmony/notebook.md, docs/claude/pitfalls.md, docs/claude/rendering.md
ISSUES: 1 blocking (MUST) -- C3 introduces a shared-member (capturePixels_/captureReadW_/H_/Ms_) that a second,
        overlapping captureFrame() call can overwrite between the first caller's promise resolving and its
        extraction under captureMutex_, delivering caller A wrong-but-correctly-sized pixel data silently
        (was: caller A safely timed out after 5s pre-C3). Builder already disclosed this honestly as
        found_not_fixed #5 ("not exercised"); not fixed in this diff. All plan conformance (C0-C3, ring lazy
        alloc, PixelConvert byte-identity, docs/probe updates, MUST-NOT-CHANGE list, fence, scope) verified
        correct against the plan and against direct source reading; RED/GREEN evidence and mutation-teeth are
        real and reproduce the claimed numbers.
METADATA: reviewer=reviewer-agent, builder_packet=renderperf-r1, date=2026-09-27
