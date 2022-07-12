#include "cofunc/asyncf.hpp"

#include <mutex>

namespace co {

namespace __detail {

std::shared_mutex Async::class_;
std::unordered_map<Async::Handler, Async::Pstack, Async::HandlerHash>
    Async::handler_stack_;

void Async::start() {
  auto pstk = std::make_shared<std::stack<Handler>>();
  pstk->push(handler_);
  {
    std::unique_lock lock(class_);
    handler_stack_[handler_] = pstk;
  }
}

void Async::resume() {
  Pstack pstk;
  {
    std::shared_lock lock(class_);
    pstk = handler_stack_.at(handler_);
  }
  pstk->top().resume();
  if (pstk->top().done()) pstk->pop();
  if (pstk->empty()) {
    std::unique_lock lock(class_);
    handler_stack_.erase(handler_);
  }
}

bool Async::done() { return !handler_ || handler_.done(); }

void Async::suspend_(Handler caller) {
  Pstack pstk;
  {
    std::unique_lock lock(class_);
    pstk = handler_stack_.at(caller);
    pstk->push(handler_);
    handler_stack_[handler_] = pstk;
  }
}

}  // namespace __detail

}  // namespace co