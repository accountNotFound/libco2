#pragma once

#include <exception>
#include <functional>

#include "promise.hpp"

namespace co {

template <typename T>
concept NotVoid = !std::is_same<T, void>::value;

template <typename NotVoid>
class Generator {
 public:
  struct promise_type : public __detail::Promise<NotVoid> {
    Generator get_return_object() {
      return Generator(
          std::coroutine_handle<promise_type>::from_promise(*this));
    }
  };

  Generator(const Generator& g) = delete;
  Generator(Generator&& g) : handler_(g.handler_) { g.handler_ = nullptr; }

  bool done() { return !handler_ || handler_.done(); }
  NotVoid next() {
    if (!done()) handler_.resume();
    if (!handler_.promise().valid_) throw std::out_of_range("stop iteration");
    handler_.promise().valid_ = false;
    return std::move(handler_.promise().value_);
  }

  void for_each(std::function<void(NotVoid&&)> func) {
    try {
      while (!done()) func(next());
    } catch (const std::out_of_range& e) {
      ;
    }
  }

 private:
  std::coroutine_handle<promise_type> handler_;

  Generator(std::coroutine_handle<promise_type> handler) : handler_(handler) {}
};

}  // namespace co
