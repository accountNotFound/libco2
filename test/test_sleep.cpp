#include "api/timer.hpp"
#include "stdio.h"

static co::Asyncf<void> foo(int id, int n, int milisecond) {
  float time = 0;
  printf("thread(%d) foo(%d) start\n", std::this_thread::get_id(), id);
  for (int i = 0; i < n; i++) {
    co_await co::sleep(milisecond);
    time += (float)milisecond / 1000;
    // printf("thread (%d) foo(%d)->%d at %f sec\n", std::this_thread::get_id(),
    //        id, i, time);
  }
  printf("thread(%d) foo(%d) end\n", std::this_thread::get_id(), id);
}

void test_sleep() {
  for (int i = 0; i < 100; i++) {
    co::go(foo(i, 10, 500));
  }
  co::event_loop(6);
}