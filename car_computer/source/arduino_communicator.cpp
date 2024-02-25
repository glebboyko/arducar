#include "arduino_communicator.hpp"

#include <chrono>

namespace AC {

/*--------------------------------- message ----------------------------------*/

void Message::PrintData(std::ostream& os) const {}
void Message::ScanData(std::istream& is) {}

std::ostream& operator<<(std::ostream& os, const Message& message) {
  message.PrintData(os);
  return os;
}
std::istream& operator>>(std::istream& is, Message& message) {
  message.ScanData(is);
  return is;
}

CTCarMove::CTCarMove(int dist_left, int dist_right, int ms_time) {
  data_ = std::to_string(dist_left) + " " + std::to_string(dist_right) + " " +
          std::to_string(ms_time);
}
void CTCarMove::PrintData(std::ostream& os) const { os << data_; }

void CRCarMoveFinish::ScanData(std::istream& is) {
  int tmp;
  is >> tmp;
}

void CTRadarScan::PrintData(std::ostream& os) const { os << TRadarScan; }

bool CRRadarScan::IsTerm() const { return dist_ < 0; }
std::pair<float, int> CRRadarScan::GetData() const { return {angle_, dist_}; }
void CRRadarScan::ScanData(std::istream& is) { is >> dist_ >> angle_; }

/*--------------------------- arduino communicator ---------------------------*/
ArduinoCommunicator::ArduinoCommunicator() {
  client_ = server_.AcceptConnection();
}

bool ArduinoCommunicator::SendMessage(MessageType type,
                                      const Message& message) {
  try {
    if (!client_.IsConnected()) {
      client_ = server_.AcceptConnection();
    }
    client_.Send(static_cast<int>(type), message);
  } catch (TCP::TcpException& exception) {
    client_.StopClient();
    return false;
  }
  return true;
}

std::shared_ptr<Message> ArduinoCommunicator::ReceiveMessage(MessageType type,
                                                             int ms_timeout) {
  if (!unclaimed_messages[type].empty()) {
    auto ptr = unclaimed_messages[type].front();
    unclaimed_messages[type].pop();
    return ptr;
  }

  if (!client_.IsConnected()) {
    client_ = server_.AcceptConnection();
  }

  try {
    while (ms_timeout >= 0) {
      int recv_type;

      auto recv_start = std::chrono::system_clock::now();
      if (!client_.Receive(ms_timeout, recv_type)) {
        return nullptr;
      }
      auto recv_end = std::chrono::system_clock::now();
      ms_timeout -= std::chrono::duration_cast<std::chrono::milliseconds>(
                        recv_end - recv_start)
                        .count();

      std::shared_ptr<Message> message;
      switch (recv_type) {
        case RCarMoveFinish:
          message = std::shared_ptr<Message>(new CRCarMoveFinish());
          break;
        case RRadarScan:
          message = std::shared_ptr<Message>(new CRRadarScan());
          break;
        default:
          return nullptr;
      }

      if (!client_.Receive(1'000, *message)) {
        return nullptr;
      }

      if (recv_type == type) {
        return message;
      }
      unclaimed_messages[recv_type].push(message);
    }
  } catch (TCP::TcpException& exception) {
    client_.StopClient();
  }

  return nullptr;
}

}  // namespace AC