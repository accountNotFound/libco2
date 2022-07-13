#include "api/timer.hpp"

namespace co {

Asyncf<void> sleep(unsigned long long milisecond, Schedule& schedule) {
  auto ptime_selector = dynamic_cast<__detail::TimerSelector*>(
      schedule.selector(__detail::Fd::Ftimer));
  auto timer = ptime_selector->create_timer(milisecond);
  auto fd = ptime_selector->submit_sleep(timer);
  co_await default_schedule_.create_awaiter(fd);
}

}  // namespace co
