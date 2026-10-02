#include <thread>
#include <cstdio>
#include <unistd.h>
struct S { int a = 0; int pad[15]; };
S* s = new S;
__attribute__((noinline)) void w1() { s->a = 1; }
__attribute__((noinline)) void w2() { s->a = 2; }
__attribute__((noinline)) int r1() { return s->a; }
__attribute__((noinline)) int r2() { return s->a; }
__attribute__((noinline)) void touch(int k) { for (int i = 0; i < k; ++i) { s->a = i; asm volatile("" ::: "memory"); } }
int main(int argc, char**) {
  std::thread t1([]{ w1(); });
  usleep(20000);
  volatile int v = r1();               // pair 1
  t1.join();
  if (argc > 1) touch(16);            // ordinary (ordered) accesses by main after the report
  std::thread t2([]{ usleep(20000); w2(); });
  std::thread t3([]{ volatile int q = r2(); (void)q; });   // pair 2: T3 read vs T2 write, unordered
  t2.join(); t3.join();
  (void)v; printf("done\n");
}
