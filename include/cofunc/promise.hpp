#pragma once

#include <coroutine>

namespace co {

namespace __detail {

template <typename T>
struct Promise {
  T value_;
  bool valid_ = false;
  std::suspend_always initial_suspend() { return {}; }
  std::suspend_always final_suspend() noexcept { return {}; }
  void unhandled_exception() noexcept {}
  void return_value(const T& value) {
    value_ = value;
    valid_ = true;
  }
  std::suspend_always yield_value(const T& value) {
    value_ = value;
    valid_ = true;
    return {};
  }
};

template <>
struct Promise<void> {
  std::suspend_always initial_suspend() { return {}; }
  std::suspend_always final_suspend() noexcept { return {}; }
  void unhandled_exception() noexcept {}
  void return_void() {}
};

}  // namespace __detail

}  // namespace co