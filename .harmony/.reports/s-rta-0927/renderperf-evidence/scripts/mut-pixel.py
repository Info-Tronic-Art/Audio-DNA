import json, subprocess, shlex, hashlib, os, re
WT = "/Users/boriskarpman/projects/RealTimeAudio/.claude/worktrees/rta0927-w8"
SP = "/private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/92434c6b-4aab-4138-83af-d5f37c430398/scratchpad/renderperf"
B = WT + "/build-lane"; M = SP + "/mutpx"
os.makedirs(M + "/render", exist_ok=True)
def sha(p): return hashlib.sha256(open(p, "rb").read()).hexdigest()
before = sha(WT + "/src/render/PixelConvert.h")
src = open(WT + "/src/render/PixelConvert.h").read()
old = "            p.premultiply();\n"
assert src.count(old) == 1
open(M + "/render/PixelConvert.h", "w").write(src.replace(old, "            // p.premultiply();   // MUTATION\n"))
e = [x for x in json.load(open(B + "/compile_commands.json")) if x["file"].endswith("tests/test_pixel_convert.cpp")][0]
args = shlex.split(e["command"])
oi = args.index("-o"); objrel = args[oi + 1]; args[oi + 1] = M + "/mut.o"
args.insert(1, "-I" + M)
r = subprocess.run(args, cwd=e["directory"], capture_output=True, text=True); print("compile rc", r.returncode, r.stderr[-400:])
link = shlex.split(open(B + "/tests/CMakeFiles/test_pixel_convert.dir/link.txt").read())
link = [M + "/mut.o" if a == objrel else a for a in link]
li = link.index("-o"); link[li + 1] = M + "/bin-mut"
r = subprocess.run(link, cwd=B + "/tests", capture_output=True, text=True); print("link rc", r.returncode, r.stderr[-400:])
print(subprocess.run(["diff", WT + "/src/render/PixelConvert.h", M + "/render/PixelConvert.h"], capture_output=True, text=True).stdout)
r = subprocess.run([M + "/bin-mut"], capture_output=True, text=True)
out = r.stdout + r.stderr
for line in out.splitlines():
    if re.search(r"FAILED|test cases|assertions|^  CHECK|differingRows|Type", line): print(line)
print("mutant exit", r.returncode)
after = sha(WT + "/src/render/PixelConvert.h")
print("deliverable PixelConvert.h sha256 before", before, "after", after, "unchanged" if before == after else "CHANGED")
