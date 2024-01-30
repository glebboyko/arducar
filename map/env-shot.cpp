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

  Primitives::Segment border(left_coord, right_coord);
  for (const auto& border_coord : border.GetGraphic()) {
    Primitives::Segment generatrix(radar_coord, border_coord);
    bool valid = true;

    for (const auto& [x, y] : generatrix.GetGraphic()) {
      if (x < 0 || y < 0 || x >= map_.size() || y >= map_[0].size()) {
        continue;
      }

      PixelData& pixel = map_[x][y];
      int px_dist = Primitives::GetDistance(radar_coord, {x, y});

      if (pixel.is_border && pixel.dist <= px_dist) {
        valid = false;
        break;
      } else if (pixel.dist > px_dist) {
        pixel.dist = px_dist;
        pixel.is_border = false;
      }
    }

    if (!valid) {
      continue;
    }

    double px_dist = Primitives::GetDistance(radar_coord, border_coord);
    PixelData& pixel = map_[border_coord.x][border_coord.y];
    if (pixel.dist > px_dist) {
      pixel.dist = px_dist;
      pixel.is_border = true;
    }
  }
}

void EnvShot::CreateImage(const char* image_name) const {
  ::CreateImage(
      image_name, map_, map_.size(), map_[0].size(), [](const PixelData& data) {
        return data.is_border ? std::tuple(0, 0, 0) : std::tuple(255, 255, 255);
      });
}