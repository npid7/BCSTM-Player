#include <3ds.h>

#include <app.hpp>

namespace D7 {
void App::Run() {
  pLast = PD::OS::GetNanoTime();
  while (aptMainLoop() && !pExit) {
    PD::u64 c = PD::OS::GetNanoTime();
    pDelta = static_cast<double>(static_cast<double>(c) -
                                 static_cast<double>(pLast)) *
             0.000001;
    pTime += pDelta * 0.001;
    pLast = c;
    Main();
  }
}

}  // namespace D7