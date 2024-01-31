#include "env-shot.hpp"

#include <tuple>

#include "image-creator.hpp"
#include "primitives.hpp"

EnvShot::EnvShot(int size_x, int size_y, float px_per_mm)
    : px_per_mm_(px_per_mm) {
  map_.resize(size_x);
  for (int32_t i = 0; i < size_y; ++i) {
    map_[i].resize(size_y);
  }
}

const double kPi = 3.1415926535;
const int kDegInCircle = 360;

double DegToRad(double deg) {
  double rad = deg * kPi / (kDegInCircle / 2);
  return rad;
}

void FulfillLine(std::vector<Primitives::Coord>& figure,
                 const Primitives::Coord& right_coord) {}

std::vector<Primitives::Coord> GetTriangle(Primitives::Coord a,
                                           Primitives::Coord b,
                                           Primitives::Coord c) {
  auto ab_graph = Primitives::Segment(a, b).GetGraphic();
  auto ac_graph = Primitives::Segment(a, c).GetGraphic();
  auto bc_graph = Primitives::Segment(b, c).GetGraphic();

  std::vector<Primitives::Coord> border;
  border.reserve(ab_graph.size() + ac_graph.size() + bc_graph.size());
  border.insert(border.cend(), ab_graph.begin(), ab_graph.end());
  border.insert(border.cend(), ac_graph.begin(), ac_graph.end());
  border.insert(border.cend(), bc_graph.begin(), bc_graph.end());

  std::sort(
      border.begin(), border.end(),
      [](const Primitives::Coord& first, const Primitives::Coord& second) {
        return first.y != second.y ? first.y < second.y : first.x < second.x;
      });

  std::vector<Primitives::Coord> triangle;
  triangle.reserve(border.size());
  for (const auto& coord : border) {
    if (triangle.empty() || triangle.back().y != coord.y) {
      triangle.push_back(coord);
      continue;
    }
    while (triangle.back().x < coord.x) {
      triangle.push_back({triangle.back().x + 1, triangle.back().y});
    }
  }
  std::sort(
      triangle.begin(), triangle.end(),
      [a](const Primitives::Coord& first, const Primitives::Coord& second) {
        auto f_dist = Primitives::GetDistance(a, first);
        auto s_dist = Primitives::GetDistance(a, second);
        if (f_dist != s_dist) {
          return f_dist < s_dist;
        }
        return first.y != second.y ? first.y < second.y : first.x < second.x;
      });
  return triangle;
}

void EnvShot::AddMeasure(double deg, int mm_dist, double deg_width,
                         const Primitives::Coord& mm_radar_coord) {
  double left_deg = deg - (deg_width / 2);
  double right_deg = deg + (deg_width / 2);

  double dist = static_cast<double>(mm_dist) * px_per_mm_;

  Primitives::Coord radar_coord = mm_radar_coord * px_per_mm_;

  Primitives::Coord left_coord = {
      static_cast<int>(mm_radar_coord.x * px_per_mm_ +
                       (cos(DegToRad(left_deg)) * dist)),
      static_cast<int>(mm_radar_coord.y * px_per_mm_ +
                       (sin(DegToRad(left_deg)) * dist))};

  Primitives::Coord right_coord = {
      static_cast<int>(mm_radar_coord.x * px_per_mm_ +
                       (cos(DegToRad(right_deg)) * dist)),
      static_cast<int>(mm_radar_coord.y * px_per_mm_ +
                       (sin(DegToRad(right_deg)) * dist))};

  static size_t call_num = 0;
  call_num = (call_num + 1) % UINT64_MAX;

  auto border_graphic =
      Primitives::Segment(left_coord, right_coord).GetGraphic();
  for (auto iter = border_graphic.begin(); iter != --border_graphic.end();
       ++iter) {
    auto tmp_iter = iter;
    Primitives::Coord border[2] = {*tmp_iter, *(++tmp_iter)};

    bool valid = true;

    for (const auto& [x, y] : GetTriangle(radar_coord, border[0], border[1])) {
      if (x < 0 || y < 0 || x >= map_.size() || y >= map_[0].size()) {
        continue;
      }

      PixelData& pixel = map_[x][y];

      if (pixel.call_identifier == call_num) {
        continue;
      }

      int px_dist = Primitives::GetDistance(radar_coord, {x, y});

      if (pixel.is_border && pixel.dist < px_dist) {
        valid = false;
        break;
      }
      if (pixel.dist >= px_dist) {
        pixel.dist = px_dist;
        pixel.is_border = false;
        pixel.call_identifier = call_num;
      }
    }

    if (!valid) {
      continue;
    }
    for (int i = 0; i < 2; ++i) {
      if (border[i].x < 0 || border[i].y < 0 || border[i].x >= map_.size() ||
          border[i].y >= map_[0].size()) {
        continue;
      }

      double px_dist = Primitives::GetDistance(radar_coord, border[i]);
      PixelData& pixel = map_[border[i].x][border[i].y];
      if (pixel.dist >= px_dist || pixel.call_identifier == call_num) {
        pixel.dist = px_dist;
        pixel.is_border = true;
        pixel.call_identifier = call_num;
      }
    }
  }
}

void EnvShot::CreateImage(const char* image_name) const {
  ::CreateImage(
      image_name, map_, map_.size(), map_[0].size(), [](const PixelData& data) {
        return data.is_border ? std::tuple(0, 0, 0) : std::tuple(255, 255, 255);
      });
}