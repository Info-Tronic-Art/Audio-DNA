# GATE-9 production absence: recompile named TUs from compile_commands.json without -DAUDIODNA_TEST_SERVER*, strings-check.
import json, shlex, subprocess, sys, os, re
M = '/Users/boriskarpman/projects/RealTimeAudio'; out = sys.argv[1]; os.makedirs(out, exist_ok=True)
want = ['src/audio/AudioEngine.cpp', 'src/audio/DeviceGuard.cpp', 'src/audio/DevicePolicy.cpp', 'src/api/ApiServer.cpp', 'src/MainComponent.cpp']
tokens = ['audio_deny', 'audio_stop', 'ADNA_AUDIO_DENY_DEVICES', 'setTestDeniedNames', 'debugSetDeniedDevices', 'debugStopDevice']
db = json.load(open(M + '/build/compile_commands.json'))
ok = True
for w in want:
    e = [x for x in db if x['file'].endswith(w) and '/tests/' not in x.get('output', x.get('command', ''))]
    if not e: print('G9 MISSING compile entry', w); ok = False; continue
    e = e[0]; args = e['arguments'] if 'arguments' in e else shlex.split(e['command'])
    dropped = [a for a in args if a.startswith('-DAUDIODNA_TEST_SERVER')]
    args = [a for a in args if not a.startswith('-DAUDIODNA_TEST_SERVER')]
    o = os.path.join(out, os.path.basename(w) + '.o')
    if '-o' in args: args[args.index('-o') + 1] = o
    r = subprocess.run(args, cwd=e['directory'], capture_output=True, text=True)
    if r.returncode: print('G9 COMPILE FAIL', w, r.stderr[-600:]); ok = False; continue
    s = subprocess.run(['strings', o], capture_output=True, text=True).stdout
    hits = {t: s.count(t) for t in tokens if t in s}
    print(f'G9 {w}: built (dropped {dropped}); token hits {hits if hits else 0}')
    ok = ok and not hits
print('GATE-9', 'PASS' if ok else 'FAIL')
