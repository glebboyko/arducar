#pragma once

#include <memory>
#include <optional>
#include <tcp-client.hpp>

#include "messages/messages.hpp"

class Communicator {
 public:
  Communicator(const char* address, int port);

  bool IsConnected();
  bool Send(MSG::CurrStatus status);
  std::shared_ptr<MSG::Message> Receive(int ms_timeout);

  std::optional<MSG::InitData> GetInitData();

 private:
  TCP::TcpClient car_;

  const char* address_;
  int port_;

  MSG::InitData init_data_;

  bool Connect();
};