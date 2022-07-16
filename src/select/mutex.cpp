#include "select/mutex.hpp"

#include <mutex>
#include <random>

namespace co {

namespace __detail {

MutexSelector::Mutex MutexSelector::create_mutex() {
  // may cause performance problem
  // default randome engine ranges in size_t32
  std::default_random_engine e;
  while (true) {
    Mutex mtx(create_fd(e(), Fd::Fdefault));
    std::unique_lock lock(self_);
    if (!mtx_queue_.count(mtx)) {
      mtx_queue_[mtx] = {};
      return mtx;
    }
  }
}

void MutexSelector::destroy_mutex(Mutex& mtx) {
  std::unique_lock lock(self_);
  mtx_queue_.erase(mtx);
}

Selector::Fd MutexSelector::submit_lock(Mutex& mtx) {
  // may cause performance problem
  // default randome engine ranges in size_t32
  std::default_random_engine e;
  while (true) {
    Fd fd = create_fd(e(), Fd::Fmutex);
    std::unique_lock lock(self_);
    if (!fd_waiting_.count(fd)) {
      fd_waiting_[fd] = mtx;
      mtx_queue_[mtx].push(fd);
      return fd;
    }
  }
}

void MutexSelector::submit_unlock(Mutex& mtx) {
  std::unique_lock lock(self_);
  auto fd = mtx_queue_.at(mtx).front();
  if (fd_waiting_.count(fd)) fd_waiting_.erase(fd);
  mtx_queue_.at(mtx).pop();
}

bool MutexSelector::submit_try_lock(Mutex& mtx) {
  // may cause performance problem
  // default randome engine ranges in size_t32
  std::default_random_engine e;
  while (true) {
    Fd fd = create_fd(e(), Fd::Fmutex);
    std::unique_lock lock(self_);
    if (!fd_waiting_.count(fd)) {
      if (mtx_queue_[mtx].empty()) {
        fd_waiting_[fd] = mtx;
        mtx_queue_[mtx].push(fd);
        return true;
      } else {
        return false;
      }
    }
  }
}

Generator<Selector::Fd> MutexSelector::select() {
  std::unique_lock lock(self_);
  for (auto& [_, que] : mtx_queue_) {
    if (!que.empty()) {
      Fd fd = que.front();
      fd_waiting_.erase(fd);
      lock.unlock();
      co_yield fd;
      lock.lock();
    }
  }
}

bool MutexSelector::check_ready(const Fd& fd) {
  std::shared_lock lock(self_);
  if (!fd_waiting_.count(fd)) return true;
  Mutex& mtx = fd_waiting_.at(fd);
  return mtx_queue_.at(mtx).front() == fd;
}

}  // namespace __detail

}  // namespace co
