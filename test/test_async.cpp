#include <string>

#include "cofunc/asyncf.hpp"
#include "stdio.h"

co::Asyncf<int> bar(int n) {
  int res = 0;
  for (int i = 0; i < n; i++) {
    printf("   bar %d\n", i);
    co_await std::suspend_always{};
    res += i + 1;
  }
  co_return res;
}

co::Asyncf<std::string> foo(int n) {
  std::string res = "";
  for (int i = 0; i < n; i++) {
    printf("  foo %d\n", i);
    res += std::to_string(co_await bar(i)) + " ";
  }
  co_return res;
}

co::Asyncf<void> biz() {
  printf("biz start\n");
  auto res = co_await foo(5);
  printf("\"%s\"\n", res.data());
  printf("biz end\n");
}

void test_async() {
  auto e = biz();
  for (e.start(); !e.done(); e.resume()) {
    printf("main\n");
  }
  printf("async test end\n");
}
