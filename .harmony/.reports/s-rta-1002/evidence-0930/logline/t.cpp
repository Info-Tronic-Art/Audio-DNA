#include <cstdio>
#include <iostream>
#include <sstream>
#include <string>
#include <thread>
template <class... A> void logLine(const A&... a) {
    std::ostringstream os; (os << ... << a); os << '\n';
    const std::string s = os.str();
    std::fwrite(s.data(), 1, s.size(), stderr);
}
int main(int argc, char** argv) {
    const int mode = argc > 1 ? std::atoi(argv[1]) : 0;
    auto w = [mode](int id) {
        for (int i = 0; i < 2000; ++i) {
            if (mode == 0) logLine("[t", id, "] line ", i, " x=", 1.5 * i);            // both threads logLine
            else if (mode == 1) { if (id == 0) logLine("[t", id, "] ", i); else std::cerr << "[t" << id << "] " << i << std::endl; } // one cerr, one logLine
            else std::cerr << "[t" << id << "] " << i << std::endl;                       // both cerr (control: must race)
        }
    };
    std::thread a(w, 0), b(w, 1); a.join(); b.join();
}
