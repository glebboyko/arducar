#pragma once

#include <string>
#include <tcp-server.hpp>
#include <memory>

namespace AC {

const int kArduinoPort = 44'440;

enum MessageType { TCarMove, RCarMoveFinish, TRadarScan, RRadarScan };

class Message {
 protected:
  virtual void PrintData(std::ostream&) const;
  virtual void ScanData(std::istream&);

  friend std::ostream& operator<<(std::ostream&, const Message&);
  friend std::istream& operator>>(std::istream&, Message&);
};

class CTCarMove : public Message {
 public:
  CTCarMove(int dist_left, int dist_right, int ms_time);

 private:
  virtual void PrintData(std::ostream&) const override;

  std::string data_;
};

class CRCarMoveFinish : public Message {
 public:
  CRCarMoveFinish() = default;

 private:
  virtual void ScanData(std::istream&) override;
};

class CTRadarScan : public Message {
 public:
  CTRadarScan() = default;

 private:
  virtual void PrintData(std::ostream&) const override;
};

class CRRadarScan : public Message {
 public:
  CRRadarScan() = default;

  bool IsTerm() const;
  std::pair<float, int> GetData() const;

 private:
  virtual void ScanData(std::istream&) override;

  int dist_;
  float angle_;
};

class ArduinoCommunicator {
 public:
  ArduinoCommunicator();

  bool SendMessage(MessageType type, const Message& message);
  std::shared_ptr<Message> ReceiveMessage(MessageType type,
                                               int ms_timeout);

 private:
  TCP::TcpServer server_ = TCP::TcpServer(kArduinoPort);
  TCP::TcpClient client_;

  std::queue<std::shared_ptr<Message>> unclaimed_messages[2];
};

}  // namespace AC