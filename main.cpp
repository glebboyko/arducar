#include <unistd.h>
#include <wx/wx.h>

#include <chrono>
#include <iostream>
#include <thread>

#include "env-shot.hpp"
#include "frame.hpp"

#define Now() std::chrono::system_clock::now()

class MainThread : public wxThread {
 public:
  MainThread(IF::Frame* frame) : wxThread(wxTHREAD_DETACHED), frame_(frame) {}

  virtual void* Entry() {
    auto t_start = Now();
    EnvShot shot(2000, 2000, 1, 10);

    double radar_deg = 30;

    auto start = Now();
    for (int i = 0; i < 200; ++i) {
      int x = i * 10 + 200;

      frame_->SetPosition(x, 1000, i * 20);

      shot.AddMeasure(i * 20, 200, radar_deg, {x, 1000},
                      frame_->GetMap());
      frame_->UpdateMap();
      std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }
    auto end = Now();

    std::cout << "Measure: "
              << std::chrono::duration_cast<std::chrono::milliseconds>(end -
                                                                       start)
                         .count() /
                     (360 / radar_deg)
              << "ms\n";

    start = Now();
    shot.CreateImage("image.ppm");
    end = Now();

    std::cout << "Image creation: "
              << std::chrono::duration_cast<std::chrono::milliseconds>(end -
                                                                       start)
                     .count()
              << "ms\n";
    return nullptr;
  }

 private:
  IF::Frame* frame_;
};

class App : public wxApp {
 public:
  virtual bool OnInit() {
    IF::Frame* frame = new IF::Frame(wxSize(2000, 2000));

    frame->Show(true);

    MainThread* thread = new MainThread(frame);
    thread->Run();
    return true;
  }
};

wxIMPLEMENT_APP(App);
/*
int main() {
  auto t_start = Now();
  EnvShot shot(4000, 4000, 0.2);

  int deg = 38.5;
  double radar_deg = 1.8;

  auto start = Now();
  for (int i = 0; i < 360 / radar_deg; ++i) {
    shot.AddMeasure(i * radar_deg, 2000, radar_deg, {10000, 10000});
  }
  auto end = Now();

  std::cout << "Measure: "
            << std::chrono::duration_cast<std::chrono::milliseconds>(end -
                                                                     start)
                       .count() /
                   (360 / radar_deg)
            << "ms\n";

  start = Now();
  shot.CreateImage("image.ppm");
  end = Now();

  std::cout << "Image creation: "
            << std::chrono::duration_cast<std::chrono::milliseconds>(end -
                                                                     start)
                   .count()
            << "ms\n";
}*/
