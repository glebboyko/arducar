#pragma once

#include <vector>

#include "image-creator.hpp"
#include "primitives.hpp"

class EnvShot {
 public:
  EnvShot(int size_x, int size_y, float px_per_mm, int border_thickness = 1);

  void AddMeasure(double deg, int mm_dist, double deg_width,
                  const Primitives::Coord& mm_radar_coord,
                  unsigned char* bitmap = nullptr);

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
  int border_thickness_;

  void SetPixel(unsigned char* bitmap, int x, int y, RGB color) const noexcept;

  bool IsPixelInMap(int x, int y) const noexcept;
};

double DegToRad(double deg);