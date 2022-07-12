#include "promise.hpp"

namespace co {

template <typename T>
class Generator {
 public:
  struct promise_type : public __detail::Promise<T> {
    Generator get_return_object() {
      return Generator(
          std::coroutine_handle<promise_type>::from_promise(*this));
    }
  };

  Generator(const Generator& g) = delete;
  Generator(Generator&& g) : handler_(g.handler_) { g.handler_ = nullptr; }

  bool done() { return !handler_ || handler_.done(); }
  auto next() {
    if (!std::is_same<T, void>::value) {
      return std::move(handler_.promise().value_);
    }
  }

 private:
  std::coroutine_handle<promise_type> handler_;

  Generator(std::coroutine_handle<promise_type> handler) : handler_(handler) {}
};

}  // namespace co
