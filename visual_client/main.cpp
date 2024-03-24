#include <wx/wx.h>

#include <chrono>
#include <memory>
#include <thread>

#include "communicator.hpp"
#include "constants.hpp"
#include "data_processor.hpp"
#include "frame.hpp"

class MainThread : public wxThread {
 public:
  MainThread(std::shared_ptr<IF::Frame> frame,
             std::shared_ptr<Communicator> communicator,
             MSG::InitData init_data)
      : wxThread(wxTHREAD_DETACHED),
        communicator_(communicator),
        frame_(frame),
        init_data_(init_data) {}

  virtual void* Entry() {
    DataProcessor data_processor(*frame_, *communicator_, init_data_,
                                 CONST::kLineWidth);
    data_processor.MainLoop();
    return nullptr;
  }

 private:
  std::shared_ptr<IF::Frame> frame_;
  std::shared_ptr<Communicator> communicator_;

  MSG::InitData init_data_;
};

class App : public wxApp {
 public:
  virtual bool OnInit() {
    std::shared_ptr<Communicator> communicator(
        new Communicator(CONST::kServerAddress.c_str(), CONST::kTcpCommPort));

    MSG::InitData init_data;
    while (true) {
      auto tmp_init_data = communicator->GetInitData();
      if (tmp_init_data.has_value()) {
        init_data = tmp_init_data.value();
        break;
      }
      std::this_thread::sleep_for(
          std::chrono::milliseconds(TCP::kDefLoopPeriod));
    }

    std::shared_ptr<IF::Frame> frame(
        new IF::Frame({init_data.px_size_x, init_data.px_size_y}));
    frame->Show(true);

    MainThread* thread = new MainThread(frame, communicator, init_data);
    thread->Run();

    return true;
  }
};

wxIMPLEMENT_APP(App);