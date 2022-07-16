#include "schedule/scheduler.hpp"

#include <chrono>
#include <functional>
#include <mutex>

#include "select/mutex.hpp"
#include "select/timer.hpp"

// #define __DEBUG_PRINT__

#ifdef __DEBUG_PRINT__
#ifndef DEBUB
#define DEBUG(format, ...) printf(format, __VA_ARGS__)
#endif
#else
#define DEBUG
#endif

namespace co {

__detail::Scheduler default_scheduler_;

namespace __detail {

std::shared_mutex Scheduler::class_;
std::unordered_map<Scheduler::Tid, Scheduler*> Scheduler::schedulers_;

Scheduler* Scheduler::this_scheduler() {
  auto tid = std::this_thread::get_id();
  std::shared_lock lock(class_);
  if (schedulers_.count(tid)) return schedulers_.at(tid);
  return &default_scheduler_;
}

Scheduler::Scheduler() {
  selectors_[Selector::Fd::Ftimer] = new TimerSelector();
  selectors_[Selector::Fd::Fmutex] = new MutexSelector();
  /* more in future */
}

Scheduler::~Scheduler() {
  for (auto& [_, selector] : selectors_) delete selector;
}

Selector* Scheduler::selector(Selector::Fd::Ftype type) {
  // no need to lock self
  return selectors_.at(type);
}

void Scheduler::submit_async(std::shared_ptr<Async>&& pfn) {
  pfn->start();
  std::unique_lock lock(self_);
  fn_readys_.push(pfn);
}

void Scheduler::event_loop(size_t thread_num) {
  std::vector<std::thread> fork_workers(thread_num - 1);
  for (int i = 0; i < thread_num - 1; i++)
    fork_workers[i] = std::thread([this]() { loop_routine_(); });
  loop_routine_();
  for (int i = 0; i < thread_num - 1; i++) fork_workers[i].join();
}

void Scheduler::loop_routine_() {
  Tid tid = std::this_thread::get_id();
  DEBUG("thread(%d) start\n", tid);
  {
    std::unique_lock lock(class_);
    schedulers_[tid] = this;
  }
  {
    std::unique_lock lock(self_);
    fn_currents_[tid] = nullptr;
  }
  while (true) {
    {
      std::unique_lock lock(self_);
      if (fn_readys_.empty() && !fn_waitings_.empty()) {
        // may cause performance problem
        for (auto& [_, selector] : selectors_) {
          selector->select().for_each([this](Selector::Fd fd) {
            if (fn_events_.count(fd)) {
              auto pfn = fn_events_.at(fd);
              fn_events_.erase(fd);
              fn_waitings_.erase(pfn);
              fn_readys_.push(pfn);
            }
          });
        }
      }
      if (!fn_readys_.empty()) {
        fn_currents_[tid] = fn_readys_.front();
        fn_readys_.pop();
      } else if (fn_waitings_.empty()) {
        break;
      }
    }
    auto current = fn_currents_.at(tid);
    if (current) {
      current->resume();
      std::unique_lock lock(self_);
      if (!current->done() && !fn_waitings_.count(current))
        fn_readys_.push(current);
      fn_currents_[tid] = nullptr;
    } else {
      // do not yield
      // yield cause wired randomly dead loop
      std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
  }
  {
    std::unique_lock lock(class_);
    schedulers_.erase(tid);
  }
  DEBUG("thread(%d) end\n", tid);
}

Scheduler::FdAwaiter Scheduler::create_awaiter(const Selector::Fd& fd) {
  return FdAwaiter(fd, *this);
}

bool Scheduler::FdAwaiter::await_ready() {
  scheduler_.self_.lock();
  await_ready_ = scheduler_.selectors_.at(fd_.type())->check_ready(fd_);
  return await_ready_;
}

void Scheduler::FdAwaiter::await_suspend(std::coroutine_handle<>) {
  // already locked in await_ready
  Tid tid = std::this_thread::get_id();
  auto current = scheduler_.fn_currents_.at(tid);
  scheduler_.fn_events_[fd_] = current;
  scheduler_.fn_waitings_.insert(current);
  await_ready_ = false;
  scheduler_.self_.unlock();
}

void Scheduler::FdAwaiter::await_resume() {
  if (await_ready_) {
    // already locked in await_ready
    await_ready_ = false;
    scheduler_.self_.unlock();
  }
  // else: unlock in await_resume
}

}  // namespace __detail

}  // namespace co
