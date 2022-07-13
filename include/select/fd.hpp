#pragma once

#include <vector>

namespace co {

namespace __detail {

struct Fd {
  enum Ftype {
    Fdefault = 0,  // use as simple uid
    Ftimer,        // timer event fd
    Fmutex,        // mutex lock event fd
    // more in futre
  };
  size_t uid_;
  Ftype type_;

  bool operator==(const Fd& rhs) const {
    return uid_ == rhs.uid_ && type_ == rhs.type_;
  }
};

struct Selector {
  virtual ~Selector() = default;
  virtual std::vector<Fd> select() = 0;
  virtual bool check_ready(const Fd& fd) = 0;
};

}  // namespace __detail

}  // namespace co

template <>
struct std::hash<co::__detail::Fd> {
  size_t operator()(const co::__detail::Fd& fd) const { return fd.uid_; }
};