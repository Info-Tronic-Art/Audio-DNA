#include <atomic>
#include <cstdint>
struct RT { int32_t a; int32_t p; float g; uint32_t q; };
RT loadIt(const std::atomic<RT>& x) { return x.load(std::memory_order_acquire); }
bool casIt(std::atomic<RT>& x, RT& e, RT d) { return x.compare_exchange_strong(e, d, std::memory_order_acq_rel, std::memory_order_acquire); }
void storeIt(std::atomic<RT>& x, RT d) { x.store(d, std::memory_order_release); }
