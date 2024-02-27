#include "visualizer.hpp"

#include <iostream>

#include "env-shot.hpp"
#include "map_analyzer.hpp"

namespace VS {

std::ostream& operator<<(std::ostream& os, const MessageGetter&) { return os; }
std::istream& operator>>(std::istream& is, MessageGetter& message) {
  int type;
  is >> type;

  message.type_ = static_cast<SPRT::MessageType>(type);

  switch (message.type_) {
    case SPRT::Position:
      message.message_ =
          std::shared_ptr<SPRT::Message>(new struct SPRT::Position());
      break;
    case SPRT::Scan:
      message.message_ =
          std::shared_ptr<SPRT::Message>(new struct SPRT::RadarScan());
      break;
    case SPRT::Destination:
      message.message_ =
          std::shared_ptr<SPRT::Message>(new struct SPRT::Destination());
      break;
    case SPRT::Route:
      message.message_ =
          std::shared_ptr<SPRT::Message>(new struct SPRT::Route());
      break;
    case SPRT::Condition:
      message.message_ =
          std::shared_ptr<SPRT::Message>(new struct SPRT::Condition());
      break;
  }

  is >> *message.message_;

  return is;
}

TCP::TcpClient ConnectToCar() {
  TCP::TcpClient client(SPRT::kServerAddress.c_str(), SPRT::kClientPort);
  int trash;
  if (!client.Receive(kMsReceivingTimeout, trash, trash, trash, trash, trash)) {
    throw TCP::TcpException(TCP::TcpException::Receiving, TCP::LoggerCap);
  }

  return client;
}

Visualizer::Visualizer(BM::VisualBitmap& bitmap, IF::Layer& position,
                       IF::Layer& destination, int mm_per_px,
                       int px_border_offset, float radar_width)
    : bitmap_(bitmap),
      position_(position),
      destination_(destination),
      mm_per_px_(mm_per_px),
      px_border_offset_(px_border_offset),
      radar_width_(radar_width),
      actor_(std::thread(&Visualizer::Actor, this)) {}
Visualizer::~Visualizer() {
  shared_data_.lock();
  is_active = false;
  shared_data_.unlock();

  actor_.join();
}

void Visualizer::Actor() {
  EnvShot env_shot(bitmap_, mm_per_px_, kBorderThickness);
  MA::WorkingAreaProcessor working_area(bitmap_, px_border_offset_);

  PTIT::Coord curr_position = {0, 0};

  while (true) {
    shared_data_.lock();
    if (!is_active) {
      shared_data_.unlock();
      return;
    }
    shared_data_.unlock();

    try {
      if (!client_.IsConnected()) {
        client_ = ConnectToCar();
      }

      MessageGetter message_getter;
      if (client_.Receive(kMsReceivingTimeout, message_getter)) {
        switch (message_getter.type_) {
          case SPRT::Position: {
            auto& message = *static_cast<struct SPRT::Position*>(
                message_getter.message_.get());
            curr_position = message.position;
            position_.SetPosition({message.position.x / mm_per_px_,
                                   message.position.y / mm_per_px_});
            position_.Rotate(message.angle);
            position_.Show();
          } break;
          case SPRT::Scan: {
            auto& message = *static_cast<struct SPRT::RadarScan*>(
                message_getter.message_.get());
            env_shot.AddMeasure(message.angle, message.dist, radar_width_,
                                curr_position);
          } break;
          case SPRT::Destination: {
            auto& message = *static_cast<struct SPRT::Destination*>(
                message_getter.message_.get());
            destination_.SetPosition({message.destination.x / mm_per_px_,
                                      message.destination.y / mm_per_px_});
            destination_.Show();
          } break;
        }
      }

    } catch (TCP::TcpException& exception) {
      client_.StopClient();
      std::this_thread::sleep_for(kSleepFail);
      continue;
    }
  }
}

}  // namespace VS