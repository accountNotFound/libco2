#pragma once

#include "cofunc/generator.hpp"

namespace co {

namespace __detail {

class Selector {
 public:
  class Fd {
    friend class Selector;

   public:
    enum Ftype { Ftimer = 0, Fmutex };

    Fd() = default;
    Fd(const Fd&) = default;
    virtual ~Fd() = default;

    virtual bool ready() const = 0;
    virtual void submit_read() const {};
    virtual void submit_write() const {};
    virtual bool submit_try_read() const { return false; }
    virtual bool submit_try_write() const { return false; }

    size_t uid() const { return uid_; }
    Ftype type() const { return type_; }
    bool operator==(const Fd& rhs) const {
      return uid_ == rhs.uid_ && type_ == rhs.type_;
    }
    struct Hash {
      size_t operator()(const Fd& fd) const { return fd.uid_; }
    };

   protected:
    size_t uid_;
    Ftype type_;

    Fd(size_t uid, Ftype type) : uid_(uid), type_(type) {}
  };

  Selector() = default;
  Selector(const Selector&) = delete;
  virtual ~Selector() = default;

  // derives' implementation (may have different input args) must make sure that
  // all fd* created by create_fd() must be valid before they are explicitly
  // destory by destory_fd()
  const Fd* create_fd() { return nullptr; }

  // derives' implementation must make sure that all fd* created by
  // create_fd() must be valid before they are explicitly destory by
  // destory_fd()
  virtual void destroy_fd(const Fd* fd) = 0;

  // select() return Fd* is absolutely safe since these Fd* are alive before
  // they are destroy by destroy_fd()
  virtual Generator<const Fd*> select() = 0;
};

}  // namespace __detail

}  // namespace co

template <>
struct std::hash<co::__detail::Selector::Fd> {
  size_t operator()(const co::__detail::Selector::Fd& fd) const {
    return fd.uid();
  }
};

template <>
struct std::hash<co::__detail::Selector::Fd*> {
  size_t operator()(const co::__detail::Selector::Fd* const fd) const {
    return fd->uid();
  }
};
