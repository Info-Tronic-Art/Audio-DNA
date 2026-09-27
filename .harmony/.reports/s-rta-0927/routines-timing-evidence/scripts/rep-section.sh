# --- REPEAT (diagnostic, s-rta-0927 routines-timing scratch only): the two flaky starts, REPS times each -----
REPS="${REPS:-12}"
for k in $(seq 1 "$REPS"); do
    # jump start (row 11j start part), once mode
    P /api/routine/stop '{"all":true}' >/dev/null
    P /api/routine/set '{"slot":0,"restoreStyle":"jump","loop":false}' >/dev/null
    P /api/set_layer_opacity '{"layer":0,"opacity":0.1}' >/dev/null
    sleep 1
    TEK="$(python3 "$RT" edge)"
    [ "$TEK" = "NA" ] && continue
    python3 "$RT" sample 3.0 "$OUT/rjump$k.json" >/dev/null &
    RS=$!
    P /api/routine/fire '{"slot":0}' >/dev/null
    TJK="$(python3 "$RT" wait 0 running 2.6)"
    wait "$RS"
    echo "{\"ev\":\"rjump\",\"k\":$k,\"TJ\":\"$TJK\"}" >> "$TIMING_LOG"
    rows python3 "$RT" jump "$TJK" "$OUT/rjump$k.json" | sed "s/^/[rep $k] /"
    # stack (row 11): Probe Ramp, then Probe Routine (ease) on the next bar
    P /api/routine/stop '{"all":true}' >/dev/null
    P /api/routine/set '{"slot":0,"restoreStyle":"ease","loop":false}' >/dev/null
    sleep 0.5
    P /api/routine/fire '{"slot":1}' >/dev/null
    T5K="$(python3 "$RT" wait 1 running 2.6)"
    [ "$T5K" = "NA" ] && continue
    P /api/routine/fire '{"slot":0}' >/dev/null
    python3 "$RT" sample "$(perl -e "printf '%.2f', 3.2 - ($(now) - $T5K)")" "$OUT/rstack$k.json" >/dev/null
    T6K="$("$PXPY" -c "
import json
s = json.load(open('$OUT/rstack$k.json'))
t = [r['t'] for r in s if r.get('bank') and r['bank'][0]['state'] == 'running']
print('%.3f' % t[0] if t else 'NA')")"
    echo "{\"ev\":\"rstack\",\"k\":$k,\"T5\":\"$T5K\",\"T6\":\"$T6K\"}" >> "$TIMING_LOG"
    [ "$T6K" != "NA" ] && rows python3 "$RT" stack "$T5K" "$T6K" "$OUT/ramp.json" "$OUT/rstack$k.json" | grep "later restore wins" | sed "s/^/[rep $k] /"
done
P /api/routine/stop '{"all":true}' >/dev/null

