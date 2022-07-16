#include <chrono>

#include "api/mutex.hpp"
#include "stdio.h"

co::Mutex mtx;

int value = 0;

static co::Asyncf<void> foo(int id, int n) {
  printf("thread(%d) foo(%d) start\n", std::this_thread::get_id(), id);
  for (int i = 0; i < n; i++) {
    co_await mtx.lock();
    // printf("thread(%d) foo(%d)->%d locked\n", std::this_thread::get_id(), id,
    //        i);

    value++;
    mtx.unlock();
    // printf("thread(%d) foo(%d)->%d unlock\n", std::this_thread::get_id(), id,
    //        i);
  }
  printf("thread(%d) foo(%d) end\n", std::this_thread::get_id(), id);
}

void test_mutex() {
  for (int i = 0; i < 500; i++) {
    co::go(foo(i, 100));
  }
  co::event_loop(3);
  printf("value=%d\n", value);
}