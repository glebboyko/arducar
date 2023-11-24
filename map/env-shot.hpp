#pragma once

#include <vector>

#include "image/primitives.hpp"

namespace EnvShot {

struct LaserRangeInf {
  int deg;
  int dist;
};

std::vector<Primitives::Segment> EnvShotProcessing(
    const std::vector<LaserRangeInf>& raw_env_shot,
    const Primitives::Coord& kCenter);

void DisplayImage(const std::vector<Primitives::Segment>& shot,
                  const Primitives::Coord& k_center,
                  const char* file_name);

}  // namespace EnvShot