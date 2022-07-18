#include "api/timer.hpp"

#include "schedule/scheduler.hpp"
#include "select/timer.hpp"

namespace co {

Asyncf<void> sleep(unsigned long long milliseconds) {
  auto scheduler = this_scheduler();
  auto selector = dynamic_cast<__detail::TimerSelector*>(
      scheduler->selector(__detail::Selector::Fd::Ftimer));
  auto fd = selector->create_fd(milliseconds);
  fd->submit_read();
  co_await this_scheduler()->create_awaiter(fd);
  selector->destroy_fd(fd);
}

}  // namespace co
