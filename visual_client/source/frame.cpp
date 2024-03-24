#include "frame.hpp"

#include <semaphore>

namespace IF {

wxImage FillImage(wxSize size, PTIT::RGB background_color) {
  wxImage image(size);
  auto* data = image.GetData();

  for (int y = 0; y < image.GetHeight(); ++y) {
    for (int x = 0; x < image.GetWidth(); ++x) {
      long pos = (y * image.GetWidth() + x) * 3;
      data[pos] = background_color.red;
      data[pos + 1] = background_color.green;
      data[pos + 2] = background_color.blue;
    }
  }

  return image;
}

wxImage FillImage(wxSize size, PTIT::RGB background_color,
                  unsigned char alpha) {
  auto image = FillImage(size, background_color);

  image.InitAlpha();
  auto* data = image.GetAlpha();
  for (int y = 0; y < image.GetHeight(); ++y) {
    for (int x = 0; x < image.GetWidth(); ++x) {
      long pos = y * image.GetWidth() + x;
      data[pos] = alpha;
    }
  }

  return image;
}

Layer::Layer(wxImage&& image, wxWindow* parent, wxSize cont_size,
             wxPoint position, double deg)
    : image_(std::move(image)),
      base_size_(cont_size),
      position_(position),
      deg_(deg) {
  bitmap_ = new wxStaticBitmap(parent, wxID_ANY, image_, wxDefaultPosition,
                               base_size_);
  Show(false);
  Update();
}

Layer::Layer(const wxImage& image, wxWindow* parent, wxSize cont_size,
             wxPoint position, double deg)
    : Layer(wxImage(image), parent, cont_size, position, deg) {}

wxImage& Layer::GetImage() { return image_; }

void Layer::SetPosition(const wxPoint& position) { position_ = position; }
void Layer::Rotate(double deg) { deg_ = deg; }

void Layer::Show(bool show) { bitmap_->Show(show); }
bool Layer::IsShown() const { return bitmap_->IsShown(); }

void Layer::Update() {
  auto rad = wxDegToRad(deg_);
  auto norm_cont_size = base_size_ * (abs(sin(rad)) + abs(cos(rad)));
  wxPoint norm_position = {position_.x - (norm_cont_size.x / 2),
                           position_.y - (norm_cont_size.y / 2)};

  bitmap_->SetBitmap(image_.Rotate(rad, norm_position));
  bitmap_->SetSize(norm_cont_size);
  bitmap_->SetPosition(norm_position);
}

const wxEventType IF::Frame::kUpdateEvent = wxNewEventType();
BEGIN_EVENT_TABLE(IF::Frame, wxFrame)
EVT_COMMAND(wxID_ANY, kUpdateEvent, IF::Frame::UpdateEvent)
END_EVENT_TABLE()

Frame::Frame(wxSize map_size)
    : wxFrame(NULL, 1, "Arducar", wxDefaultPosition, wxDefaultSize) {
  wxPanel* panel = new wxPanel(this);
  panel->Bind(wxEVT_KEY_DOWN, &Frame::KeyPressed, this);

  wxInitAllImageHandlers();

  SetSize(map_size);

  wxPoint center_position = {map_size.x / 2, map_size.y / 2};
  base_ =
      Layer(FillImage(map_size, kBaseColor), this, map_size, center_position);

  radar_scans_ = Layer(FillImage(map_size, kScanColor, 0), this, map_size,
                       center_position);
  working_space_ = Layer(FillImage(map_size, kWorkingSpaceColor, 50), this,
                         map_size, center_position);
  route_ = Layer(FillImage(map_size, kRouteColor, 0), this, map_size,
                 center_position);
  borders_ = Layer(FillImage(map_size, kBorderColor, 0), this, map_size,
                   center_position);
  dest_mark_ = Layer(wxImage("../images/dest_mark.png", wxBITMAP_TYPE_PNG),
                     this, kDestMarkSize);
  position_ =
      Layer(wxImage("../images/arducar_position.png", wxBITMAP_TYPE_PNG), this,
            kPosiSize);
  disconnected_ = Layer(FillImage(map_size, kDisconnectedColor), this, map_size,
                        center_position);
}

Layer& Frame::GetRadarScans() { return radar_scans_; }
Layer& Frame::GetWorkingSpace() { return working_space_; }
Layer& Frame::GetRoute() { return route_; }
Layer& Frame::GetBorders() { return borders_; }
Layer& Frame::GetDestMark() { return dest_mark_; }
Layer& Frame::GetPosition() { return position_; }

struct UpdateData {
  Layer& layer;
  std::binary_semaphore semaphore = std::binary_semaphore(0);
};

void Frame::Update(Layer& layer) {
  wxCommandEvent update_event(kUpdateEvent, wxID_ANY);
  UpdateData* data = new UpdateData{.layer = layer};
  update_event.SetClientData(data);
  wxPostEvent(this, update_event);

  data->semaphore.acquire();
  delete data;
}

void Frame::UpdateEvent(wxCommandEvent& event) {
  UpdateData& data = *static_cast<UpdateData*>(event.GetClientData());

  data.layer.Update();
  data.semaphore.release();
}

void Frame::KeyPressed(wxKeyEvent& event) {
  switch (event.GetUnicodeKey()) {
    case 'S':
      radar_scans_.Show(!radar_scans_.IsShown());
      break;
    case 'W':
      working_space_.Show(!working_space_.IsShown());
      break;
    case 'R':
      route_.Show(!route_.IsShown());
      break;
    case 'B':
      borders_.Show(!borders_.IsShown());
      break;
    case 'D':
      dest_mark_.Show(!dest_mark_.IsShown());
      break;
    case 'P':
      position_.Show(!position_.IsShown());
      break;
    default:
      std::cout << (char)event.GetUnicodeKey();
  }
}

}  // namespace IF