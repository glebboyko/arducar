#pragma once

#include <mutex>
#include <queue>
#include <tcp-server.hpp>
#include <thread>

#include "messages/messages.hpp"

namespace CCM {

class Communicator {
 public:
  Communicator(int port);
  ~Communicator();

  std::shared_ptr<MSG::Message> Receive();
  void Send(std::shared_ptr<MSG::Message>);

 private:
  TCP::TcpServer server_;
  TCP::TcpClient client_;

  std::thread acceptor_;
  std::mutex acceptor_mutex_;
  std::queue<std::shared_ptr<MSG::Message>> lost_messages_;

  void Acceptor();
  bool SendLostMessages(TCP::TcpClient&);
};

}  // namespace CCM