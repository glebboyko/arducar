#pragma once

#include <mutex>
#include <optional>
#include <queue>
#include <tcp-server.hpp>
#include <thread>

#include "messages/messages.hpp"

namespace CCM {

class DesktopCommunicator {
 public:
  DesktopCommunicator(int port, const MSG::InitData& init_data);
  ~DesktopCommunicator();

  std::shared_ptr<MSG::Message> Receive();

  template <typename T, typename... Args>
  void Send(const Args&... args) {
    T msg(args...);

    acceptor_mutex_.lock();
    try {
      client_.Send(msg);
    } catch (TCP::TcpException& exception) {
      client_.StopClient();
      lost_messages_.push(std::shared_ptr<MSG::Message>(new T(msg)));
    }
    acceptor_mutex_.unlock();
  }

 private:
  TCP::TcpServer server_;
  TCP::TcpClient client_;

  std::thread acceptor_;
  std::mutex acceptor_mutex_;
  std::queue<std::shared_ptr<MSG::Message>> lost_messages_;

  MSG::InitData init_data_;

  void Acceptor();

  bool SendInitData(TCP::TcpClient&);
  bool SendLostMessages(TCP::TcpClient&);
};

class ArduinoCommunicator {
 public:
  enum Direction { Left = 0, Forward = 1, Right = 2 };
  enum Speed { Full = 1, Half = 2, Quarter = 4, Eighth = 8, Sixteenth = 16 };

  ArduinoCommunicator();

  void StartRadar();
  void MoveCar(int step_num, Direction direction, Speed speed);

  std::optional<std::pair<float, int>> GetScan();
  int GetStep();

 private:
  TCP::TcpClient arduino_;

  const int kTimeout = 1'000;
  enum ArduinoType { Wheels = 0, Radar = 1 };
};

}  // namespace CCM