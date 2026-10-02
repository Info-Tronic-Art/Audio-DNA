// SCRATCH (architect, ruling-bt2-seats): AE2 -- AudioEngine::setSourceMode's body names no device call (C3 teeth that
// also catch a re-open GUARDED by getCurrentAudioDevice(), which AE1's deny-everything engine cannot see).
#include <catch2/catch_test_macros.hpp>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <string>

namespace {
std::string readFile(const std::string& p) { std::ifstream f(p); std::stringstream s; s << f.rdbuf(); return s.str(); }
std::string stripLineComments(const std::string& s)
{
    std::string out; std::istringstream in(s); std::string line;
    while (std::getline(in, line)) { auto c = line.find("//"); out += (c == std::string::npos ? line : line.substr(0, c)) + "\n"; }
    return out;
}
// The body of the first definition `sig` ... `{ ... }` (brace-matched), or "" when absent.
std::string bodyOf(const std::string& src, const std::string& sig)
{
    const auto at = src.find(sig);
    if (at == std::string::npos) return {};
    const auto open = src.find('{', at);
    if (open == std::string::npos) return {};
    int depth = 0;
    for (size_t i = open; i < src.size(); ++i)
    {
        if (src[i] == '{') ++depth;
        else if (src[i] == '}' && --depth == 0) return src.substr(open, i - open + 1);
    }
    return {};
}
}

TEST_CASE("AE2 [lint] AudioEngine::setSourceMode is an atomic flag flip: its body names no device manager / reconciler", "[lint]")
{
    const char* env = std::getenv("LINT_AE");
    const std::string path = env != nullptr ? env : "";
    const auto src = stripLineComments(readFile(path));
    const auto body = bodyOf(src, "void AudioEngine::setSourceMode(");
    INFO(path);
    REQUIRE_FALSE(body.empty());   // the function is found: a rename never makes this lint vacuous
    CHECK(body.find("deviceManager_") == std::string::npos);
    CHECK(body.find("deviceReconciler_") == std::string::npos);
}
