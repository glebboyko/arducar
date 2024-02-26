#pragma once

#include <tcp-server.hpp>

namespace AC {

const int kArduinoPort = 44'440;

enum MessageType { CarMove, RadarScan };

class ArduinoCommunicator {
 public:
  ArduinoCommunicator();

  template <MessageType type, typename... Args>
  void SendMessage(const Args&... args) {
    auto& client = type == CarMove ? move_client_ : radar_client_;

    try {
      client.Send(args...);
    } catch (std::exception& exception) {
      perror(exception.what());
      exit(1);
    }
  }

  template <MessageType type>
  auto ReceiveMessage(int ms_timeout) {
    try {
      if constexpr (type == CarMove) {
        int test;
        return move_client_.Receive(ms_timeout, test);
      } else {
        std::pair<float, int> scan;
        if (radar_client_.Receive(ms_timeout, scan.first, scan.second)) {
          return std::optional(scan);
        }
        return std::optional<decltype(scan)>();
      }
    } catch (std::exception& exception) {
      perror(exception.what());
      exit(2);
    }
  }

 private:
  TCP::TcpServer server_ = TCP::TcpServer(kArduinoPort);
  TCP::TcpClient move_client_;
  TCP::TcpClient radar_client_;
};

}  // namespace AC