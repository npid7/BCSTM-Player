#pragma once

namespace D7 {
namespace Cxx {
bool RTTIEnabled() {
#ifdef __RTTI
  return true;
#else
  return false;
#endif
}
bool ExceptionsEnabled() {
#ifdef __EXCEPTIONS
  return true;
#else
  return false;
#endif
}
}  // namespace Cxx
}  // namespace D7