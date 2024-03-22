#include "map/env-shot.hpp"

#include <algorithm>
#include <map>
#include <tuple>

#include "ptit_lib/source/supply.hpp"

EnvShot::EnvShot(BM::Bitmap& bitmap) : bitmap_(bitmap) {}

std::vector<PTIT::Coord> GetSortedTriangle(const PTIT::Coord& base,
                                           const PTIT::Coord& b,
                                           const PTIT::Coord& c) {
  auto list_sector = PTIT::FulfillArea(PTIT::Triangle(base, b, c).GetGraphic());

  std::vector<PTIT::Coord> vec_sector;
  vec_sector.reserve(list_sector.size());
  vec_sector.assign(list_sector.begin(), list_sector.end());

  std::sort(vec_sector.begin(), vec_sector.end(),
            [base](const PTIT::Coord& first, const PTIT::Coord& second) {
              auto f_dist = PTIT::GetDistance(base, first);
              auto s_dist = PTIT::GetDistance(base, second);
              if (f_dist != s_dist) {
                return f_dist < s_dist;
              }
              return first.y != second.y ? first.y < second.y
                                         : first.x < second.x;
            });

  return vec_sector;
}

void EnvShot::AddMeasure(double deg, int mm_dist, double deg_width,
                         const PTIT::Coord& mm_radar_coord) {
  deg *= -1;
  double left_deg = deg - (deg_width / 2);
  double right_deg = deg + (deg_width / 2);

  double px_dist = static_cast<double>(mm_dist) / bitmap_.GetDensity();

  PTIT::Coord radar_coord = mm_radar_coord / bitmap_.GetDensity();

  PTIT::Coord left_coord = {
      static_cast<int>(radar_coord.x +
                       (cos(PTIT::DegToRad(left_deg)) * px_dist)),
      static_cast<int>(radar_coord.y +
                       (sin(PTIT::DegToRad(left_deg)) * px_dist))};

  PTIT::Coord right_coord = {
      static_cast<int>(radar_coord.x +
                       (cos(PTIT::DegToRad(right_deg)) * px_dist)),
      static_cast<int>(radar_coord.y +
                       (sin(PTIT::DegToRad(right_deg)) * px_dist))};

  auto border_graphic = PTIT::Segment(left_coord, right_coord).GetGraphic();
  for (auto iter = border_graphic.begin(); iter != --border_graphic.end();
       ++iter) {
    PTIT::Coord border[2] = {*iter, *(std::next(iter))};

    bool valid = true;

    for (const auto& [x, y] :
         GetSortedTriangle(radar_coord, border[0], border[1])) {
      if (!bitmap_.IsPointInRange(x, y)) {
        continue;
      }

      BM::Bitmap::ScanData pixel = bitmap_.GetScanPoint(x, y);

      if (pixel.call_identifier == measure_num_) {
        continue;
      }

      int dist = PTIT::GetDistance(mm_radar_coord,
                                   PTIT::Coord(x, y) * bitmap_.GetDensity());

      if (pixel.is_border && pixel.dist < dist) {
        valid = false;
        break;
      }
      if (pixel.dist >= dist) {
        pixel.dist = dist;
        pixel.is_border = false;
        pixel.call_identifier = measure_num_;
      }
      bitmap_.SetScanData(x, y, pixel);
    }

    if (!valid) {
      continue;
    }
    for (int i = 0; i < 2; ++i) {
      if (!bitmap_.IsPointInRange(border[i].x, border[i].y)) {
        continue;
      }
      int dist =
          PTIT::GetDistance(mm_radar_coord, border[i] * bitmap_.GetDensity());
      BM::Bitmap::ScanData pixel =
          bitmap_.GetScanPoint(border[i].x, border[i].y);
      if (pixel.dist >= dist || pixel.call_identifier == measure_num_) {
        pixel = bitmap_.GetScanPoint(border[i].x, border[i].y);
        pixel.dist = dist;
        pixel.is_border = true;
        pixel.call_identifier = measure_num_;
        bitmap_.SetScanData(border[i].x, border[i].y, pixel);
      }
    }
  }

  measure_num_ += 1;
}

void EnvShot::CreateImage(const char* image_name) const {
  auto [x_size, y_size] = bitmap_.GetSize();
  std::map<size_t, PTIT::RGB> colors;
  PTIT::CreateImage(image_name, bitmap_, x_size, y_size,
                    [this, &colors](const BM::Bitmap& bitmap, int x, int y) {
                      auto pixel = bitmap.GetScanPoint(x, y);
                      if (pixel.call_identifier == UINT64_MAX) {
                        return PTIT::RGB{255, 255, 255};
                      }
                      if (pixel.is_border) {
                        return PTIT::RGB{0, 0, 0};
                      }
                      if (colors.contains(pixel.call_identifier)) {
                        return colors.operator[](pixel.call_identifier);
                      }
                      auto curr_color = BM::GetRandomColor();
                      colors.insert({pixel.call_identifier, curr_color});
                      return curr_color;
                    });
}