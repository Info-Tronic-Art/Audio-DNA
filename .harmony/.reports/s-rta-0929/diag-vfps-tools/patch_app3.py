#!/usr/bin/env python3
"""patch_app3.py -- DIAG-VFPS TEMPORARY round 3 (after patch_app2.py): GL thread CPU time (kind 17 per frame: thread CPU ns
at renderOpenGL entry / exit; kind 7 at every JUCE swap / wait hook), ADNA_VFPS_QOS=3 (decode threads UTILITY) / 4 (GL
USER_INTERACTIVE only), ADNA_VFPS_DECSTAGGER=ms (decoder i sleeps ms x (i mod 4) after a ring-full wake, before sws)."""
W = "/Users/boriskarpman/projects/RealTimeAudio/.claude/worktrees/rta0929-vfps"
def patch(path, old, new):
    p = W + "/" + path; s = open(p, newline="").read()
    assert s.count(old) == 1, (path, s.count(old), old[:60])
    open(p, "w", newline="").write(s.replace(old, new)); print("patched", path)
patch("src/diag/DiagVfps.h", "    int cpu() noexcept;                        // pthread_cpu_number_np of the calling thread (-1 = unknown)\n",
      "    int cpu() noexcept;                        // pthread_cpu_number_np of the calling thread (-1 = unknown)\n"
      "    uint64_t threadCpuNs() noexcept;           // CLOCK_THREAD_CPUTIME_ID of the calling thread\n")
patch("src/diag/DiagVfps.mm", "int cpu() noexcept { size_t c = 0; return pthread_cpu_number_np(&c) == 0 ? (int) c : -1; }\n",
      "int cpu() noexcept { size_t c = 0; return pthread_cpu_number_np(&c) == 0 ? (int) c : -1; }\n"
      "uint64_t threadCpuNs() noexcept { return clock_gettime_nsec_np(CLOCK_THREAD_CPUTIME_ID); }\n")
patch("src/diag/DiagVfps.mm", "    void hookFn(int kind, double a, double b, double c, double d) { rec(kind, a, b, c, d, 0.0); }\n",
      "    void hookFn(int kind, double a, double b, double c, double d)\n    {\n        rec(kind, a, b, c, d, 0.0);\n"
      "        if (kind == 2 || kind == 3)   // the GL render thread: its CPU time at this point (kind 7)\n"
      "            rec(7, (double) now(), (double) threadCpuNs(), (double) kind);\n    }\n")
patch("src/render/Renderer.cpp", "        uint64_t t0 = dvf::now();\n        int c0 = dvf::cpu();\n",
      "        uint64_t t0 = dvf::now();\n        int c0 = dvf::cpu();\n        uint64_t cpuNs0 = dvf::on() ? dvf::threadCpuNs() : 0;\n")
patch("src/render/Renderer.cpp", "            dvf::rec(12, (double) t0, (double) tRs, (double) tComp, (double) tChain, (double) tPres);\n",
      "            dvf::rec(12, (double) t0, (double) tRs, (double) tComp, (double) tChain, (double) tPres);\n"
      "            dvf::rec(17, (double) t0, (double) cpuNs0, (double) dvf::threadCpuNs());\n")
patch("src/diag/DiagVfps.h", "        int qos = 0;          // 1: GL thread USER_INTERACTIVE, decode threads USER_INITIATED; 2: GL thread UTILITY\n",
      "        int qos = 0;          // 1: GL USER_INTERACTIVE + decode threads USER_INITIATED; 2: GL UTILITY; 3: decode threads UTILITY only;\n"
      "                              // 4: GL USER_INTERACTIVE only (decoders unchanged)\n")
patch("src/render/Renderer.cpp", "            if (dvf::arms().qos == 1) pthread_set_qos_class_self_np(QOS_CLASS_USER_INTERACTIVE, 0);\n",
      "            if (dvf::arms().qos == 1 || dvf::arms().qos == 4) pthread_set_qos_class_self_np(QOS_CLASS_USER_INTERACTIVE, 0);\n")
patch("src/media/VideoPlayer.cpp", "    if (dvf::arms().qos == 1)   // DIAG-VFPS TEMPORARY: the H2 counterfactual\n        pthread_set_qos_class_self_np(QOS_CLASS_USER_INITIATED, 0);\n",
      "    if (dvf::arms().qos == 1)   // DIAG-VFPS TEMPORARY: the H2 counterfactual\n        pthread_set_qos_class_self_np(QOS_CLASS_USER_INITIATED, 0);\n"
      "    else if (dvf::arms().qos == 3)\n        pthread_set_qos_class_self_np(QOS_CLASS_UTILITY, 0);\n")
patch("src/diag/DiagVfps.h", "        int iosurf = 0;       // c4a: IOSurface ring slots + CGLTexImageIOSurface2D rectangle textures + a GPU blit per new frame\n",
      "        int iosurf = 0;       // c4a: IOSurface ring slots + CGLTexImageIOSurface2D rectangle textures + a GPU blit per new frame\n"
      "        int decStagger = 0;   // Q1: N ms x (player index mod 4) sleep before a ring-full-woken conversion (de-syncs the decoders)\n")
patch("src/diag/DiagVfps.mm", '    theArms.iosurf = envInt("ADNA_VFPS_IOSURF");\n', '    theArms.iosurf = envInt("ADNA_VFPS_IOSURF");\n    theArms.decStagger = envInt("ADNA_VFPS_DECSTAGGER");\n')
patch("src/diag/DiagVfps.mm", "\\tiosurf=%d\\n\", tb.numer", "\\tiosurf=%d\\tdecStagger=%d\\n\", tb.numer")
patch("src/diag/DiagVfps.mm", "theArms.upcap, theArms.iosurf);\n", "theArms.upcap, theArms.iosurf, theArms.decStagger);\n")
patch("src/media/VideoPlayer.cpp", """    const uint64_t dvfW0 = dvf::now();   // DIAG-VFPS TEMPORARY
    while ((s = ring_.acquireWrite()) < 0)   // the ring is full: the reader frees a slot and notifies
    {""", """    const uint64_t dvfW0 = dvf::now();   // DIAG-VFPS TEMPORARY
    bool dvfWaited = false;
    while ((s = ring_.acquireWrite()) < 0)   // the ring is full: the reader frees a slot and notifies
    {
        dvfWaited = true;""")
patch("src/media/VideoPlayer.cpp", """    const uint64_t dvfS0 = dvf::now();   // DIAG-VFPS TEMPORARY
    convertInto(s);""", """    if (dvfWaited && dvf::arms().decStagger > 0)   // DIAG-VFPS TEMPORARY (Q1): de-synchronise the 4 decoders' bursts
    {
        static std::atomic<int> dvfNextIdx { 0 };
        static thread_local int dvfIdx = dvfNextIdx.fetch_add(1);
        std::this_thread::sleep_for(std::chrono::milliseconds(dvf::arms().decStagger * (dvfIdx % 4)));
    }
    const uint64_t dvfS0 = dvf::now();   // DIAG-VFPS TEMPORARY
    convertInto(s);""")
patch("src/media/VideoPlayer.cpp", '#include <sys/qos.h>\n', '#include <sys/qos.h>\n#include <thread>   // DIAG-VFPS TEMPORARY\n')
