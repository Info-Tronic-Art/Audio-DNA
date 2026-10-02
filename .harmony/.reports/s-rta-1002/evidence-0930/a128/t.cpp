#include <atomic>
#include <cstdint>
#include <cstdio>
#include <thread>
struct RT { int32_t active; int32_t previous; float progress; int16_t pending; uint8_t snap; uint8_t reserved; };
static_assert(sizeof(RT) == 16);
std::atomic<RT> g{RT{-1,-1,1.0f,-1,0,0}};
int main() {
    std::printf("always_lock_free=%d is_lock_free=%d\n", (int)std::atomic<RT>::is_always_lock_free, (int)g.is_lock_free());
    std::printf("u64 always=%d\n", (int)std::atomic<uint64_t>::is_always_lock_free);
    std::thread t1([]{ for (int i = 0; i < 200000; ++i) { RT o = g.load(std::memory_order_acquire); RT n; do { n = o; n.progress = n.progress < 1.0f ? n.progress + 0.01f : n.progress; if (n.progress >= 1.0f) n.previous = -1; } while (!g.compare_exchange_weak(o, n, std::memory_order_acq_rel, std::memory_order_acquire)); } });
    std::thread t2([]{ for (int i = 0; i < 200000; ++i) { RT o = g.load(std::memory_order_acquire); RT n; do { n = o; n.previous = n.active; n.active = i % 7; n.progress = 0.0f; n.pending = -1; } while (!g.compare_exchange_weak(o, n, std::memory_order_acq_rel, std::memory_order_acquire)); } });
    t1.join(); t2.join();
    RT f = g.load(); std::printf("final active=%d prev=%d prog=%f\n", f.active, f.previous, f.progress);
}
