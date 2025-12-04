#pragma once

#include <pd.hpp>

namespace PD {
namespace Flex {
class Container {
 public:
  Container() {}
  ~Container() = default;
  PD_SMART_CTOR(Container)

  virtual void Draw(PD::LI::DrawList::Ref l) const {}
};
}  // namespace Flex
}  // namespace PD