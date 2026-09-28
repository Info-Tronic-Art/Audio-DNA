// test_shader_param_lint -- every REGISTERED parameter is read by its shader (s-rta-0927 source-defects,
// .harmony/.reports/s-rta-0927/plan-source-defects.md T1 / A2). A control whose uniform the shader never reads is a
// dead knob in the inspector: it moves, nothing changes (the Tier-1 diagnosis found 81 of them).
//
// Static, no GL: the files are read at test time from AUDIODNA_SRC_DIR.
//   - src/render/Renderer.cpp        compile("key", EmbeddedShaders::sym)      -> shader key -> GLSL block
//   - src/render/EmbeddedShaders.h   (inline|static) [constexpr] const char* sym = R"delim( ... )delim";
//   - src/sources/SourceRegistry.cpp addParam("Name", "u_x", Vf): its shader key is the nearest PRECEDING
//                                    "source_..." literal (the ProceduralSource constructor's shader argument)
//   - src/effects/EffectLibrary.cpp  registerEffect({"Name", "cat", "key", { {"p", "u_x", Vf}, ... } [, true]});
// A parameter is READ iff its identifier occurs in its block outside its own `uniform <type> <name>;` line.
// DEBT_FILED (s-rta-0927 source-defects review): READ is textual, not "changes the picture". A use that is a
// mathematical no-op still passes -- e.g. spectrum_landscape's `energy = mix(energy, energy, u_src_smoothing);`
// (mix(x, x, t) == x), so its Smoothing knob is dead yet lint-clean. Catching that class needs a render-based check
// (the Tier-1 "has effect" sweep) or a follow-up static pass; this lint only proves the name is used.
// Plus one structural rule (Pitfall 44): a helper lambda that adds params but constructs no ProceduralSource is
// refused -- its addParam calls sit far from any shader key, which is exactly how 70 dead torus knobs hid.
#include <catch2/catch_test_macros.hpp>
#include <algorithm>
#include <cctype>
#include <fstream>
#include <map>
#include <regex>
#include <set>
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

std::vector<std::string> lines(const std::string& s)
{
    std::vector<std::string> out;
    std::stringstream ss(s);
    for (std::string l; std::getline(ss, l);) out.push_back(l);
    return out;
}

bool isIdent(char c) { return std::isalnum(static_cast<unsigned char>(c)) || c == '_'; }

// symbol -> GLSL text, scanning raw string literals by hand (no regex over the whole header).
std::map<std::string, std::string> shaderBlocks(const std::string& src)
{
    std::map<std::string, std::string> out;
    for (size_t p = src.find("R\""); p != std::string::npos; p = src.find("R\"", p + 2))
    {
        if (p > 0 && isIdent(src[p - 1])) continue;              // e.g. uR"..." or an identifier ending in R
        const size_t open = src.find('(', p + 2);
        if (open == std::string::npos) break;
        const std::string delim = src.substr(p + 2, open - (p + 2));
        if (delim.size() > 16 || delim.find_first_of(" \n\"") != std::string::npos) continue;
        const std::string closeTok = ")" + delim + "\"";
        const size_t close = src.find(closeTok, open + 1);
        if (close == std::string::npos) break;
        // the declaration head: "... const char* sym = " right before R"
        size_t e = p;
        while (e > 0 && std::isspace(static_cast<unsigned char>(src[e - 1]))) --e;
        if (e == 0 || src[e - 1] != '=') { p = close; continue; }
        --e;
        while (e > 0 && std::isspace(static_cast<unsigned char>(src[e - 1]))) --e;
        size_t b = e;
        while (b > 0 && isIdent(src[b - 1])) --b;
        const std::string sym = src.substr(b, e - b);
        const size_t lineStart = src.rfind('\n', b) == std::string::npos ? 0 : src.rfind('\n', b) + 1;
        const std::string head = src.substr(lineStart, b - lineStart);
        if (head.find("const char") != std::string::npos && !sym.empty())
            out[sym] = src.substr(open + 1, close - (open + 1));
        p = close;
    }
    return out;
}

// Is `ident` used in `block` outside its own `uniform <type> ident;` / `uniform <type> ident[N];` declaration?
bool isRead(const std::string& block, const std::string& ident)
{
    for (const auto& l : lines(block))
    {
        for (size_t p = l.find(ident); p != std::string::npos; p = l.find(ident, p + 1))
        {
            const bool left = p == 0 || !isIdent(l[p - 1]);
            const bool right = p + ident.size() >= l.size() || !isIdent(l[p + ident.size()]);
            if (!left || !right) continue;
            // a declaration line: "uniform <type> ident;" (whitespace-insensitive)
            std::string code = l.substr(0, l.find("//"));
            std::istringstream toks(code);
            std::string t0, t1, t2;
            toks >> t0 >> t1 >> t2;
            const bool decl = t0 == "uniform" && (t2 == ident + ";" || t2.rfind(ident + "[", 0) == 0 || t2 == ident);
            if (!decl) return true;
        }
    }
    return false;
}

struct Reg { std::string owner, name, uniform, key; int line; };

