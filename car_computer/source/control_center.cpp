#include "control_center.hpp"

#include <memory>

#include "constants.hpp"

namespace CC {

ControlCenter::ControlCenter(int px_size_x, int px_size_y, float mm_per_px,
                             int border_offset, int dist_threshold)
    : bitmap_(px_size_x, px_size_y, mm_per_px),
      env_shot_(bitmap_),
      working_area_(bitmap_, border_offset, dist_threshold),
      desktop_(CONST::kTcpCommPort, {px_size_x, px_size_y, mm_per_px,
                                     border_offset, dist_threshold}) {
  desktop_.Send<MSG::Status>(MSG::Act);
  desktop_.Send<MSG::Action>(MSG::Init);

  car_position_ = PTIT::Coord{px_size_x, px_size_y} * mm_per_px;
}

std::list<ScanData> ControlCenter::ScanEnvironment() {
  desktop_.Send<MSG::Action>(MSG::Scanning);
  arduino_.StartRadar();

  std::list<ScanData> scan_data;

  while (true) {
    auto scan = arduino_.GetScan();
    if (!scan.has_value()) {
      break;
    }

    auto [angle, dist] = scan.value();

    angle = car_angle_ + angle;
    if (angle >= 360) {
      angle -= 360;
    }

    scan_data.push_back({angle, dist});
    desktop_.Send<MSG::Scan>(angle, dist);

    env_shot_.AddMeasure(angle, dist, CONST::kRadarWidth, car_position_);
  }

  desktop_.Send<MSG::Action>(MSG::Computing);
  processed_area_ = false;

  return scan_data;
}

std::optional<PTIT::Coord> ControlCenter::GetDestination() {
  desktop_.Send<MSG::Action>(MSG::Computing);

  processed_area_ = true;
  return working_area_.ProcessArea(car_position_);
}

void ControlCenter::GoToPoint(PTIT::Coord point) {
  desktop_.Send<MSG::Action>(MSG::Computing);

  if (!processed_area_) {
    working_area_.ProcessArea(car_position_);
    processed_area_ = true;
  }

  auto route = MA::GetRoute(bitmap_, point);

  desktop_.Send<MSG::Action>(MSG::MovingToPoint);
  for (auto coord : route) {
    auto [new_angle, move] = ComputeLowMove(coord);
    for (auto [step_num, dir, speed] : move) {
      arduino_.MoveCar(step_num, dir, speed);
      arduino_.GetStep();
    }
    car_angle_ = new_angle;
    car_position_ = coord;
  }

  desktop_.Send<MSG::Action>(MSG::Computing);
}

void ControlCenter::SetStatus(CC::CurrStatus curr_status) {
  desktop_.Send<MSG::Action>(MSG::Computing);
  desktop_.Send<MSG::Status>(curr_status);
}

CurrStatus ControlCenter::GetMode() {
  desktop_.Send<MSG::Action>(MSG::Computing);

  std::optional<CurrStatus> new_status;
  while (true) {
    auto status = desktop_.Receive();
    if (status == nullptr) {
      break;
    }
    new_status = dynamic_cast<MSG::Status&>(*status).status;
  }

  if (new_status.has_value()) {
    status_ = new_status.value();
  }

  return status_;
}

PTIT::Coord ControlCenter::GetCurrPosition() {
  desktop_.Send<MSG::Action>(MSG::Computing);

  return car_position_;
}

std::pair<float, std::list<ControlCenter::LowMove>>
ControlCenter::ComputeLowMove(PTIT::Coord destination) {
  std::list<ControlCenter::LowMove> move;

  auto angle = PTIT::NormalizeDeg(
      PTIT::RadToDeg(PTIT::Segment(car_position_, destination).GetAngle()));

  if (angle != car_angle_) {
    auto norm_angle = angle - car_angle_;
    if (norm_angle < 0) {
      norm_angle += 360;
    }

    CCM::ArduinoCommunicator::Direction dir =
        norm_angle > 180 ? CCM::ArduinoCommunicator::Right
                         : CCM::ArduinoCommunicator::Left;

    int steps = abs(angle - car_angle_) / 360 * CONST::kStepsPerCircle;

    move.push_back({.steps_num = steps, .dir = dir});
  }

  int forward_step_num = PTIT::GetDistance(car_position_, destination) / 1'000 *
                         CONST::kStepsPerMeter;
  move.push_back({.steps_num = forward_step_num});

  return {angle, move};
}

}  // namespace CC