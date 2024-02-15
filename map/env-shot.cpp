#include "env-shot.hpp"

#include <iostream>
#include <random>
#include <tuple>

#include "image-creator.hpp"
#include "primitives.hpp"

EnvShot::EnvShot(int size_x, int size_y, float px_per_mm, int border_thickness)
    : px_per_mm_(px_per_mm), border_thickness_(border_thickness) {
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

std::vector<PTIT::Coord> GetTriangle(PTIT::Coord a, PTIT::Coord b,
                                     PTIT::Coord c) {
  auto ab_graph = PTIT::Segment(a, b).GetGraphic();
  auto ac_graph = PTIT::Segment(a, c).GetGraphic();
  auto bc_graph = PTIT::Segment(b, c).GetGraphic();

  std::vector<PTIT::Coord> border;
  border.reserve(ab_graph.size() + ac_graph.size() + bc_graph.size());
  border.insert(border.cend(), ab_graph.begin(), ab_graph.end());
  border.insert(border.cend(), ac_graph.begin(), ac_graph.end());
  border.insert(border.cend(), bc_graph.begin(), bc_graph.end());

  std::sort(border.begin(), border.end(),
            [](const PTIT::Coord& first, const PTIT::Coord& second) {
              return first.y != second.y ? first.y < second.y
                                         : first.x < second.x;
            });

  std::vector<PTIT::Coord> triangle;
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
  std::sort(triangle.begin(), triangle.end(),
            [a](const PTIT::Coord& first, const PTIT::Coord& second) {
              auto f_dist = PTIT::GetDistance(a, first);
              auto s_dist = PTIT::GetDistance(a, second);
              if (f_dist != s_dist) {
                return f_dist < s_dist;
              }
              return first.y != second.y ? first.y < second.y
                                         : first.x < second.x;
            });
  return triangle;
}

void EnvShot::AddMeasure(double deg, int mm_dist, double deg_width,
                         const PTIT::Coord& mm_radar_coord,
                         unsigned char* bitmap) {
  double left_deg = deg - (deg_width / 2);
  double right_deg = deg + (deg_width / 2);

  double dist = static_cast<double>(mm_dist) * px_per_mm_;

  PTIT::Coord radar_coord = mm_radar_coord * px_per_mm_;

  PTIT::Coord left_coord = {static_cast<int>(mm_radar_coord.x * px_per_mm_ +
                                             (cos(DegToRad(left_deg)) * dist)),
                            static_cast<int>(mm_radar_coord.y * px_per_mm_ +
                                             (sin(DegToRad(left_deg)) * dist))};

  PTIT::Coord right_coord = {
      static_cast<int>(mm_radar_coord.x * px_per_mm_ +
                       (cos(DegToRad(right_deg)) * dist)),
      static_cast<int>(mm_radar_coord.y * px_per_mm_ +
                       (sin(DegToRad(right_deg)) * dist))};

  size_t call_num = sectors_color_.size();
  sectors_color_.push_back(GetRandomColor());

  auto border_graphic = PTIT::Segment(left_coord, right_coord).GetGraphic();
  for (auto iter = border_graphic.begin(); iter != --border_graphic.end();
       ++iter) {
    auto tmp_iter = iter;
    PTIT::Coord border[2] = {*tmp_iter, *(++tmp_iter)};

    bool valid = true;

    for (const auto& [x, y] : GetTriangle(radar_coord, border[0], border[1])) {
      if (!IsPixelInMap(x, y)) {
        continue;
      }

      PixelData& pixel = map_[x][y];

      if (pixel.call_identifier == call_num) {
        continue;
      }

      int px_dist = PTIT::GetDistance(radar_coord, {x, y});

      if (pixel.is_border && pixel.dist < px_dist) {
        valid = false;
        break;
      }
      if (pixel.dist >= px_dist) {
        pixel.dist = px_dist;
        pixel.is_border = false;
        pixel.call_identifier = call_num;
        SetPixel(bitmap, x, y, sectors_color_.back());
      }
    }

    if (!valid) {
      continue;
    }
    for (int i = 0; i < 2; ++i) {
      if (!IsPixelInMap(border[i].x, border[i].y)) {
        continue;
      }
      double px_dist = PTIT::GetDistance(radar_coord, border[i]);
      PixelData& pixel = map_[border[i].x][border[i].y];
      if (pixel.dist >= px_dist || pixel.call_identifier == call_num) {
        for (int xb = border[i].x - (border_thickness_ / 2);
             xb <=
             border[i].x + (border_thickness_ / 2) + (border_thickness_ % 2);
             ++xb) {
          for (int yb = border[i].y - (border_thickness_ / 2);
               yb <=
               border[i].y + (border_thickness_ / 2) + (border_thickness_ % 2);
               ++yb) {
            if (!IsPixelInMap(xb, yb)) {
              continue;
            }
            pixel = map_[xb][yb];
            pixel.dist = px_dist;
            pixel.is_border = true;
            pixel.call_identifier = call_num;
            SetPixel(bitmap, xb, yb, {0, 0, 0});
          }
        }
      }
    }
  }
}

void EnvShot::CreateImage(const char* image_name) const {
  ::PTIT::CreateImage(image_name, map_, map_.size(), map_[0].size(),
                [this](const PixelData& data) {
                  if (data.call_identifier == UINT64_MAX) {
                    return PTIT::RGB{255, 255, 255};
                  }
                  if (data.is_border) {
                    return PTIT::RGB{0, 0, 0};
                  }
                  return sectors_color_[data.call_identifier];
                });
}

PTIT::RGB EnvShot::GetRandomColor() noexcept {
  static bool uninitialized = true;
  if (uninitialized) {
    srand(time(NULL));
    uninitialized = false;
  }

  PTIT::RGB color;
  color.red = rand() % (PTIT::RGB::kMaxColor / 2) + (PTIT::RGB::kMaxColor / 2);
  color.green =
      rand() % (PTIT::RGB::kMaxColor / 2) + (PTIT::RGB::kMaxColor / 2);
  color.blue = rand() % (PTIT::RGB::kMaxColor / 2) + (PTIT::RGB::kMaxColor / 2);

  short color_sum = color.red + color.green + color.blue;
  if (color_sum > PTIT::RGB::kMaxColor * 3 - 90) {
    short diff = color_sum - (PTIT::RGB::kMaxColor + 90);
    color.red -= diff, color.green -= diff, color.blue -= diff;
  }

  return color;
}

void EnvShot::SetPixel(unsigned char* bitmap, int x, int y,
                       PTIT::RGB color) const noexcept {
  if (bitmap != nullptr) {
    long pos = (map_.size() * y + x) * 3;
    bitmap[pos] = color.red;
    bitmap[pos + 1] = color.green;
    bitmap[pos + 2] = color.blue;
  }
}

bool EnvShot::IsPixelInMap(int x, int y) const noexcept {
  return x >= 0 && y >= 0 && x < map_.size() && y < map_[0].size();
}