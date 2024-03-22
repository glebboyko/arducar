#pragma once

#include <list>
#include <memory>
#include <primitives.hpp>
#include <string>

namespace MSG {

struct InitData {
  int px_size_x;
  int px_size_y;
  float mm_per_px;
  int border_offset;
  int dist_threshold;
};

std::istream& operator>>(std::istream&, InitData&);
std::ostream& operator<<(std::ostream&, const InitData&);

enum MessageType { TPosition, TScan, TDestination, TRoute, TStatus, TAction };

enum CurrStatus { Act, Pause, Stop };
enum CurrAction { Init, Scanning, MovingToPoint, Computing, Finish };

struct Message {
 public:
  virtual MessageType GetType() const = 0;

 protected:
  virtual void PrintData(std::ostream&) const = 0;

  friend std::ostream& operator<<(std::ostream&, const Message&);
};

struct MessageContainer {
 public:
  std::shared_ptr<Message> message;

  friend std::istream& operator>>(std::istream&, MessageContainer&);
};

struct Position : Message {
 public:
  Position(PTIT::Coord posistion, float angle);
  Position(std::istream&);

  PTIT::Coord position;
  float angle;

 private:
  virtual void PrintData(std::ostream&) const override;

  virtual MessageType GetType() const override;
};

struct Scan : Message {
 public:
  Scan(float angle, int dist);
  Scan(std::istream&);

  float angle;
  int dist;

 private:
  virtual void PrintData(std::ostream&) const override;

  virtual MessageType GetType() const override;
};

struct Destination : Message {
 public:
  Destination(PTIT::Coord destination);
  Destination(std::istream&);

  PTIT::Coord destination;

 private:
  virtual void PrintData(std::ostream&) const override;

  virtual MessageType GetType() const override;
};

struct Route : Message {
 public:
  Route(const std::list<PTIT::Coord>& route);
  Route(std::istream&);

  std::list<PTIT::Coord> route;

 private:
  virtual void PrintData(std::ostream&) const override;

  virtual MessageType GetType() const override;
};

struct Status : Message {
 public:
  Status(CurrStatus status);
  Status(std::istream&);

  CurrStatus status;

 private:
  virtual void PrintData(std::ostream&) const override;

  virtual MessageType GetType() const override;
};

struct Action : Message {
 public:
  Action(CurrAction status);
  Action(std::istream&);

  CurrAction action;

 private:
  virtual void PrintData(std::ostream&) const override;

  virtual MessageType GetType() const override;
};

}  // namespace MSG