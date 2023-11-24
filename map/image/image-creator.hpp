#pragma once

#include <fstream>

#include "primitives.hpp"

class ImageCreator {
 public:
  ImageCreator(float px_per_mm, float line_width,
               const Primitives::Primitive::Coord& size);

  void Draw(const Primitives::Primitive& primitive);

  void CreateImage(const char* image_name) const;

 private:
  float px_per_mm_;
  int line_width_;
  Primitives::Primitive::Coord size_;

  std::vector<std::vector<bool>> map_;
};