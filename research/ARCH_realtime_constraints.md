# Real-Time Audio Programming Constraints and Rules

**Document ID:** ARCH_realtime_constraints
**Status:** Active
**Cross-references:** [ARCH_pipeline.md](ARCH_pipeline.md), [ARCH_audio_io.md](ARCH_audio_io.md), [REF_latency_numbers.md](REF_latency_numbers.md), [IMPL_testing_validation.md](IMPL_testing_validation.md)

---

## 1. The Golden Rules of Real-Time Audio

An audio callback runs on a high-priority thread managed by the operating system's audio subsystem (CoreAudio, ALSA/JACK/PulseAudio, WASAPI). At 48 kHz with a 128-sample buffer, the callback must complete in **2.67 milliseconds**. Every violation of the rules below introduces unbounded latency, and unbounded latency in a real-time context means audible glitches, dropouts, and xruns.

### Rule 1: Never Allocate Memory

```cpp
// BROKEN: malloc inside audio callback
void audioCallback(float* output, int numFrames) {
    // malloc acquires a process-wide heap lock (pthread_mutex internally).
    // If another thread is in malloc at the same time, this thread blocks
    // for an unbounded duration. The OS audio subsystem does not wait --
    // it delivers silence or stale data to the DAC.
    float* temp = new float[numFrames];  // DO NOT DO THIS
    process(temp, numFrames);
    std::copy(temp, temp + numFrames, output);
    delete[] temp;
}
```

**Why it fails:** `malloc`/`new` on every major platform (glibc, jemalloc, tcmalloc, macOS libmalloc) internally acquires a mutex to protect the heap metadata. Even "fast-path" allocators like tcmalloc use thread-local caches that periodically synchronize with a central allocator under a lock. Worse, `malloc` can trigger `mmap`/`sbrk` system calls to request pages from the kernel, which can block for milliseconds on memory pressure. The `delete`/`free` path has identical problems -- it may coalesce free blocks, call `munmap`, or contend on the same heap lock.

**Fix:** Pre-allocate all memory before the audio stream starts.

```cpp
class AudioProcessor {
    std::vector<float> scratchBuffer_;  // allocated once in constructor
public:
    AudioProcessor(int maxFrames) : scratchBuffer_(maxFrames) {}

    void audioCallback(float* output, int numFrames) {
        // scratchBuffer_ already allocated -- no heap activity
        process(scratchBuffer_.data(), numFrames);
        std::copy(scratchBuffer_.begin(),
                  scratchBuffer_.begin() + numFrames, output);
    }
};
```

### Rule 2: Never Use Mutexes, Locks, or Blocking Synchronization

```cpp
// BROKEN: mutex in audio callback
std::mutex paramMutex_;
float cutoffFreq_ = 1000.0f;

void audioCallback(float* output, int numFrames) {
    std::lock_guard<std::mutex> lock(paramMutex_);  // DO NOT DO THIS
    for (int i = 0; i < numFrames; ++i) {
        output[i] = filter(input[i], cutoffFreq_);
    }
}

void setParameter(float freq) {
    std::lock_guard<std::mutex> lock(paramMutex_);
    cutoffFreq_ = freq;
}
```

**Why it fails:** If the GUI thread holds `paramMutex_` when the audio callback fires, the audio thread blocks. The GUI thread runs at normal priority and can be preempted by any other normal-priority thread, or stall on a page fault, or get descheduled entirely for 10+ ms by the OS scheduler. The audio thread inherits that entire delay. Even `std::try_lock` is not a solution -- if you skip processing when the lock is contended, you produce silence, which is also a glitch.

**Fix:** Use `std::atomic` for simple parameters, lock-free queues for complex state changes.

```cpp
std::atomic<float> cutoffFreq_{1000.0f};

void audioCallback(float* output, int numFrames) {
    float freq = cutoffFreq_.load(std::memory_order_relaxed);
    for (int i = 0; i < numFrames; ++i) {
        output[i] = filter(input[i], freq);
    }
}

void setParameter(float freq) {
    cutoffFreq_.store(freq, std::memory_order_relaxed);
}
```

### Rule 3: Never Make System Calls

```cpp
// BROKEN: file I/O in audio callback
void audioCallback(float* output, int numFrames) {
    process(output, numFrames);
    // write() is a system call that traps into the kernel.
    // The kernel may:
    //   - block on disk I/O (HDD seek: 5-10ms, SSD: 50-200us)
    //   - block on a filesystem journal lock
    //   - trigger page cache writeback
    //   - deschedule the thread if the disk queue is full
    fprintf(logFile_, "processed %d frames\n", numFrames);  // DO NOT DO THIS
}
```

**Other prohibited system calls include:** `open`, `close`, `read`, `write`, `stat`, `mmap`, `munmap`, `futex` (used internally by `pthread_mutex_lock`), `nanosleep`, `clock_gettime` on some platforms (though on Linux with vDSO it is safe), `socket`, `send`, `recv`, `poll`, `select`, Objective-C message dispatch (triggers objc_msgSend which may acquire locks), and any CoreFoundation/Cocoa API.

**Fix:** Enqueue data to a lock-free queue and let a separate I/O thread drain it.

### Rule 4: Never Throw Exceptions

```cpp
// BROKEN: exception in audio callback
void audioCallback(float* output, int numFrames) {
    if (numFrames > maxFrames_) {
        // Stack unwinding involves:
        //   - _Unwind_RaiseException: walks the call stack
        //   - Calls destructors for all stack-allocated objects
        //   - May allocate memory for exception object (via __cxa_allocate_exception)
        //   - On GCC/Clang: involves dl_iterate_phdr which takes a global lock
        throw std::runtime_error("buffer overflow");  // DO NOT DO THIS
    }
}
```

