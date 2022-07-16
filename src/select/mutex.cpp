#include "select/mutex.hpp"

#include <stdio.h>

#include <exception>
#include <mutex>
#include <thread>

#include "util/random.hpp"

// #define __DEBUG_PRINT__

#ifdef __DEBUG_PRINT__
#ifndef DEBUB
#define DEBUG(format, ...) printf(format, __VA_ARGS__)
#endif
#else
#define DEBUG
#endif

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
      DEBUG("thread(%d) submit lock fd(%d)\n", std::this_thread::get_id(),
            fd.hash());
      return fd;
    }
  }
}

void MutexSelector::submit_unlock(Mutex& mtx) {
  std::unique_lock lock(self_);
  auto fd = mtx_queue_.at(mtx).front();
  fd_usings_.erase(fd);
  mtx_queue_.at(mtx).pop();
  DEBUG("thread(%d) submit unlock fd(%d)\n", std::this_thread::get_id(),
        fd.hash());
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
  int yield_cnt = 1;
  std::unique_lock lock(self_);
  for (auto& [_, que] : mtx_queue_) {
    if (!que.empty()) {
      Fd fd = que.front();
      lock.unlock();
      DEBUG("thread(%d) yield %dth fd(%d)\n", std::this_thread::get_id(),
            yield_cnt, fd.hash());
      co_yield fd;
      yield_cnt++;
      lock.lock();
    }
  }
}

bool MutexSelector::check_ready(const Fd& fd) {
  DEBUG("thread(%d) check fd(%d)\n", std::this_thread::get_id(), fd.hash());
  std::shared_lock lock(self_);
  if (!fd_usings_.count(fd)) throw std::out_of_range("invalid lock fd");
  Mutex& mtx = fd_usings_.at(fd);
  return mtx_queue_.at(mtx).front() == fd;
}

}  // namespace __detail

}  // namespace co
