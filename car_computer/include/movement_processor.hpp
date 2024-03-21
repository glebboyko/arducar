#pragma once

#include <list>
#include <primitives.hpp>

namespace MP {

class MovementProcessor {
 public:
  MovementProcessor();

  PTIT::Coord GetPosition();
  bool IsMoving();

  void RouteMove(const std::list<PTIT::Coord>& route);

 private:
};

}  // namespace MP