#include <queue>
#include <shared_mutex>
#include <unordered_map>
#include <unordered_set>

#include "selector.hpp"

namespace co {

namespace __detail {

class MutexSelector : public Selector {
 public:
  using Mutex = size_t;

  class MutexFd : public Fd {
    friend class MutexSelector;

   public:
    MutexFd() = default;
    MutexFd(const MutexFd&) = default;
    ~MutexFd() = default;

    bool ready() const override;
    void submit_read() const override;       // use for unlock
    void submit_write() const override;      // use for lock
    bool submit_try_write() const override;  // use for try lock

   private:
    Mutex mutex_;
    MutexSelector* selector_;

    MutexFd(size_t uid, Mutex mutex, MutexSelector* selector)
        : Fd(uid, Fd::Fmutex), mutex_(mutex), selector_(selector) {}
  };

  MutexSelector() = default;
  MutexSelector(const MutexSelector&) = delete;
  ~MutexSelector() = default;

  Generator<const Fd*> select() override;

  const Mutex create_mutex();
  void destroy_mutex(const Mutex& mutex);
  const MutexFd* create_fd(const Mutex& mutex);
  const MutexFd* active_fd(const Mutex& mutex);
  void destroy_fd(const Fd* fd) override;

 private:
  std::shared_mutex self_;
  std::unordered_map<Mutex, std::queue<MutexFd>> mtx_queue_;
  std::unordered_set<MutexFd, MutexFd::Hash> fd_usings_;
};

}  // namespace __detail

}  // namespace co
