#pragma once

#include <colors.hpp>
#include <memory>
#include <palladium>

#define PD_SHARED(x)                                         \
  using Ref = std::shared_ptr<x>;                            \
  template <typename... Args>                                \
  static Ref New(Args&&... args) {                           \
    return std::make_shared<x>(std::forward<Args>(args)...); \
  }

namespace PD {
namespace Flex {
class Container {
 public:
  Container(Theme* t) : gTheme(t) {}
  ~Container() = default;
  PD_SHARED(Container)

  virtual void Draw(PD::Li::Drawlist& l) const {}

  Theme* gTheme;
};
}  // namespace Flex
}  // namespace PD