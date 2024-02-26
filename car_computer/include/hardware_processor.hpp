#pragma once

#include <chrono>
#include <optional>
#include <primitives.hpp>

#include "arduino_communicator.hpp"

namespace CM {

const int kDefaultStraightSpeed = 150;  // mm / s
const int kDefaultAngularSpeed = 45;    // deg / s

const int kCarWidth = 210;      // mm
const float kWheelLen = 210.5;  // mm

const std::chrono::milliseconds kFailSleep(100);

class Movement {
 public:
  Movement(AC::ArduinoCommunicator& communicator);

  void MoveStraight(int dist, int speed = 0);
  void Rotate(float angle, int speed = 0);

  bool IsMoving();

 private:
  AC::ArduinoCommunicator& communicator_;
  bool is_moving_ = false;
};

class Car {
 public:
  Car(PTIT::Coord position);

  std::pair<PTIT::Coord, float> GetPosition() const;
  bool IsMoving() const;

  void HeadTo(PTIT::Coord destination);
  std::list<std::pair<float, int>> Scan();

 private:
  mutable AC::ArduinoCommunicator communicator_;
  mutable Movement movement_ = Movement(communicator_);

  mutable PTIT::Coord position_;
  mutable float angle_ = 0;

  PTIT::Coord destination_;
  float dest_angle_;

  mutable std::optional<std::chrono::time_point<std::chrono::system_clock>>
      started_moving_;
};

void RouteMove(Car& car, const std::list<PTIT::Coord>& route, int mm_per_px);

}  // namespace CM