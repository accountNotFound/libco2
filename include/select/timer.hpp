#pragma once

#include <ctime>
#include <queue>
#include <shared_mutex>
#include <unordered_set>
#include <vector>

#include "fd.hpp"

namespace co {

namespace __detail {

class Timer : public Selector {
 public:
  Timer() = default;
  ~Timer() override = default;

  Fd submit_sleep(unsigned long long milisecond);
  std::vector<Fd> select() override;
  bool check_ready(const Fd& fd) override;

 private:
  struct TimedFd {
    Fd fd_;
    std::clock_t expired_time;

    struct Greater {
      bool operator()(const TimedFd& a, const TimedFd& b) const {
        return a.expired_time > b.expired_time;
      }
    };
  };

  std::shared_mutex self_;
  std::priority_queue<TimedFd, std::vector<TimedFd>, TimedFd::Greater>
      expired_queue_;
  std::unordered_set<Fd> fd_waitings_;
};

}  // namespace __detail

}  // namespace co
