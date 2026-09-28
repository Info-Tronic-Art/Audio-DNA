import re,sys
p='/Users/boriskarpman/projects/RealTimeAudio/.claude/worktrees/rta0928-w4/src/recording/RoutineEngine.cpp'
s=open(p).read()
def rep(old,new,count=1):
    global s
    n=s.count(old)
    assert n==count, (old,n)
    s=s.replace(old,new)
rep('#include <cmath>\n','#include <cmath>\n#include "diag/DiagTrace.h"\n')
# startNow
rep('''void RoutineEngine::startNow(Running& r)
{
''','''void RoutineEngine::startNow(Running& r)
{
    DIAG_SCOPE_MIN("eng.startNow", -1);
    diag::event("startNow", r.slot, r.restore ? 1 : 0, r.jump ? 1 : 0, r.glideScheduled ? 1 : 0);
''')
rep('''        const int discreteRefused = r.player->firePreambleDiscrete(*r.sink);
        r.preambleFired += static_cast<int>(r.program->preamble.size()) - discreteRefused;
        r.preambleRefused += discreteRefused;
        refused = discreteRefused;''','''        int discreteRefused = 0;
        { DIAG_SCOPE_MIN("eng.start.firePreambleDiscrete", -1); discreteRefused = r.player->firePreambleDiscrete(*r.sink); }
        r.preambleFired += static_cast<int>(r.program->preamble.size()) - discreteRefused;
        r.preambleRefused += discreteRefused;
        refused = discreteRefused;''')
rep('''            stepGlides(r);   // due glides land and let go here; spills go on
            refused += r.glideRefused;''','''            { DIAG_SCOPE_MIN("eng.start.stepGlides", -1); stepGlides(r); }   // due glides land and let go here; spills go on
            refused += r.glideRefused;''')
rep('''            const int continuousRefused = r.player->firePreambleContinuous(*r.sink);
            r.preambleFired += static_cast<int>(r.program->preambleContinuous.size()) - continuousRefused;
            r.preambleRefused += continuousRefused;
            refused += continuousRefused;''','''            int continuousRefused = 0;
            { DIAG_SCOPE_MIN("eng.start.firePreambleContinuous", -1); continuousRefused = r.player->firePreambleContinuous(*r.sink); }
            r.preambleFired += static_cast<int>(r.program->preambleContinuous.size()) - continuousRefused;
            r.preambleRefused += continuousRefused;
            refused += continuousRefused;''')
rep('''    r.glideRefused = 0;
    r.player->advanceTo(0.0, *r.sink);
''','''    r.glideRefused = 0;
    { DIAG_SCOPE_MIN("eng.start.advanceTo0", -1); r.player->advanceTo(0.0, *r.sink); }
''')
rep('''    notify(msg + ".");
}''','''    { DIAG_SCOPE_MIN("eng.start.notify", -1); notify(msg + "."); }
}''')
# tick
rep('''    ROUTINE_ENGINE_ASSERT_MESSAGE_THREAD();
    lastForcedSnap_ = forcedSnap;   // s-rta-0927: a pending slot's `startsOn`

    // Plan 4.2 step 1''','''    ROUTINE_ENGINE_ASSERT_MESSAGE_THREAD();
    DIAG_SCOPE_MIN("eng.tick", 0.5);
    lastForcedSnap_ = forcedSnap;   // s-rta-0927: a pending slot's `startsOn`

    // Plan 4.2 step 1''')
rep('''            r.player->stop(*r.sink);
            ++r.restarts;
            startNow(r);''','''            { DIAG_SCOPE_MIN("eng.restart.playerStop", -1); r.player->stop(*r.sink); }
            ++r.restarts;
            startNow(r);''')
