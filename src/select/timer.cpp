#include "select/timer.hpp"

#include <mutex>
#include <random>

namespace co {

namespace __detail {

TimerSelector::Timer TimerSelector::create_timer(
    unsigned long long milisecond) {
  // may cause performance problem
  // default randome engine ranges in size_t32
  std::default_random_engine e;
  while (true) {
    Fd fd(e(), Fd::Ftimer);
    std::unique_lock lock(self_);
    if (!fd_waitings_.count(fd)) {
      fd_waitings_.insert(fd);
      return Timer(fd, std::clock() + milisecond);
    }
  }
}

Fd TimerSelector::submit_sleep(const TimerSelector::Timer& timer) {
  std::unique_lock lock(self_);
  expired_queue_.emplace(timer);
  return timer.fd_;
}

std::vector<Fd> TimerSelector::select() {
  std::vector<Fd> res;
  {
    std::unique_lock lock(self_);
    while (!expired_queue_.empty() &&
           std::clock() >= expired_queue_.top().expired_time_) {
      auto fd = expired_queue_.top().fd_;
      expired_queue_.pop();
      fd_waitings_.erase(fd);
      res.push_back(fd);
    }
  }
  return res;
}

bool TimerSelector::check_ready(const Fd& fd) {
  std::shared_lock lock(self_);
  return !fd_waitings_.count(fd);
}

}  // namespace __detail

}  // namespace co
