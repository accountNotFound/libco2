#include "schedule/scheduler.hpp"

#include <functional>
#include <mutex>

// #include "select/mutex.hpp"
#include "select/timer.hpp"

namespace co {

__detail::Scheduler default_scheduler_;

namespace __detail {

std::shared_mutex Scheduler::class_;
std::unordered_map<Scheduler::Tid, Scheduler*> Scheduler::schedulers_;

Scheduler::Scheduler() {
  selectors_[Selector::Fd::Ftimer] = new TimerSelector();
  // selectors_[Selector::Fd::Fmutex] = new MutexSelector();
  /* more in future */
}

Scheduler::~Scheduler() {
  for (auto& [_, selector] : selectors_) delete selector;
}

Selector* Scheduler::selector(Selector::Fd::Ftype type) {
  return selectors_.at(type);
}

void Scheduler::submit_async(std::shared_ptr<Async>&& pfn) {
  pfn->start();
  std::unique_lock lock(self_);
  fn_readys_.push(pfn);
}

void Scheduler::event_loop(size_t thread_num) {
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

void Scheduler::loop_routine_() {
  Tid tid = std::this_thread::get_id();
  {
    std::unique_lock lock(class_);
    schedulers_[tid] = this;
  }
  {
    std::unique_lock lock(self_);
    fn_currents_[tid] = nullptr;
  }
  loop_body_(tid);
  {
    std::unique_lock lock(class_);
    schedulers_.erase(tid);
  }
}

void Scheduler::loop_body_(std::thread::id tid) {
  auto pick_ready = [this, tid]() {
    if (!fn_readys_.empty()) {
      fn_currents_[tid] = fn_readys_.front();
      fn_readys_.pop();
    }
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
            auto pfn = fn_events_.at(fd);
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

Scheduler::FdAwaiter Scheduler::create_awaiter(const Selector::Fd& fd) {
  return FdAwaiter(fd, *this);
}

void Scheduler::suspend_awaiter_(const FdAwaiter& awaiter) {
  Tid tid = std::this_thread::get_id();
  std::unique_lock lock(self_);
  auto current = fn_currents_.at(tid);
  fn_events_[awaiter.fd_] = current;
  fn_waitings_.insert(current);
}

}  // namespace __detail

}  // namespace co