**Why it fails:** Exception handling on Itanium ABI (used by GCC, Clang on Linux and macOS) requires walking the `.eh_frame` section, which involves calling `dl_iterate_phdr` -- a function that acquires a global lock over the dynamic linker state. Even the "zero-cost" exception model (no overhead when exceptions are not thrown) has non-zero cost when exceptions *are* thrown, and that cost is unbounded.

**Fix:** Use error codes, sentinel values, or `std::expected`/`std::optional` for error handling in audio paths.

### Rule 5: Never Use Allocating STL Containers

```cpp
// BROKEN: vector resize in audio callback
std::vector<float> peakHistory_;

void audioCallback(float* output, int numFrames) {
    float peak = computePeak(output, numFrames);
    // push_back may reallocate: O(n) copy + malloc + free
    peakHistory_.push_back(peak);  // DO NOT DO THIS
}
```

Prohibited operations include: `std::vector::push_back` (may reallocate), `std::string` construction or concatenation (heap allocates), `std::map`/`std::unordered_map` insertion (allocates nodes), `std::shared_ptr` creation (allocates control block), `std::function` assignment (may heap-allocate callable), and `std::any`/`std::variant` with types exceeding small-object threshold.

**Fix:** Use fixed-capacity containers, pre-allocated ring buffers, or stack arrays.

```cpp
// Fixed-capacity ring buffer -- no allocation
template<typename T, size_t N>
class FixedRingBuffer {
    std::array<T, N> buffer_;
    size_t writePos_ = 0;
public:
    void push(T value) {
        buffer_[writePos_ % N] = value;
        ++writePos_;
    }
    T get(size_t ago) const {
        return buffer_[(writePos_ - 1 - ago) % N];
    }
};

FixedRingBuffer<float, 1024> peakHistory_;  // stack or member -- no heap
```

### Rule 6: Why Each Rule Matters -- The Cascade Effect

A single violation does not always produce an audible glitch. The danger is probabilistic: under load, with other threads contending for resources, the probability of a worst-case delay compounds. A `malloc` that takes 200 ns in isolation can take 5 ms when another thread is in `free` performing coalescing. A `fprintf` that returns instantly when the kernel buffer is empty can block for 50 ms when the disk queue is saturated. Real-time audio demands **worst-case** guarantees, not average-case performance.

---

## 2. Lock-Free Programming Patterns

### Memory Ordering

`std::atomic` operations in C++ accept a memory ordering argument that controls how the compiler and CPU reorder loads and stores around the atomic operation.

| Ordering | Guarantee | Use Case |
|---|---|---|
| `relaxed` | Atomicity only, no ordering | Counters, flags read by one thread |
| `acquire` | Loads after this see stores before the paired release | Consumer side of producer-consumer |
| `release` | Stores before this are visible after the paired acquire | Producer side of producer-consumer |
| `acq_rel` | Both acquire and release | CAS loops (read-modify-write) |
| `seq_cst` | Total order across all threads | Rarely needed; default but expensive on ARM |

**Critical nuance for audio:** On x86/x64, `relaxed` and `acquire`/`release` compile to the same instructions (x86 has a strong memory model). On ARM (Apple Silicon), `acquire`/`release` generate `ldar`/`stlr` instructions with barrier semantics. Using `seq_cst` on ARM generates full `dmb` barriers, which can cost 20-40 ns per operation. In an audio callback processing 128 samples, this adds up.

```cpp
// Parameter update: relaxed is sufficient for independent atomic values.
// The audio thread does not need to see a consistent "snapshot" of
// multiple parameters -- each parameter is independent.
std::atomic<float> gain_{1.0f};
std::atomic<float> pan_{0.5f};

// Audio thread:
float g = gain_.load(std::memory_order_relaxed);
float p = pan_.load(std::memory_order_relaxed);

// GUI thread:
gain_.store(newGain, std::memory_order_relaxed);
pan_.store(newPan, std::memory_order_relaxed);
```

```cpp
// When publishing a pointer to a new data structure, use release/acquire
// to ensure the consumer sees all writes to the pointed-to data.
struct FilterState {
    float coefficients[5];
    int order;
};

std::atomic<FilterState*> currentFilter_{nullptr};

// Producer (GUI thread):
void updateFilter(float cutoff, float q) {
    // Allocate from pre-allocated pool (NOT malloc)
    FilterState* newState = filterPool_.allocate();
    newState->coefficients[0] = computeA0(cutoff, q);
    newState->coefficients[1] = computeA1(cutoff, q);
    // ... fill all fields ...
    newState->order = 2;

    FilterState* old = currentFilter_.exchange(
        newState, std::memory_order_release  // publishes the writes above
    );
    // Reclaim old state via deferred deletion (not immediate free)
    recycleQueue_.push(old);
}

// Consumer (audio thread):
void audioCallback(float* output, int numFrames) {
    FilterState* state = currentFilter_.load(std::memory_order_acquire);
    // acquire guarantees we see all writes made before the release store
    applyFilter(output, numFrames, state->coefficients, state->order);
}
```

### Compare-and-Swap (CAS)

CAS is the fundamental building block for lock-free algorithms. It atomically: reads the current value, compares it to an expected value, and if equal, replaces it with a desired value.

