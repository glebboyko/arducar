#pragma once

#include <mutex>
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
  void Send(std::shared_ptr<MSG::Message>);

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

}  // namespace CCM