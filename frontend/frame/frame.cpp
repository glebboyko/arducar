#include "frame.hpp"

namespace IF {

const wxEventType IF::Frame::kMapUpdateEvent = wxNewEventType();
const wxEventType IF::Frame::kPosUpdateEvent = wxNewEventType();
BEGIN_EVENT_TABLE(IF::Frame, wxFrame)
EVT_COMMAND(wxID_ANY, kMapUpdateEvent, IF::Frame::MapUpdateEvent)
EVT_COMMAND(wxID_ANY, kPosUpdateEvent, IF::Frame::PosUpdateEvent)
END_EVENT_TABLE()

Frame::Frame(wxSize map_size)
    : wxFrame(NULL, 1, "Arducar", wxDefaultPosition, kDisplaySize) {
  wxPanel* panel = new wxPanel(this, wxID_ANY);
  wxInitAllImageHandlers();

  map_img_ = wxImage(map_size);
  auto* data = map_img_.GetData();
  for (int y = 0; y < map_img_.GetHeight(); ++y) {
    for (int x = 0; x < map_img_.GetWidth(); ++x) {
      long pos = (y * map_img_.GetWidth() + x) * 3;
      data[pos] = 255;
      data[pos + 1] = 255;
      data[pos + 2] = 255;
    }
  }

  map_ = new wxStaticBitmap(this, wxID_ANY, map_img_, wxDefaultPosition,
                            kDisplaySize);
  position_img_ = wxImage("../arducar_position.png", wxBITMAP_TYPE_PNG);
  position_ = new wxStaticBitmap(map_, wxID_ANY, position_img_,
                                 wxDefaultPosition, kPositionSize);
}

unsigned char* Frame::GetMap() { return map_img_.GetData(); }

void Frame::UpdateMap() {
  wxCommandEvent update_event(kMapUpdateEvent, wxID_ANY);
  wxPostEvent(this, update_event);
}

void Frame::MapUpdateEvent(wxCommandEvent& event) {
  map_->SetBitmap(map_img_);
  map_->SetSize(kDisplaySize);
}

void Frame::SetPosition(int x, int y, double rotation) {
  rotation *= -1;

  PosUpdate* data = new PosUpdate();

  data->size = kPositionSize *
               (abs(sin(DegToRad(rotation))) + abs(cos(DegToRad(rotation))));
  data->position.x =
      x / (static_cast<double>(map_img_.GetWidth()) / kDisplaySize.GetWidth()) -
      (data->size.GetWidth() / 2);
  data->position.y = y / (static_cast<double>(map_img_.GetHeight()) /
                          kDisplaySize.GetHeight()) -
                     (data->size.GetHeight() / 2);

  data->rotation = DegToRad(rotation);

  wxCommandEvent update_event(kPosUpdateEvent, wxID_ANY);
  update_event.SetClientData(data);
  wxPostEvent(this, update_event);
}

void Frame::PosUpdateEvent(wxCommandEvent& event) {
  auto* data = static_cast<PosUpdate*>(event.GetClientData());
  position_->SetBitmap(position_img_.Rotate(data->rotation, data->position));
  position_->SetPosition(data->position);
  position_->SetSize(data->size);

  delete data;
}

}  // namespace IF