```cpp
// Lock-free counter increment using CAS loop
std::atomic<uint64_t> frameCount_{0};

void incrementFrameCount(uint64_t numFrames) {
    uint64_t expected = frameCount_.load(std::memory_order_relaxed);
    while (!frameCount_.compare_exchange_weak(
        expected,                           // updated on failure
        expected + numFrames,               // desired value
        std::memory_order_relaxed,          // success ordering
        std::memory_order_relaxed           // failure ordering
    )) {
        // CAS failed: another thread modified frameCount_.
        // expected now contains the current value. Retry.
    }
}
```

`compare_exchange_weak` can spuriously fail (return false even when the comparison would succeed) on LL/SC architectures (ARM, POWER). Use it in loops. `compare_exchange_strong` never spuriously fails but may be slower on those architectures.

### SPSC Lock-Free Queue

The single-producer, single-consumer (SPSC) queue is the workhorse of real-time audio. It connects the audio thread to a processing or I/O thread without any locks.

```cpp
// Production-grade SPSC queue.
// - Fixed capacity (power of 2 for fast modulo via bitmask).
// - Cache-line aligned head and tail to prevent false sharing.
// - Uses acquire/release ordering for correct publication.

#include <atomic>
#include <array>
#include <cstddef>
#include <new>      // std::hardware_destructive_interference_size
#include <optional>

// Portable cache line size (C++17 may not define it on all platforms)
#ifdef __cpp_lib_hardware_interference_size
    constexpr size_t kCacheLineSize = std::hardware_destructive_interference_size;
#else
    constexpr size_t kCacheLineSize = 64;
#endif

template<typename T, size_t Capacity>
class SPSCQueue {
    static_assert((Capacity & (Capacity - 1)) == 0,
                  "Capacity must be a power of 2");
    static_assert(std::is_trivially_copyable_v<T>,
                  "T must be trivially copyable for real-time safety");

    static constexpr size_t kMask = Capacity - 1;

    // Each index on its own cache line to prevent false sharing.
    // The producer writes head_, the consumer writes tail_.
    // Without alignment, both could share a cache line, causing
    // the MESI protocol to bounce the line between cores at
    // every read/write -- destroying performance.
    alignas(kCacheLineSize) std::atomic<size_t> head_{0};
    alignas(kCacheLineSize) std::atomic<size_t> tail_{0};
    alignas(kCacheLineSize) std::array<T, Capacity> buffer_;

public:
    // Called from producer thread only.
    bool tryPush(const T& item) {
        const size_t head = head_.load(std::memory_order_relaxed);
        const size_t tail = tail_.load(std::memory_order_acquire);

        if (head - tail >= Capacity) {
            return false;  // queue full
        }

        buffer_[head & kMask] = item;

        // Release: ensures the buffer write above is visible
        // before the consumer sees the updated head.
        head_.store(head + 1, std::memory_order_release);
        return true;
    }

    // Called from consumer thread only.
    std::optional<T> tryPop() {
        const size_t tail = tail_.load(std::memory_order_relaxed);
        const size_t head = head_.load(std::memory_order_acquire);

        if (tail >= head) {
            return std::nullopt;  // queue empty
        }

        T item = buffer_[tail & kMask];

        // Release: ensures the buffer read above completes
        // before the producer sees the freed slot.
        tail_.store(tail + 1, std::memory_order_release);
        return item;
    }

    size_t sizeApprox() const {
        return head_.load(std::memory_order_relaxed)
             - tail_.load(std::memory_order_relaxed);
    }

    bool empty() const { return sizeApprox() == 0; }
};
```

**Usage pattern for audio parameter changes:**

```cpp
struct ParameterChange {
    uint32_t paramId;
    float value;
};

SPSCQueue<ParameterChange, 256> paramQueue_;

// GUI thread (producer):
void onSliderMoved(uint32_t id, float val) {
    paramQueue_.tryPush({id, val});  // non-blocking, returns false if full
}

// Audio thread (consumer):
void audioCallback(float* output, int numFrames) {
    // Drain all pending parameter changes before processing
    while (auto change = paramQueue_.tryPop()) {
        applyParameter(change->paramId, change->value);
    }
    processAudio(output, numFrames);
}
```

### Lock-Free Stack and Pool

A lock-free object pool is essential for managing variable-size messages or events in the audio path without heap allocation.

```cpp
// Treiber stack: lock-free LIFO using CAS.
// Used as the free-list for an object pool.
template<typename T>
class LockFreeStack {
    struct Node {
        T data;
        Node* next;
    };

    std::atomic<Node*> head_{nullptr};

public:
    void push(Node* node) {
        node->next = head_.load(std::memory_order_relaxed);
        while (!head_.compare_exchange_weak(
            node->next, node,
            std::memory_order_release,
            std::memory_order_relaxed
        )) {}
    }

    Node* pop() {
        Node* oldHead = head_.load(std::memory_order_acquire);
        while (oldHead && !head_.compare_exchange_weak(
            oldHead, oldHead->next,
            std::memory_order_acq_rel,
            std::memory_order_acquire
        )) {}
        return oldHead;  // nullptr if empty
    }
};
```

### The ABA Problem

The ABA problem occurs when a CAS operation succeeds incorrectly:

1. Thread A reads head = X.
2. Thread A is preempted.
3. Thread B pops X, pops Y, pushes X back.
4. Thread A resumes. CAS sees head == X (same pointer), succeeds.
5. But head->next is now wrong -- Y was freed or reused.

**Solutions:**

- **Tagged pointers:** Pack a monotonically increasing counter into the upper bits of the pointer (possible on x86-64 where only 48 bits are used for addresses). Use a 128-bit CAS (`cmpxchg16b` on x86-64) or pack counter + pointer into 64 bits.
- **Hazard pointers:** Each thread publishes which nodes it's currently accessing. Other threads defer reclamation until no hazard pointer references the node.
- **Epoch-based reclamation:** Threads announce entry/exit from critical sections. Reclamation is deferred until all threads have passed through a quiescent state.

