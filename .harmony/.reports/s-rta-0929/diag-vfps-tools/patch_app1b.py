#!/usr/bin/env python3
"""patch_app1b.py -- DIAG-VFPS TEMPORARY round 1b (after patch_app.py): the Q1 upload-cap arm ADNA_VFPS_UPCAP=N (at most
N video picks + uploads per render frame; a player over the cap HOLDS its shown frame one more render frame)."""
W = "/Users/boriskarpman/projects/RealTimeAudio/.claude/worktrees/rta0929-vfps"
def patch(path, old, new):
    p = W + "/" + path; s = open(p, newline="").read()
    assert s.count(old) == 1, (path, s.count(old), old[:60])
    open(p, "w", newline="").write(s.replace(old, new)); print("patched", path)
patch("src/diag/DiagVfps.h", "        int pbo = 0;          // c3b: 1 = PBO (orphan + map + memcpy on the GL thread) then glTexSubImage2D from it\n",
      "        int pbo = 0;          // c3b: 1 = PBO (orphan + map + memcpy on the GL thread) then glTexSubImage2D from it\n        int upcap = 0;        // Q1: at most N video uploads per render frame (a player over the cap holds one more frame)\n")
patch("src/diag/DiagVfps.mm", '    theArms.pbo = envInt("ADNA_VFPS_PBO");\n', '    theArms.pbo = envInt("ADNA_VFPS_PBO");\n    theArms.upcap = envInt("ADNA_VFPS_UPCAP");\n')
patch("src/diag/DiagVfps.mm", '\\tpbo=%d\\tmainQos=%d\\n", tb.numer', '\\tpbo=%d\\tmainQos=%d\\tupcap=%d\\n", tb.numer')
patch("src/diag/DiagVfps.mm", "                 theArms.pbo, (int) qos_class_self());\n", "                 theArms.pbo, (int) qos_class_self(), theArms.upcap);\n")
patch("src/media/VideoPlayer.cpp", """    if (firstDrawMs_ < 0)
        firstDrawMs_ = nowMs();   // W3: the first draw request starts the first-frame timeout
""", """    if (firstDrawMs_ < 0)
        firstDrawMs_ = nowMs();   // W3: the first draw request starts the first-frame timeout
    // DIAG-VFPS TEMPORARY (Q1 counterfactual ADNA_VFPS_UPCAP): over this frame's upload cap -> hold (pick next frame).
    if (dvf::arms().upcap > 0 && dvf::frame().uploadN >= static_cast<uint64_t>(dvf::arms().upcap) && shown_.everShown
        && textureCreated_)
        return texture_;
""")
