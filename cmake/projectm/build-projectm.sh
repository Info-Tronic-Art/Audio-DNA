#!/usr/bin/env bash
# build-projectm.sh -- build + install the PATCHED libprojectM 4.1.1 that Audio-DNA links (BF10).
#
# Why: libprojectM 4.1.1's projectm_opengl_render_frame() binds DRAW framebuffer 0 itself before its final
# copy, i.e. it always draws into the window (the Preview panel), never into a canvas-sized framebuffer.
# 0001-render-frame-fbo.patch backports the upstream-shaped (4.2.0) projectm_opengl_render_frame_fbo()
# onto the exact 4.1.1 commit; nothing else in the engine changes.
#
# Installs into its own prefix (default $HOME/.local/opt/projectm-4.1.1-fbo1) and never touches any other
# projectM install. Idempotent: exits 0 at once if the prefix already exports the function.
#
# Usage:  cmake/projectm/build-projectm.sh
# Env:    PREFIX=<dir>            install prefix (default above)
#         PROJECTM_SRC=<dir|.tar.gz>  build offline from a 4.1.1 source tree with its submodules (a git tree
#                                 must be at the pinned commit), e.g. the archive this script keeps at
#                                 $HOME/.local/src/projectm-03aa8a7.tar.gz
#         JOBS=<n>                parallel compile jobs (default 3)
# Exit:   0 = installed (or already installed), 3 = BLOCKED (no network and no PROJECTM_SRC), other = error.
set -euo pipefail

SHA=03aa8a7ffdf81165136ee64643c6a781f5c6a391
REPO=https://github.com/projectM-visualizer/projectm.git
PREFIX=${PREFIX:-$HOME/.local/opt/projectm-4.1.1-fbo1}
ARCHIVE=$HOME/.local/src/projectm-03aa8a7.tar.gz
JOBS=${JOBS:-3}
HERE=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)
PATCH=$HERE/0001-render-frame-fbo.patch
SYM=_projectm_opengl_render_frame_fbo

has_symbol() { [ -f "$1" ] && { nm -gU "$1" 2>/dev/null || true; } | awk -v s="$SYM" '$3 == s {f = 1} END {exit !f}'; }

if has_symbol "$PREFIX/lib/libprojectM-4.dylib"; then
    echo "build-projectm: $PREFIX already exports $SYM -- nothing to do"
    exit 0
fi

[ -f "$PATCH" ] || { echo "build-projectm: missing $PATCH" >&2; exit 1; }
WORK=$(mktemp -d "${TMPDIR:-/tmp}/projectm-fbo1.XXXXXX")
SRC=$WORK/src
echo "build-projectm: work dir $WORK"

# (0) PROJECTM_SRC (offline) -- (1) network pre-check -- (2) clone at the pinned commit
if [ -n "${PROJECTM_SRC:-}" ]; then
    if [ -d "$PROJECTM_SRC" ]; then
        if git -C "$PROJECTM_SRC" rev-parse --is-inside-work-tree >/dev/null 2>&1; then
            head=$(git -C "$PROJECTM_SRC" rev-parse HEAD)
            [ "$head" = "$SHA" ] || { echo "build-projectm: PROJECTM_SRC HEAD is $head, expected $SHA" >&2; exit 1; }
        fi
        mkdir -p "$SRC" && (cd "$PROJECTM_SRC" && tar -cf - --exclude=.git .) | (cd "$SRC" && tar -xf -)
    elif [ -f "$PROJECTM_SRC" ]; then
        mkdir -p "$SRC" && tar -xzf "$PROJECTM_SRC" -C "$SRC"
    else
        echo "build-projectm: PROJECTM_SRC=$PROJECTM_SRC is neither a directory nor a file" >&2; exit 1
    fi
    if [ -f "$SRC/.projectm-commit" ] && [ "$(cat "$SRC/.projectm-commit")" != "$SHA" ]; then
        echo "build-projectm: PROJECTM_SRC is commit $(cat "$SRC/.projectm-commit"), expected $SHA" >&2; exit 1
    fi
    grep -q 'VERSION 4.1.1' "$SRC/CMakeLists.txt" || { echo "build-projectm: PROJECTM_SRC is not projectM 4.1.1" >&2; exit 1; }
    [ -f "$SRC/vendor/projectm-eval/CMakeLists.txt" ] || { echo "build-projectm: PROJECTM_SRC lacks the vendor/projectm-eval submodule" >&2; exit 1; }
else
    if ! perl -e 'alarm 20; exec @ARGV' git ls-remote "$REPO" HEAD >/dev/null 2>&1; then
        echo "BLOCKED: no network -- set PROJECTM_SRC=<dir|tarball>"
        rm -rf "$WORK"
        exit 3
    fi
    git clone -q "$REPO" "$SRC"
    git -C "$SRC" checkout -q --detach "$SHA"
    git -C "$SRC" submodule update -q --init --recursive
    # (3) keep the UNPATCHED tree (with submodules, without .git) for offline re-runs
    if [ ! -f "$ARCHIVE" ]; then
        mkdir -p "$(dirname "$ARCHIVE")"
        echo "$SHA" > "$SRC/.projectm-commit"
        (cd "$SRC" && tar -czf "$ARCHIVE.tmp" --exclude=.git .) && mv "$ARCHIVE.tmp" "$ARCHIVE"
        rm -f "$SRC/.projectm-commit"
        echo "build-projectm: archived the unpatched source to $ARCHIVE"
    fi
fi

# (4) patch, configure (the stock install's options: Release, shared, playlist ON, tests / SDL UI OFF), build, install
(cd "$SRC" && git apply --check "$PATCH" && git apply "$PATCH")
# Configure FROM the source dir: projectM's VCSVersion.cmake runs `git rev-parse HEAD` in the current directory
# and FORCEs the result, so configuring from inside another git checkout would embed THAT checkout's SHA.
cd "$SRC"
cmake -S "$SRC" -B "$WORK/build" \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX="$PREFIX" \
    -DBUILD_SHARED_LIBS=ON \
    -DENABLE_PLAYLIST=ON \
    -DBUILD_TESTING=OFF \
    -DENABLE_SDL_UI=OFF \
    -DPROJECTM_VCS_VERSION="$SHA"
cmake --build "$WORK/build" --config Release -j"$JOBS"
cmake --install "$WORK/build" --config Release

# Verify: the new export, and the install name the app links (@rpath/libprojectM-4.4.dylib)
LIB=$PREFIX/lib/libprojectM-4.dylib
has_symbol "$LIB" || { echo "build-projectm: $LIB does not export $SYM" >&2; exit 1; }
idname=$(otool -D "$LIB" | tail -1)
[ "$idname" = "@rpath/libprojectM-4.4.dylib" ] || { echo "build-projectm: install name is $idname" >&2; exit 1; }
vcs=$({ strings "$LIB" || true; } | grep -c "$SHA" || true)
[ "$vcs" -ge 1 ] || { echo "build-projectm: $LIB does not embed VCS SHA $SHA" >&2; exit 1; }

cd /
rm -rf "$WORK"
echo "build-projectm: installed $PREFIX ($SYM: yes, install name $idname)"