For audio applications, the simplest solution is to use pool-based allocation where nodes are never freed back to the OS -- they are only returned to the pool's free list. Since the pool owns all nodes for the lifetime of the application, the ABA problem is mitigated: even if a node is recycled, it remains a valid pool entry.

### False Sharing and Cache Line Alignment

False sharing occurs when two variables accessed by different threads reside on the same cache line (typically 64 bytes on x86, 128 bytes on Apple Silicon M-series). The hardware cache coherency protocol (MESI/MOESI) forces the entire line to bounce between cores on each write, even though the threads are accessing different variables.

```cpp
// BROKEN: producer and consumer indices on same cache line
struct BadQueue {
    std::atomic<size_t> head;  // offset 0
    std::atomic<size_t> tail;  // offset 8 -- SAME CACHE LINE
    // Every write to head invalidates tail's cache line on the other core.
    // Measured penalty: 20-80 ns per access instead of 1-2 ns.
};

// FIXED: padding or alignas to separate cache lines
struct GoodQueue {
    alignas(64) std::atomic<size_t> head;
    alignas(64) std::atomic<size_t> tail;
};
```

**Apple Silicon note:** M1/M2/M3 chips have 128-byte cache lines in the performance cores. Use `alignas(128)` or query at runtime when targeting macOS on ARM.

---

## 3. Memory Management

### Pre-Allocated Buffer Pools

```cpp
// Real-time-safe memory pool.
// Allocates a contiguous block at construction time.
// Hands out fixed-size slots from a lock-free free list.
// No system calls, no locks, no fragmentation.

template<typename T, size_t PoolSize>
class RTMemoryPool {
    struct Slot {
        alignas(T) std::byte storage[sizeof(T)];
        std::atomic<Slot*> next;
    };

    std::unique_ptr<Slot[]> slots_;
    std::atomic<Slot*> freeList_{nullptr};

public:
    RTMemoryPool() : slots_(std::make_unique<Slot[]>(PoolSize)) {
        // Build the free list. This is the ONLY allocation.
        for (size_t i = 0; i < PoolSize; ++i) {
            slots_[i].next.store(
                (i + 1 < PoolSize) ? &slots_[i + 1] : nullptr,
                std::memory_order_relaxed
            );
        }
        freeList_.store(&slots_[0], std::memory_order_release);
    }

    // O(1), lock-free, real-time safe
    T* allocate() {
        Slot* slot = popFreeList();
        if (!slot) return nullptr;  // pool exhausted
        return new (slot->storage) T{};  // placement new -- no heap alloc
    }

    // O(1), lock-free, real-time safe
    void deallocate(T* ptr) {
        ptr->~T();
        auto* slot = reinterpret_cast<Slot*>(ptr);
        pushFreeList(slot);
    }

private:
    Slot* popFreeList() {
        Slot* head = freeList_.load(std::memory_order_acquire);
        while (head && !freeList_.compare_exchange_weak(
            head, head->next.load(std::memory_order_relaxed),
            std::memory_order_acq_rel, std::memory_order_acquire
        )) {}
        return head;
    }

    void pushFreeList(Slot* slot) {
        slot->next.store(freeList_.load(std::memory_order_relaxed),
                         std::memory_order_relaxed);
        while (!freeList_.compare_exchange_weak(
            slot->next, slot,
            std::memory_order_release, std::memory_order_relaxed
        )) {}
    }
};
```

### Memory-Mapped I/O for Large Buffers

For loading large audio files or lookup tables, `mmap` avoids explicit `read` calls and lets the kernel page in data on demand. **However**, page faults are system calls, so all pages must be faulted in before the audio stream starts.

```cpp
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>

class MappedBuffer {
    void* data_ = nullptr;
    size_t size_ = 0;
    int fd_ = -1;

public:
    bool open(const char* path) {
        fd_ = ::open(path, O_RDONLY);
        if (fd_ < 0) return false;

        size_ = lseek(fd_, 0, SEEK_END);
        data_ = mmap(nullptr, size_, PROT_READ, MAP_PRIVATE, fd_, 0);
        if (data_ == MAP_FAILED) { ::close(fd_); return false; }

        // Pre-fault all pages NOW -- before audio starts.
        // madvise WILLNEED asks the kernel to read-ahead.
        madvise(data_, size_, MADV_WILLNEED);

        // Force-fault by touching every page (4096 bytes apart).
        volatile char* p = static_cast<volatile char*>(data_);
        for (size_t i = 0; i < size_; i += 4096) {
            (void)p[i];
        }

        return true;
    }

    const float* asFloat() const {
        return static_cast<const float*>(data_);
    }

    ~MappedBuffer() {
        if (data_ && data_ != MAP_FAILED) munmap(data_, size_);
        if (fd_ >= 0) ::close(fd_);
    }
};
```

### Stack Allocation for Temporary Buffers

For small temporary buffers within the audio callback, stack allocation is free (just a stack pointer adjustment).

```cpp
void audioCallback(float* output, int numFrames) {
    // Stack allocation: zero cost, automatically freed on return.
    // Safe for small buffers (< ~8 KB to avoid stack overflow).
    float temp[512];  // known maximum buffer size
    assert(numFrames <= 512);

    computeEnvelope(temp, numFrames);
    applyEnvelope(output, temp, numFrames);
}
```

