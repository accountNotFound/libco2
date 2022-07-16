#include "api/mutex.hpp"

namespace co {

Mutex::Mutex() { mtx_ = selector_().create_mutex(); }

Mutex::~Mutex() { selector_().destroy_mutex(mtx_); }

__detail::MutexSelector& Mutex::selector_() {
  return *dynamic_cast<__detail::MutexSelector*>(
      this_scheduler()->selector(__detail::Selector::Fd::Fmutex));
}

Asyncf<void> Mutex::lock() {
  auto fd = selector_().submit_lock(mtx_);
  co_await this_scheduler()->create_awaiter(fd);
}

void Mutex::unlock() { selector_().submit_unlock(mtx_); }

bool Mutex::try_lock() { return selector_().submit_try_lock(mtx_); }

}  // namespace co
