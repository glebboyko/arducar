#pragma once

#include <string>

#include "primitives.hpp"

namespace SPRT {

const std::string kServerAddress = "192.168.1.100";
const int kTcpPort = 3470;

enum TcpClientTypes { Visualizer, Arduino };

enum QueryType {
  QRadarScan,  // {Позиция радара, Угол поворота, Ширина снимка, Расстояние}
  QWorkingArea,  // std::list<Coord> границ
  QCarPosition,  // {Позиция машинки, угол поворота машинки}
  QDestination,  // Позиция точки назначения
  QRoute,        // std::list<Coord> точки маршрута
  QStatus        // std::string статус
};

struct RadarScan {
  PTIT::Coord position;
  double deg;
  double radar_width;

  int distance;
};

using WorkingArea = std::list<PTIT::Coord>;

struct CarPosition {
  PTIT::Coord position;
  double deg;
};

using Destination = PTIT::Coord;

using Route = std::list<PTIT::Coord>;

using Status = std::string;

}  // namespace SPRT