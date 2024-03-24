#include <unistd.h>
#include <wx/wx.h>

#include <any>
#include <iostream>
#include <tcp-client.hpp>
#include <thread>
#include <vector>

#include "env-shot.hpp"
#include "frame.hpp"
#include "messages.hpp"

std::vector<std::string> Split(const std::string& string,
                               const char& delimiter) {
  std::vector<std::string> split;
  size_t start_from = 0;
  while (true) {
    size_t pos = string.find(delimiter, start_from);
    if (pos == std::variant_npos) {
      split.push_back(string.substr(start_from, string.size() - start_from));
      break;
    }
    split.push_back(string.substr(start_from, pos - start_from));
    start_from = pos + 1;
  }
  return split;
}

std::pair<SPRT::QueryType, std::any> GetQuery(TCP::TcpClient& client) noexcept {
  while (true) {
    try {
      if (!client.IsConnected()) {
        client.Connect(SPRT::kServerAddress.c_str(), SPRT::kTcpPort);
        return {SPRT::QStatus, "Connection established"};
      }

      int query_type;
      std::string query_value;
      if (!client.Receive(1000, query_type, query_value)) {
        continue;
      }

      auto query_arr = Split(query_value, ',');
      switch (query_type) {
        case SPRT::QRadarScan: {
          SPRT::RadarScan scan = {
              {std::stoi(query_arr[0]), std::stoi(query_arr[1])},
              std::stod(query_arr[2]),
              std::stod(query_arr[3]),
              std::stoi(query_arr[4])};
          return {SPRT::QRadarScan, scan};
        }
        case SPRT::QWorkingArea: {
          SPRT::WorkingArea area;
          for (int i = 0; i < query_arr.size(); i += 2) {
            area.push_back(
                {std::stoi(query_arr[i]), std::stoi(query_arr[i + 1])});
          }
          return {SPRT::QWorkingArea, area};
        }
        case SPRT::QCarPosition: {
          SPRT::CarPosition position = {
              {std::stoi(query_arr[0]), std::stoi(query_arr[1])},
              std::stod(query_arr[2])};
          return {SPRT::QCarPosition, position};
        }
        case SPRT::QDestination: {
          SPRT::Destination destination = {std::stoi(query_arr[0]),
                                           std::stoi(query_arr[1])};
          return {SPRT::QDestination, destination};
        }
        case SPRT::QRoute: {
          SPRT::Route route;
          for (int i = 0; i < query_arr.size(); i += 2) {
            route.push_back(
                {std::stoi(query_arr[i]), std::stoi(query_arr[i + 1])});
          }
          return {SPRT::QRoute, route};
        }
        case SPRT::QStatus:
          return {SPRT::QStatus, query_arr[0]};
      }
    } catch (std::exception& exception) {
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
      return {SPRT::QStatus, "Connection failed. Connecting"};
    }
  }
}

void SetAlpha(const std::list<PTIT::Coord>& mask, unsigned char* alpha,
              wxSize image_size) {
  auto iter = mask.begin();
  for (int y = 0; y < image_size.y; ++y) {
    for (int x = 0; x < image_size.x; ++x) {
      long pos = y * image_size.x + x;
      if (iter == mask.end()) {
        alpha[pos] = 0;
        continue;
      }
      if (*iter == PTIT::Coord{x, y}) {
        alpha[pos] = 255;
        ++iter;
      } else {
        alpha[pos] = 0;
      }
    }
  }
}

class MainThread : public wxThread {
 public:
  MainThread(IF::Frame* frame, wxSize map_size, TCP::TcpClient&& tcp_client)
      : wxThread(wxTHREAD_DETACHED),
        frame_(frame),
        map_size_(map_size),
        tcp_client_(std::move(tcp_client)) {
    map_compression_ = IF::kStandardHeight / map_size_.y;
  }

  virtual void* Entry() {
    BM::VisualBitmap bitmap(map_size_.x, map_size_.y,
                            frame_->GetRadarScans().GetImage().GetData(),
                            frame_->GetRadarScans().GetImage().GetAlpha(),
                            frame_->GetBorders().GetImage().GetAlpha(),
                            frame_->GetWorkingSpace().GetImage().GetAlpha(),
                            frame_->GetRoute().GetImage().GetAlpha());
    EnvShot map(bitmap, 1, 4);

    while (true) {
      auto [query_type, query_val] = GetQuery(tcp_client_);

      switch (query_type) {
        case SPRT::QRadarScan: {
          auto scan = std::any_cast<SPRT::RadarScan>(query_val);

          map.AddMeasure(scan.deg, scan.distance, scan.radar_width,
                         scan.position);
        } break;
        case SPRT::QWorkingArea: {
          auto area = std::any_cast<SPRT::WorkingArea>(query_val);
          SetAlpha(PTIT::FulfillArea(area),
                   frame_->GetWorkingSpace().GetImage().GetAlpha(), map_size_);
        } break;
        case SPRT::QCarPosition: {
          auto pos = std::any_cast<SPRT::CarPosition>(query_val);
          frame_->GetPosition().SetPosition({pos.position.x, pos.position.y});
          frame_->GetPosition().Rotate(pos.deg);
        } break;
        case SPRT::QDestination: {
          auto dest = std::any_cast<SPRT::Destination>(query_val);
          frame_->GetDestMark().SetPosition({dest.x, dest.y});
          frame_->GetDestMark().Show();
        } break;
        case SPRT::QRoute: {
          auto route = std::any_cast<SPRT::Route>(query_val);
          SetAlpha(route, frame_->GetRoute().GetImage().GetAlpha(), map_size_);
        } break;
        case SPRT::QStatus: {
          auto status = std::any_cast<SPRT::Status>(query_val);
          std::cout << status << "\n";
        } break;
      }
    }

    return nullptr;
  }

 private:
  IF::Frame* frame_;
  TCP::TcpClient tcp_client_;
  wxSize map_size_;
  double map_compression_;
};

class App : public wxApp {
 public:
  virtual bool OnInit() {
    wxSize map_size;
    TCP::TcpClient client;
    try {
      client.Connect(SPRT::kServerAddress.c_str(), SPRT::kTcpPort);
      client.Send(static_cast<int>(SPRT::Visualizer));
      if (!client.Receive(map_size.x, map_size.y)) {
        throw TCP::TcpException(TCP::TcpException::Receiving, TCP::LoggerCap);
      }
    } catch (std::exception& exception) {
      perror(exception.what());
      return false;
    }
    IF::Frame* frame = new IF::Frame(map_size);

    frame->Show(true);

    MainThread* thread = new MainThread(frame, map_size, std::move(client));
    thread->Run();
    return true;
  }
};

wxIMPLEMENT_APP(App);