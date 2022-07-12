#include <memory>
#include <shared_mutex>
#include <stack>
#include <unordered_map>

#include "promise.hpp"

namespace co {

namespace __detail {

class Async {
 public:
  void start();
  void resume();
  bool done();

  Async(const Async& f) = delete;
  Async(Async&& f) : handler_(f.handler_) { f.handler_ = nullptr; }
  virtual ~Async() {
    if (handler_) handler_.destroy();
  }

 protected:
  using Handler = std::coroutine_handle<>;
  using Pstack = std::shared_ptr<std::stack<Handler>>;
  struct HandlerHash {
    size_t operator()(const Handler& h) const {
      return reinterpret_cast<size_t>(h.address());
    }
  };

  static std::shared_mutex class_;
  static std::unordered_map<Handler, Pstack, HandlerHash> handler_stack_;

  Handler handler_;

  Async(Handler handler) : handler_(handler) {}
  void suspend_(Handler caller);
};

}  // namespace __detail

template <typename T>
class Asyncf : public __detail::Async {
 public:
  struct promise_type : public __detail::Promise<T> {
    Asyncf get_return_object() {
      return Asyncf(std::coroutine_handle<promise_type>::from_promise(*this));
    }
  };

  bool await_ready() { return done(); }
  void await_suspend(Handler caller) { suspend_(caller); }
  auto await_resume() {
    if (!std::is_same<T, void>::value) {
      return std::move(
          std::coroutine_handle<promise_type>::from_address(handler_.address())
              .promise()
              .value_);
    }
  }

 private:
  Asyncf(std::coroutine_handle<promise_type> handler)
      : __detail::Async(handler) {}
};

}  // namespace co
