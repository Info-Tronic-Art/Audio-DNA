// test_output_law -- the screen-safety law of the output window, asserted in its SOURCE
// (s-rta-0927 outputs-c1 = plan5 slice C1, .harmony/.reports/s-rta-0926b/plan5-final.md 7 / 10.1 / 14).
// No automated gate may ever open an output window, so the mechanism is checked here, statically:
//   - NORMAL window level: never always-on-top (NSFloatingWindowLevel -> the black-overlay-on-every-Space
//     bug, .harmony/black-overlay-rootcause.md), never kiosk, never native fullscreen;
//   - never able to take the keyboard: the peer carries ComponentPeer::windowIgnoresKeyPresses
//     (TopLevelWindow::visibilityChanged otherwise calls toFront(true) on every show), nothing in the window
//     asks for focus; the constructor checks the peer's flag in every build type (a jassert alone is a Release
//     no-op) and re-adds the window to the desktop if it is missing;
//   - bounds before visible (the GL context attaches once, already at display size).
// From plan5 C2 (s-rta-0927 outputs-c2) the law also covers src/output/OutputManager.{h,cpp}, the one owner of the
// output windows: it never raises, focuses, floats or shows a window itself (only OutputWindow::openOnDisplay shows
// one, bounds first), and no deck/composition file's "outputDisplay" reaches the output code (plan5 R7).
// From plan5 C3 (s-rta-0927 outputs-c3): nothing opens an output at launch -- the saved set opens only through the
// "Restore Last Outputs" menu case (plan5 Q1).
// The files are read at test time from AUDIODNA_SRC_DIR; comments and string literals are stripped and all
// whitespace removed before matching, so a comment that NAMES a forbidden call is not a violation and
// "toFront (true)" is.
#include <catch2/catch_test_macros.hpp>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#ifndef AUDIODNA_SRC_DIR
#error "AUDIODNA_SRC_DIR must point at src/"
#endif

namespace
{
std::string readFile(const std::string& rel)
{
    std::ifstream in(std::string(AUDIODNA_SRC_DIR) + "/" + rel);
    REQUIRE(in.good());
    std::stringstream ss;
    ss << in.rdbuf();
    return ss.str();
}

// Code only: comments and the contents of string/char literals removed, then every whitespace char removed.
std::string codeOnly(const std::string& s)
{
    std::string out;
    enum { Code, Line, Block, Str, Chr } st = Code;
    for (size_t i = 0; i < s.size(); ++i)
    {
        const char c = s[i], n = i + 1 < s.size() ? s[i + 1] : '\0';
        switch (st)
        {
            case Code:
                if (c == '/' && n == '/') { st = Line; ++i; }
                else if (c == '/' && n == '*') { st = Block; ++i; }
                else if (c == '"') { st = Str; out += '"'; }
                else if (c == '\'') { st = Chr; out += '\''; }
                else if (c != ' ' && c != '\t' && c != '\n' && c != '\r') out += c;
                break;
            case Line: if (c == '\n') st = Code; break;
            case Block: if (c == '*' && n == '/') { st = Code; ++i; } break;
            case Str: if (c == '\\') ++i; else if (c == '"') { st = Code; out += '"'; } break;
            case Chr: if (c == '\\') ++i; else if (c == '\'') { st = Code; out += '\''; } break;
        }
    }
    return out;
}

// Comments removed and whitespace removed, string literal CONTENTS KEPT (a JSON key read by name must be seen).
std::string codeWithStrings(const std::string& s)
{
    std::string out;
    enum { Code, Line, Block, Str, Chr } st = Code;
    for (size_t i = 0; i < s.size(); ++i)
    {
        const char c = s[i], n = i + 1 < s.size() ? s[i + 1] : '\0';
        switch (st)
        {
            case Code:
                if (c == '/' && n == '/') { st = Line; ++i; }
                else if (c == '/' && n == '*') { st = Block; ++i; }
                else { if (c == '"') st = Str; else if (c == '\'') st = Chr;
                       if (c != ' ' && c != '\t' && c != '\n' && c != '\r') out += c; }
                break;
            case Line: if (c == '\n') st = Code; break;
            case Block: if (c == '*' && n == '/') { st = Code; ++i; } break;
            case Str: out += c; if (c == '\\' && i + 1 < s.size()) out += s[++i]; else if (c == '"') st = Code; break;
            case Chr: out += c; if (c == '\\' && i + 1 < s.size()) out += s[++i]; else if (c == '\'') st = Code; break;
        }
    }
    return out;
}

struct Sources
{
    std::string h, cpp;
    Sources() : h(codeOnly(readFile("ui/OutputWindow.h"))), cpp(codeOnly(readFile("ui/OutputWindow.cpp"))) {}
    bool has(const std::string& token) const { return h.find(token) != std::string::npos || cpp.find(token) != std::string::npos; }
};

void forbid(const char* token)
{
    const Sources src;
    INFO("src/ui/OutputWindow.{h,cpp} must never contain `" << token << "` (comments excluded)");
    CHECK_FALSE(src.has(token));
}
} // namespace