For larger dynamic sizes, use `alloca` with extreme caution (no bounds checking, no destructor calls) or better yet, use pre-allocated member buffers.

### SIMD Alignment

DSP operations (FFT, FIR filters, vector math) benefit enormously from SIMD instructions (SSE, AVX, NEON), which require aligned memory.

```cpp
// Ensure 32-byte alignment for AVX operations
struct alignas(32) AudioBlock {
    float samples[256];
};

// For dynamically allocated aligned memory:
// C++17: std::aligned_alloc(32, sizeof(AudioBlock))
// or use alignas on the pool's storage
template<size_t Alignment, size_t Size>
class AlignedBuffer {
    alignas(Alignment) float data_[Size];
public:
    float* get() { return data_; }
    static constexpr size_t size() { return Size; }
};

// Usage with NEON intrinsics (Apple Silicon):
#if defined(__ARM_NEON)
#include <arm_neon.h>
void applyGain(float* __restrict output,
               const float* __restrict input,
               float gain, int numFrames) {
    float32x4_t gainVec = vdupq_n_f32(gain);
    for (int i = 0; i < numFrames; i += 4) {
        float32x4_t in = vld1q_f32(input + i);   // aligned load
        float32x4_t out = vmulq_f32(in, gainVec);
        vst1q_f32(output + i, out);               // aligned store
    }
}
#endif
```

---

## 4. Thread Priority and Scheduling

The audio callback thread must preempt all non-real-time threads. If a background task (GC, indexing, virus scanner) preempts the audio thread for even 3 ms, you get a dropout. OS-level thread priority configuration is not optional -- it is a hard requirement.

### Linux: SCHED_FIFO and mlockall

```cpp
#include <pthread.h>
#include <sched.h>
#include <sys/mman.h>

bool setRealtimePriority(pthread_t thread, int priority = 80) {
    struct sched_param param;
    param.sched_priority = priority;  // 1-99, higher = more priority

    // SCHED_FIFO: real-time FIFO scheduling.
    // The thread runs until it voluntarily yields or a higher-priority
    // SCHED_FIFO thread becomes runnable. It is NEVER preempted by
    // SCHED_OTHER (normal) threads.
    int ret = pthread_setschedparam(thread, SCHED_FIFO, &param);
    if (ret != 0) {
        // Requires CAP_SYS_NICE or root. On modern systems,
        // set /etc/security/limits.conf:
        //   @audio  -  rtprio  95
        //   @audio  -  memlock unlimited
        return false;
    }

    // Lock all current and future pages in RAM.
    // Prevents page faults (which are system calls) during audio processing.
    // Without this, the first access to a new page triggers a fault
    // that can take 10-100 us.
    if (mlockall(MCL_CURRENT | MCL_FUTURE) != 0) {
        // Requires CAP_IPC_LOCK or memlock ulimit
        return false;
    }

    return true;
}
```

`SCHED_RR` is similar to `SCHED_FIFO` but adds a time quantum: if two threads have the same priority, they round-robin at quantum boundaries (typically 100 ms). For audio, `SCHED_FIFO` is preferred because we want the audio thread to run to completion without involuntary preemption.

### macOS: THREAD_TIME_CONSTRAINT_POLICY

CoreAudio sets real-time priority on its own callback thread, but if you spawn additional processing threads, you must set the policy manually.

```cpp
#include <mach/mach.h>
#include <mach/thread_policy.h>
#include <mach/mach_time.h>

bool setRealtimePriorityMacOS() {
    mach_timebase_info_data_t timebase;
    mach_timebase_info(&timebase);

    // Convert nanoseconds to Mach absolute time units
    auto nsToAbs = [&](uint64_t ns) -> uint32_t {
        return static_cast<uint32_t>(ns * timebase.denom / timebase.numer);
    };

    // At 48kHz, 128 samples = 2.667 ms
    // period: how often the thread needs to run
    // computation: worst-case time the thread needs
    // constraint: hard deadline (must finish by this time after becoming runnable)
    thread_time_constraint_policy_data_t policy;
    policy.period      = nsToAbs(2'666'667);   // 2.667 ms in Mach units
    policy.computation = nsToAbs(1'500'000);   // 1.5 ms budget
    policy.constraint  = nsToAbs(2'666'667);   // hard deadline = period
    policy.preemptible = true;                  // can be preempted by higher-priority RT

    kern_return_t kr = thread_policy_set(
        mach_thread_self(),
        THREAD_TIME_CONSTRAINT_POLICY,
        reinterpret_cast<thread_policy_t>(&policy),
        THREAD_TIME_CONSTRAINT_POLICY_COUNT
    );

    return kr == KERN_SUCCESS;
}
```

### Windows: MMCSS and SetThreadPriority

```cpp
#include <windows.h>
#include <avrt.h>    // Link with avrt.lib
#pragma comment(lib, "avrt.lib")

bool setRealtimePriorityWindows() {
    // MMCSS (Multimedia Class Scheduler Service) is the correct way on Windows.
    // It elevates the thread's priority and guarantees a percentage of CPU time.
    DWORD taskIndex = 0;
    HANDLE hTask = AvSetMmThreadCharacteristicsW(L"Pro Audio", &taskIndex);
    if (hTask == NULL) {
        // Fallback: raw priority (less effective, no CPU guarantee)
        SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);
        return false;
    }

    // Request maximum priority within the MMCSS task
    AvSetMmThreadPriority(hTask, AVRT_PRIORITY_CRITICAL);

    // Optional: set processor affinity to keep the thread on one core
    // (improves cache locality, avoids cross-core migration overhead)
    SetThreadAffinityMask(GetCurrentThread(), 1 << 0);  // pin to core 0

    return true;

    // On shutdown: AvRevertMmThreadCharacteristics(hTask);
}
```

