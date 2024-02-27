#include <chrono>
#include <memory>
#include <mutex>
#include <tcp-client.hpp>
#include <thread>

#include "bitmap.hpp"
#include "frame.hpp"
#include "support.hpp"

namespace VS {

const int kBorderThickness = 10;

const std::chrono::milliseconds kSleepFail(100);
const int kMsReceivingTimeout = 1'000;

struct MessageGetter {
  MessageGetter() = default;

  std::shared_ptr<SPRT::Message> message_;
  SPRT::MessageType type_;

  friend std::ostream& operator<<(std::ostream&, const MessageGetter&);
  friend std::istream& operator>>(std::istream&, MessageGetter&);
};

class Visualizer {
 public:
  Visualizer(BM::VisualBitmap& bitmap, IF::Layer& position,
             IF::Layer& destination, int mm_per_px, int px_border_offset,
             float radar_width);
  ~Visualizer();

 private:
  BM::VisualBitmap& bitmap_;
  IF::Layer& position_;
  IF::Layer& destination_;

  std::thread actor_;
  std::mutex shared_data_;

  bool is_active = true;

  TCP::TcpClient client_;

  int mm_per_px_;
  int px_border_offset_;
  float radar_width_;

  void Actor();
};

}  // namespace VS