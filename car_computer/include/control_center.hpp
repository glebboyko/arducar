#pragma once

#include <list>
#include <optional>
#include <primitives.hpp>

#include "communicators.hpp"
#include "constants.hpp"
#include "map/bitmap.hpp"
#include "map/env-shot.hpp"
#include "map/map_analyzer.hpp"
#include "messages/messages.hpp"

namespace CC {

struct ScanData {
  double angle;
  int mm_dist;
};

using CurrStatus = MSG::CurrStatus;

class ControlCenter {
 public:
  ControlCenter(int px_size_x, int px_size_y, float mm_per_px,
                int border_offset = CONST::kBorderOffset,
                int dist_threshold = CONST::kDistThreshold);

  std::list<ScanData> ScanEnvironment();

  std::optional<PTIT::Coord> GetDestination();
  void GoToPoint(PTIT::Coord point);
  void SetStatus(CurrStatus curr_status);

  CurrStatus GetMode();
  PTIT::Coord GetCurrPosition();

 private:
  struct LowMove {
    int steps_num;
    CCM::ArduinoCommunicator::Direction dir = CCM::ArduinoCommunicator::Forward;
    CCM::ArduinoCommunicator::Speed speed =
        static_cast<CCM::ArduinoCommunicator::Speed>(CONST::kDefaultSpeed);
  };

  BM::Bitmap bitmap_;
  EnvShot env_shot_;
  MA::WorkingAreaProcessor working_area_;

  CCM::DesktopCommunicator desktop_;
  CCM::ArduinoCommunicator arduino_;

  float car_angle_ = 0;
  PTIT::Coord car_position_;
  CurrStatus status_ = MSG::Act;
  bool processed_area_ = false;

  std::pair<float, std::list<LowMove>> ComputeLowMove(PTIT::Coord destination);
};

}  // namespace CC