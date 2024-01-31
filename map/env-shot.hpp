#pragma once

#include <vector>

#include "image-creator.hpp"
#include "primitives.hpp"

class EnvShot {
 public:
  EnvShot(int size_x, int size_y, float px_per_mm);

  void AddMeasure(double deg, int mm_dist, double deg_width,
                  const Primitives::Coord& mm_radar_coord);

  void CreateImage(const char* image_name) const;

  static RGB GetRandomColor() noexcept;

 private:
  struct PixelData {
    bool is_border = false;
    int dist = INT_MAX;
    size_t call_identifier = UINT64_MAX;
  };

  std::vector<std::vector<PixelData>> map_;

  std::vector<RGB> sectors_color_;
  float px_per_mm_;
};