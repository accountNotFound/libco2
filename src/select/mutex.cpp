#include "select/mutex.hpp"

#include <stdio.h>

#include <exception>
#include <mutex>
#include <thread>

#include "util/random.hpp"

namespace co {

namespace __detail {

const MutexSelector::Mutex MutexSelector::create_mutex() {
  while (true) {
    Mutex mtx = random();
    std::unique_lock lock(self_);
    if (!mtx_queue_.count(mtx)) {
      mtx_queue_[mtx] = {};
      return mtx;
    }
  }
}

void MutexSelector::destroy_mutex(const Mutex& mtx) {
  std::unique_lock lock(self_);
  mtx_queue_.erase(mtx);
}

const MutexSelector::MutexFd* MutexSelector::create_fd(const Mutex& mtx) {
  MutexFd mutexfd(0, mtx, this);
  while (true) {
    mutexfd.uid_ = random();
    std::unique_lock lock(self_);
    if (!fd_usings_.count(mutexfd)) {
      fd_usings_.insert(mutexfd);
      return &*fd_usings_.find(mutexfd);
    }
  }
}

const MutexSelector::MutexFd* MutexSelector::active_fd(const Mutex& mtx) {
  std::shared_lock lock(self_);
  auto mutexfd = mtx_queue_.at(mtx).front();
  return &*fd_usings_.find(mutexfd);
}

void MutexSelector::destroy_fd(const Fd* fd) {
  std::unique_lock lock(self_);
  auto mutexfd = dynamic_cast<const MutexFd*>(fd);
  fd_usings_.erase(*mutexfd);
}

Generator<const Selector::Fd*> MutexSelector::select() {
  std::shared_lock lock(self_);
  for (auto& [_, que] : mtx_queue_) {
    if (!que.empty()) {
      MutexFd mutexfd = que.front();
      lock.unlock();
      co_yield &*fd_usings_.find(mutexfd);
      lock.lock();
    }
  }
}

bool MutexSelector::MutexFd::ready() const {
  std::shared_lock lock(selector_->self_);
  return selector_->mtx_queue_.at(mutex_).front() == *this;
}

void MutexSelector::MutexFd::submit_write() const {
  std::unique_lock lock(selector_->self_);
  selector_->mtx_queue_.at(mutex_).push(*this);
}

bool MutexSelector::MutexFd::submit_try_write() const {
  std::unique_lock lock(selector_->self_);
  if (selector_->mtx_queue_.at(mutex_).empty()) {
    selector_->mtx_queue_.at(mutex_).push(*this);
    return true;
  }
  return false;
}

void MutexSelector::MutexFd::submit_read() const {
  std::unique_lock lock(selector_->self_);
  selector_->mtx_queue_.at(mutex_).pop();
}

}  // namespace __detail

}  // namespace co
