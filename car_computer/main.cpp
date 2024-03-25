#include "constants.hpp"
#include "control_center.hpp"
#include "runners.hpp"

int main() {
  CC::ControlCenter control_center(CONST::kPxSizeX, CONST::kPxSizeY,
                                   CONST::kMmPerPx);

  RNS::AutoMapBuilder(control_center);
}