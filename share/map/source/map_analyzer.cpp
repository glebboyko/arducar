#include "map/map_analyzer.hpp"

#include <queue>

namespace MA {

/*-------------------------- working area processor --------------------------*/
WorkingAreaProcessor::WorkingAreaProcessor(BM::Bitmap& bitmap,
                                           int border_offset)
    : bitmap_(bitmap), border_offset_(border_offset) {}

std::optional<PTIT::Coord> WorkingAreaProcessor::ProcessArea(
    const PTIT::Coord& position, int dist_threshold) {
  bitmap_.CleanBFS();
  SetBannedArea();

  std::optional<PTIT::Coord> destination;

  std::queue<PTIT::Coord> bfs;

  bfs.push(position);
  BM::Bitmap::BFSData curr_pixel = bitmap_.GetBFSPoint(position.x, position.y);
  curr_pixel.visited = true, curr_pixel.parent = position;
  bitmap_.SetBFSData(position.x, position.y, curr_pixel);

  while (!bfs.empty()) {
    auto curr_point = bfs.front();
    bfs.pop();

    if (!bitmap_.GetBFSPoint(curr_point.x, curr_point.y).banned &&
        bitmap_.GetScanPoint(curr_point.x, curr_point.y).dist >=
            dist_threshold &&
        !destination.has_value()) {
      destination = curr_point;
    }

    bool neigh_has_border = false;
    auto neighs = GetNeighbours(curr_point);
    for (auto neigh : neighs) {
      if (bitmap_.GetScanPoint(neigh.x, neigh.y).is_border) {
        neigh_has_border = true;
        break;
      }
    }
    if (neigh_has_border) {
      continue;
    }

    for (auto neigh_point : neighs) {
      BM::Bitmap::BFSData neigh_pixel =
          bitmap_.GetBFSPoint(neigh_point.x, neigh_point.y);
      if (neigh_pixel.visited) {
        continue;
      }

      neigh_pixel.visited = true, neigh_pixel.parent = curr_point;
      bitmap_.SetBFSData(neigh_point.x, neigh_point.y, neigh_pixel);

      bfs.push(neigh_point);
    }
  }

  return destination;
}

void WorkingAreaProcessor::SetBannedArea() {
  auto [x_size, y_size] = bitmap_.GetSize();
  auto extracted_segms = PTIT::ExtractPrimitives(
      bitmap_, x_size, y_size, [](const BM::Bitmap& bitmap, int x, int y) {
        return bitmap.GetScanPoint(x, y).is_border;
      });

  for (auto segm : extracted_segms) {
    for (auto [x, y] : segm.GetArea(border_offset_)) {
      if (!IsPointInPole(x, y)) {
        continue;
      }

      bitmap_.SetBFSData(x, y, {.banned = true});
    }
  }
}

std::list<PTIT::Coord> WorkingAreaProcessor::GetNeighbours(
    const PTIT::Coord& coord) const {
  std::list<PTIT::Coord> neighbours;

  for (int x = coord.x - 1; x <= coord.x + 1; ++x) {
    for (int y = coord.y - 1; y <= coord.y; ++y) {
      if (!IsPointInPole(x, y) || (x == coord.x && y == coord.y)) {
        continue;
      }
      neighbours.push_back({x, y});
    }
  }

  return neighbours;
}

bool WorkingAreaProcessor::IsPointInPole(int x, int y) const {
  auto [x_size, y_size] = bitmap_.GetSize();

  return x >= 0 && y >= 0 && x < x_size && y < y_size;
}

/*------------------------------- route getter -------------------------------*/
std::list<PTIT::Coord> GetRoute(const BM::Bitmap& bitmap,
                                const PTIT::Coord& destination) {
  std::list<PTIT::Coord> route;
  route.push_front(destination);

  PTIT::Coord curr_coord = destination;

  while (bitmap.GetBFSPoint(curr_coord.x, curr_coord.y).parent != curr_coord) {
    curr_coord = bitmap.GetBFSPoint(curr_coord.x, curr_coord.y).parent;
    route.push_front(curr_coord);
  }

  return route;
}

}  // namespace MA