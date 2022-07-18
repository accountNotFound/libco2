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
  class TimedFd : public Fd {
    friend class TimerSelector;

   public:
    enum Status { Waiting = 0, Ready };

    TimedFd() = default;
    TimedFd(const TimedFd&) = default;
    ~TimedFd() = default;

    bool ready() const override;
    void submit_read() const override;

    struct Greater {
      bool operator()(const TimedFd& a, const TimedFd& b) const {
        return a.expired_time_ > b.expired_time_;
      }
    };

   private:
    std::clock_t expired_time_;
    TimerSelector* selector_;

    TimedFd(size_t uid, std::clock_t expired_time, TimerSelector* selector)
        : Fd(uid, Fd::Ftimer),
          expired_time_(expired_time),
          selector_(selector) {}
  };

  TimerSelector() = default;
  TimerSelector(const TimerSelector&) = delete;
  ~TimerSelector() = default;

  Generator<const Fd*> select() override;

  const TimedFd* create_fd(unsigned long long milliseconds);
  void destroy_fd(const Fd* fd) override;

 private:
  std::shared_mutex self_;
  std::priority_queue<TimedFd, std::vector<TimedFd>, TimedFd::Greater>
      expired_queue_;
  std::unordered_map<TimedFd, TimedFd::Status, TimedFd::Hash> fd_status_;
};

}  // namespace __detail

}  // namespace co
