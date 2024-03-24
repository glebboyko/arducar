#include "communicators.hpp"

#include "constants.hpp"

namespace CCM {

/*--------------------------------- desktop ----------------------------------*/
DesktopCommunicator::DesktopCommunicator(int port,
                                         const MSG::InitData& init_data)
    : server_(port), init_data_(init_data) {
  acceptor_ = std::thread(&DesktopCommunicator::Acceptor, this);
}

DesktopCommunicator::~DesktopCommunicator() {
  acceptor_mutex_.lock();
  server_.CloseListener();
  acceptor_mutex_.unlock();
  acceptor_.join();
}

std::shared_ptr<MSG::Message> DesktopCommunicator::Receive() {
  MSG::MessageContainer container;

  std::shared_ptr<MSG::Message> message(nullptr);
  acceptor_mutex_.lock();
  try {
    client_.Receive(0, container);
    message = container.message;
  } catch (TCP::TcpException& exception) {
  }
  acceptor_mutex_.unlock();

  return message;
}

void DesktopCommunicator::Acceptor() {
  while (true) {
    try {
      auto client = server_.AcceptConnection();
      if (!SendInitData(client) || !SendLostMessages(client)) {
        continue;
      }

      acceptor_mutex_.lock();
      client_ = std::move(client);
      acceptor_mutex_.unlock();

    } catch (TCP::TcpException& exception) {
      if (!server_.IsListenerOpen()) {
        break;
      }
    }
  }
}

bool DesktopCommunicator::SendInitData(TCP::TcpClient& client) {
  try {
    client.Send(init_data_);
    return true;
  } catch (TCP::TcpException& exception) {
    return false;
  }
}

bool DesktopCommunicator::SendLostMessages(TCP::TcpClient& client) {
  while (true) {
    acceptor_mutex_.lock();
    if (lost_messages_.empty()) {
      acceptor_mutex_.unlock();
      break;
    }

    try {
      client.Send(*lost_messages_.front());
      lost_messages_.pop();
    } catch (TCP::TcpException& exception) {
      acceptor_mutex_.unlock();
      return false;
    }
    acceptor_mutex_.unlock();
  }

  return true;
}

/*--------------------------------- arduino ----------------------------------*/
ArduinoCommunicator::ArduinoCommunicator()
    : arduino_(TCP::TcpServer(CONST::kTcpArduinoPort).AcceptConnection()) {}

void ArduinoCommunicator::StartRadar() { arduino_.Send(Radar, 0); }
void ArduinoCommunicator::MoveCar(int step_num, Direction direction,
                                  Speed speed) {
  arduino_.Send(Wheels, step_num, direction, speed);
}

std::optional<std::pair<float, int>> ArduinoCommunicator::GetScan() {
  float angle;
  int dist = -1;
  while (!arduino_.Receive(kTimeout, angle, dist))
    ;

  if (dist == -1) {
    return {};
  }
  return std::pair{angle, dist};
}
int ArduinoCommunicator::GetStep() {
  int step;
  while (!arduino_.Receive(kTimeout, step));

  return step;
}

}  // namespace CCM