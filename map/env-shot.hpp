#pragma once

#include <vector>

#include "image-creator.hpp"
#include "primitives.hpp"

class EnvShot {
 public:
  EnvShot(int size_x, int size_y, float px_per_mm, int border_thickness = 1,
          unsigned char* bitmap_scan_color = nullptr,
          unsigned char* bitmap_scan_alpha = nullptr,
          unsigned char* bitmap_border_alpha = nullptr);

  void AddMeasure(double deg, int mm_dist, double deg_width,
                  const PTIT::Coord& mm_radar_coord);

  void CreateImage(const char* image_name) const;

  static PTIT::RGB GetRandomColor() noexcept;

 private:
  struct PixelData {
    bool is_border = false;
    int dist = INT_MAX;
    size_t call_identifier = UINT64_MAX;
  };

  std::vector<std::vector<PixelData>> map_;

  std::vector<PTIT::RGB> sectors_color_;
  float px_per_mm_;
  int border_thickness_;

  unsigned char* bitmap_scan_color_;
  unsigned char* bitmap_scan_alpha_;
  unsigned char* bitmap_border_alpha_;

  void SetPixel(int x, int y, PTIT::RGB color) const noexcept;

  bool IsPixelInMap(int x, int y) const noexcept;
};

double DegToRad(double deg);