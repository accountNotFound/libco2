#pragma once

#include <memory>
#include <queue>
#include <shared_mutex>
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
    FdAwaiter(const Selector::Fd& fd, Scheduler& scheduler)
        : fd_(fd), scheduler_(scheduler) {}

    bool await_ready() {
      return scheduler_.selectors_.at(fd_.type())->check_ready(fd_);
    }
    void await_suspend(std::coroutine_handle<> caller) {
      scheduler_.suspend_awaiter_(*this);
    }
    void await_resume() {}

   private:
    Selector::Fd fd_;
    Scheduler& scheduler_;
  };

  Scheduler();
  virtual ~Scheduler();

  void submit_async(std::shared_ptr<Async>&& pfn);
  void event_loop(size_t thread_num = 1);
  Selector* selector(Selector::Fd::Ftype type);
  FdAwaiter create_awaiter(const Selector::Fd& fd);

  static Scheduler* this_thread_scheduler() {
    std::shared_lock lock(class_);
    return schedulers_.at(std::this_thread::get_id());
  }

 protected:
  virtual void loop_body_(std::thread::id tid);

 private:
  using Tid = std::thread::id;
  using Pasync = std::shared_ptr<Async>;

  static std::shared_mutex class_;
  static std::unordered_map<Tid, Scheduler*> schedulers_;

  std::shared_mutex self_;
  std::unordered_map<Tid, Pasync> fn_currents_;
  std::queue<Pasync> fn_readys_;
  std::unordered_set<Pasync> fn_waitings_;
  std::unordered_map<Selector::Fd, Pasync> fn_events_;
  std::unordered_map<Selector::Fd::Ftype, Selector*> selectors_;

  void loop_routine_();
  void suspend_awaiter_(const FdAwaiter& awaiter);
};

}  // namespace __detail

extern __detail::Scheduler default_scheduler_;

template <typename T>
inline void go(Asyncf<T>&& fn,
               __detail::Scheduler& schedule = default_scheduler_) {
  auto pfn = std::make_shared<__detail::Async>(std::move(fn));
  schedule.submit_async(std::move(pfn));
}

inline void event_loop(size_t thread_num = 1,
                       __detail::Scheduler& scheduler = default_scheduler_) {
  scheduler.event_loop(thread_num);
}

}  // namespace co
