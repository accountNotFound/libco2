#pragma once

#include <vector>

namespace co {

namespace __detail {

class Selector {
 public:
  struct Fd {
    friend class Selector;

   public:
    enum Ftype {
      Fdefault = 0,  // use as simple uid
      Ftimer,        // timer event fd
      Fmutex,        // mutex lock event fd
      // more in futre
    };

    Fd() = default;
    Fd(const Fd& fd) : uid_(fd.uid_), type_(fd.type_) {}

    bool operator==(const Fd& rhs) const { return uid_ == rhs.uid_; }
    size_t hash() const { return uid_; }
    size_t uid() const { return uid_; }
    Ftype type() const { return type_; }

   private:
    size_t uid_;
    Ftype type_;

    Fd(size_t uid, Ftype type) : uid_(uid), type_(type) {}
  };

  virtual ~Selector() = default;
  virtual std::vector<Fd> select() = 0;
  virtual bool check_ready(const Fd& fd) = 0;

 protected:
  Fd create_fd(size_t uid, Fd::Ftype type) { return Fd(uid, type); }
};

}  // namespace __detail

}  // namespace co

template <>
struct std::hash<co::__detail::Selector::Fd> {
  size_t operator()(const co::__detail::Selector::Fd& fd) const {
    return fd.hash();
  }
};