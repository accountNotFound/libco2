#include <iosfwd>

namespace co {

namespace __detail {

struct Fd {
  enum Type { Atime = 0, /* more in futre*/ };
  size_t uid_;
  Type type_;
};

}  // namespace __detail

}  // namespace co

template <>
struct std::hash<co::__detail::Fd> {
  size_t operator()(const co::__detail::Fd& fd) const { return fd.uid_; }
};