**MMCSS guarantees:** By default, MMCSS reserves 80% of CPU for the registered thread (configurable in registry under `HKLM\SOFTWARE\Microsoft\Windows NT\CurrentVersion\Multimedia\SystemProfile`). Non-MMCSS threads share the remaining 20%.

---

## 5. Priority Inversion

### What It Is

Priority inversion occurs when a high-priority thread (audio) is blocked waiting for a resource held by a low-priority thread, and a medium-priority thread preempts the low-priority thread, effectively causing the high-priority thread to wait behind the medium-priority thread. This is an inversion of the intended scheduling order.

**Concrete scenario:**

1. Low-priority thread L acquires mutex M (e.g., protecting a parameter struct).
2. Audio thread H attempts to lock M, blocks.
3. Medium-priority thread M (GUI rendering, network I/O) preempts L because M > L.
4. Thread H (highest priority) is now blocked behind thread M (medium priority). This is the inversion.
5. Thread M can run for an unbounded time (e.g., rendering a complex UI), keeping H blocked.

This is how the Mars Pathfinder mission nearly failed in 1997: a priority inversion between the bus management task and a low-priority meteorological task caused the system watchdog to fire repeatedly.

### Priority Inheritance Protocol

When a high-priority thread blocks on a mutex held by a low-priority thread, the OS temporarily **boosts** the low-priority thread's priority to match the blocked thread. This prevents medium-priority threads from preempting the lock holder.

```cpp
// On Linux: PTHREAD_PRIO_INHERIT
pthread_mutexattr_t attr;
pthread_mutexattr_init(&attr);
pthread_mutexattr_setprotocol(&attr, PTHREAD_PRIO_INHERIT);

pthread_mutex_t mutex;
pthread_mutex_init(&mutex, &attr);
// Now if a SCHED_FIFO thread blocks on this mutex, the holder's
// priority is temporarily raised.
```

**Limitation:** Priority inheritance only helps if you use mutexes, but the first golden rule says never use mutexes in the audio thread. Priority inheritance is a mitigation, not a solution. The correct approach for audio is to eliminate shared mutable state between threads and use lock-free communication exclusively.

### Priority Ceiling Protocol

A stronger alternative: each mutex is assigned a "ceiling priority" equal to the highest priority of any thread that might lock it. Any thread that acquires the mutex immediately has its priority raised to the ceiling. This prevents priority inversion entirely but requires knowing all threads' priorities at design time.

---

## 6. Testing for Real-Time Safety

### Detecting Xruns Programmatically

An xrun (overrun or underrun) occurs when the audio callback fails to deliver data in time. Most audio APIs report this via callbacks.

```cpp
// JACK xrun callback
int xrunCallback(void* arg) {
    auto* stats = static_cast<AudioStats*>(arg);
    stats->xrunCount.fetch_add(1, std::memory_order_relaxed);
    // DO NOT log here -- this is called from the JACK server thread.
    // Log from a separate monitoring thread that polls xrunCount.
    return 0;
}

// CoreAudio: IOProc overload detection
// Check the AudioTimeStamp flags in the render callback:
OSStatus renderCallback(
    void* inRefCon,
    AudioUnitRenderActionFlags* ioActionFlags,
    const AudioTimeStamp* inTimeStamp,
    UInt32 inBusNumber,
    UInt32 inNumberFrames,
    AudioBufferList* ioData)
{
    // If this flag is set, we missed our deadline last time
    if (*ioActionFlags & kAudioUnitRenderAction_OutputIsSilence) {
        // xrun detected
    }

    // Self-monitoring: measure callback duration
    uint64_t start = mach_absolute_time();
    processAudio(ioData, inNumberFrames);
    uint64_t elapsed = mach_absolute_time() - start;

    auto* stats = static_cast<AudioStats*>(inRefCon);
    // Store worst-case time for monitoring thread to read
    uint64_t prev = stats->worstCaseNs.load(std::memory_order_relaxed);
    if (elapsed > prev) {
        stats->worstCaseNs.store(elapsed, std::memory_order_relaxed);
    }

    return noErr;
}
```

### ASAN and TSAN

**AddressSanitizer (ASAN)** detects use-after-free, buffer overflows, and heap corruption. Since these bugs in audio code often cause intermittent glitches rather than immediate crashes, ASAN is invaluable.

**ThreadSanitizer (TSAN)** detects data races. A data race on a non-atomic variable accessed from both the audio thread and GUI thread is undefined behavior -- it may "work" on x86 (strong memory model) but crash on ARM.

```bash
# Build with ASAN + TSAN (cannot use both simultaneously):
# ASAN:
clang++ -fsanitize=address -fno-omit-frame-pointer -g -O1 ...
# TSAN:
clang++ -fsanitize=thread -g -O1 ...
```

**Important caveat:** Sanitizers add significant overhead (ASAN: 2x, TSAN: 5-15x). Audio callbacks will miss deadlines under sanitizers. This is expected. Run sanitizer builds for correctness testing, not performance testing. Use a large buffer size (2048+ samples) to give the callback enough time.

### RealtimeSanitizer (RTSan)

RTSan (available in Clang 18+) specifically targets real-time violations. It detects calls to `malloc`, `free`, `pthread_mutex_lock`, system calls, and other non-RT-safe operations from threads marked as real-time.

