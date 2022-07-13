#include "api/timer.hpp"

#include "schedule/schedule.hpp"

namespace co {

Asyncf<void> sleep(unsigned long long milisecond) {
  auto ptimer = dynamic_cast<__detail::Timer*>(
      default_schedule_.selector(__detail::Fd::Timer));
  auto fd = ptimer->submit_sleep(milisecond);
  co_await default_schedule_.create_awaiter(fd);
}

}  // namespace co
