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
#include <algorithm>
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
    INFO("elapsed ms = " << p.ms);
    CHECK(p.head == "HTTP/1.1 200 OK"); CHECK(f.hits == 1);
    REQUIRE(p.ms < kBudgetMs);
}
TEST_CASE("control: POST with Content-Length: 0 is answered immediately", "[httplib][bodyless]") {
    ServerFixture f;  // GREEN on 0.18.3 too — guards the harness itself
    const auto p = probe(f.port, "POST /hit HTTP/1.1\r\nHost: 127.0.0.1\r\nContent-Length: 0\r\n\r\n");
    INFO("elapsed ms = " << p.ms);
    CHECK(p.head == "HTTP/1.1 200 OK"); REQUIRE(p.ms < kBudgetMs);
}
TEST_CASE("bodyless POST with Connection: close is answered immediately", "[httplib][bodyless]") {
    ServerFixture f;
    const auto p = probe(f.port, "POST /hit HTTP/1.1\r\nHost: 127.0.0.1\r\nConnection: close\r\n\r\n");
    INFO("elapsed ms = " << p.ms);
    CHECK(p.head == "HTTP/1.1 200 OK"); REQUIRE(p.ms < kBudgetMs);
}
TEST_CASE("bodyless POST over HTTP/1.0 is answered immediately", "[httplib][bodyless]") {
    ServerFixture f;
    const auto p = probe(f.port, "POST /hit HTTP/1.0\r\nHost: 127.0.0.1\r\n\r\n");
    INFO("elapsed ms = " << p.ms);
    CHECK(p.head == "HTTP/1.1 200 OK"); REQUIRE(p.ms < kBudgetMs);
}
