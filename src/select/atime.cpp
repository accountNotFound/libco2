#include "select/atime.hpp"

#include <mutex>
#include <random>

namespace co {

namespace __detail {

bool Atime::check_ready(const Fd& fd) {
  std::shared_lock lock(self_);
  return !fd_waitings_.count(fd);
}

void Atime::submit_sleep(unsigned long long milisecond) {
  std::unique_lock lock(self_);
  auto fd = random_create_();
  fd_waitings_.insert(fd);
  expired_queue_.emplace(fd, std::clock() + milisecond);
}

std::vector<Fd> Atime::select() {
  std::vector<Fd> res;
  {
    std::unique_lock lock(self_);
    while (!expired_queue_.empty() &&
           std::clock() >= expired_queue_.top().expired_time) {
      auto fd = expired_queue_.top().fd_;
      expired_queue_.pop();
      fd_waitings_.erase(fd);
      res.push_back(fd);
    }
  }
  return res;
}

Fd Atime::random_create_() {
  // should be called with lock
  // may cause performance problem
  // default randome engine ranges in size_t32
  std::default_random_engine e;
  while (true) {
    auto r = e();
    if (!fd_waitings_.count({r, Fd::Atime})) return {r, Fd::Atime};
  }
}

}  // namespace __detail

}  // namespace co
