#pragma once

#include <list>
#include <optional>
#include <primitives.hpp>

#include "messages/messages.hpp"

namespace CC {

struct ScanData {
  double angle;
  int mm_dist;
};

class ControlCenter {
 public:
  ControlCenter(int px_size_x, int px_size_y, float mm_per_px);

  std::list<ScanData> ScanEnvironment();
  std::optional<PTIT::Coord> GetDestination();
  void GoToPoint(PTIT::Coord point);

  MSG::CurrStatus GetMode();

 private:
};

}  // namespace CC