#include "env-shot.hpp"
#include "map_analyzer.hpp"
#include "modules.hpp"

namespace ML {

void MainLoop(ClientCommunication& client, BM::Bitmap& bitmap, CM::Car& car) {
  SPRT::Status status = SPRT::Act;

  EnvShot env_shot(bitmap, 1.0 / kMmPerPx);
  MA::WorkingAreaProcessor working_area(bitmap, kBorderOffset);

  while (true) {
    auto new_status = client.Receive();
    if (new_status.has_value()) {
      new_status = status;
    }

    {
      struct SPRT::Condition condition(status, SPRT::Scanning);
      client.Send(SPRT::Condition, condition);
    }

    if (status == SPRT::Pause) {
      std::this_thread::sleep_for(kSleepFail);
      continue;
    }
    if (status == SPRT::Stop) {
      break;
    }

    {
      struct SPRT::Position position(car.GetPosition().first,
                                     car.GetPosition().second);
      client.Send(SPRT::Position, position);
    }
    auto scan = car.Scan();
    {
      for (auto [angle, dist] : scan) {
        struct SPRT::RadarScan radar_scan(angle, dist);
        client.Send(SPRT::Scan, radar_scan);
      }
      struct SPRT::Condition condition(status, SPRT::Computing);
      client.Send(SPRT::Condition, condition);
    }

    for (auto [angle, dist] : scan) {
      env_shot.AddMeasure(angle, dist, kRadarWidth, car.GetPosition().first);
    }

    auto destination =
        working_area.ProcessArea(car.GetPosition().first, kBorderOffset);
    if (!destination.has_value()) {
      status = SPRT::Stop;
      struct SPRT::Condition condition(status, SPRT::Finish);
      return;
    }

    auto route = MA::GetRoute(bitmap, destination.value());
    {
      struct SPRT::Route client_route(route);
      client.Send(SPRT::Route, client_route);

      struct SPRT::Condition condition(status, SPRT::MovingToPoint);
      client.Send(SPRT::Condition, condition);
    }

    RouteMove(car, route, kMmPerPx);
  }
}

}  // namespace ML