#include "cofunc/asyncf.hpp"
#include "schedule/scheduler.hpp"
#include "select/timer.hpp"

namespace co {

Asyncf<void> sleep(unsigned long long milisecond);

}  // namespace co