```cpp
// Mark a function as real-time-critical.
// RTSan will flag any non-RT-safe call within this function or
// anything it transitively calls.
[[clang::nonblocking]]
void audioCallback(float* output, int numFrames) {
    // Any malloc, mutex, syscall, or exception here will be flagged
    processAudio(output, numFrames);
}
```

```bash
# Build with RTSan:
clang++ -fsanitize=realtime -g -O1 ...

# RTSan output example:
# ==12345==ERROR: RealtimeSanitizer: unsafe-library-call
# Call to malloc in real-time context
#     #0 malloc /usr/lib/libc.so
#     #1 std::vector<float>::push_back(float&&) ...
#     #2 AudioProcessor::audioCallback(float*, int) main.cpp:42
```

### Stress Testing

```cpp
// Stress test: run the audio callback under adverse conditions
// to find latent real-time violations.
void stressTestAudioCallback(AudioProcessor& proc, int iterations) {
    const int bufferSize = 128;
    std::vector<float> buffer(bufferSize);

    // Spawn threads that compete for resources
    std::atomic<bool> running{true};
    std::vector<std::thread> stressors;

    // Memory pressure: force malloc contention
    stressors.emplace_back([&] {
        while (running.load(std::memory_order_relaxed)) {
            std::vector<std::vector<char>> allocs;
            for (int i = 0; i < 100; ++i) {
                allocs.emplace_back(1024 * 1024);  // 1 MB each
            }
        }
    });

    // CPU pressure: saturate all cores
    for (unsigned i = 0; i < std::thread::hardware_concurrency(); ++i) {
        stressors.emplace_back([&] {
            volatile double x = 1.0;
            while (running.load(std::memory_order_relaxed)) {
                x = std::sin(x) + std::cos(x);
            }
        });
    }

    // Measure callback durations under stress
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < iterations; ++i) {
        auto cbStart = std::chrono::high_resolution_clock::now();
        proc.audioCallback(buffer.data(), bufferSize);
        auto cbEnd = std::chrono::high_resolution_clock::now();

        auto us = std::chrono::duration_cast<std::chrono::microseconds>(
            cbEnd - cbStart).count();

        // At 48kHz/128 samples, deadline is 2667 us
        if (us > 2000) {
            // Near-miss: callback took >75% of deadline under stress
            // Log to lock-free queue for offline analysis
        }
    }

    running.store(false, std::memory_order_relaxed);
    for (auto& t : stressors) t.join();
}
```

### Lock-Free Logging from the Audio Thread

```cpp
// Safe logging: audio thread writes to a lock-free queue,
// a dedicated logger thread drains it and writes to disk.

struct LogEntry {
    uint64_t timestamp;    // mach_absolute_time() or rdtsc
    uint32_t eventType;    // enum: XRUN, PEAK_CLIP, BUFFER_UNDERFLOW, etc.
    float    value;        // associated data
};

SPSCQueue<LogEntry, 4096> logQueue_;

// Audio thread: enqueue, never block
void logFromAudioThread(uint32_t event, float value) {
    LogEntry entry;
    entry.timestamp = __rdtsc();  // no syscall on x86
    entry.eventType = event;
    entry.value = value;
    logQueue_.tryPush(entry);  // drop if full -- acceptable tradeoff
}

// Logger thread: drain and write to disk
void loggerThread() {
    FILE* f = fopen("audio_log.bin", "wb");
    while (running_) {
        while (auto entry = logQueue_.tryPop()) {
            fwrite(&*entry, sizeof(LogEntry), 1, f);
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    fclose(f);
}
```

---

## 7. Common Mistakes Catalog

### Mistake 1: Using std::function in the Audio Path

```cpp
// BROKEN:
std::function<float(float)> processSample_;  // may heap-allocate

// FIX: Use function pointers or templates
using ProcessFn = float(*)(float);
ProcessFn processSample_;

// Or: template the processor class
template<typename ProcessFn>
class AudioEngine {
    ProcessFn process_;
    void audioCallback(float* out, int n) {
        for (int i = 0; i < n; ++i) out[i] = process_(out[i]);
    }
};
```

### Mistake 2: String Formatting for Debug Output

```cpp
// BROKEN: std::string construction + printf in callback
void audioCallback(float* output, int numFrames) {
    if (peak > 0.99f) {
        std::string msg = "CLIP at frame " + std::to_string(frameCount_);
        printf("%s\n", msg.c_str());  // malloc + syscall
    }
}

// FIX: Enqueue a fixed-size event to the lock-free log queue
void audioCallback(float* output, int numFrames) {
    if (peak > 0.99f) {
        logFromAudioThread(EVENT_CLIP, peak);
    }
}
```

### Mistake 3: Calling Objective-C Methods on macOS

```cpp
// BROKEN: Objective-C message dispatch in audio callback
// objc_msgSend may acquire the Objective-C runtime lock,
// trigger autorelease pool operations, or call into CoreFoundation.
void audioCallback(float* output, int numFrames) {
    NSNumber* level = @(computeLevel());  // Obj-C boxing: allocates
    [delegate updateLevel:level];          // message dispatch: may lock
}

// FIX: Use C/C++ only in the audio callback. Communicate with Obj-C
// via atomics or lock-free queues.
```

### Mistake 4: Using std::shared_ptr Across Threads

```cpp
// BROKEN: shared_ptr reference counting uses atomic operations,
// and destruction may call delete (free → heap lock).
std::shared_ptr<FilterState> currentFilter_;

void audioCallback(float* output, int numFrames) {
    auto filter = currentFilter_;  // atomic ref count increment
    // ... use filter ...
    // destructor: atomic ref count decrement, may trigger delete
}

// FIX: Use raw pointers with explicit lifetime management via
// lock-free exchange and deferred deletion.
std::atomic<FilterState*> currentFilter_;
SPSCQueue<FilterState*, 64> recycleBin_;
```

