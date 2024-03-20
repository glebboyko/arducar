#include "runners.hpp"

#include <chrono>
#include <thread>

void AutoMapBuilder(ControlCenter& control_center) {
  const std::chrono::milliseconds kLoopWait(100);
  while (true) {
    switch (control_center.GetMode()) {
      case Stop:
        return;
      case Pause:
        std::this_thread::sleep_for(kLoopWait);
        continue;
    }

    control_center.ScanEnvironment();
    auto point = control_center.GetDestination();
    if (!point.has_value()) {
      return;
    }

    control_center.GoToPoint(point.value());
  }
}