#pragma once

#include <vector>

#include "primitives.hpp"

class EnvShot {
 public:
  EnvShot(int32_t size_x, int32_t size_y, float px_per_mm);

  void AddMeasure(int deg, int32_t mm_dist,
                  const Primitives::Coord& mm_radar_coord);

  void CreateImage(const char* image_name) const;

 private:
  struct PixelData {
    bool is_border = false;
    int32_t dist = INT32_MAX;
  };

  std::vector<std::vector<PixelData>> shot_;
  float px_per_mm_;
};