#include "env-shot.hpp"

#include "image/image-creator.hpp"

namespace EnvShot {

const double kPi = 3.1415926535;
const int kDegInCircle = 360;

double DegToRad(double deg) {
  double rad = deg * kPi / (kDegInCircle / 2);
  return rad;
}

std::vector<Primitives::Segment> EnvShotProcessing(
    const std::vector<LaserRangeInf>& raw_env_shot,
    const Primitives::Coord& kCenter) {
  std::vector<Primitives::Segment> result;
  result.reserve(raw_env_shot.size());

  for (int i = 0; i < raw_env_shot.size(); ++i) {
    int curr_deg = raw_env_shot[i].deg;
    int prev_deg = (i > 0 ? raw_env_shot[i - 1] : raw_env_shot.back()).deg;
    if (prev_deg > curr_deg) {
      prev_deg += kDegInCircle;
    }

    int next_deg = (i + 1 != raw_env_shot.size() ? raw_env_shot[i + 1]
                                                 : raw_env_shot.front())
                       .deg;
    if (next_deg < curr_deg) {
      next_deg += kDegInCircle;
    }

    int left_deg = (prev_deg + curr_deg) / 2;
    int right_deg = (curr_deg + next_deg) / 2;
    int dist = raw_env_shot[i].dist;

    Primitives::Coord left_coord = {
        static_cast<int>(kCenter.x + (cos(DegToRad(left_deg)) * dist)),
        static_cast<int>(kCenter.y + (sin(DegToRad(left_deg)) * dist))};

    Primitives::Coord right_coord = {
        static_cast<int>(kCenter.x + (cos(DegToRad(right_deg)) * dist)),
        static_cast<int>(kCenter.y + (sin(DegToRad(right_deg)) * dist))};

    result.push_back(Primitives::Segment(left_coord, right_coord));
  }
  return result;
}

void DisplayImage(const std::vector<Primitives::Segment>& shot,
                  const Primitives::Coord& k_center,
                  const char* file_name) {
  ImageCreator image_creator(1, 1, k_center * 2);

  for (int i = 0; i < shot.size(); ++i) {
    image_creator.Draw(shot[i]);
    Primitives::Segment connector(shot[i].GetB(),
                                  shot[(i + 1) % shot.size()].GetA());
    image_creator.Draw(connector);
  }

  image_creator.CreateImage(file_name);
}

}  // namespace EnvShot