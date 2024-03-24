#pragma once

#include <limits.h>

#include <image-creator.hpp>
#include <primitives.hpp>
#include <random>
#include <vector>

namespace BM {

PTIT::RGB GetRandomColor() noexcept;

class Bitmap {
 public:
  struct ScanData {
    bool is_border = false;
    int dist = INT_MAX;
    size_t call_identifier = UINT64_MAX;
  };
  struct BFSData {
    bool banned = false;
    bool visited = false;
    PTIT::Coord parent;
  };

  Bitmap(int size_x, int size_y, float mm_per_px);

  PTIT::Coord GetSize() const noexcept;

  const ScanData& GetScanPoint(int x, int y) const noexcept;
  const BFSData& GetBFSPoint(int x, int y) const noexcept;

  virtual void SetScanData(int x, int y, const ScanData& scan_data) noexcept;
  virtual void SetBFSData(int x, int y, const BFSData& bfs_data) noexcept;

  virtual void CleanBFS();

  bool IsPointInRange(int x, int y) const noexcept;
  float GetDensity() const noexcept;

 private:
  struct PixelData {
    ScanData scan;
    BFSData bfs;
  };

  std::vector<std::vector<PixelData>> bitmap_;
  float mm_per_px_;
};

class VisualBitmap : public Bitmap {
 public:
  VisualBitmap(int size_x, int size_y, float mm_per_px, int line_width,
               unsigned char* scan_data, unsigned char* scan_alpha,
               unsigned char* border_alpha, unsigned char* working_space_alpha,
               unsigned char* route_alpha);

  virtual void SetScanData(int x, int y,
                           const ScanData& scan_data) noexcept override;
  virtual void SetBFSData(int x, int y,
                          const BFSData& bfs_data) noexcept override;

  virtual void CleanBFS() override;
  void CleanRoute();

  void SetRouteData(int x, int y, bool visible) noexcept;

 private:
  size_t curr_sector_ = 0;
  PTIT::RGB sector_color_ = GetRandomColor();

  int line_width_;

  unsigned char* scan_data_;
  unsigned char* scan_alpha_;

  unsigned char* border_alpha_;

  unsigned char* working_space_alpha_;

  unsigned char* route_alpha_;

  size_t GetPosition(int x, int y) noexcept;
};

}  // namespace BM