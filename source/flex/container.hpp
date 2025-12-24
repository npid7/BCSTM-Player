#pragma once

#include <colors.hpp>
#include <palladium>

namespace PD {
namespace Flex {
class Container {
 public:
  Container(Theme* t) : gTheme(t) {}
  ~Container() = default;
  PD_SHARED(Container)

  virtual void Draw(PD::Li::DrawList::Ref l) const {}

  Theme* gTheme;
};
}  // namespace Flex
}  // namespace PD