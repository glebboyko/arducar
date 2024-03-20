#include "map/bitmap.hpp"

namespace BM {

/*------------------------------- base bitmap --------------------------------*/
Bitmap::Bitmap(int size_x, int size_y)
    : bitmap_(std::vector<std::vector<PixelData>>(
          size_x, std::vector<PixelData>(size_y))) {}

PTIT::Coord Bitmap::GetSize() const noexcept {
  return {static_cast<int>(bitmap_.size()),
          static_cast<int>(bitmap_[0].size())};
}

const Bitmap::ScanData& Bitmap::GetScanPoint(int x, int y) const noexcept {
  return bitmap_[x][y].scan;
}
const Bitmap::BFSData& Bitmap::GetBFSPoint(int x, int y) const noexcept {
  return bitmap_[x][y].bfs;
}

void Bitmap::SetScanData(int x, int y,
                         const Bitmap::ScanData& scan_data) noexcept {
  bitmap_[x][y].scan = scan_data;
}
void Bitmap::SetBFSData(int x, int y,
                        const Bitmap::BFSData& bfs_data) noexcept {
  bitmap_[x][y].bfs = bfs_data;
}

void Bitmap::CleanBFS() {
  for (auto& ordinates : bitmap_) {
    for (auto& [scan, bfs] : ordinates) {
      bfs = BFSData();
    }
  }
}

/*------------------------------ visual bitmap -------------------------------*/
VisualBitmap::VisualBitmap(int size_x, int size_y, unsigned char* scan_data,
                           unsigned char* scan_alpha,
                           unsigned char* border_alpha,
                           unsigned char* working_space_alpha,
                           unsigned char* route_alpha)
    : Bitmap(size_x, size_y),
      scan_data_(scan_data),
      scan_alpha_(scan_alpha),
      border_alpha_(border_alpha),
      working_space_alpha_(working_space_alpha),
      route_alpha_(route_alpha) {}

void VisualBitmap::SetScanData(int x, int y,
                               const BM::Bitmap::ScanData& scan_data) noexcept {
  Bitmap::SetScanData(x, y, scan_data);

  auto position = GetPosition(x, y);
  if (scan_data.is_border) {
    scan_alpha_[position] = 0;
    border_alpha_[position] = 255;
  } else {
    if (curr_sector_ != scan_data.call_identifier) {
      curr_sector_ = scan_data.call_identifier;
      sector_color_ = GetRandomColor();
    }

    scan_data_[position * 3] = sector_color_.red;
    scan_data_[position * 3 + 1] = sector_color_.green;
    scan_data_[position * 3 + 2] = sector_color_.blue;

    scan_alpha_[position] = 255;
    border_alpha_[position] = 0;
  }
}

void VisualBitmap::SetBFSData(int x, int y,
                              const BM::Bitmap::BFSData& bfs_data) noexcept {
  Bitmap::SetBFSData(x, y, bfs_data);

  auto position = GetPosition(x, y);
  if (!bfs_data.banned && bfs_data.visited) {
    working_space_alpha_[position] = 255;
  } else {
    working_space_alpha_[position] = 0;
  }
}

void VisualBitmap::CleanBFS() {
  Bitmap::CleanBFS();

  auto [size_x, size_y] = GetSize();
  for (int i = 0; i < size_x * size_y; ++i) {
    working_space_alpha_[i] = 0;
  }
}
void VisualBitmap::CleanRoute() {
  auto [size_x, size_y] = GetSize();
  for (int i = 0; i < size_x * size_y; ++i) {
    route_alpha_[i] = 0;
  }
}

void VisualBitmap::SetRouteData(int x, int y, bool visible) noexcept {
  route_alpha_[GetPosition(x, y)] = visible ? 255 : 0;
}

size_t VisualBitmap::GetPosition(int x, int y) noexcept {
  return GetSize().x * y + x;
}

/*------------------------------ free functions ------------------------------*/
PTIT::RGB GetRandomColor() noexcept {
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

}  // namespace BM