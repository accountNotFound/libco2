#include "cofunc/asyncf.hpp"
#include "schedule/scheduler.hpp"
#include "select/mutex.hpp"

namespace co {

class Mutex {
 public:
  Mutex();
  ~Mutex();
  Mutex(const Mutex&) = delete;
  Mutex(Mutex&& m) = default;

  Asyncf<void> lock();
  void unlock();
  bool try_lock();

 private:
  __detail::MutexSelector::Mutex mtx_;

  __detail::MutexSelector& selector_();
};

}  // namespace co
