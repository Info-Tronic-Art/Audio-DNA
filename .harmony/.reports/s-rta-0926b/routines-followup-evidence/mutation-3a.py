# mutation-3a.py -- ITEM 3a: show the new ctest exercises the REAL beatsUntilBoundary. Mutates a COPY of
# RoutineEngine.cpp (never the committed file), compiles it with the test target's own flags, links a scratch
# test_routine_engine with that object swapped in, and runs the new TEST_CASE against each mutant.
import json, shlex, subprocess, sys, hashlib
WT = '/Users/boriskarpman/projects/RealTimeAudio/.claude/worktrees/wf_156f9cfa-a36-1'
SP = '/private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/a07fe8e4-258f-47f0-88b1-97bfb5129bb6/scratchpad/routines-followup/mut'
SRC = WT + '/src/recording/RoutineEngine.cpp'
TESTS = WT + '/build-lane/tests'
CASE = 'RoutineEngine glide: the 2 Bar and 4 Bar boundary prediction puts the glide in the last beat before the start'
before = hashlib.sha256(open(SRC, 'rb').read()).hexdigest()
text = open(SRC).read()
cc = [e for e in json.load(open(WT + '/build-lane/compile_commands.json'))
      if e['file'].endswith('src/recording/RoutineEngine.cpp') and 'test_routine_engine.dir' in e['command']][0]
link = open(TESTS + '/CMakeFiles/test_routine_engine.dir/link.txt').read().strip()
mutants = {
    'M1-twobar-extra-bars-dropped': ('case RoutineSnap::TwoBar:  return toBar + kBeatsPerBar * ((2 - (lastBarCount_ + 1) % 2) % 2);',
                                     'case RoutineSnap::TwoBar:  return toBar;'),
    'M2-fourbar-parity-off-by-one': ('case RoutineSnap::FourBar: return toBar + kBeatsPerBar * ((4 - (lastBarCount_ + 1) % 4) % 4);',
                                     'case RoutineSnap::FourBar: return toBar + kBeatsPerBar * ((4 - (lastBarCount_) % 4) % 4);'),
}
for name, (old, new) in mutants.items():
    assert text.count(old) == 1, name
    msrc = '%s/%s.cpp' % (SP, name)
    open(msrc, 'w').write(text.replace(old, new))
    obj = '%s/%s.o' % (SP, name)
    args = shlex.split(cc['command'])
    i = args.index('-o'); args[i + 1] = obj
    args[args.index('-c') + 1] = msrc
    r = subprocess.run(args, cwd=cc['directory'], capture_output=True, text=True)
    if r.returncode: print(name, 'COMPILE FAILED', r.stderr[-2000:]); sys.exit(1)
    exe = '%s/test_routine_engine-%s' % (SP, name)
    largs = shlex.split(link)
    largs = [obj if a == 'CMakeFiles/test_routine_engine.dir/__/src/recording/RoutineEngine.cpp.o' else a for a in largs]
    largs[largs.index('-o') + 1] = exe
    r = subprocess.run(largs, cwd=TESTS, capture_output=True, text=True)
    if r.returncode: print(name, 'LINK FAILED', r.stderr[-2000:]); sys.exit(1)
    r = subprocess.run([exe, CASE], capture_output=True, text=True)
    out = r.stdout.splitlines()
    print('=== %s (%s -> %s): exit %d' % (name, old.split('return ')[1], new.split('return ')[1], r.returncode))
    for ln in out:
        if 'FAILED' in ln or ln.startswith('with expansion') or ln.startswith('test cases') or ln.startswith('assertions') or ln.startswith('  ') and ('SECTION' in ln or 'Bar' in ln):
            print(ln)
after = hashlib.sha256(open(SRC, 'rb').read()).hexdigest()
print('committed RoutineEngine.cpp sha256 before %s after %s (%s)' % (before, after, 'UNCHANGED' if before == after else 'CHANGED!'))
