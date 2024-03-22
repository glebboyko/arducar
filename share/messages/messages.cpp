#include "messages.hpp"

#include <iostream>

namespace MSG {

std::istream& operator>>(std::istream& is, InitData& init) {
  return is >> init.px_size_x >> init.px_size_y >> init.mm_per_px >>
         init.border_offset >> init.dist_threshold;
}
std::ostream& operator<<(std::ostream& os, const InitData& init) {
  return os << init.px_size_x << " " << init.px_size_y << " " << init.mm_per_px
            << " " << init.border_offset << " " << init.dist_threshold;
}

std::ostream& operator<<(std::ostream& os, const Message& message) {
  os << message.GetType() << " ";
  message.PrintData(os);
  return os;
}

std::istream& operator>>(std::istream& is, MessageContainer& container) {
  int type;
  is >> type;
  switch (static_cast<MessageType>(type)) {
    case TPosition:
      container.message = std::shared_ptr<Message>(new Position(is));
      break;
    case TScan:
      container.message = std::shared_ptr<Message>(new Scan(is));
      break;
    case TDestination:
      container.message = std::shared_ptr<Message>(new Destination(is));
      break;
    case TRoute:
      container.message = std::shared_ptr<Message>(new Route(is));
      break;
    case TStatus:
      container.message = std::shared_ptr<Message>(new Status(is));
      break;
    case TAction:
      container.message = std::shared_ptr<Message>(new Action(is));
      break;
  }
  return is;
}

Position::Position(PTIT::Coord posistion, float angle)
    : position(posistion), angle(angle) {}
Position::Position(std::istream& is) {
  is >> position.x >> position.y >> angle;
}
void Position::PrintData(std::ostream& os) const {
  os << position.x << " " << position.y << " " << angle;
}
MessageType Position::GetType() const { return TPosition; }

Scan::Scan(float angle, int dist) : angle(angle), dist(dist) {}
Scan::Scan(std::istream& is) { is >> angle >> dist; }
void Scan::PrintData(std::ostream& os) const { os << angle << " " << dist; }
MessageType Scan::GetType() const { return TScan; }

Destination::Destination(PTIT::Coord destination) : destination(destination) {}
Destination::Destination(std::istream& is) {
  is >> destination.x >> destination.y;
}
void Destination::PrintData(std::ostream& os) const {
  os << destination.x << " " << destination.y;
}
MessageType Destination::GetType() const { return TDestination; }

Route::Route(const std::list<PTIT::Coord>& route) : route(route) {}
Route::Route(std::istream& is) {
  int size;
  is >> size;
  for (int i = 0; i < size; ++i) {
    PTIT::Coord coord;
    is >> coord.x >> coord.y;
    route.push_back(std::move(coord));
  }
}
void Route::PrintData(std::ostream& os) const {
  os << route.size();
  for (const auto& coord : route) {
    os << coord.x << " " << coord.y << " ";
  }
}
MessageType Route::GetType() const { return TRoute; }

Status::Status(MSG::CurrStatus status) : status(status) {}
Status::Status(std::istream& is) {
  int tmp_status;
  is >> tmp_status;
  status = static_cast<CurrStatus>(tmp_status);
}
void Status::PrintData(std::ostream& os) const { os << status; }
MessageType Status::GetType() const { return TStatus; }

Action::Action(MSG::CurrAction action) : action(action) {}
Action::Action(std::istream& is) {
  int tmp_action;
  is >> tmp_action;
  action = static_cast<CurrAction>(tmp_action);
}
void Action::PrintData(std::ostream& os) const { os << action; }
MessageType Action::GetType() const { return TAction; }

}  // namespace MSG
