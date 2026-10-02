#!/bin/bash
# probe-tsan-unit.sh -- lane tsan (s-rta-1002; ruling .harmony/.reports/s-rta-0930/ruling-tsan.md amendment 11, gate
# G2). The ThreadSanitizer unit gate: configures a -DADNA_SANITIZE=thread build dir (the prebuild recipe:
# RelWithDebInfo, TEST_SERVER ON, SYPHON ON, FetchContent sources from a local _deps dir) if it is absent, builds the
# tsan-labelled ctest targets, runs `ctest -L tsan --output-on-failure` and exits with ctest's code.
# REQUIRED before merging any change to a model field another thread reads (Pitfall 63).
# Launches NO app. Puts no TSAN_OPTIONS in the environment: each [tsan] test's ENVIRONMENT property pins them
# (exitcode=66:halt_on_error=0:abort_on_error=0:report_signal_unsafe=0:history_size=4) and its
# FAIL_REGULAR_EXPRESSION fails a case on any "WARNING: ThreadSanitizer" (tests/CMakeLists.txt).
#
# usage: probe-tsan-unit.sh [build-dir]
#   build-dir      default: <tree>/build-tsan (never point it at build/, the live Release dir)
#   ADNA_DEPS_DIR  FetchContent sources (juce-src, httplib-src, catch2-src, melatonin_inspector-src, syphon-src);
#                  default: <main checkout>/build/_deps (the checkout that owns this tree's .git), used only for a
#                  fresh configure
#   ADNA_JOBS      build parallelism (default 3)
set -u
TREE=$(cd "$(dirname "$0")/.." && pwd)
B=${1:-$TREE/build-tsan}
JOBS=${ADNA_JOBS:-3}
# The [tsan] targets (each registered with LABELS tsan in tests/CMakeLists.txt).
TARGETS=(test_layer_runtime_race test_manual_scalar_race)

if [ ! -f "$B/CMakeCache.txt" ]; then
    COMMON=$(git -C "$TREE" rev-parse --path-format=absolute --git-common-dir 2>/dev/null)
    D=${ADNA_DEPS_DIR:-$(dirname "$COMMON")/build/_deps}
    for dep in juce httplib catch2 melatonin_inspector syphon; do
        [ -d "$D/$dep-src" ] || { echo "probe-tsan-unit: missing $D/$dep-src (set ADNA_DEPS_DIR)" >&2; exit 2; }
    done
    mkdir -p "$B"
    echo "probe-tsan-unit: configure $B (ADNA_SANITIZE=thread, RelWithDebInfo) $(date '+%F %T')"
    cmake -S "$TREE" -B "$B" -DCMAKE_BUILD_TYPE=RelWithDebInfo -DADNA_SANITIZE=thread \
        -DAUDIODNA_BUILD_TEST_SERVER=ON -DAUDIODNA_BUILD_SYPHON=ON -DFETCHCONTENT_FULLY_DISCONNECTED=ON \
        -DFETCHCONTENT_SOURCE_DIR_JUCE="$D/juce-src" -DFETCHCONTENT_SOURCE_DIR_HTTPLIB="$D/httplib-src" \
        -DFETCHCONTENT_SOURCE_DIR_CATCH2="$D/catch2-src" \
        -DFETCHCONTENT_SOURCE_DIR_MELATONIN_INSPECTOR="$D/melatonin_inspector-src" \
        -DFETCHCONTENT_SOURCE_DIR_SYPHON="$D/syphon-src" -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
        > "$B/configure.log" 2>&1 || { echo "probe-tsan-unit: configure FAILED (see $B/configure.log)" >&2; exit 2; }
fi
if ! grep -q '^ADNA_SANITIZE:STRING=thread$' "$B/CMakeCache.txt"; then
    echo "probe-tsan-unit: $B is not a -DADNA_SANITIZE=thread build dir" >&2
    exit 2
fi

echo "probe-tsan-unit: build ${TARGETS[*]} $(date '+%F %T')"
cmake --build "$B" --target "${TARGETS[@]}" -j"$JOBS" || { echo "probe-tsan-unit: build FAILED" >&2; exit 2; }

echo "probe-tsan-unit: ctest -L tsan $(date '+%F %T')"
ctest --test-dir "$B" -L tsan --output-on-failure
rc=$?
echo "probe-tsan-unit: ctest rc=$rc $(date '+%F %T')"
exit $rc
