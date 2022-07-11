#include <coroutine>

namespace co {

namespace __detail {

template <typename T>
struct Promise {
  T value_;
  std::suspend_always initial_suspend() { return {}; }
  std::suspend_always final_suspend() noexcept { return {}; }
  void unhandled_exception() noexcept {}
  void return_value(const T& value) { value_ = value; }
  std::suspend_always yield_value(const T& value) {
    value_ = value;
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