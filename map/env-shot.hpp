#pragma once

#include <image-creator.hpp>
#include <primitives.hpp>
#include <vector>

#include "bitmap.hpp"

class EnvShot {
 public:
  EnvShot(BM::Bitmap& bitmap, float px_per_mm, int border_thickness = 1);

  void AddMeasure(double deg, int mm_dist, double deg_width,
                  const PTIT::Coord& mm_radar_coord);

  void CreateImage(const char* image_name) const;

 private:
  float px_per_mm_;
  int border_thickness_;
  size_t measure_num_ = 0;

  BM::Bitmap& bitmap_;

  bool IsPixelInMap(int x, int y) const noexcept;
};