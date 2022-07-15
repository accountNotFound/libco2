#pragma once

#include <ctime>
#include <queue>
#include <shared_mutex>
#include <unordered_set>
#include <vector>

#include "selector.hpp"

namespace co {

namespace __detail {

class TimerSelector : public Selector {
 public:
  class Timer {
    friend class TimerSelector;

   public:
    struct Greater {
      bool operator()(const Timer& a, const Timer& b) const {
        return a.expired_time_ > b.expired_time_;
      }
    };

    Timer() = default;
    Timer(Fd fd, std::clock_t expired_time)
        : fd_(fd), expired_time_(expired_time) {}
    ~Timer() = default;

   private:
    Fd fd_;
    std::clock_t expired_time_;
  };

  TimerSelector() = default;
  ~TimerSelector() override = default;

  Timer create_timer(unsigned long long milisecond);
  Fd submit_sleep(const Timer& timer);

  Generator<Fd> select() override;
  bool check_ready(const Fd& fd) override;

 private:
  std::shared_mutex self_;
  std::priority_queue<Timer, std::vector<Timer>, Timer::Greater> expired_queue_;
  std::unordered_set<Fd> fd_waitings_;
};

}  // namespace __detail

}  // namespace co
