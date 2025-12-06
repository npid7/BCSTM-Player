#pragma once

#include <palladium>

namespace PD {
namespace Flex {
class Container {
 public:
  Container() {}
  ~Container() = default;
  PD_SHARED(Container)

  virtual void Draw(PD::Li::DrawList::Ref l) const {}
};
}  // namespace Flex
}  // namespace PD