#include "schedule/schedule.hpp"

#include <functional>
#include <mutex>

namespace co {

Schedule default_schedule_;

Schedule::Schedule() {
  selectors_[__detail::Fd::Atime] = new __detail::Atime();
  /* more in future */
}

Schedule::~Schedule() {
  for (auto& [_, selector] : selectors_) delete selector;
}

void Schedule::submit_async(std::shared_ptr<__detail::Async>&& pfn) {
  pfn->start();
  std::unique_lock lock(self_);
  fn_readys_.push(pfn);
}

void Schedule::event_loop(size_t thread_num) {
  if (thread_num == 1) {
    loop_routine_();
  } else {
    auto workers = new std::thread[thread_num];
    for (int i = 0; i < thread_num; i++) {
      workers[i] = std::thread([this]() { loop_routine_(); });
    }
    for (int i = 0; i < thread_num; i++) workers[i].join();
  }
}

void Schedule::loop_routine_() {
  Tid tid = std::this_thread::get_id();
  {
    std::unique_lock lock(self_);
    fn_currents_[tid] = nullptr;
  }
  auto pick_ready = [this, tid]() {
    fn_currents_[tid] = fn_readys_.front();
    fn_readys_.pop();
  };
  while (true) {
    {
      std::unique_lock lock(self_);
      if (!fn_readys_.empty()) {
        pick_ready();
      } else if (!fn_waitings_.empty()) {
        // may cause performance problem
        for (auto& [_, selector] : selectors_) {
          for (auto fd : selector->select()) {
            auto pfn = fn_events_[fd];
            fn_events_.erase(fd);
            fn_waitings_.erase(pfn);
            fn_readys_.push(pfn);
          }
        }
        pick_ready();
      } else {
        return;
      }
    }
    auto current = fn_currents_[tid];
    if (current) {
      current->resume();
      {
        std::unique_lock lock(self_);
        if (!current->done() && !fn_waitings_.count(current))
          fn_readys_.push(current);
      }
      fn_currents_[tid] = nullptr;
    } else {
      // may cause performance problem
      std::this_thread::yield();
      // or sleep
    }
  }
}

Schedule::FdAwaiter Schedule::create_awaiter(const __detail::Fd& fd) {
  return FdAwaiter{fd, *this};
}

void Schedule::suspend_awaiter_(const FdAwaiter& awaiter) {
  Tid tid = std::this_thread::get_id();
  std::unique_lock lock(self_);
  auto current = fn_currents_[tid];
  fn_events_[awaiter.fd_] = current;
  fn_waitings_.insert(current);
}

}  // namespace co
