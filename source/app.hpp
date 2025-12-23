#pragma once

#include <palladium>

namespace D7 {
class App {
 public:
  App() {
    pDelta = 0.0;
    pTime = 0.0;
    pLast = 0LL;
    pExit = false;
  }
  ~App() {}

  virtual void Main() = 0;
  void Run();
  double Delta() const { return pDelta; }
  double Time() const { return pTime; }
  void Exit() { pExit = true; }

 private:
  bool pExit;
  PD::u64 pLast;
  double pDelta;
  double pTime;
};
}  // namespace D7