#pragma once

#include <wx/wx.h>

#include "env-shot.hpp"
#include "primitives.hpp"
#include "tcp-client.hpp"

namespace IF {

const int kStandardHeight = 1200;
const wxSize kPosiSize = {60, 60};
const wxSize kDestMarkSize = {50, 50};

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

  void Update(Layer&);

 private:
  Layer base_;
  Layer radar_scans_;
  Layer working_space_;
  Layer route_;
  Layer borders_;
  Layer dest_mark_;
  Layer position_;

  wxThread* update_thread_;

  void UpdateEvent(wxCommandEvent& event);
  void KeyPressed(wxKeyEvent& event);

  static const wxEventType kUpdateEvent;

  DECLARE_EVENT_TABLE();
};

}  // namespace IF