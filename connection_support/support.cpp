#include "support.hpp"

#include <iostream>

namespace SPRT {

std::istream& operator>>(std::istream& is, Message& message) {
  message.ScanData(is);
  return is;
}
std::ostream& operator<<(std::ostream& os, const Message& message) {
  message.PrintData(os);
  return os;
}

Position::Position(PTIT::Coord posistion, float angle)
    : position(posistion), angle(angle) {}

void Position::PrintData(std::ostream& os) const {
  os << position.x << position.y << angle;
}
void Position::ScanData(std::istream& is) {
  is >> position.x >> position.y >> angle;
}

RadarScan::RadarScan(float angle, int dist) : angle(angle), dist(dist) {}

void RadarScan::PrintData(std::ostream& os) const { os << angle << dist; }
void RadarScan::ScanData(std::istream& is) { is >> angle >> dist; }

Destination::Destination(PTIT::Coord destination) : destination(destination) {}

void Destination::PrintData(std::ostream& os) const {
  os << destination.x << destination.y;
}
void Destination::ScanData(std::istream& is) {
  is >> destination.x >> destination.y;
}

Route::Route(const std::list<PTIT::Coord>& route) : route(route) {}

void Route::PrintData(std::ostream& os) const {
  os << route.size();
  for (auto [x, y] : route) {
    os << x << y;
  }
}
void Route::ScanData(std::istream& is) {
  size_t size;
  is >> size;
  for (size_t i = 0; i < size; ++i) {
    PTIT::Coord coord;
    is >> coord.x >> coord.y;
    route.push_back(coord);
  }
}

Condition::Condition(SPRT::Status status, SPRT::CurrAction action)
    : status(status), action(action) {}

void Condition::PrintData(std::ostream& os) const { os << status << action; }
void Condition::ScanData(std::istream& is) {
  int tmp_status;
  int tmp_action;
  is >> tmp_status >> tmp_action;

  status = static_cast<Status>(tmp_status);
  action = static_cast<CurrAction>(tmp_action);
}

}  // namespace SPRT
