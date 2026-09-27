# verify item (c): compile ApiServer.cpp with the AudioDNA target's exact command, once as built (flag ON) and once
# with -DAUDIODNA_TEST_SERVER=1 removed (what AUDIODNA_BUILD_TEST_SERVER=OFF compiles); count the route string.
import json, subprocess, shlex
WT='/Users/boriskarpman/projects/RealTimeAudio/.claude/worktrees/rta0927-w7'
S='/private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/92434c6b-4aab-4138-83af-d5f37c430398/scratchpad/beatclock'
cc = json.load(open(WT + '/build-lane/compile_commands.json'))
e = [x for x in cc if x['file'].endswith('src/api/ApiServer.cpp') and 'AudioDNA.dir' in x.get('output', x['command'])][0]
args = shlex.split(e['command'])
for tag, drop in (('ON', False), ('OFF', True)):
    a = [x for x in args if not (drop and x == '-DAUDIODNA_TEST_SERVER=1')]
    i = a.index('-o'); a[i + 1] = S + '/ApiServer-%s.o' % tag
    print(tag, 'define present:', '-DAUDIODNA_TEST_SERVER=1' in a)
    r = subprocess.run(a, cwd=e['directory'], capture_output=True, text=True)
    print(tag, 'compile exit', r.returncode, r.stderr[-300:] if r.returncode else '')
    n = subprocess.run('strings %s/ApiServer-%s.o | grep -c stall_message_thread' % (S, tag), shell=True, capture_output=True, text=True).stdout.strip()
    print(tag, 'route string count in ApiServer.o:', n)
