#include "util/random.hpp"

namespace co {

namespace __detail {

size_t random() {
  static std::default_random_engine e;
  // set random seed in product environment
  return e();
}

}  // namespace __detail

}  // namespace co
