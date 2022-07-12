#include "select/atime.hpp"

#include <mutex>
#include <random>

namespace co {

namespace __detail {

Atime::~Atime() {}

Fd Atime::submit_sleep(unsigned long long milisecond) {
  auto random_create = [this]() -> Fd {
    // called with lock
    // may cause performance problem
    // default randome engine ranges in size_t32
    std::default_random_engine e;
    while (true) {
      auto r = e();
      if (!fd_waitings_.count({r, Fd::Atime})) return {r, Fd::Atime};
    }
  };
  std::unique_lock lock(self_);
  auto fd = random_create();
  fd_waitings_.insert(fd);
  expired_queue_.emplace(fd, std::clock() + milisecond);
  return fd;
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

bool Atime::check_ready(const Fd& fd) {
  std::shared_lock lock(self_);
  return !fd_waitings_.count(fd);
}

}  // namespace __detail

}  // namespace co
