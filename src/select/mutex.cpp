#include "select/mutex.hpp"

#include <exception>
#include <mutex>

#include "util/random.hpp"

namespace co {

namespace __detail {

MutexSelector::Mutex MutexSelector::create_mutex() {
  while (true) {
    Mutex mtx(create_fd_(random(), Fd::Fdefault));
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
  while (true) {
    Fd fd = create_fd_(random(), Fd::Fmutex);
    std::unique_lock lock(self_);
    if (!fd_usings_.count(fd)) {
      fd_usings_[fd] = mtx;
      mtx_queue_[mtx].push(fd);
      return fd;
    }
  }
}

void MutexSelector::submit_unlock(Mutex& mtx) {
  std::unique_lock lock(self_);
  auto fd = mtx_queue_.at(mtx).front();
  fd_usings_.erase(fd);
  mtx_queue_.at(mtx).pop();
}

bool MutexSelector::submit_try_lock(Mutex& mtx) {
  while (true) {
    Fd fd = create_fd_(random(), Fd::Fmutex);
    std::unique_lock lock(self_);
    if (!fd_usings_.count(fd)) {
      if (mtx_queue_[mtx].empty()) {
        fd_usings_[fd] = mtx;
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
      lock.unlock();
      co_yield fd;
      lock.lock();
    }
  }
}

bool MutexSelector::check_ready(const Fd& fd) {
  std::shared_lock lock(self_);
  if (!fd_usings_.count(fd)) throw std::out_of_range("invalid lock fd");
  Mutex& mtx = fd_usings_.at(fd);
  return mtx_queue_.at(mtx).front() == fd;
}

}  // namespace __detail

}  // namespace co
