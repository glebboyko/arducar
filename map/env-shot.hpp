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
    const Primitives::Primitive::Coord& kCenter);

}  // namespace EnvShot