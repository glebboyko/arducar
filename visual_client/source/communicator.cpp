#include "communicator.hpp"

Communicator::Communicator(const char* address, int port)
    : address_(address), port_(port) {
  Connect();
}

bool Communicator::IsConnected() { return Connect(); }

bool Communicator::Send(MSG::CurrStatus status) {
  if (!Connect()) {
    return false;
  }

  try {
    car_.Send(MSG::Status(status));
  } catch (TCP::TcpException& exception) {
    car_.StopClient();
    return false;
  }
  return true;
}

std::shared_ptr<MSG::Message> Communicator::Receive(int ms_timeout) {
  std::shared_ptr<MSG::Message> message(nullptr);

  if (!Connect()) {
    return message;
  }

  try {
    MSG::MessageContainer container;
    car_.Receive(ms_timeout, container);
    message = container.message;
  } catch (TCP::TcpException& exception) {
    car_.StopClient();
  }

  return message;
}

std::optional<MSG::InitData> Communicator::GetInitData() {
  if (!Connect()) {
    return {};
  }

  return init_data_;
}

bool Communicator::Connect() {
  if (car_.IsConnected()) {
    return true;
  }

  try {
    car_.Connect(address_, port_);

    MSG::InitData new_init;
    car_.Receive(car_.GetMsPingThreshold(), new_init);
    init_data_ = new_init;
  } catch (TCP::TcpException& exception) {
    car_.StopClient();
    return false;
  }

  return true;
}