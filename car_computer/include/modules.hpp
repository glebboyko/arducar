#pragma once
#include <chrono>
#include <memory>
#include <mutex>
#include <optional>
#include <queue>
#include <tcp-server.hpp>
#include <thread>

#include "bitmap.hpp"
#include "hardware_processor.hpp"
#include "support.hpp"

namespace ML {

const std::chrono::milliseconds kSleepFail(100);
const std::chrono::milliseconds kPositionDelay(20);

const int kMmPerPx = 10;
const float kRadarWidth = 1.8;             // deg
const int kBorderOffset = 500 / kMmPerPx;  // px
const PTIT::Coord kPxMapSize(1000, 1000);

class ClientCommunication {
 public:
  ClientCommunication(CM::Car& car);
  ~ClientCommunication();

  void Send(SPRT::MessageType type, const SPRT::Message& message);
  std::optional<SPRT::Status> Receive();

 private:
  TCP::TcpServer server_ = TCP::TcpServer(SPRT::kClientPort);
  TCP::TcpClient client_;

  std::queue<std::pair<SPRT::MessageType, std::shared_ptr<SPRT::Message>>>
      to_send_;
  std::optional<SPRT::Status> received_status_;

  std::thread tcp_actor_;
  std::thread position_sender_;

  std::mutex shared_data_;

  bool is_active_ = true;

  CM::Car& car_;

  void TCPActor();
  void PositionSender();
};

void MainLoop(ClientCommunication& client, BM::Bitmap& bitmap, CM::Car& car);

}  // namespace ML