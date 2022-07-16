#include <chrono>
#include <iostream>

void test_async();
void test_schedule();
void test_sleep();
void test_mutex();

void test_case() { test_mutex(); }

int main() {
  auto start = std::chrono::high_resolution_clock::now();
  test_case();
  auto end = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double, std::milli> elapsed = end - start;
  std::cout << "Waited " << elapsed.count() / 1000 << " s\n";
}