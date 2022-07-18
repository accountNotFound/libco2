#include "api/mutex.hpp"

#include "schedule/scheduler.hpp"

// #define __PRINT_DEBUG__

#ifdef __PRINT_DEBUG__
#include "stdio.h"
#define DEBUG(format, ...) printf(format, __VA_ARGS__)
#else
#define DEBUG(format, ...)
#endif

namespace co {

Mutex::Mutex() { mtx_ = selector_()->create_mutex(); }

Mutex::~Mutex() { selector_()->destroy_mutex(mtx_); }

__detail::MutexSelector* Mutex::selector_() {
  return dynamic_cast<__detail::MutexSelector*>(
      this_scheduler()->selector(__detail::Selector::Fd::Fmutex));
}

Asyncf<void> Mutex::lock() {
  auto mutexfd = selector_()->create_fd(mtx_);
  mutexfd->submit_write();
  DEBUG("try lock fd(%d)\n", mutexfd->uid());
  co_await this_scheduler()->create_awaiter(mutexfd);
  DEBUG("locked fd(%d)\n", mutexfd->uid());
}

void Mutex::unlock() {
  auto mutexfd = selector_()->active_fd(mtx_);
  mutexfd->submit_read();
  DEBUG("unlock fd(%d)\n", mutexfd->uid());
  selector_()->destroy_fd(mutexfd);
}

bool Mutex::try_lock() {
  auto mutexfd = selector_()->create_fd(mtx_);
  return mutexfd->submit_try_write();
}

}  // namespace co
