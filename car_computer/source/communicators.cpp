#include "communicators.hpp"

namespace CCM {

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

void DesktopCommunicator::Send(std::shared_ptr<MSG::Message> message) {
  acceptor_mutex_.lock();
  try {
    client_.Send(*message);
  } catch (TCP::TcpException& exception) {
    lost_messages_.push(message);
  }
  acceptor_mutex_.unlock();
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

}  // namespace CCM