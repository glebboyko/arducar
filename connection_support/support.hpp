#pragma once

#include <list>
#include <primitives.hpp>
#include <string>

namespace SPRT {

const std::string kServerAddress = "192.168.1.100";
const int kClientPort = 44'450;

enum MessageType { Position, Scan, Destination, Route, Condition };

enum Status { Act, Pause, Stop };
enum CurrAction { Init, Scanning, MovingToPoint, Computing, Finish };

struct Message {
 protected:
  virtual void PrintData(std::ostream&) const = 0;
  virtual void ScanData(std::istream&) = 0;

  friend std::istream& operator>>(std::istream&, Message&);
  friend std::ostream& operator<<(std::ostream&, const Message&);
};

struct Position : Message {
 public:
  Position() = default;
  Position(PTIT::Coord posistion, float angle);

  PTIT::Coord position;
  float angle;

 private:
  virtual void PrintData(std::ostream&) const override;
  virtual void ScanData(std::istream&) override;
};

struct RadarScan : Message {
 public:
  RadarScan() = default;
  RadarScan(float angle, int dist);

  float angle;
  int dist;

 private:
  virtual void PrintData(std::ostream&) const override;
  virtual void ScanData(std::istream&) override;
};

struct Destination : Message {
 public:
  Destination() = default;
  Destination(PTIT::Coord destination);

  PTIT::Coord destination;

 private:
  virtual void PrintData(std::ostream&) const override;
  virtual void ScanData(std::istream&) override;
};

struct Route : Message {
 public:
  Route() = default;
  Route(const std::list<PTIT::Coord>& route);

  std::list<PTIT::Coord> route;

 private:
  virtual void PrintData(std::ostream&) const override;
  virtual void ScanData(std::istream&) override;
};

struct Condition : Message {
 public:
  Condition() = default;
  Condition(Status status, CurrAction action);

  Status status;
  CurrAction action;

 private:
  virtual void PrintData(std::ostream&) const override;
  virtual void ScanData(std::istream&) override;
};

}  // namespace SPRT