# Bodyless POST stalls 5 s on the production REST API (port 7070) — mechanism, upstream fix, plan

Session: s-rta-0924b · Author: Architect (Fable) · Date: 2026-09-25
Repo: /Users/boriskarpman/projects/RealTimeAudio · vendored header: build/_deps/httplib-src/httplib.h (cpp-httplib 0.18.3, commit a7bc00e3 "Release v0.18.3", 2024-12-03)
Confidence labels: VERIFIED = read from disk / measured this session; INFERRED = derived from verified facts; ASSUMED = not checked.

QUESTION: Why does `POST /api/perf/stop` with no body/Content-Length take ~5.0 s before the handler runs, has upstream fixed it, and what is the fix (not touching clients) with a fail-first test and a live check?

APPROACH (recommended): bump cpp-httplib from v0.18.3 to v0.57.1 (current published release, 2026-09-21) in the root FetchContent block, pinning zstd auto-detect OFF so the link set stays identical. One-line dependency change fixes both servers (ApiServer 7070 and TestServer 8080 share the header), zero app-code change, and the app's two server TUs compile unchanged against v0.57.1 (VERIFIED, syntax-only with the build's real flags). Fail-first Catch2 test (no app) is RED today at 5005 ms and GREEN on v0.57.1 at <1 ms (VERIFIED by a prototype rig this session).

---

## 1. Why 0.18.3 waits exactly ~5 s (VERIFIED, all `path:line` in build/_deps/httplib-src/httplib.h)

Call chain for a `POST` whose headers carry neither `Content-Length` nor `Transfer-Encoding`:

1. `Server::process_and_close_socket` :7219 → `detail::process_server_socket` :3336 → `process_server_socket_core` :3318 builds `SocketStream strm(sock, read_timeout_sec_, read_timeout_usec_, …)` :3344. `read_timeout_sec_` defaults to `CPPHTTPLIB_SERVER_READ_TIMEOUT_SECOND` = **5** (:37-38, member default :1027). Neither ApiServer nor TestServer ever calls `set_read_timeout` (VERIFIED: grep of src/api/ApiServer.cpp and src/test/TestServer.cpp — only `listen`, `stop`, `Get`, `Post`, `set_post_routing_handler` are used).
2. `Server::process_request` parses request line + headers, then calls `routing(req, res, strm)` (:7139 region).
3. `Server::routing` :6828 — no pre-routing handler set; not GET/HEAD; then **`detail::expect_content(req)` :5295-5301 returns `true` for every POST/PUT/PATCH/PRI/DELETE regardless of framing** — the gap is acknowledged in-source: `// TODO: check if Content-Length is set` :5300. No content-reader handlers are registered, so it falls to `read_content(strm, req, res)` :6879 → `Server::read_content` :6539 → `Server::read_content_core` :6580.
4. `read_content_core` :6623-6625 already special-cases **DELETE** without Content-Length (`return true`), but POST falls through to `detail::read_content` :6627 → :4375.
5. `detail::read_content` :4384-4387: not chunked; `!has_header(x.headers, "Content-Length")` → **`read_content_without_length`** :4240-4253, which loops `strm.read(buf, 16384)` until `n <= 0` ("read until EOF" — correct for a *response* with no framing, wrong for a *request*: RFC 9112 §6.3 says a request with neither header has a zero-length body).
6. `SocketStream::read` :5885: the internal buffer was fully consumed by header parsing, so it hits `if (!is_readable()) { return -1; }` :5905. `is_readable` :5876 = `select_read(sock_, read_timeout_sec_=5, 0) > 0` → `select_read` :3105 = `poll()`/`select()` with a **5000 ms** timeout. curl sent nothing further and is itself waiting for the response → nothing arrives → timeout → `read` returns −1 → `n <= 0` → `read_content_without_length` returns **true with an empty body** → routing continues → the handler finally runs → 200 written. Total ≈ 5.005 s (Harmony measured 5.006 s live; the rig below measured 5005.1 ms).

