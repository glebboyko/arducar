#include "runners.hpp"

#include <chrono>
#include <thread>

#include "messages/messages.hpp"

namespace RNS {

void AutoMapBuilder(CC::ControlCenter& control_center) {
  const std::chrono::milliseconds kLoopWait(100);
  while (true) {
    switch (control_center.GetMode()) {
      case MSG::Stop:
        return;
      case MSG::Pause:
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

}  // namespace RNS