### Mistake 5: Unbounded Loops

```cpp
// BROKEN: processing queue with unbounded item count
void audioCallback(float* output, int numFrames) {
    while (auto msg = messageQueue_.tryPop()) {
        processMessage(*msg);  // if 10,000 messages queued, deadline missed
    }
}

// FIX: Cap processing per callback invocation
void audioCallback(float* output, int numFrames) {
    constexpr int kMaxMessagesPerCallback = 16;
    for (int i = 0; i < kMaxMessagesPerCallback; ++i) {
        auto msg = messageQueue_.tryPop();
        if (!msg) break;
        processMessage(*msg);
    }
}
```

### Mistake 6: Accessing the File System for Presets/Tables

```cpp
// BROKEN: loading a wavetable on a note-on event
void onNoteOn(int note) {
    // fopen + fread: system calls with unbounded latency
    wavetable_ = loadWavetable("wave_" + std::to_string(note) + ".wav");
}

// FIX: Pre-load all wavetables at startup, index by note number
void init() {
    for (int i = 0; i < 128; ++i) {
        wavetables_[i] = loadWavetable("wave_" + std::to_string(i) + ".wav");
    }
}
void onNoteOn(int note) {
    currentWavetable_ = wavetables_[note].data();  // pointer assignment only
}
```

### Mistake 7: Forgetting to Pin Memory (mlockall)

Without `mlockall` on Linux or equivalent on macOS, the first access to a memory page that has been swapped out triggers a **major page fault**: the kernel must read the page from disk, which can take 1-10 ms on SSD, 5-20 ms on HDD. This exceeds most audio deadlines.

### Mistake 8: Using condition_variable for Audio-to-UI Communication

```cpp
// BROKEN: condition_variable uses a mutex internally
std::condition_variable cv_;
std::mutex mtx_;

void audioCallback(float* output, int numFrames) {
    computeSpectrum(output, numFrames, spectrumData_);
    cv_.notify_one();  // acquires mutex internally on some implementations
}

// FIX: Use an atomic flag and let the UI poll
std::atomic<bool> spectrumReady_{false};

void audioCallback(float* output, int numFrames) {
    computeSpectrum(output, numFrames, spectrumData_);
    spectrumReady_.store(true, std::memory_order_release);
}

// UI thread (runs at 60 fps -- polling is fine):
void updateUI() {
    if (spectrumReady_.exchange(false, std::memory_order_acquire)) {
        drawSpectrum(spectrumData_);
    }
}
```

### Mistake 9: Virtual Function Calls with Unpredictable Dispatch

Virtual dispatch itself is fast (indirect call through vtable), but it can cause **instruction cache misses** if the concrete type varies per call, causing the branch predictor to mispredict. In tight sample-processing loops, this matters.

```cpp
// PROBLEMATIC: virtual dispatch per sample
for (int i = 0; i < numFrames; ++i) {
    output[i] = effect_->process(input[i]);  // vtable lookup per sample
}

// BETTER: dispatch once, process block
effect_->processBlock(output, input, numFrames);  // one vtable lookup
```

### Mistake 10: Ignoring Denormalized Floats

Denormalized (subnormal) floating-point numbers near zero are handled by a microcode trap on most CPUs, costing 50-200x more than a normal floating-point operation. IIR filter feedback paths naturally produce denormals as signals decay toward zero.

```cpp
// FIX: Flush denormals to zero at the start of the audio callback
#include <xmmintrin.h>  // x86
void audioCallback(float* output, int numFrames) {
    // Set DAZ (Denormals Are Zero) and FTZ (Flush To Zero) bits
    _mm_setcsr(_mm_getcsr() | 0x8040);

    processAudio(output, numFrames);
}

// On ARM/NEON (Apple Silicon): denormals are flushed by default
// when using NEON instructions. For scalar code:
#if defined(__ARM_NEON)
    // FZ bit in FPCR
    uint64_t fpcr;
    asm volatile("mrs %0, fpcr" : "=r"(fpcr));
    fpcr |= (1 << 24);  // FZ bit
    asm volatile("msr fpcr, %0" : : "r"(fpcr));
#endif
```

---

## Summary

Real-time audio programming is deterministic systems programming. The audio callback is a hard real-time context where worst-case execution time, not average, determines correctness. Every rule in this document exists because violating it introduces **unbounded latency** -- a condition that manifests as audible artifacts ranging from clicks and pops to full silence.

The defense layers are:

1. **Elimination:** Remove all allocations, locks, and system calls from the audio path.
2. **Isolation:** Use lock-free queues to communicate between real-time and non-real-time threads.
3. **Pre-allocation:** Allocate all resources before the audio stream starts.
4. **Prioritization:** Configure OS-level thread priority to prevent preemption.
5. **Verification:** Use RTSan, TSAN, stress tests, and xrun monitoring to catch violations.

Apply these principles consistently and the audio path becomes a predictable, bounded computation that never misses a deadline.

---

*Cross-references:*
- [ARCH_pipeline.md](ARCH_pipeline.md) -- Audio processing pipeline architecture
- [ARCH_audio_io.md](ARCH_audio_io.md) -- Platform-specific audio I/O layer
- [REF_latency_numbers.md](REF_latency_numbers.md) -- Measured latency data for operations
- [IMPL_testing_validation.md](IMPL_testing_validation.md) -- Test harnesses and validation procedures
