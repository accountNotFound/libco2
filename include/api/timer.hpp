#include "cofunc/asyncf.hpp"
#include "schedule/schedule.hpp"
#include "select/timer.hpp"

namespace co {

Asyncf<void> sleep(unsigned long long milisecond,
                   Schedule& schedule = default_schedule_);

}  // namespace co
