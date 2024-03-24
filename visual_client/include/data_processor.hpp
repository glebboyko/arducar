#pragma once

#include <primitives.hpp>

#include "communicator.hpp"
#include "frame.hpp"
#include "map/bitmap.hpp"
#include "map/env-shot.hpp"
#include "map/map_analyzer.hpp"
#include "messages/messages.hpp"

class DataProcessor {
 public:
  DataProcessor(IF::Frame& frame, Communicator& communicator,
                const MSG::InitData& init_data, int line_width);

  void MainLoop();

 private:
  IF::Frame& frame_;
  Communicator& communicator_;

  BM::VisualBitmap bitmap_;
  EnvShot env_shot_;
  MA::WorkingAreaProcessor working_area_;

  PTIT::Coord car_coord_;

  int line_width_;
  MSG::CurrAction action_ = MSG::Init;

  bool ProcessMessage(MSG::Message& message);

  void ProcessPosition(const MSG::Position& position);
  void ProcessScan(const MSG::Scan& scan);
  void ProcessDestination(const MSG::Destination& destination);
  void ProcessRoute(const MSG::Route& route);
  void ProcessAction(const MSG::Action& action);
  bool ProcessStatus(const MSG::Status& status);

  void SetConnectStatus(bool connected);
  wxPoint FromRealToMap(const PTIT::Coord& position);
};