Why the fast cases are fast (VERIFIED by rig): `-d '{}'` makes curl send `Content-Length: 2` → :4389-4395 `read_content_with_length(…, 2)` → 0.1 ms. An explicit `Content-Length: 0` takes the same `else` branch with `len == 0`, so the `else if (len > 0)` read is skipped → 0.1 ms. This is the discriminating control that pins the `has_header("Content-Length")` branch as the cause.

Not the culprit (common misread): `keep_alive()` :3283 with `CPPHTTPLIB_KEEPALIVE_TIMEOUT_SECOND` = 5 (:17-18) only waits *between* requests for the next one; here the *response to the current request* is delayed, which only the body-read path explains. `Connection: close` and HTTP/1.0 requests stall identically (rig: 5005.3 ms each), consistent with the read-timeout path and inconsistent with any keep-alive theory.

Who is hit today (VERIFIED): any bodyless POST to either server. In-repo: 9 probe rows send bodyless POSTs and each silently absorbs 5.0 s (their `--max-time 6` masks it): `.harmony/probe-step3.sh:302,663,712,724,725,812,896`, `.harmony/probe-onset-render.sh:122`, `.harmony/probe-finalize-loop.sh:78` (all `perf/stop`, `perf/stop_play`, `perf/repair`). The multi-line curls in those files carry `-d` on the continuation line and are fine. `tests/visual/*.py` use `requests.post(json=…)` (always framed) — unaffected. Production handlers `handlePerfStop` (src/api/ApiServer.cpp:1240) and `handleSnapshot` (:587) ignore the body entirely, so the 5 s is 100 % waste; `handleTriggerClip` (:374) parses the body and returns a JSON error when it is empty — still delayed 5 s.

## 2. Upstream status (VERIFIED by raw-source bisect over tags + GitHub API; network reachable this session)

- **Fix commit**: https://github.com/yhirose/cpp-httplib/commit/337fbb0793c598046d636d4450a5dcf8d3620a0b — "Fix #2279 — Enhance request handling: add support for requests without Content-Length or Transfer-Encoding headers" (2025-11-26, touches httplib.h + test/test.cc).
- **Issue**: https://github.com/yhirose/cpp-httplib/issues/2279 — "POST requests without Content-Length or Transfer-Encoding return 400 Bad Request" (opened 2025-11-22, closed 2025-11-26; reporter's repro is literally `curl -X POST` against `svr.Post`). In v0.27.0 the symptom was a 400; in 0.18.3 it is the 5 s stall then 200 — same root (server tries to read an unframed request body), different failure mode by version.
- **First release containing it**: **v0.28.0** (2025-11-26) https://github.com/yhirose/cpp-httplib/releases/tag/v0.28.0. NOTE: the v0.28.0 release notes do NOT list it (they list only #2260 and #2273), so release-note searching alone misses this fix — it was found by fetching `httplib.h` at each tag and bisecting on the gate text. v0.27.0 still has only the DELETE special case (:7898).
- What the fix is: in `Server::read_content_core`, before any body read: `if (!req.has_header("Content-Length") && !detail::is_chunked_transfer_encoding(req.headers)) return true;` (v0.28.0 :7935-7941, comment cites RFC 7230 §3.3.3). `detail::expect_content` itself is unchanged through v0.58.0 (still `return true` for POST) — the gate lives in the server's read path, not in `expect_content`.
- Later refinements (all keep bodyless POST immediate): v0.29.0 adds a non-SSL `MSG_PEEK` probe for stray bytes; v0.35.0 replaces it with `strm.is_readable()` (now a pure buffer check, :11984-11986 at v0.57.1 — the blocking variant was split out as `wait_readable`) + `select_read(s, 0, 0)`; **v0.45.0** (2026-05-15) restricts that probe to non-persistent connections after issue #2450 (https://github.com/yhirose/cpp-httplib/issues/2450, keep-alive drain bug) and re-labels the comment "RFC 9112 §6: no Transfer-Encoding and no Content-Length means no body." Current release v0.57.1: gate at :13524-13556; with `CPPHTTPLIB_SSL_ENABLED` (which this app's build defines — see §3) the branch is the plain `return true` :13552-13555.
- Tags after v0.18.3 (releases API, 2026-09-25): v0.18.4 … v0.57.1 (latest published release, 2026-09-21); a v0.58.0 tag exists but has no release entry yet — pin v0.57.1.

