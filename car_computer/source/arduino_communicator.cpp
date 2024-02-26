#include "arduino_communicator.hpp"

namespace AC {

/*--------------------------- arduino communicator ---------------------------*/
ArduinoCommunicator::ArduinoCommunicator() {
  move_client_ = server_.AcceptConnection();
  radar_client_ = server_.AcceptConnection();
}

}  // namespace AC