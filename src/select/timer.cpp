#include "select/timer.hpp"

#include <mutex>

#include "util/random.hpp"

namespace co {

namespace __detail {

TimerSelector::Timer TimerSelector::create_timer(
    unsigned long long milisecond) {
  while (true) {
    auto fd = create_fd_(random(), Fd::Ftimer);
    std::unique_lock lock(self_);
    if (!fd_status_.count(fd)) {
      fd_status_[fd] = Fd::Fwaiting;
      return Timer(fd, std::clock() + milisecond);
    }
  }
}

Selector::Fd TimerSelector::submit_sleep(const Timer& timer) {
  std::unique_lock lock(self_);
  expired_queue_.emplace(timer);
  return timer.fd_;
}

void TimerSelector::destroy_timer(TimerSelector::Timer& timer) {
  std::unique_lock lock(self_);
  fd_status_.erase(timer.fd_);
}

Generator<Selector::Fd> TimerSelector::select() {
  std::unique_lock lock(self_);
  while (!expired_queue_.empty() &&
         std::clock() >= expired_queue_.top().expired_time_) {
    auto fd = expired_queue_.top().fd_;
    expired_queue_.pop();
    fd_status_[fd] = Fd::Fready;
    lock.unlock();
    co_yield fd;
    lock.lock();
  }
}

bool TimerSelector::check_ready(const Fd& fd) {
  std::shared_lock lock(self_);
  return fd_status_.at(fd) == Fd::Fready;
}

}  // namespace __detail

}  // namespace co
