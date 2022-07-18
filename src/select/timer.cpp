#include "select/timer.hpp"

#include <mutex>

#include "util/random.hpp"

namespace co {

namespace __detail {

const TimerSelector::TimedFd* TimerSelector::create_fd(
    unsigned long long miliseconds) {
  TimedFd timefd(0, std::clock() + miliseconds, this);
  while (true) {
    timefd.uid_ = random();
    std::unique_lock lock(self_);
    if (!fd_status_.count(timefd)) {
      fd_status_[timefd] = TimedFd::Waiting;
      return &fd_status_.find(timefd)->first;
    }
  }
}

void TimerSelector::destroy_fd(const Fd* fd) {
  std::unique_lock lock(self_);
  fd_status_.erase(*dynamic_cast<const TimedFd*>(fd));
}

Generator<const Selector::Fd*> TimerSelector::select() {
  std::unique_lock lock(self_);
  while (!expired_queue_.empty() &&
         std::clock() >= expired_queue_.top().expired_time_) {
    TimedFd timefd = expired_queue_.top();
    expired_queue_.pop();
    fd_status_[timefd] = TimedFd::Ready;
    lock.unlock();
    co_yield &fd_status_.find(timefd)->first;
    lock.lock();
  }
}

bool TimerSelector::TimedFd::ready() const {
  std::shared_lock lock(selector_->self_);
  return selector_->fd_status_[*this] == TimedFd::Ready;
}

void TimerSelector::TimedFd::submit_read() const {
  std::unique_lock lock(selector_->self_);
  selector_->expired_queue_.push(*this);
}

}  // namespace __detail

}  // namespace co
