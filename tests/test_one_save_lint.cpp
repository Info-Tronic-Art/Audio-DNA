// test_one_save_lint -- lane one-save (ruling-one-save section 5, [onesavelint]): source greps over src/, the
// pattern of tests/test_output_law.cpp. The files are read at test time from AUDIODNA_SRC_DIR.
//   LINT-1 (S1): the show is written by ONE function -- Composition::saveToFile is called only inside
//                MainComponent::writeShow (which runs the copy-before-overwrite rule first).
//   LINT-7 (S1): the three files that write the show, the copy kept before an overwrite and settings.json name no
//                unverified JUCE text writer.
#include <catch2/catch_test_macros.hpp>
#include <filesystem>
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

// Code only: comments and the contents of string/char literals removed, then every whitespace char removed
// (tests/test_output_law.cpp's codeOnly).
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

// Every .h / .cpp / .mm under src/, as paths relative to it.
std::vector<std::string> sourceFiles()
{
    namespace fs = std::filesystem;
    std::vector<std::string> out;
    const fs::path root(AUDIODNA_SRC_DIR);
    for (const auto& e : fs::recursive_directory_iterator(root))
    {
        const auto ext = e.path().extension().string();
        if (e.is_regular_file() && (ext == ".h" || ext == ".cpp" || ext == ".mm"))
            out.push_back(fs::relative(e.path(), root).generic_string());
    }
    return out;
}

bool endsWith(const std::string& s, size_t end, const std::string& tail)
{
    return end >= tail.size() && s.compare(end - tail.size(), tail.size(), tail) == 0;
}

struct Call { std::string file; size_t at; };

// Every CALL of a member named saveToFile on something that is not the key bindings' manager: a declaration or a
// definition (`bool saveToFile(`, `X::saveToFile(`) is not a call; BindingManager has a saveToFile of its own, called
// as bindingManager_.saveToFile( until the keys stage removes it.
std::vector<Call> showSaveCalls(const std::string& rel, const std::string& code)
{
    std::vector<Call> out;
    const std::string needle = "saveToFile(";
    for (size_t at = code.find(needle); at != std::string::npos; at = code.find(needle, at + 1))
    {
        if (endsWith(code, at, "bool") || endsWith(code, at, "::") || endsWith(code, at, "bindingManager_."))
            continue;
        out.push_back({ rel, at });
    }
    return out;
}
} // namespace

TEST_CASE("`saveToFile(` on a Composition is called only inside MainComponent::writeShow", "[onesavelint]")
{
    const auto files = sourceFiles();
    REQUIRE(files.size() > 100);   // the walk found the tree

    std::vector<Call> calls;
    for (const auto& rel : files)
    {
        const auto found = showSaveCalls(rel, codeOnly(readFile(rel)));
        calls.insert(calls.end(), found.begin(), found.end());
    }
    for (const auto& c : calls)
        UNSCOPED_INFO("a show save call in src/" << c.file);
    REQUIRE(calls.size() == 1);
    CHECK(calls[0].file == "MainComponent.cpp");

    // ... and that one call sits in the body of MainComponent::writeShow.
    const std::string mc = codeOnly(readFile("MainComponent.cpp"));
    const size_t begin = mc.find("boolMainComponent::writeShow(");
    REQUIRE(begin != std::string::npos);
    const size_t end = mc.find("MainComponent::", begin + 20);   // the next member definition
    REQUIRE(end != std::string::npos);
    CHECK(calls[0].at > begin);
    CHECK(calls[0].at < end);
    // writeShow runs the write through the copy-before-overwrite rule, never bare.
    CHECK(mc.substr(begin, end - begin).find("showfile::saveWithBackup(") != std::string::npos);

    // The scanner is not blind: it sees a bare call, and skips the three shapes that are not one.
    CHECK(showSaveCalls("x", codeOnly("void f() { composition_.saveToFile(file, {}); }")).size() == 1);
    CHECK(showSaveCalls("x", codeOnly("auto ok = comp.saveToFile (f, extras); // c")).size() == 1);
    CHECK(showSaveCalls("x", codeOnly("bool saveToFile(const juce::File& f) const;")).empty());
    CHECK(showSaveCalls("x", codeOnly("bool BindingManager::saveToFile(const juce::File& f) const {}")).empty());
    CHECK(showSaveCalls("x", codeOnly("bindingManager_.saveToFile(f); // composition_.saveToFile(f)")).empty());
}

TEST_CASE("Composition.h, AppSettings.cpp and ShowFile.h name no `replaceWithText`", "[onesavelint]")
{
    for (const char* rel : { "model/Composition.h", "model/AppSettings.cpp", "core/ShowFile.h" })
    {
        INFO("src/" << rel << " must not name replaceWithText (comments included): the verified writer is "
                               "safewrite::writeTextVerified, src/core/SafeFileWrite.h");
        CHECK(readFile(rel).find("replaceWithText") == std::string::npos);
    }
    // Each of the three goes through the verified writer instead.
    CHECK(codeOnly(readFile("model/Composition.h")).find("safewrite::writeTextVerified(") != std::string::npos);
    CHECK(codeOnly(readFile("model/AppSettings.cpp")).find("safewrite::writeTextVerified(") != std::string::npos);
    CHECK(codeOnly(readFile("core/ShowFile.h")).find("safewrite::copyVerified(") != std::string::npos);
}
