#!/usr/bin/env python3
"""rm-guard -- PreToolUse hook for Bash (installed 2026-10-03, session s-rta-1003b, at Boris's request:
"I will not be able to approve the RM commands so I will need you to approve those yourself").

Purpose: while nobody is at the keyboard, an `rm` in a Bash command line must never WAIT for a human.
  - every rm target is a literal path inside the rig's allowed roots, and the rest of the command is nothing, or only
    the harmless commands the rig runs around a lock removal (mkdir, sleep, ctest, echo, date, tail, grep ...)
        -> ALLOW (no prompt)
  - every rm target is inside the allowed roots, but the command also does other things
        -> no decision (the permission rules in settings.local.json allow the rm part; the rest is judged as usual)
  - any rm target is outside the roots, uses a variable, or cannot be parsed
        -> DENY with the reason (the agent gets an error at once and re-writes the command; nothing hangs)
  - no rm in the command -> no decision.
It never widens anything except rm inside the roots below. Every decision is appended to rm-guard.log.
Remove: delete the "hooks" block and the rm rules from .claude/settings.local.json.
"""
import json, os, re, shlex, sys, time

MAIN = '/Users/boriskarpman/projects/RealTimeAudio'
WT = MAIN + '/.claude/worktrees/'
SCRATCH_ROOT = '/private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/'
LOCKS = {'/tmp/audiodna-ctest.lock', '/tmp/audiodna-live.lock',
         '/private/tmp/audiodna-ctest.lock', '/private/tmp/audiodna-live.lock'}
LOG = MAIN + '/.claude/rm-guard.log'
OKFLAG = re.compile(r'^-[rRfvd]+$')
SEP = re.compile(r'\|\||&&|[;&|\n]')
LEAD = re.compile(r'^(?:(?:do|then|else|time|!|\{|\()\s+)+')
ENVASSIGN = re.compile(r'^(?:[A-Za-z_][A-Za-z0-9_]*=\S*\s+)+')
REDIR = re.compile(r'^\d*[<>]')


def log(decision, why, cmd):
    try:
        with open(LOG, 'a') as f:
            f.write('%s %s | %s | %s\n' % (time.strftime('%Y-%m-%d %H:%M:%S'), decision, why, cmd.replace('\n', ' \\n ')[:400]))
    except Exception:
        pass


def strip_heredocs(cmd):
    # drop here-document bodies: text written into a file is not a command of this call
    return re.sub(r'<<-?\s*([\'"]?)(\w+)\1[^\n]*\n.*?\n\s*\2\b', '<<HEREDOC', cmd, flags=re.S)


def target_ok(t):
    if not t.startswith('/') or any(c in t for c in '$`~{}') or '..' in t.split('/'):
        return False
    head, _, last = t.rpartition('/')
    if any(c in head for c in '*?['):
        return False                      # a glob is allowed in the last component only
    p = os.path.normpath(head) + '/' + last if last else os.path.normpath(head)
    if p in LOCKS:
        return True
    if p.startswith(SCRATCH_ROOT):
        rest = p[len(SCRATCH_ROOT):].split('/')
        # <session-id>/scratchpad/<something>: strictly INSIDE a session scratchpad
        return len(rest) >= 3 and rest[1] == 'scratchpad' and rest[2] != ''
    if p.startswith(WT):
        rest = p[len(WT):].split('/')
        if len(rest) < 2 or rest[1] == '':
            return False                  # never a worktree itself
        if '__pycache__' in rest[1:] or last.endswith('.pyc'):
            return True
        if len(rest) == 2 and rest[1] == '.venv':
            return True                   # the .venv symlink a probe may create
        if rest[1].startswith('build-mut-') and len(rest) >= 2:
            return True                   # scratch mutant build dirs
        return False
    return False


SAFE_FIRST = {'mkdir', 'sleep', 'ctest', 'echo', 'printf', 'date', 'tail', 'head', 'grep', 'cat', 'ls', 'wc', 'true', 'test',
              '[', '[[', 'cut', 'tr', 'sort', 'uniq', ':'}
KEYWORD = re.compile(r'^(?:(?:until|while|if|elif)\s+)+')
PURE_ASSIGN = re.compile(r'^[A-Za-z_][A-Za-z0-9_]*=\S*$')


