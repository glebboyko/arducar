#include "hardware_processor.hpp"

namespace CM {

const float kRotateLen = kCarWidth * 3.141592;

/*--------------------------------- movement ---------------------------------*/
Movement::Movement(AC::ArduinoCommunicator& communicator)
    : communicator_(communicator) {}

void Movement::MoveStraight(int dist, int speed) {
  if (speed == 0) {
    speed = kDefaultStraightSpeed;
  }

  communicator_.SendMessage<AC::CarMove>(dist, dist, 1000 * dist / speed);
  is_moving_ = true;
}

void Movement::Rotate(float angle, int speed) {
  float dist = angle * kRotateLen / 360;
  speed = speed * kRotateLen / 360;

  communicator_.SendMessage<AC::CarMove>(-dist, dist, 1000 * dist / speed);
  is_moving_ = true;
}

bool Movement::IsMoving() {
  if (!is_moving_) {
    return false;
  }

  is_moving_ = communicator_.ReceiveMessage<AC::CarMove>(0);
  return is_moving_;
}

/*----------------------------------- car ------------------------------------*/
Car::Car(PTIT::Coord position) : position_(position) {}

std::pair<PTIT::Coord, float> Car::GetPosition() const {
  if (!started_moving_.has_value()) {
    return {position_, angle_};
  }

  int time_passed =
      std::chrono::duration_cast<std::chrono::milliseconds>(
          std::chrono::system_clock::now() - started_moving_.value())
          .count();

  int time_to_rotate =
      1000 * (abs(dest_angle_ - angle_)) / kDefaultAngularSpeed;

  if (time_passed <= time_to_rotate) {
    float angle_ratio = static_cast<float>(time_passed) / time_to_rotate;

    return {position_, angle_ + ((dest_angle_ - angle_) * angle_ratio)};
  }

  int time_to_move =
      1000 * PTIT::GetDistance(destination_, position_) / kDefaultStraightSpeed;

  float dist_ratio = static_cast<float>(time_passed) / time_to_move;

  PTIT::Coord curr_position = {
      static_cast<int>(position_.x +
                       ((destination_.x - position_.x) * dist_ratio)),
      static_cast<int>(position_.y +
                       ((destination_.y - position_.y) * dist_ratio))};

  return {curr_position, dest_angle_};
}

bool Car::IsMoving() const {
  if (!started_moving_.has_value()) {
    return false;
  }

  if (movement_.IsMoving()) {
    return true;
  }

  position_ = destination_;
  angle_ = dest_angle_;
  started_moving_.reset();

  return false;
}

void Car::HeadTo(PTIT::Coord destination) {
  destination_ = destination;
  dest_angle_ = PTIT::Segment(position_, destination).GetAngle();

  movement_.Rotate(dest_angle_ - angle_);
  movement_.MoveStraight(PTIT::GetDistance(position_, destination_));

  started_moving_ = std::chrono::system_clock::now();
}

std::list<std::pair<float, int>> Car::Scan() {
  communicator_.SendMessage<AC::RadarScan>(1);

  std::list<std::pair<float, int>> scan;

  while (true) {
    auto data = communicator_.ReceiveMessage<AC::RadarScan>(1000);
    if (!data.has_value()) {
      continue;
    }
    if (data->second < 0) {
      break;
    }

    scan.push_back(data.value());
  }

  return scan;
}

/*------------------------------ free functions ------------------------------*/
void RouteMove(Car& car, const std::list<PTIT::Coord>& route, int mm_per_px) {
  for (auto iter = std::next(route.begin()); iter != route.end(); ++iter) {
    car.HeadTo(*iter * mm_per_px);
    while (car.IsMoving()) {
      std::this_thread::sleep_for(kFailSleep);
    }
  }
}

}  // namespace CM