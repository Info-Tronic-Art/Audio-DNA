#!/bin/bash
# probe-asan-unit.sh -- lane bf9b fix (s-rta-1003; ruling .harmony/.reports/s-rta-1003/ruling-bf9b-merge.md AM-5, gate
# B3b). The AddressSanitizer unit gate, probe-tsan-unit.sh's shape: configures a -DADNA_SANITIZE=address build dir
# (RelWithDebInfo, TEST_SERVER ON, SYPHON ON, FetchContent sources from a local _deps dir) if it is absent, builds the
# targets that host [asan] cases, runs `ctest -L asan --no-tests=error --output-on-failure` and exits with ctest's code.
# Fails closed: exit 3 when `ctest -L asan` finds fewer than EXPECTED_ASAN_CASES (a dropped tag, a failed
# catch_discover_tests, a target missing from TARGETS) -- "No tests were found" is never a pass.
# REQUIRED before merging any change to code that keeps a raw pointer into the show model across a fenced edit (the
# Layer / Clip inspectors, the stack-move hook: Pitfall 33).
# Launches NO app. Puts no ASAN_OPTIONS in the environment: each asan test's ENVIRONMENT property pins them
# (abort_on_error=0:halt_on_error=1) and its FAIL_REGULAR_EXPRESSION fails a case on any "ERROR: AddressSanitizer"
# (tests/CMakeLists.txt, ADNA_ASAN_TEST_PROPERTIES).
# Last line: `PROBE-ASAN-UNIT GREEN (<n> cases, 0 reports)` or `PROBE-ASAN-UNIT RED (<k> of <n> failed)`.
#
# usage: probe-asan-unit.sh [build-dir]
#   build-dir      default: <tree>/build-asan (never point it at build/, the live Release dir)
#   ADNA_DEPS_DIR  FetchContent sources (juce-src, httplib-src, catch2-src, melatonin_inspector-src, syphon-src);
#                  default: <main checkout>/build/_deps (the checkout that owns this tree's .git), used only for a
#                  fresh configure
#   ADNA_JOBS      build parallelism (default 3)
set -u
TREE=$(cd "$(dirname "$0")/.." && pwd)
B=${1:-$TREE/build-asan}
JOBS=${ADNA_JOBS:-3}
# The targets that host [asan] cases (each registered under LABELS asan by a second catch_discover_tests in
# tests/CMakeLists.txt). A new hosting target must be added here AND its cases counted in EXPECTED_ASAN_CASES: a
# target missing from this list is not (re)built.
TARGETS=(test_show_model)
# AS0 (the "bf9b fix: a fenced edit that moves or resizes the shared layer stack ..." case), AS1, AS2, AS3b, AS5, AS6,
# AS7 (4 at FIX-1's first commit: AS0, AS5, AS6, AS7); FIX-2 (AM-7): T6f, T6g, T6j.
EXPECTED_ASAN_CASES=10

if [ ! -f "$B/CMakeCache.txt" ]; then
    COMMON=$(git -C "$TREE" rev-parse --path-format=absolute --git-common-dir 2>/dev/null)
    D=${ADNA_DEPS_DIR:-$(dirname "$COMMON")/build/_deps}
    for dep in juce httplib catch2 melatonin_inspector syphon; do
        [ -d "$D/$dep-src" ] || { echo "probe-asan-unit: missing $D/$dep-src (set ADNA_DEPS_DIR)" >&2; exit 2; }
    done
    mkdir -p "$B"
    echo "probe-asan-unit: configure $B (ADNA_SANITIZE=address, RelWithDebInfo) $(date '+%F %T')"
    cmake -S "$TREE" -B "$B" -DCMAKE_BUILD_TYPE=RelWithDebInfo -DADNA_SANITIZE=address \
        -DAUDIODNA_BUILD_TEST_SERVER=ON -DAUDIODNA_BUILD_SYPHON=ON -DFETCHCONTENT_FULLY_DISCONNECTED=ON \
        -DFETCHCONTENT_SOURCE_DIR_JUCE="$D/juce-src" -DFETCHCONTENT_SOURCE_DIR_HTTPLIB="$D/httplib-src" \
        -DFETCHCONTENT_SOURCE_DIR_CATCH2="$D/catch2-src" \
        -DFETCHCONTENT_SOURCE_DIR_MELATONIN_INSPECTOR="$D/melatonin_inspector-src" \
        -DFETCHCONTENT_SOURCE_DIR_SYPHON="$D/syphon-src" -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
        > "$B/configure.log" 2>&1 || { echo "probe-asan-unit: configure FAILED (see $B/configure.log)" >&2; exit 2; }
fi
if ! grep -q '^ADNA_SANITIZE:STRING=address$' "$B/CMakeCache.txt"; then
    echo "probe-asan-unit: $B is not a -DADNA_SANITIZE=address build dir" >&2
    exit 2
fi

echo "probe-asan-unit: build ${TARGETS[*]} $(date '+%F %T')"
cmake --build "$B" --target "${TARGETS[@]}" -j"$JOBS" || { echo "probe-asan-unit: build FAILED" >&2; exit 2; }

FOUND=$(ctest --test-dir "$B" -L asan -N 2>/dev/null | sed -n 's/^Total Tests: *//p')
FOUND=${FOUND:-0}
echo "probe-asan-unit: ctest -L asan finds $FOUND asan cases (expected $EXPECTED_ASAN_CASES)"
if [ "$FOUND" -lt "$EXPECTED_ASAN_CASES" ]; then
    echo "probe-asan-unit: FAIL -- $FOUND asan cases, expected $EXPECTED_ASAN_CASES (tag dropped or discovery failed?)" >&2
    exit 3
fi
echo "probe-asan-unit: ctest -L asan $(date '+%F %T')"
LOG=$(mktemp "${TMPDIR:-/tmp}/probe-asan-unit.XXXXXX")
ctest --test-dir "$B" -L asan --no-tests=error --output-on-failure 2>&1 | tee "$LOG"
rc=${PIPESTATUS[0]}
echo "probe-asan-unit: ctest rc=$rc $(date '+%F %T')"
# "<p>% tests passed, <k> tests failed out of <n>" is ctest's own summary line.
FAILED=$(sed -n 's/^.*tests passed, \([0-9][0-9]*\) tests failed out of \([0-9][0-9]*\).*$/\1/p' "$LOG" | tail -1)
RAN=$(sed -n 's/^.*tests passed, \([0-9][0-9]*\) tests failed out of \([0-9][0-9]*\).*$/\2/p' "$LOG" | tail -1)
rm -f "$LOG"
if [ "$rc" -eq 0 ] && [ "${FAILED:-x}" = "0" ] && [ "${RAN:-0}" -ge "$EXPECTED_ASAN_CASES" ]; then
    echo "PROBE-ASAN-UNIT GREEN ($RAN cases, 0 reports)"
    exit 0
fi
[ "$rc" -eq 0 ] && rc=1   # a summary this script could not read is never a pass
echo "PROBE-ASAN-UNIT RED (${FAILED:-?} of ${RAN:-$FOUND} failed)"
exit $rc
