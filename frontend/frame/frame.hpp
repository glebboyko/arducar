#pragma once

#include <wx/wx.h>

#include "env-shot.hpp"

namespace IF {

const wxSize kDisplaySize = wxSize{1000, 1000};
const wxSize kPositionSize = wxSize{50, 50};

class Frame : public wxFrame {
 public:
  Frame(wxSize map_size);
  ~Frame() = default;

  unsigned char* GetMap();
  void UpdateMap();

  void SetPosition(int x, int y, double rotation);

 private:
  wxImage map_img_;
  wxStaticBitmap* map_;

  wxImage position_img_;
  wxStaticBitmap* position_;

  void MapUpdateEvent(wxCommandEvent& event);

  struct PosUpdate {
    wxSize size;
    wxPoint position;
    double rotation;
  };
  void PosUpdateEvent(wxCommandEvent& event);

  static const wxEventType kMapUpdateEvent;
  static const wxEventType kPosUpdateEvent;

  DECLARE_EVENT_TABLE();
};

}  // namespace IF