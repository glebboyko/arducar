#pragma once

namespace CONST {

// map
const int kPxSizeX = 1500;
const int kPxSizeY = 1000;

const float kMmPerPx = 5;

// movement
const int kBorderOffset = 500;
const int kDistThreshold = 500;

// connection
const std::string kServerAddress = "192.168.1.100";
const int kTcpCommPort = 44'450;

const int kTcpArduinoPort = 44'440;

// hardware
const float kRadarWidth = 1.8;

const int kStepsPerCircle = 1'570;
const int kStepsPerMeter = 2'000;

const int kDefaultSpeed = 2;

}  // namespace CONST