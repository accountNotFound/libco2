#include "api/timer.hpp"

namespace co {

Asyncf<void> sleep(unsigned long long milisecond) {
  auto scheduler = __detail::Scheduler::this_thread_scheduler();
  auto time_selector = dynamic_cast<__detail::TimerSelector*>(
      scheduler->selector(__detail::Selector::Fd::Ftimer));
  auto timer = time_selector->create_timer(milisecond);
  auto fd = time_selector->submit_sleep(timer);
  co_await scheduler->create_awaiter(fd);
}

}  // namespace co