rep('''            // Everything due up to the end first, so a point just before it is never skipped.
            r.player->advanceTo(r.lengthBeats, *r.sink);
''','''            // Everything due up to the end first, so a point just before it is never skipped.
            DIAG_SCOPE_MIN("eng.loopEnd", -1);
            diag::event("loopEnd", r.slot, r.loop ? 1 : 0, r.restore ? 1 : 0, r.glideScheduled ? 1 : 0);
            { DIAG_SCOPE_MIN("eng.end.advanceToEnd", -1); r.player->advanceTo(r.lengthBeats, *r.sink); }
''')
rep('''            r.player->stop(*r.sink);   // every grip released (R9)
            if (r.loop && r.lengthBeats > 0.0)''','''            { DIAG_SCOPE_MIN("eng.end.playerStop", -1); r.player->stop(*r.sink); }   // every grip released (R9)
            if (r.loop && r.lengthBeats > 0.0)''')
rep('''                    const int discreteRefused = r.player->firePreambleDiscrete(*r.sink);
                    r.preambleFired += static_cast<int>(r.program->preamble.size()) - discreteRefused;
                    r.preambleRefused += discreteRefused;
                    if (!glidePath)
                    {
                        const int continuousRefused = r.player->firePreambleContinuous(*r.sink);''','''                    int discreteRefused = 0;
                    { DIAG_SCOPE_MIN("eng.loop.firePreambleDiscrete", -1); discreteRefused = r.player->firePreambleDiscrete(*r.sink); }
                    r.preambleFired += static_cast<int>(r.program->preamble.size()) - discreteRefused;
                    r.preambleRefused += discreteRefused;
                    if (!glidePath)
                    {
                        int continuousRefused = 0;
                        { DIAG_SCOPE_MIN("eng.loop.firePreambleContinuous", -1); continuousRefused = r.player->firePreambleContinuous(*r.sink); }''')
rep('''                    r.player->advanceTo(pos, *r.sink);
                    stepGlides(r);   // due return glides land here (their t1 is this new startBeat); spills go on''','''                    { DIAG_SCOPE_MIN("eng.loop.advanceTo", -1); r.player->advanceTo(pos, *r.sink); }
                    { DIAG_SCOPE_MIN("eng.loop.stepGlides", -1); stepGlides(r); }   // due return glides land here (their t1 is this new startBeat); spills go on''')
rep('''        r.player->advanceTo(pos, *r.sink);
        r.position = pos;
''','''        { DIAG_SCOPE_MIN("eng.run.advanceTo", 0.25); r.player->advanceTo(pos, *r.sink); }
        r.position = pos;
''')
rep('''        stepGlides(r);   // after advanceTo: a same-tick Player touch cancels the glide before it writes
    }''','''        { DIAG_SCOPE_MIN("eng.run.stepGlides", 0.25); stepGlides(r); }   // after advanceTo: a same-tick Player touch cancels the glide before it writes
    }''')
rep('''            else
                stepGlides(r);   // plan3 C: the restore glides in over the last beat of the wait''','''            else
            { DIAG_SCOPE_MIN("eng.pend.stepGlides", 0.25); stepGlides(r); }   // plan3 C: the restore glides in over the last beat of the wait''')
rep('''    refreshBank(comp);
    publishStatus();
}

std::string RoutineEngine::fire(''','''    { DIAG_SCOPE_MIN("eng.refreshBank", 0.25); refreshBank(comp); }
    { DIAG_SCOPE_MIN("eng.publishStatus", 0.25); publishStatus(); }
}

std::string RoutineEngine::fire(''')
rep('''    ROUTINE_ENGINE_ASSERT_MESSAGE_THREAD();
    lastForcedSnap_ = forcedSnap;   // s-rta-0927: a pending slot's `startsOn`

    refreshBank(comp);''','''    ROUTINE_ENGINE_ASSERT_MESSAGE_THREAD();
    DIAG_SCOPE_MIN("eng.fire", -1);
    diag::event("fire", slot);
    lastForcedSnap_ = forcedSnap;   // s-rta-0927: a pending slot's `startsOn`

    refreshBank(comp);''')
rep('''    r.program = compileRoutine(*routine, comp);   // targets resolved once, on the active deck (D2)''','''    { DIAG_SCOPE_MIN("eng.fire.compileRoutine", -1); r.program = compileRoutine(*routine, comp); }   // targets resolved once, on the active deck (D2)''')
open(p,'w').write(s)
print("ok")
