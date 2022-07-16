#include "api/mutex.hpp"
#include "stdio.h"

co::Mutex mtx;

int value = 0;

static co::Asyncf<void> foo(int id, int n) {
  printf("foo(%d) start\n", id);
  for (int i = 0; i < n; i++) {
    printf("foo(%d)->%d\n", id, i);
    co_await mtx.lock();
    value++;
    mtx.unlock();
  }
  printf("foo(%d) end\n", id);
}

void test_mutex() {
  co::go(foo(1, 100));
  co::go(foo(2, 50));
  co::go(foo(3, 30));
  co::event_loop(2);
  printf("value=%d\n", value);
}