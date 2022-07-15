#include "api/timer.hpp"
#include "stdio.h"

static co::Asyncf<void> foo(int id, int n, int milisecond) {
  float time = 0;
  printf("thread(%d) foo(%d) start\n", std::this_thread::get_id(), id);
  for (int i = 0; i < n; i++) {
    co_await co::sleep(milisecond);
    time += (float)milisecond / 1000;
    printf("thread (%d) foo(%d)->%d at %f sec\n", std::this_thread::get_id(),
           id, i, time);
  }
}

void test_sleep() {
  co::go(foo(1, 10, 1000));
  co::go(foo(2, 5, 3000));
  co::go(foo(3, 30, 200));
  co::event_loop(4);
}