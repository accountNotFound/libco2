#include <queue>
#include <shared_mutex>
#include <unordered_map>
#include <unordered_set>

#include "selector.hpp"

namespace co {

namespace __detail {

class MutexSelector : public Selector {
 public:
  class Mutex {
    friend class MutexSelector;

   public:
    Mutex() = default;
    Mutex(Fd fd) : mutex_id_(fd) {}
    ~Mutex() = default;

    struct Hash {
      size_t operator()(const Mutex& mtx) const {
        return std::hash<Fd>()(mtx.mutex_id_);
      }
    };
    bool operator==(const Mutex& rhs) const {
      return mutex_id_ == rhs.mutex_id_;
    }

   private:
    Fd mutex_id_;
  };

  MutexSelector() = default;
  ~MutexSelector() override = default;

  Mutex create_mutex();
  void destroy_mutex(Mutex& mtx);
  Fd submit_lock(Mutex& mtx);
  void submit_unlock(Mutex& mtx);
  bool submit_try_lock(Mutex& mtx);

  Generator<Fd> select() override;
  bool check_ready(const Fd& fd) override;

 private:
  std::shared_mutex self_;
  std::unordered_map<Mutex, std::queue<Fd>, Mutex::Hash> mtx_queue_;
  std::unordered_map<Fd, Mutex> fd_using_;
};

}  // namespace __detail

}  // namespace co