## 3. Options and blast radius

Current build facts (VERIFIED): root CMakeLists.txt:48-54 FetchContent `GIT_TAG v0.18.3`, `GIT_SHALLOW TRUE`; `target_link_libraries(AudioDNA PRIVATE httplib::httplib)` :417; `AUDIODNA_BUILD_TEST_SERVER=ON` in build/CMakeCache.txt:31 so TestServer.cpp is in the app build; httplib's own CMake auto-detected OpenSSL/zlib/brotli — build/CMakeFiles/AudioDNA.dir/flags.make carries `CPPHTTPLIB_OPENSSL_SUPPORT`, `CPPHTTPLIB_ZLIB_SUPPORT`, `CPPHTTPLIB_BROTLI_SUPPORT`, and `otool -L` on the app shows libssl.3/libcrypto.3 (Homebrew openssl@3), libz, libbrotli{common,enc,dec}. Nothing in the repo defines any `CPPHTTPLIB_*` macro or sets any `HTTPLIB_*` option. httplib API surface used by both servers (VERIFIED grep): `httplib::Server`, `Post(pattern, Handler)`, `Get(pattern, Handler)`, `listen`, `stop`, `set_post_routing_handler`, `Request::body`, `Response::set_content/set_header/status` — every one present with identical signature at v0.57.1 (:2169, :2168, :2272, :2276, :2214, :1809-1811, :1802, :1785).

