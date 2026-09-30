# TSan / ASan sweep harness (from s-rta-0929b, parameterized s-rta-0930)
Env: SWEEP_SP (scratch root), SWEEP_DIR (default $SWEEP_SP/sweep; must contain these scripts), SWEEP_MEDIA (fixtures, made by mkfix.sh),
LOCK_LIB (lock.sh helper), LANE (lock owner, default sweep), TSAN_APP / ASAN_APP (app bundle under test), SWEEP_ROOT (source root whose src/ + build/_deps basenames count as app frames).
Run: bash run_batch.sh tsan <batch> "1:a 2:b 3:c ..." ; then python analyze.py (reads $SWEEP_DIR/runs/tsan-*/tsan.* + launches.tsv) -> unique.json / table.
Scenarios (scen.py): a = idle 15 s; b = load 16x1080p + 4x4K composition, trigger_column 0, hold 20 s; c = mixed composition + 17 REST calls (trigger_clip / trigger_column / switch_deck / audio/source / load_source / set_layer_opacity).
Options baked in: TSAN_OPTIONS=halt_on_error=0:abort_on_error=0:exitcode=0:report_signal_unsafe=0 ; ASAN abort_on_error=0.
