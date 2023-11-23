#include "image-creator.hpp"

ImageCreator::ImageCreator(float px_per_mm, float line_width,
                           const Primitive::Coord& size)
    : px_per_mm_(px_per_mm), line_width_(line_width * px_per_mm_), size_(size) {
  map_ = std::vector<std::vector<bool>>(size_.y,
                                        std::vector<bool>(size_.x, false));
}

void ImageCreator::Draw(const Primitive& primitive) {
  auto graph = primitive.GetGraphic(px_per_mm_, line_width_);

  for (const auto& point : graph) {
    try {
      map_.at(point.y).at(point.x) = true;
    } catch (...) {
    }
  }
}

void ImageCreator::CreateImage(const char* image_name) const {
  std::ofstream img_file(image_name);
  img_file << "P3"
           << "\n";
  img_file << size_.x << " " << size_.y << "\n";
  img_file << "255"
           << "\n";

  for (int y = size_.y - 1; y >= 0; --y) {
    for (int x = 0; x < size_.x; ++x) {
      int color = map_[y][x] ? 0 : 255;
      img_file << color << " " << color << " " << color << "\n";
    }
  }

  img_file.close();
}