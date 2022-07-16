#include <chrono>
#include <iostream>

#include "api/mutex.hpp"
#include "stdio.h"

co::Mutex mtx;

int value = 0;

static co::Asyncf<void> foo(int id, int n) {
  printf("thread(%d) foo(%d) start\n", std::this_thread::get_id(), id);
  for (int i = 0; i < n; i++) {
    printf("thread(%d) foo(%d)->%d\n", std::this_thread::get_id(), id, i);
    co_await mtx.lock();
    // std::this_thread::sleep_for(std::chrono::milliseconds(100));
    value++;
    mtx.unlock();
  }
  printf("thread(%d) foo(%d) end\n", std::this_thread::get_id(), id);
}

void test_mutex() {
  auto start = std::chrono::high_resolution_clock::now();
  co::go(foo(1, 100));
  co::go(foo(2, 50));
  co::go(foo(3, 30));
  co::event_loop(2);
  printf("value=%d\n", value);
  auto end = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double, std::milli> elapsed = end - start;
  std::cout << "Waited " << elapsed.count() / 1000 << " s\n";
}