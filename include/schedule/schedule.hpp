#pragma once

#include <memory>
#include <queue>
#include <shared_mutex>
#include <thread>
#include <unordered_map>
#include <unordered_set>

#include "cofunc/asyncf.hpp"
#include "select/fd.hpp"

namespace co {

class Schedule {
 public:
  class FdAwaiter {
    friend class Schedule;

   public:
    FdAwaiter(__detail::Fd fd, Schedule& schedule)
        : fd_(fd), schedule_(schedule) {}

    bool await_ready() {
      return schedule_.selectors_.at(fd_.type_)->check_ready(fd_);
    }
    void await_suspend(std::coroutine_handle<> caller) {
      schedule_.suspend_awaiter_(*this);
    }
    void await_resume() {}

   private:
    __detail::Fd fd_;
    Schedule& schedule_;
  };

  Schedule();
  ~Schedule();

  void submit_async(std::shared_ptr<__detail::Async>&& pfn);
  void event_loop(size_t thread_num = 1);
  __detail::Selector* selector(__detail::Fd::Ftype type);
  FdAwaiter create_awaiter(const __detail::Fd& fd);

 private:
  using Tid = std::thread::id;
  using Pasync = std::shared_ptr<__detail::Async>;

  std::shared_mutex self_;
  std::unordered_map<Tid, Pasync> fn_currents_;
  std::queue<Pasync> fn_readys_;
  std::unordered_set<Pasync> fn_waitings_;
  std::unordered_map<__detail::Fd, Pasync> fn_events_;
  std::unordered_map<__detail::Fd::Ftype, __detail::Selector*> selectors_;

  void loop_routine_();
  void suspend_awaiter_(const FdAwaiter& awaiter);
};

extern Schedule default_schedule_;

template <typename T>
inline void go(Asyncf<T>&& fn, Schedule& schedule = default_schedule_) {
  auto pfn = std::make_shared<__detail::Async>(std::move(fn));
  schedule.submit_async(std::move(pfn));
}

inline void event_loop(size_t thread_num = 1,
                       Schedule& schedule = default_schedule_) {
  schedule.event_loop(thread_num);
}

}  // namespace co
