import re
S = "/private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/73d4f54c-e9d5-409c-8b5d-694bfd57c171/scratchpad/bf10-fix"
def rm(s, chunk):
    assert s.count(chunk) == 1, chunk[:80]
    return s.replace(chunk, "")
def rep(s, a, b):
    assert s.count(a) == 1, a[:80]
    return s.replace(a, b)
sh = open(f"{S}/final/probe-milkdrop.sh").read()
nit1 = [l for l in sh.splitlines(True) if l.startswith('[ -n "${MILKDROP_APP:-}" ] && [ -n "${VIDEO_APP:-}" ]')]
assert len(nit1) == 1
shC = rm(sh, nit1[0])
r2block = shC[shC.index("# R2: the override is for calibrating P1"):shC.index('ATTACH="${MILKDROP_ATTACH:-0}"')]
shB = rm(shC, r2block)
shB = rm(shB, """#   MILKDROP_P1                replaces the pinned P1 ONLY with MILKDROP_MODE=pre (calibration); in lane (gate) mode
#                              the probe exits 2 (Harmony ruling R2). probe-milkdrop.py prints "P1 = <path>" first.
""")
open(f"{S}/inter/probe-milkdrop.sh.B", "w").write(shB)
open(f"{S}/inter/probe-milkdrop.sh.C", "w").write(shC)
py = open(f"{S}/final/probe-milkdrop.py").read()
pyC = rep(py, 'CA = np.array(FIX["colourA"], float)\n', 'CA, CB = np.array(FIX["colourA"], float), np.array(FIX["colourB"], float)\n')
open(f"{S}/inter/probe-milkdrop.py.C", "w").write(pyC)
st = open(f"{S}/final/probe-milkdrop-selftest.sh").read()
stB = rep(st, "(Harmony rulings R1 / R2 on review-bf10-gates-r1)", "(Harmony ruling R1 on review-bf10-gates-r1)")
stB = rm(stB, """#   R2  MILKDROP_P1 replaces the pinned P1 only in calibration mode (MILKDROP_MODE=pre); in lane (gate) mode the probe
#       exits 2; every probe-milkdrop.py run prints "P1 = <preset path>" first.
""")
stB = rep(stB, """ The py cases
# run the REAL probe-milkdrop.py with a row name that does not exist, so neither version sends a request.
""", "\n")
stB = rm(stB, stB[stB.index("    p1refuse)\n"):stB.index("  esac\n")])
stB = rm(stB, stB[stB.index('echo "R2 -- MILKDROP_P1 (probe-milkdrop.sh'):stB.index('rm -rf "$D"\necho; echo "SELFTEST')])
open(f"{S}/inter/probe-milkdrop-selftest.sh.B", "w").write(stB)