TEST_CASE("output law: no setAlwaysOnTop(true) -- normal window level", "[output_law]") { forbid("setAlwaysOnTop(true)"); }
TEST_CASE("output law: no NSFloatingWindowLevel", "[output_law]") { forbid("NSFloatingWindowLevel"); }
TEST_CASE("output law: no kiosk mode", "[output_law]") { forbid("setKioskModeComponent"); }
TEST_CASE("output law: no native fullscreen (setFullScreen(true))", "[output_law]") { forbid("setFullScreen(true)"); }
TEST_CASE("output law: no native fullscreen (toggleFullScreen)", "[output_law]") { forbid("toggleFullScreen"); }
TEST_CASE("output law: never takes the keyboard -- no toFront(true)", "[output_law]") { forbid("toFront(true)"); }
TEST_CASE("output law: never takes the keyboard -- no grabKeyboardFocus", "[output_law]") { forbid("grabKeyboardFocus"); }
TEST_CASE("output law: never takes the keyboard -- no setWantsKeyboardFocus(true)", "[output_law]") { forbid("setWantsKeyboardFocus(true)"); }

TEST_CASE("output law: the peer carries windowIgnoresKeyPresses", "[output_law]")
{
    // The flag must be in the body of the window's getDesktopWindowStyleFlags() override -- the flags every
    // (re)created peer is built with -- not merely named somewhere (e.g. in an assertion).
    const Sources src;
    std::string body;
    for (const std::string* code : { &src.cpp, &src.h })
    {
        const std::string sig = code == &src.cpp ? "OutputWindow::getDesktopWindowStyleFlags()" : "getDesktopWindowStyleFlags()";
        const size_t at = code->find(sig);
        const size_t open = at == std::string::npos ? std::string::npos : code->find('{', at);
        if (open == std::string::npos || (code == &src.h && code->find(';', at) < open))
            continue;   // no definition here (the header only declares it)
        int depth = 0;
        size_t i = open;
        for (; i < code->size(); ++i)
        {
            if ((*code)[i] == '{') ++depth;
            else if ((*code)[i] == '}' && --depth == 0) break;
        }
        body = code->substr(open, i - open + 1);
        break;
    }
    INFO("OutputWindow must override getDesktopWindowStyleFlags() and add juce::ComponentPeer::windowIgnoresKeyPresses "
         "(found body: " << (body.empty() ? std::string("<no override>") : body) << ")");
    CHECK(body.find("windowIgnoresKeyPresses") != std::string::npos);
}

