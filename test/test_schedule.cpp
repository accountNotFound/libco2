#include "cofunc/asyncf.hpp"
#include "schedule/schedule.hpp"
#include "stdio.h"

static co::Asyncf<int> bar(int n) {
  printf("  thread(%d), bar start(%d)\n", std::this_thread::get_id(), n);
  int res = 0;
  for (int i = 0; i < n; i++) {
    co_await std::suspend_always{};
    res += i + 1;
  }
  printf("  thread(%d), bar end(%d)\n", std::this_thread::get_id(), res);
  co_return res;
}

static co::Asyncf<std::string> foo(int id, int n) {
  printf("thread(%d), foo(%d) start(%d)\n", std::this_thread::get_id(), id, n);
  std::string res = "";
  for (int i = 0; i < n; i++) {
    res += std::to_string(co_await bar(i)) + " ";
    if (i % 2 == 0) co::go(bar(i / 2));
  }
  printf("thread(%d), foo(%d) end \"%s\"\n", std::this_thread::get_id(), id,
         res.data());
  co_return res;
}

void test_schedule() {
  co::go(foo(1, 10));
  co::go(foo(2, 5));
  co::go(foo(3, 20));
  co::event_loop(2);
}