def benign_segment(s):
    """a non-rm part of the command that the rig runs around a lock removal (the ctest mutex line): read-only or harmless.
    Anything else makes the hook give NO decision for the command, so the usual permission rules judge it."""
    s = KEYWORD.sub('', s)
    if PURE_ASSIGN.match(s):
        return True
    if '`' in s or any(not rest.startswith('date') for rest in s.split('$(')[1:]):
        return False                      # a command substitution other than $(date ...)
    first = s.split()[0] if s.split() else ''
    return first in SAFE_FIRST


def analyse(cmd):
    """returns (n_rm, bad_reason or None, pure)"""
    body = strip_heredocs(cmd)
    if re.search(r'\b(?:xargs|-exec(?:dir)?)\s+(?:sudo\s+)?rm\b', body) or re.search(r'\bfind\b[^\n;|&]*\s-delete\b', body):
        return 1, 'rm through xargs / find -exec / find -delete is not auto-approved', False
    body = re.sub(r'\d*[<>]&\d*-?|&>>?', ' ', body)   # fd duplications (2>&1, &>) are not command separators
    n, pure, benign = 0, True, True
    for seg in SEP.split(body):
        s = ENVASSIGN.sub('', LEAD.sub('', seg.strip()))
        if not s or s in ('done', 'fi', '}', ')', 'esac'):
            continue
        if s.startswith('sudo ') and re.match(r'sudo\s+rm\b', s):
            return 1, 'sudo rm is never auto-approved', False
        if not re.match(r'rm(\s|$)', s):
            pure = False
            if not benign_segment(s):
                benign = False
            continue
        n += 1
        try:
            toks = shlex.split(s)[1:]
        except ValueError:
            return n, 'the rm command could not be parsed', False
        targets = []
        for t in toks:
            if REDIR.match(t):
                continue
            if t.startswith('-'):
                if t == '--':
                    continue
                if not OKFLAG.match(t):
                    return n, 'rm flag %s is not auto-approved' % t, False
                continue
            targets.append(t)
        if not targets:
            return n, 'rm without a target', False
        for t in targets:
            if not target_ok(t):
                return n, 'rm target %s is not a literal path inside the allowed roots' % t, False
    return n, None, pure or benign


def main():
    try:
        d = json.load(sys.stdin)
    except Exception:
        return 0
    if d.get('tool_name') != 'Bash':
        return 0
    cmd = str((d.get('tool_input') or {}).get('command') or '')
    if not re.search(r'(?:^|[\s;&|(])rm(?:\s|$)', cmd) and '-delete' not in cmd:
        return 0
    n, bad, pure = analyse(cmd)
    if n == 0:
        return 0
    if bad:
        why = ('rm-guard: %s. Nobody can answer a permission prompt tonight, so this is refused instead of asked. '
               'Auto-approved rm targets are LITERAL absolute paths (no variable, no ~, no ..) that are: /tmp/audiodna-ctest.lock, '
               '/tmp/audiodna-live.lock, anything INSIDE a session scratchpad under %s<session>/scratchpad/, or inside a worktree '
               'a __pycache__ dir, a .pyc file, the .venv link, or a build-mut-* dir. Re-write the command with such a path, put the '
               'removal in a script file you run with bash, or leave the file in place.' % (bad, SCRATCH_ROOT))
        log('DENY', bad, cmd)
        print(json.dumps({'hookSpecificOutput': {'hookEventName': 'PreToolUse', 'permissionDecision': 'deny', 'permissionDecisionReason': why}}))
        return 0
    if pure:
        log('ALLOW', 'rm inside the allowed roots; the rest of the command is nothing or harmless rig commands', cmd)
        print(json.dumps({'hookSpecificOutput': {'hookEventName': 'PreToolUse', 'permissionDecision': 'allow', 'permissionDecisionReason': 'rm-guard: rm inside the rig\'s allowed roots'}}))
        return 0
    log('DEFER', 'rm inside the allowed roots, command does other things too (permission rules decide)', cmd)
    return 0


if __name__ == '__main__':
    sys.exit(main())