TEST_CASE("output law: the constructor checks the peer's windowIgnoresKeyPresses in every build type", "[output_law]")
{
    // A jassert is compiled out of Release, so it cannot be the only check that the peer really carries the flag.
    // The constructor body, with every jassert(...) removed, must still read the peer's style flags, test
    // windowIgnoresKeyPresses and re-add the window to the desktop (the repair) when the flag is missing.
    const Sources src;
    const std::string& c = src.cpp;
    const std::string sig = "OutputWindow::OutputWindow(";
    const size_t at = c.find(sig);
    REQUIRE(at != std::string::npos);
    const size_t open = c.find('{', at);
    REQUIRE(open != std::string::npos);
    int depth = 0;
    size_t end = open;
    for (; end < c.size(); ++end)
    {
        if (c[end] == '{') ++depth;
        else if (c[end] == '}' && --depth == 0) break;
    }
    std::string body = c.substr(open, end - open + 1);
    for (size_t j = body.find("jassert("); j != std::string::npos; j = body.find("jassert("))
    {
        size_t k = j + 7;   // the '(' of jassert(
        int d = 0;
        for (; k < body.size(); ++k)
        {
            if (body[k] == '(') ++d;
            else if (body[k] == ')' && --d == 0) break;
        }
        body.erase(j, k - j + 1);
    }
    INFO("OutputWindow's constructor, jasserts removed: " << body);
    CHECK(body.find("getStyleFlags()") != std::string::npos);
    CHECK(body.find("windowIgnoresKeyPresses") != std::string::npos);
    CHECK(body.find("addToDesktop(") != std::string::npos);
}

TEST_CASE("output law: bounds before visible -- every setVisible(true) follows a setBounds( in its function", "[output_law]")
{
    const Sources src;
    const std::string& c = src.cpp;
    const std::string show = "setVisible(true)";
    size_t count = 0;
    for (size_t pos = c.find(show); pos != std::string::npos; pos = c.find(show, pos + 1))
    {
        ++count;
        // The enclosing member function: the last "OutputWindow::<name>(" definition before this call.
        size_t fn = std::string::npos;
        for (size_t p = c.find("OutputWindow::"); p != std::string::npos && p < pos; p = c.find("OutputWindow::", p + 1))
            fn = p;
        REQUIRE(fn != std::string::npos);
        const std::string name = c.substr(fn, c.find('(', fn) - fn);
        INFO(name << " shows the window: setBounds( must come first in the same function");
        CHECK(c.find("setBounds(", fn) < pos);
    }
    INFO("the window is shown somewhere (a setVisible(true) exists)");
    CHECK(count >= 1);
}

// ---- plan5 C2 (s-rta-0927 outputs-c2): the output windows' one owner, OutputManager ----

TEST_CASE("output law: OutputManager never raises, focuses, floats or shows a window itself", "[output_law]")
{
    const std::string h = codeOnly(readFile("output/OutputManager.h"));
    const std::string cpp = codeOnly(readFile("output/OutputManager.cpp"));
    for (const char* token : { "setAlwaysOnTop(true)", "NSFloatingWindowLevel", "setKioskModeComponent", "setFullScreen(true)",
                               "toggleFullScreen", "toFront(true)", "grabKeyboardFocus", "setWantsKeyboardFocus(true)",
                               "setVisible(true)" })
    {
        INFO("src/output/OutputManager.{h,cpp} must never contain `" << token << "` (comments excluded) -- a window is "
             "shown only by OutputWindow::openOnDisplay (bounds first, ordered front, never key)");
        CHECK(h.find(token) == std::string::npos);
        CHECK(cpp.find(token) == std::string::npos);
    }
    INFO("OutputManager opens a window through OutputWindow::openOnDisplay");
    CHECK(cpp.find("->openOnDisplay(") != std::string::npos);
}

TEST_CASE("output law: no deck or composition file reaches the output code (outputDisplay, plan5 R7)", "[output_law]")
{
    // An old deck/composition saved with an output open must never open one on load: the output code -- the
    // manager and the MainComponent that wires it -- never names the file key or the struct field.
    for (const char* rel : { "output/OutputManager.h", "output/OutputManager.cpp", "MainComponent.h", "MainComponent.cpp" })
    {
        INFO("src/" << rel << " must never read `outputDisplay` (comments excluded, string literals included)");
        CHECK(codeWithStrings(readFile(rel)).find("outputDisplay") == std::string::npos);
    }
}

