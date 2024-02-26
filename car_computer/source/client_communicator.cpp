#include "modules.hpp"

namespace ML {

ClientCommunication::ClientCommunication(CM::Car& car)
    : car_(car),
      tcp_actor_(std::thread(&ClientCommunication::TCPActor, this)),
      position_sender_(
          std::thread(&ClientCommunication::PositionSender, this)) {}

ClientCommunication::~ClientCommunication() {
  shared_data_.lock();
  is_active_ = false;
  shared_data_.unlock();

  tcp_actor_.join();
  position_sender_.join();
}

void ClientCommunication::Send(SPRT::MessageType type,
                               const SPRT::Message& message) {
  std::shared_ptr<SPRT::Message> ptr;
  switch (type) {
    case SPRT::Position:
      ptr = std::shared_ptr<SPRT::Message>(new struct SPRT::Position(
          static_cast<const struct SPRT::Position&>(message)));
      break;
    case SPRT::Scan:
      ptr = std::shared_ptr<SPRT::Message>(new struct SPRT::RadarScan(
          static_cast<const struct SPRT::RadarScan&>(message)));
      break;
    case SPRT::Destination:
      ptr = std::shared_ptr<SPRT::Message>(new struct SPRT::Destination(
          static_cast<const struct SPRT::Destination&>(message)));
      break;
    case SPRT::Route:
      ptr = std::shared_ptr<SPRT::Message>(new struct SPRT::Route(
          static_cast<const struct SPRT::Route&>(message)));
      break;
    case SPRT::Condition:
      ptr = std::shared_ptr<SPRT::Message>(new struct SPRT::Condition(
          static_cast<const struct SPRT::Condition&>(message)));
  }

  shared_data_.lock();
  to_send_.push({type, ptr});
  shared_data_.unlock();
}
std::optional<SPRT::Status> ClientCommunication::Receive() {
  shared_data_.lock();
  if (!received_status_.has_value()) {
    shared_data_.unlock();
    return {};
  }

  SPRT::Status status = received_status_.value();
  received_status_.reset();

  shared_data_.unlock();
  return status;
}

void ClientCommunication::TCPActor() {
  decltype(to_send_) local_buffer;
  while (true) {
    shared_data_.lock();
    if (!is_active_) {
      shared_data_.unlock();
      return;
    }

    while (!to_send_.empty()) {
      local_buffer.push(to_send_.front());
      to_send_.pop();
    }

    try {
      if (!local_buffer.empty()) {
        auto [type, data] = local_buffer.front();
        client_.Send(static_cast<int>(type), *data);
        local_buffer.pop();
      }
      int status;
      if (client_.Receive(0, status)) {
        shared_data_.lock();
        received_status_ = static_cast<SPRT::Status>(status);
        shared_data_.unlock();
      }
    } catch (TCP::TcpException& exception) {
      client_.StopClient();
      try {
        client_ = server_.AcceptConnection();
      } catch (TCP::TcpException& accept_exc) {
        perror(accept_exc.what());
      }
    }

    if (local_buffer.empty()) {
      std::this_thread::sleep_for(kSleepFail);
    }
  }
}

void ClientCommunication::PositionSender() {
  while (true) {
    shared_data_.lock();
    if (!is_active_) {
      shared_data_.unlock();
      return;
    }
    shared_data_.unlock();
  }

  auto [loc, angle] = car_.GetPosition();
  struct SPRT::Position position;
  position.position = loc;
  position.angle = angle;

  Send(SPRT::Position, position);

  std::this_thread::sleep_for(kPositionDelay);
}

}  // namespace ML