| Option | Verdict | Why |
|---|---|---|
| **A. Bump to v0.57.1** (chosen) | ACCEPT | Root cause fixed upstream (§2); one-line change; both servers; no app code; `ApiServer.cpp` and `TestServer.cpp` pass `-fsyntax-only` against v0.57.1 using the build's exact compile commands with the new header shadowing the vendored one (VERIFIED, exit 0, shadow confirmed via `-E`); rig against v0.57.1 compiled with the app's feature set (OpenSSL3/zlib/brotli) answers every framing in ≤0.3 ms (VERIFIED). Picks up 39 releases of server hardening (414 hang #2260, chunked parsing, TE+CL smuggling rejection) — relevant for a network control API even though it binds loopback by default (src/api/ApiServer.cpp:86-88). Rollback = revert the tag. |
| B. FetchContent `PATCH_COMMAND` applying the 4-line v0.28.0 gate to the vendored 0.18.3 header | FALLBACK only | Exact and surgical, but forks a vendored dependency: must be idempotent, must be removed on any future bump, leaves 10 months of upstream fixes unapplied, and has the same "already-populated `build/_deps` may not re-run the step" hazard as A. No precedent in this repo (VERIFIED: no PATCH_COMMAND anywhere in CMake). Use only if A's audit build fails. |
| C. `server_.set_read_timeout(0, 200000)` on both servers | REJECT | Shrinks the stall to 200 ms, never to zero, and caps how long the server waits for *legitimate* body bytes after the headers — a remote client whose body arrives in a later packet gets a truncated read → 400. Trades one bug for a new, quieter one. |
| D. `set_pre_routing_handler` short-circuit | REJECT | It does run before the body read (:6829-6832), but it receives `const Request&`, cannot inject `Content-Length: 0`, and cannot reach the private `dispatch_request` — it would have to re-implement routing for 41 POST routes. |
| E. Bump only to v0.28.0 (first fixed tag) | REJECT | Same populate/bump cost as A with none of the later hardening; pins a 10-month-old release. |
| F. Change clients | EXCLUDED by dispatch. |

Blast radius of A (what changes besides the fix):
- Header grows (≈9.5 k → ≈19 k lines): compile time of exactly two TUs (ApiServer.cpp, TestServer.cpp) rises modestly. INFERRED.
- New optional dependency auto-detect in v0.57.1's CMake: `HTTPLIB_USE_ZSTD_IF_AVAILABLE` (default ON, v0.57.1 CMakeLists.txt:123). If Homebrew zstd is installed (likely, as an FFmpeg dependency — ASSUMED) the app would gain a libzstd dylib link. Pin it OFF (spec below) so `otool -L` stays identical. Also new: `HTTPLIB_USE_NON_BLOCKING_GETADDRINFO` ON (:121) — affects name resolution for `listen("localhost")` in TestServer.cpp:73; ASSUMED harmless, validated by the live Eyes health check in §5. Keychain option renamed (`HTTPLIB_USE_CERTS_FROM_MACOSX_KEYCHAIN` → `HTTPLIB_DISABLE_MACOSX_AUTOMATIC_ROOT_CERTIFICATES`, :120) — unused here.
- `CPPHTTPLIB_PAYLOAD_MAX_LENGTH` default changes from unlimited (0.18.3 :98) to 100 MB (v0.57.1 :129-130). Every API body here is small JSON (file *paths*, not contents) — no impact. VERIFIED by handler inventory.
- Stricter framing: requests carrying both `Transfer-Encoding` and `Content-Length` are rejected (RFC 9112 §6.3). No in-repo client does this. INFERRED.
- OpenSSL floor 3.0.0 (`_HTTPLIB_OPENSSL_MIN_VER`, v0.57.1 CMakeLists.txt:97) — satisfied by Homebrew openssl@3 (VERIFIED via otool). Existing ctest targets (37 executables) link no httplib → unaffected (VERIFIED grep of tests/CMakeLists.txt).

## 4. DECISION / SPEC (Builder-executable)

Files touched: `CMakeLists.txt` (root, lines 47-54), `tests/CMakeLists.txt` (append), `tests/test_httplib_bodyless_post.cpp` (new), `CLAUDE.md` (tech-stack row + Common Pitfalls entry). No src/ changes.

### 4.1 Fail-first test (write FIRST; must be RED on v0.18.3)

`tests/test_httplib_bodyless_post.cpp` — the prototype below ran this session in $TMPDIR against both headers; convert verbatim into Catch2 v3. Key design point: the client MUST be a raw socket — `httplib::Client` always sends `Content-Length`, so it cannot reproduce the bug.

```cpp
// tests/test_httplib_bodyless_post.cpp — s-rta-0924b: a POST with neither
// Content-Length nor Transfer-Encoding has a zero-length body (RFC 9112 §6.3)
// and must be answered immediately, not after CPPHTTPLIB_SERVER_READ_TIMEOUT
// (5 s). Headless: httplib::Server on an ephemeral loopback port + raw POSIX
// socket client. No JUCE. RED on cpp-httplib 0.18.3 (~5005 ms), GREEN >= v0.28.0.
#include <catch2/catch_test_macros.hpp>
#include <httplib.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <atomic>
#include <chrono>
#include <string>
#include <thread>

namespace {
struct Probe { double ms; std::string head; };

Probe probe(int port, const std::string& raw) {
    int fd = ::socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in a{}; a.sin_family = AF_INET; a.sin_port = htons(static_cast<uint16_t>(port));
    a.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    REQUIRE(::connect(fd, reinterpret_cast<sockaddr*>(&a), sizeof a) == 0);
    timeval tv{9, 0}; ::setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof tv);
    const auto t0 = std::chrono::steady_clock::now();
    REQUIRE(::send(fd, raw.data(), raw.size(), 0) == static_cast<ssize_t>(raw.size()));
    char buf[512]; const ssize_t n = ::recv(fd, buf, sizeof buf - 1, 0);
    const double ms = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - t0).count();
    ::close(fd);
    return { ms, n > 0 ? std::string(buf, buf + std::min<ssize_t>(n, 15)) : std::string("<no response>") };
}

struct ServerFixture {
    httplib::Server svr; std::atomic<int> hits{0}; int port = 0; std::thread th;
    ServerFixture() {
        svr.Post("/hit", [this](const httplib::Request&, httplib::Response& res) {
            ++hits; res.set_content(R"({"ok":true})", "application/json");
        });
        port = svr.bind_to_any_port("127.0.0.1");
        th = std::thread([this] { svr.listen_after_bind(); });
        svr.wait_until_ready();
    }
    ~ServerFixture() { svr.stop(); th.join(); }
};
constexpr double kBudgetMs = 500.0;  // RED today: ~5005 ms; GREEN: < 1 ms
} // namespace

TEST_CASE("bodyless POST (HTTP/1.1 keep-alive) is answered immediately", "[httplib][bodyless]") {
    ServerFixture f; REQUIRE(f.port > 0);
    const auto p = probe(f.port, "POST /hit HTTP/1.1\r\nHost: 127.0.0.1\r\n\r\n");
    CHECK(p.head == "HTTP/1.1 200 OK"); CHECK(f.hits == 1);
    REQUIRE(p.ms < kBudgetMs);
}
TEST_CASE("control: POST with Content-Length: 0 is answered immediately", "[httplib][bodyless]") {
    ServerFixture f;  // GREEN on 0.18.3 too — guards the harness itself
    const auto p = probe(f.port, "POST /hit HTTP/1.1\r\nHost: 127.0.0.1\r\nContent-Length: 0\r\n\r\n");
    CHECK(p.head == "HTTP/1.1 200 OK"); REQUIRE(p.ms < kBudgetMs);
}
TEST_CASE("bodyless POST with Connection: close is answered immediately", "[httplib][bodyless]") {
    ServerFixture f;
    const auto p = probe(f.port, "POST /hit HTTP/1.1\r\nHost: 127.0.0.1\r\nConnection: close\r\n\r\n");
    CHECK(p.head == "HTTP/1.1 200 OK"); REQUIRE(p.ms < kBudgetMs);
}
TEST_CASE("bodyless POST over HTTP/1.0 is answered immediately", "[httplib][bodyless]") {
    ServerFixture f;
    const auto p = probe(f.port, "POST /hit HTTP/1.0\r\nHost: 127.0.0.1\r\n\r\n");
    CHECK(p.head == "HTTP/1.1 200 OK"); REQUIRE(p.ms < kBudgetMs);
}
```

Measured this session with the identical probe logic (VERIFIED): 0.18.3 → 5005.1 / 0.1 / 5005.3 / 5005.3 ms (three RED, control GREEN); v0.57.1 plain → 0.4 / 0.2 / 0.2 / 0.2 ms; v0.57.1 with OpenSSL3+zlib+brotli defines → 0.3 / 0.2 / 0.1 / 0.1 ms. Total RED runtime ≈ 15 s, so set the ctest timeout to 60 s.

Append to `tests/CMakeLists.txt` (after `test_record_panel_model`, matching the file's target convention; POSIX sockets → not built on Windows):

```cmake
# --- test_httplib_bodyless_post (s-rta-0924b: a POST with no Content-Length /
# Transfer-Encoding must not stall for CPPHTTPLIB_SERVER_READ_TIMEOUT_SECOND).
# Headless: httplib::Server on an ephemeral loopback port + raw POSIX-socket
# client (httplib::Client always sends Content-Length, so it cannot repro).
# Links httplib::httplib only -- no JUCE. POSIX sockets -> skipped on Windows.
if(NOT WIN32)
    add_executable(test_httplib_bodyless_post test_httplib_bodyless_post.cpp)
    target_link_libraries(test_httplib_bodyless_post PRIVATE
        Catch2::Catch2WithMain
        httplib::httplib
    )
    if(NOT MSVC)
        target_compile_options(test_httplib_bodyless_post PRIVATE
            -Wno-old-style-cast
            -Wno-conversion
            -Wno-sign-conversion
        )
    endif()
    apply_sanitizers(test_httplib_bodyless_post)
    catch_discover_tests(test_httplib_bodyless_post PROPERTIES TIMEOUT 60)
endif()
```

`httplib::httplib` is the INTERFACE target from the FetchContent'd project (root CMakeLists.txt:54 runs before `add_subdirectory(tests)` :635), already carrying Threads/OpenSSL/zlib/brotli links (vendored CMakeLists.txt:225-231). VERIFIED.

RED gate: `cmake -S . -B build && cmake --build build --target test_httplib_bodyless_post -j && ./build/tests/test_httplib_bodyless_post` → expect 3 failed / 1 passed with the three failures reporting ≈5005 ms. (Binary location: wherever the other test executables land in this tree — same rule as `test_record_panel_model`.)

### 4.2 The fix — root `CMakeLists.txt` lines 47-54 become:

```cmake
# --- cpp-httplib (HTTP server: production API + test server) ---
# v0.57.1 (2026-09-21). Bodyless POST (no Content-Length / Transfer-Encoding)
# stalled CPPHTTPLIB_SERVER_READ_TIMEOUT (5 s) on 0.18.3; fixed upstream in
# v0.28.0 (yhirose/cpp-httplib#2279, commit 337fbb07). Pinned to the current
# release. zstd auto-detect OFF: keep the link set identical (no new dylib).
set(HTTPLIB_USE_ZSTD_IF_AVAILABLE OFF CACHE BOOL "cpp-httplib: no zstd" FORCE)
FetchContent_Declare(
    httplib
    GIT_REPOSITORY https://github.com/yhirose/cpp-httplib.git
    GIT_TAG        v0.57.1
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(httplib)
```

Re-population: FetchContent re-runs the download/update step when `GIT_TAG` changes (INFERRED, standard behaviour). Builder MUST verify after reconfigure: `grep -m1 CPPHTTPLIB_VERSION build/_deps/httplib-src/httplib.h` prints `"0.57.1"`; if it still prints 0.18.3, `rm -rf build/_deps/httplib-src build/_deps/httplib-build build/_deps/httplib-subbuild` and reconfigure. `build/` is the live Release dir the app runs from (cmake/Sanitizers.cmake:11-13) — stop the app on 7070 before rebuilding.

### 4.3 Sequence

1. Add 4.1 (test + CMake target). Build only that target on the *unchanged* v0.18.3 tree; run it; confirm RED (3 × ≈5005 ms) and the control GREEN. Do not proceed if the control is RED (harness bug, not the library).
2. Apply 4.2. Reconfigure; verify the header version (4.2). `cmake --build build --config Release -j$(sysctl -n hw.ncpu)` — app + all tests.
3. Run the new test → 4/4 GREEN in <1 s. `cd build && ctest` → all existing targets still GREEN (none link httplib; they are a regression canary for the reconfigure itself).
4. `otool -L build/AudioDNA_artefacts/Release/Audio-DNA.app/Contents/MacOS/Audio-DNA | grep -iE "ssl|crypto|libz|zstd|brotli"` → same six lines as before the bump (ssl.3, crypto.3, libz.1, brotli ×3), no zstd.
5. Relaunch the app; run §5 live checks.
6. Docs (CLAUDE.md is the current truth per its own rule): tech-stack row `cpp-httplib | 0.18.3` → `0.57.1`; Common Pitfalls #24 mentions httplib — add a new numbered pitfall: "Bodyless POST must be answered immediately — cpp-httplib < v0.28.0 read an unframed request body until the 5 s server read timeout (RFC 9112 §6.3 says zero-length). Guarded by tests/test_httplib_bodyless_post.cpp; do not downgrade below v0.28.0." Do NOT add a LESSONS_LEARNED entry without Boris's confirmation (CLAUDE.md gate).

"Done" = steps 1-6 green, plus §5 live numbers recorded in the session ledger.

## 5. Live check Harmony runs (app relaunched on 7070 after the rebuild)

Before the bump (optional baseline, expect ≈5.00 s each) and after (PASS: every `t` < 0.100 s):

```bash
for p in perf/stop trigger_clip snapshot; do
  curl -s -o /dev/null --max-time 8 -X POST -w "$p  code=%{http_code}  t=%{time_total}s\n" "http://127.0.0.1:7070/api/$p"
done
# discriminating control (fast on both versions):
curl -s -o /dev/null --max-time 8 -X POST -H 'Content-Length: 0' -w "trigger_clip CL0  code=%{http_code}  t=%{time_total}s\n" http://127.0.0.1:7070/api/trigger_clip
# CORS post-routing handler still fires on the new header:
curl -si --max-time 8 -X POST -d '{}' http://127.0.0.1:7070/api/perf/stop | grep -i 'access-control-allow-origin'
```

Expected codes: all 200 (`trigger_clip` bodyless returns 200 with `{"ok":false,"error":"Missing 'layer' or 'column'"}` — src/api/ApiServer.cpp:380-383 never sets `res.status`; `snapshot` writes a PNG under ~/Documents/Audio-DNA/Snapshots/ and may take tens of ms for the GL readback — still far under 100 ms; `perf/stop` while not recording just dispatches `onPerfStop()` async). Then the Eyes canary for TestServer on the new header: `AUDIODNA_NO_SPAWN=1 pytest tests/visual/test_render_pipeline.py -v` (needs the `--test-mode` launch). Side note for the ledger: the 9 probe rows in §1 each get ≈5.0 s faster — any wall-clock expectations tuned with the stall present (loop 40/0, step3 69/0 timings) will shift; that is the fix working, not drift.

## RISKS

- Strongest counterargument to the bump: this is a 39-release jump of the library serving the live control API, made mid-session on the live Release tree; an unnoticed behavioural delta would surface as flaky probe rows that are hard to attribute. Why it loses: the surface used is nine symbols, all verified identical at v0.57.1; both TUs compile unchanged with the build's real flags; the rig exercises the exact gate branch the app takes (SSL_ENABLED); ctest + Eyes + the §5 curls cover both servers; rollback is one line. Option B remains available if step 2 fails for a build reason (e.g. an unexpected new dependency probe).
- Re-population hazard (INFERRED): if FetchContent does not refresh `build/_deps/httplib-src`, the build silently stays on 0.18.3 — that is exactly what the RED→GREEN test detects; step 4.2's version grep is the belt-and-braces check.
- zstd pin: if Homebrew zstd is absent the pin is a no-op; if present and unpinned, the app gains a dylib — not a functional risk, but it changes the bundle's dependency set silently. Pinned in the spec.
- The test measures wall-clock on a loaded CI box; 500 ms budget vs a 5000 ms failure mode leaves 10× margin, and the GREEN path measured ≤0.4 ms. Under TSan (`ADNA_SANITIZE=thread`) httplib's thread pool is instrumented — the test still holds (no timing-sensitive code besides the assertion), ASSUMED; run once in the TSan build dir to confirm.
- Session hygiene (gotchas T28/G28): the rebuild touches `build/` (live tree) and needs the app stopped; do not commit mid-battery; the report itself is the only file this agent wrote.

STATUS: COMPLETE — root cause VERIFIED (read timeout path via `expect_content` → `read_content_without_length`), upstream fix cited (commit 337fbb07 / #2279, first in v0.28.0), plan = bump to v0.57.1 with zstd pinned OFF, fail-first Catch2 test RED 5005 ms today / GREEN <1 ms on v0.57.1 (rig-verified), live curl gate specified.
