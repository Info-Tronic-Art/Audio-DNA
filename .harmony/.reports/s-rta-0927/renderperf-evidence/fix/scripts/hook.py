# TEMPORARY test hook (never committed): widen the window between a capture's signal and its caller's collection.
# usage: python hook.py add|remove
import sys, re
P = "/Users/boriskarpman/projects/RealTimeAudio/.claude/worktrees/rta0927-w8/src/render/Renderer.cpp"
HOOK = ("    { const char* d = std::getenv(\"AUDIODNA_TEST_CAPTURE_COLLECT_DELAY_MS\"); if (d) { std::cerr << \"[Eyes] TESTHOOK collect delay \" << d << \" ms\" << std::endl;"
        " std::this_thread::sleep_for(std::chrono::milliseconds(std::atoi(d))); } } // TEMP-HOOK\n")
s = open(P).read()
if sys.argv[1] == "add":
    assert "TEMP-HOOK" not in s
    anchors = ["    if (!future.get())\n        return false;\n", "    CaptureRead read = future.get();\n"]
    hit = [a for a in anchors if a in s]
    assert len(hit) == 1 and s.count(hit[0]) == 1, hit
    s = s.replace(hit[0], HOOK + hit[0])
    s = s.replace("#include <chrono>\n", "#include <chrono>\n#include <thread> // TEMP-HOOK-INC\n#include <cstdlib> // TEMP-HOOK-INC\n", 1)
    print("hook added at:", hit[0].strip())
else:
    s = "".join(l for l in s.splitlines(True) if "TEMP-HOOK" not in l)
    print("hook removed")
open(P, "w").write(s)
