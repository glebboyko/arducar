#pragma once

#include <list>
#include <optional>
#include <primitives.hpp>

#include "bitmap.hpp"

namespace MA {

class WorkingAreaProcessor {
 public:
  WorkingAreaProcessor(BM::Bitmap& bitmap, int border_offset,
                       int dist_threshold);
  std::optional<PTIT::Coord> ProcessArea(const PTIT::Coord& position);

 private:
  BM::Bitmap& bitmap_;
  int border_offset_;
  int dist_threshold_;

  void SetBannedArea();
  std::list<PTIT::Coord> GetNeighbours(const PTIT::Coord& coord) const;
};

std::list<PTIT::Coord> GetRoute(const BM::Bitmap& bitmap,
                                const PTIT::Coord& destination);

}  // namespace MA