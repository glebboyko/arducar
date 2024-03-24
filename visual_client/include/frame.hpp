#pragma once

#include <wx/wx.h>

#include <image-creator.hpp>

namespace IF {

const wxSize kPosiSize = {60, 60};
const wxSize kDestMarkSize = {50, 50};

const PTIT::RGB kBaseColor = PTIT::RGB{229, 236, 233};
const PTIT::RGB kScanColor = PTIT::RGB{255, 255, 255};
const PTIT::RGB kWorkingSpaceColor = PTIT::RGB{68, 175, 105};
const PTIT::RGB kRouteColor = PTIT::RGB{252, 171, 16};
const PTIT::RGB kBorderColor = PTIT::RGB{0, 0, 0};
const PTIT::RGB kDisconnectedColor = PTIT::RGB{0, 0, 0};

wxImage FillImage(wxSize size, PTIT::RGB background_color);
wxImage FillImage(wxSize size, PTIT::RGB background_color, unsigned char alpha);

class Layer {
 public:
  Layer() = default;

  Layer(const wxImage& image, wxWindow* parent, wxSize cont_size,
        wxPoint position = wxDefaultPosition, double deg = 0);
  Layer(wxImage&& image, wxWindow* parent, wxSize cont_size,
        wxPoint position = wxDefaultPosition, double deg = 0);

  Layer& operator=(Layer&&) = default;

  ~Layer() = default;

  wxImage& GetImage();

  void SetPosition(const wxPoint& position);
  void Rotate(double deg);
  void Show(bool show = true);
  bool IsShown() const;

  void Update();

 private:
  wxImage image_;
  wxStaticBitmap* bitmap_;
  wxSize base_size_;

  wxPoint position_;
  double deg_;
};

class Frame : public wxFrame {
 public:
  Frame(wxSize map_size);
  ~Frame() = default;

  Layer& GetRadarScans();
  Layer& GetWorkingSpace();
  Layer& GetRoute();
  Layer& GetBorders();
  Layer& GetDestMark();
  Layer& GetPosition();
  Layer& GetDisconnected();

  void Update(Layer&);

 private:
  Layer base_;
  Layer radar_scans_;
  Layer working_space_;
  Layer route_;
  Layer borders_;
  Layer dest_mark_;
  Layer position_;
  Layer disconnected_;

  wxThread* update_thread_;

  void UpdateEvent(wxCommandEvent& event);
  void KeyPressed(wxKeyEvent& event);

  static const wxEventType kUpdateEvent;

  DECLARE_EVENT_TABLE();
};

}  // namespace IF