#pragma once

#include <memory>
#include <mutex>
#include <queue>
#include <thread>
#include <unordered_map>
#include <unordered_set>

#include "cofunc/asyncf.hpp"
#include "select/selector.hpp"

namespace co {

namespace __detail {

class Scheduler {
 public:
  class FdAwaiter {
    friend class Scheduler;

   public:
    FdAwaiter(const Selector::Fd* fd, Scheduler* scheduler)
        : fd_(fd), scheduler_(scheduler) {}

    bool await_ready();
    void await_suspend(std::coroutine_handle<>);
    void await_resume();

   private:
    const Selector::Fd* fd_;
    Scheduler* scheduler_;
    bool suspended_ = false;
  };

  static Scheduler* this_scheduler();

  Scheduler();
  virtual ~Scheduler();

  void submit_async(std::shared_ptr<Async>&& pfn);
  void event_loop(size_t thread_num = 1);
  FdAwaiter create_awaiter(const Selector::Fd* fd);
  Selector* selector(Selector::Fd::Ftype type);

 private:
  using Tid = std::thread::id;
  using Pasync = std::shared_ptr<Async>;

  static std::shared_mutex class_;
  static std::unordered_map<Tid, Scheduler*> schedulers_;

  std::mutex self_;
  std::unordered_map<Tid, Pasync> fn_currents_;
  std::queue<Pasync> fn_readys_;
  std::unordered_set<Pasync> fn_waitings_;
  std::unordered_map<const Selector::Fd*, Pasync> fn_events_;
  std::unordered_map<Selector::Fd::Ftype, Selector*> selectors_;

  void loop_routine_();
};

}  // namespace __detail

extern __detail::Scheduler default_scheduler_;

template <typename T>
inline void go(Asyncf<T>&& fn,
               __detail::Scheduler& scheduler = default_scheduler_) {
  auto pfn = std::make_shared<__detail::Async>(std::move(fn));
  scheduler.submit_async(std::move(pfn));
}

inline void event_loop(size_t thread_num = 1,
                       __detail::Scheduler& scheduler = default_scheduler_) {
  scheduler.event_loop(thread_num);
}

inline __detail::Scheduler* this_scheduler() {
  return __detail::Scheduler::this_scheduler();
}

}  // namespace co