// ---- plan5 C3 (s-rta-0927 outputs-c3): hot-plug + the saved output set ----

namespace
{
// The body (braces included) of the first definition `sig` in whitespace-free code, or "" if there is none.
std::string functionBody(const std::string& code, const std::string& sig)
{
    const size_t at = code.find(sig);
    const size_t open = at == std::string::npos ? std::string::npos : code.find('{', at);
    if (open == std::string::npos)
        return {};
    int depth = 0;
    size_t i = open;
    for (; i < code.size(); ++i)
    {
        if (code[i] == '{') ++depth;
        else if (code[i] == '}' && --depth == 0) break;
    }
    return code.substr(open, i - open + 1);
}
} // namespace

TEST_CASE("output law: nothing opens an output at launch -- Restore Last Outputs is the saved set's only door (plan5 Q1)",
          "[output_law]")
{
    // MainComponent calls OutputManager::restoreLast() exactly once, as the first statement of the
    // kOutputRestoreLast menu case (the Output menu / TopBar item), never at startup or on a file load.
    const std::string mc = codeOnly(readFile("MainComponent.cpp"));
    const std::string call = "outputs_.restoreLast(";
    size_t calls = 0;
    for (size_t p = mc.find(call); p != std::string::npos; p = mc.find(call, p + 1))
        ++calls;
    INFO("src/MainComponent.cpp must call outputs_.restoreLast( exactly once, in `case C::kOutputRestoreLast:`");
    REQUIRE(calls == 1);
    const size_t at = mc.find(call);
    const std::string label = "caseC::kOutputRestoreLast:";
    const size_t caseAt = mc.rfind(label, at);
    REQUIRE(caseAt != std::string::npos);
    CHECK(mc.substr(caseAt + label.size(), at - caseAt - label.size()).empty());

    // OutputManager's constructor and attachSettings() (which loads the saved set) never open a window.
    const std::string om = codeOnly(readFile("output/OutputManager.cpp"));
    for (const char* sig : { "OutputManager::OutputManager(", "OutputManager::attachSettings(" })
    {
        const std::string body = functionBody(om, sig);
        INFO(sig << " body: " << body);
        REQUIRE_FALSE(body.empty());
        for (const char* opener : { "openWindow(", "openDisplay(", "toggleDisplay(", "restoreLast(", "reconcile(",
                                    "openOnDisplay(" })
            CHECK(body.find(opener) == std::string::npos);
    }
}

TEST_CASE("output law: every settings write keeps the saved set not yet restored; a manual open/close decides for "
          "its display",
          "[output_law]")
{
    // settings.json "outputs" is replaced whole on each write, so persistWanted() must write output::wantedSet(live,
    // interrupted, saved_) -- never live + interrupted alone, which drops every saved target Restore has not opened
    // (a partial Restore, an unrelated output change, All Outputs Off). A manual open or close of a display removes
    // that display's saved target (the user decided), and the loaded set is what the file already holds.
    const std::string om = codeOnly(readFile("output/OutputManager.cpp"));
    const std::string persist = functionBody(om, "voidOutputManager::persistWanted(");
    INFO("persistWanted body: " << persist);
    REQUIRE_FALSE(persist.empty());
    CHECK(persist.find("wantedSet(") != std::string::npos);
    CHECK(persist.find("saved_)") != std::string::npos);
    for (const char* sig : { "voidOutputManager::openDisplay(", "voidOutputManager::closeDisplay(",
                             "voidOutputManager::closeWindow(" })
    {
        const std::string body = functionBody(om, sig);
        INFO(sig << " body: " << body);
        REQUIRE_FALSE(body.empty());
        CHECK(body.find("forgetSaved(") != std::string::npos);
    }
    const std::string attach = functionBody(om, "voidOutputManager::attachSettings(");
    INFO("attachSettings body: " << attach);
    CHECK(attach.find("lastWanted_=saved_;") != std::string::npos);
}
