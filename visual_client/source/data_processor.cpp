#define GET_RAW_DATA() GetImage().GetData()
#define GET_RAW_ALPHA() GetImage().GetAlpha()

#include "data_processor.hpp"

#include <list>

#include "constants.hpp"

DataProcessor::DataProcessor(IF::Frame& frame, Communicator& communicator,
                             const MSG::InitData& init_data, int line_width)
    : frame_(frame),
      communicator_(communicator),
      bitmap_(init_data.px_size_x, init_data.px_size_y, init_data.mm_per_px,
              line_width, frame.GetRadarScans().GET_RAW_DATA(),
              frame.GetRadarScans().GET_RAW_ALPHA(),
              frame.GetBorders().GET_RAW_ALPHA(),
              frame.GetWorkingSpace().GET_RAW_ALPHA(),
              frame.GetRoute().GET_RAW_ALPHA()),
      env_shot_(bitmap_),
      working_area_(bitmap_, init_data.border_offset, init_data.dist_threshold),
      car_coord_(init_data.init_coord),
      line_width_(line_width) {
  frame.GetRadarScans().Show();
  frame.GetWorkingSpace().Show();
  frame.GetBorders().Show();

  frame.Update(frame.GetRadarScans());
  frame.Update(frame.GetWorkingSpace());
  frame.Update(frame.GetBorders());
}

void DataProcessor::MainLoop() {
  while (true) {
    auto message = communicator_.Receive(TCP::kDefPingThreshold);
    if (message == nullptr) {
      if (!communicator_.IsConnected()) {
        SetConnectStatus(false);
      }
      continue;
    }
    SetConnectStatus(true);

    if (!ProcessMessage(*message)) {
      break;
    }
  }
}

bool DataProcessor::ProcessMessage(MSG::Message& message) {
  switch (message.GetType()) {
    case MSG::TPosition:
      ProcessPosition(dynamic_cast<MSG::Position&>(message));
      break;
    case MSG::TScan:
      ProcessScan(dynamic_cast<MSG::Scan&>(message));
      break;
    case MSG::TDestination:
      ProcessDestination(dynamic_cast<MSG::Destination&>(message));
      break;
    case MSG::TRoute:
      ProcessRoute(dynamic_cast<MSG::Route&>(message));
      break;
    case MSG::TStatus:
      if (!ProcessStatus(dynamic_cast<MSG::Status&>(message))) {
        return false;
      }
      break;
    case MSG::TAction:
      ProcessAction(dynamic_cast<MSG::Action&>(message));
      break;
  }

  return true;
}

void DataProcessor::ProcessPosition(const MSG::Position& position) {
  car_coord_ = position.position;

  auto& layer = frame_.GetPosition();

  layer.SetPosition(FromRealToMap(car_coord_));
  layer.Rotate(position.angle);

  frame_.Update(layer);
}
void DataProcessor::ProcessScan(const MSG::Scan& scan) {
  env_shot_.AddMeasure(scan.angle, scan.dist, CONST::kRadarWidth, car_coord_);
  working_area_.ProcessArea(car_coord_);

  frame_.Update(frame_.GetRadarScans());
  frame_.Update(frame_.GetBorders());
  frame_.Update(frame_.GetWorkingSpace());
}
void DataProcessor::ProcessDestination(const MSG::Destination& destination) {
  auto& layer = frame_.GetDestMark();
  layer.SetPosition(FromRealToMap(destination.destination));
  layer.Show();

  frame_.Update(layer);
}
void DataProcessor::ProcessRoute(const MSG::Route& route) {
  const auto& point_route = route.route;

  std::list<PTIT::Segment> segments;
  segments.push_back(PTIT::Segment(car_coord_, point_route.front()));

  for (auto iter = std::next(point_route.begin()); iter != point_route.end();
       ++iter) {
    segments.push_back(PTIT::Segment(*std::prev(iter), *iter));
  }

  bitmap_.CleanRoute();
  for (const auto& segment : segments) {
    if (line_width_ > 1) {
      for (const auto& [x, y] : segment.GetArea(line_width_ / 2)) {
        bitmap_.SetRouteData(x, y, true);
      }
    } else {
      for (const auto& [x, y] : segment.GetGraphic()) {
        bitmap_.SetRouteData(x, y, true);
      }
    }
  }

  auto& layer = frame_.GetRoute();
  layer.Show();
  frame_.Update(layer);
}
void DataProcessor::ProcessAction(const MSG::Action& action) {
  if (action_ == MSG::MovingToPoint && action.action != MSG::MovingToPoint) {
    frame_.GetDestMark().Show(false);
    frame_.GetRoute().Show(false);
    frame_.Update(frame_.GetDestMark());
    frame_.Update(frame_.GetRoute());
  }

  action_ = action.action;
}
bool DataProcessor::ProcessStatus(const MSG::Status& status) {
  return status.status != MSG::Stop;
}

void DataProcessor::SetConnectStatus(bool connected) {
  auto& layer = frame_.GetDisconnected();
  if (layer.IsShown() != !connected) {
    layer.Show(!connected);
    frame_.Update(layer);
  }
}

wxPoint DataProcessor::FromRealToMap(const PTIT::Coord& position) {
  return wxPoint(position.x / bitmap_.GetDensity(),
                 position.y / bitmap_.GetDensity());
}