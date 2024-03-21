#include "control_center.hpp"
#include "runners.hpp"

const int kSizeX = 1500;
const int kSizeY = 1000;
const float kMmPerPx = 0.1;

int main() {
  CC::ControlCenter control_center(kSizeX, kSizeY, kMmPerPx);

  RNS::AutoMapBuilder(control_center);
}