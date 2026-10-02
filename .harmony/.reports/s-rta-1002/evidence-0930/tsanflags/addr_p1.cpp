#include <thread>
#include <atomic>
#include <cstdio>
#include <unistd.h>
struct S { int a = 0; int pad[15]; };
S* s = new S;
__attribute__((noinline)) void w1() { s->a = 1; }
__attribute__((noinline)) void w2() { s->pad[1] = 2; }
__attribute__((noinline)) int r1() { return s->a; }
__attribute__((noinline)) int r2() { return s->pad[1]; }
int main() {
  std::thread t1([]{ w1(); });
  usleep(20000);
  volatile int v = r1();       // race pair 1: w1 (T1) vs r1 (main)
  t1.join();
  std::thread t2([]{ usleep(20000); w2(); });   // race pair 2: r2 (main, earlier) vs w2 (T2) -- different stacks, same address
  // make main's read unordered with t2's write: t2 created after, so creation orders main->t2.
  // Use a different thread for the read instead:
  std::thread t3([]{ volatile int q = r2(); (void)q; });
  t2.join(); t3.join();
  (void)v;
  printf("done\n");
}