// C++-consumed parameters (not dead even though the GLSL never names them).
bool allowlisted(const Reg& r)
{
    if (r.key == "source_layer_router" && r.uniform == "u_src_layer") return true;       // Renderer.cpp: picks the layer
    if (r.owner == "effect:Screen Split" || r.owner == "effect:Frame Stutter") return true; // CompositorEngine ring (Pitfall 19)
    return false;
}
} // namespace

TEST_CASE("every registered source and effect parameter is read by its shader", "[lint][shader-params]")
{
    const auto blocks = shaderBlocks(readFile("render/EmbeddedShaders.h"));
    REQUIRE(blocks.size() > 200);

    std::map<std::string, std::string> keyToSym;
    {
        static const std::regex re(R"re(compile\(\s*"(\w+)",\s*EmbeddedShaders::(\w+)\s*\))re");
        for (const auto& l : lines(readFile("render/Renderer.cpp")))
        {
            std::smatch m;
            if (std::regex_search(l, m, re)) keyToSym[m[1].str()] = m[2].str();
        }
    }
    REQUIRE(keyToSym.size() > 200);

    std::vector<Reg> regs;
    std::vector<std::string> structural;

    // --- sources
    {
        const auto ls = lines(readFile("sources/SourceRegistry.cpp"));
        static const std::regex keyRe(R"re("(source_\w+)")re");
        static const std::regex paramRe(R"re(addParam\("([^"]+)",\s*"(\w+)",\s*(?:[0-9.]+f|\w+)\))re");
        static const std::regex lambdaRe(R"re(^(\s*)auto\s+(\w+)\s*=\s*\[)re");
        std::string key;
        for (size_t i = 0; i < ls.size(); ++i)
        {
            const auto& l = ls[i];
            std::smatch m;
            if (std::regex_search(l, m, lambdaRe))
            {
                const std::string indent = m[1].str(), name = m[2].str();
                bool adds = false, constructs = false;
                for (size_t j = i; j < ls.size(); ++j)
                {
                    adds = adds || ls[j].find("addParam(") != std::string::npos;
                    constructs = constructs || ls[j].find("ProceduralSource>(") != std::string::npos;
                    if (j > i && ls[j].rfind(indent + "};", 0) == 0) break;
                }
                if (adds && !constructs)
                    structural.push_back("SourceRegistry.cpp:" + std::to_string(i + 1) + " helper lambda '" + name
                                         + "' adds params but constructs no ProceduralSource (no shader key in scope)");
            }
            if (std::regex_search(l, m, keyRe)) key = m[1].str();
            if (l.find("addParam(") != std::string::npos)
            {
                REQUIRE(std::regex_search(l, m, paramRe));   // an addParam this lint cannot read FAILS
                REQUIRE(!key.empty());
                regs.push_back({ "source:" + key, m[1].str(), m[2].str(), key, int(i + 1) });
            }
        }
    }
    const size_t nSource = regs.size();

    // --- effects
    {
        const std::string lib = readFile("effects/EffectLibrary.cpp");
        static const std::regex headRe(R"re(registerEffect\(\{"([^"]+)",\s*"(\w+)",\s*"(\w+)")re");
        static const std::regex paramRe(R"re(\{"([^"]+)",\s*"(\w+)",\s*-?[0-9.]+f\s*\})re");
        for (size_t p = lib.find("registerEffect({"); p != std::string::npos; p = lib.find("registerEffect({", p + 1))
        {
            size_t end = lib.find("registerEffect({", p + 1);
            if (end == std::string::npos) end = lib.size();
            const std::string span = lib.substr(p, end - p);
            std::smatch m;
            REQUIRE(std::regex_search(span, m, headRe));
            const std::string name = m[1].str(), key = m[3].str();
            const size_t lineNo = static_cast<size_t>(std::count(lib.begin(), lib.begin() + static_cast<long>(p), '\n')) + 1;
            const std::string body = span.substr(static_cast<size_t>(m.position(0) + m.length(0)));
            for (auto it = std::sregex_iterator(body.begin(), body.end(), paramRe); it != std::sregex_iterator(); ++it)
                regs.push_back({ "effect:" + name, (*it)[1].str(), (*it)[2].str(), key, int(lineNo) });
        }
    }
    const size_t nEffect = regs.size() - nSource;
    INFO("parsed " << nSource << " source params, " << nEffect << " effect params");
    REQUIRE(nSource > 500);
    REQUIRE(nEffect > 300);

    std::vector<std::string> dead;
    for (const auto& r : regs)
    {
        const auto ks = keyToSym.find(r.key);
        if (ks == keyToSym.end() || blocks.find(ks->second) == blocks.end())
        {
            dead.push_back(r.owner + " '" + r.name + "' (" + r.uniform + "): shader key '" + r.key + "' has no compiled block");
            continue;
        }
        if (allowlisted(r)) continue;
        if (!isRead(blocks.at(ks->second), r.uniform))
            dead.push_back(r.owner + " '" + r.name + "' (" + r.uniform + ") -- never read by " + ks->second
                           + " (registered at line " + std::to_string(r.line) + ")");
    }

    std::ostringstream report;
    for (const auto& s : structural) report << "  STRUCTURE " << s << "\n";
    for (const auto& d : dead) report << "  DEAD " << d << "\n";
    INFO(structural.size() << " structural + " << dead.size() << " dead parameter(s):\n" << report.str());
    CHECK(structural.empty());
    CHECK(dead.empty());
}
