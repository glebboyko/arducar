#pragma once

#include <vector>

#include "primitives.hpp"

class EnvShot {
 public:
  EnvShot(size_t size_x, size_t size_y, float px_per_mm);

  void AddMeasure(int deg, int mm_dist,
                  const Primitives::Coord& mm_radar_coord);

  void CreateImage(const char* image_name) const;

 private:
  struct PixelData {
    bool is_empty;
    int dist;
  };

  std::vector<std::vector<PixelData>> shot_;
};