#include <SPI.h>

#include "RF24.h"
#include "nRF24L01.h"

#define STEP_NUM 200
#define CIRCLE_DEG_NUM 360

// radio
#define CE_PIN 9
#define CSN_PIN 10

#define PIPE 111156789
#define CHANNEL 0x6f

// motor
#define STEP_PIN 5
#define DIR_PIN 4
#define SPEED_PIN 6

#define FROM_MAGNET_MC_STEPS -240

// magnet detector
#define HOLL_PIN A7

class Radio {
 public:
  Radio() {
    radio_.begin();
    radio_.setDataRate(RF24_250KBPS);  // Скорость обмена данными 1 Мбит/сек
    radio_.setCRCLength(RF24_CRC_8);
    radio_.setChannel(CHANNEL);
    radio_.setAutoAck(false);
    radio_.setPALevel(RF24_PA_LOW);
    radio_.powerUp();
  }

  void SetReceiveMode() { radio_.openReadingPipe(1, PIPE); }

  void SetSendMode() {
    StopListening();
    radio_.closeReadingPipe(PIPE);
    radio_.openWritingPipe(PIPE);
  }

  void StopListening() { radio_.stopListening(); }

  void StartListening() { radio_.startListening(); }

  bool IsAvailable() { return radio_.available(); }

  int Receive() {
    int data;
    radio_.read(&data, sizeof(data));

    return data;
  }

  void Send(int data) { radio_.write(&data, sizeof(data)); }

 private:
  RF24 radio_ = RF24(CE_PIN, CSN_PIN);
};

class Motor {
 public:
  Motor() {
    pinMode(STEP_PIN, OUTPUT);
    pinMode(DIR_PIN, OUTPUT);
    pinMode(SPEED_PIN, OUTPUT);

    digitalWrite(STEP_PIN, LOW);
    digitalWrite(DIR_PIN, LOW);
    digitalWrite(SPEED_PIN, LOW);
  }

  void ChangeDir() {
    curr_dir_ = !curr_dir_;
    digitalWrite(DIR_PIN, curr_dir_ ? HIGH : LOW);
  }

  void SetSpeed(bool fast) {
    curr_speed_ = fast;
    digitalWrite(SPEED_PIN, curr_speed_ ? LOW : HIGH);
  }

  void MakeStep() {
    digitalWrite(STEP_PIN, HIGH);
    digitalWrite(STEP_PIN, LOW);
    delay(5);

    int step_diff = curr_speed_ ? 16 : 1;
    if (!curr_dir_) {
      step_diff *= -1;
    }

    curr_step_ = (kStepNum + curr_step_ + step_diff) % kStepNum;
  }

  void MakeFullStep() {
    int step_num = curr_speed_ ? 1 : 16;
    for (int i = 0; i < step_num; ++i) {
      MakeStep();
    }
  }

  int GetStep() { return curr_step_; }

  const int FullStepNum = 200;

 private:
  const int kStepNum = FullStepNum * 16;

  bool curr_dir_ = false;
  bool curr_speed_ = true;
  int curr_step_ = 0;
};

bool MagnetDetected() { return analogRead(HOLL_PIN) >= 1000; }

Motor* InitMotor() {
  Motor* motor = new Motor;

  while (!MagnetDetected()) {
    motor->MakeStep();
  }

  motor->SetSpeed(false);
  while (MagnetDetected()) {
    motor->MakeStep();
  }

  if (FROM_MAGNET_MC_STEPS < 0) {
    motor->ChangeDir();
  }
  for (int i = 0; i < abs(FROM_MAGNET_MC_STEPS); ++i) {
    motor->MakeStep();
  }

  if (FROM_MAGNET_MC_STEPS < 0) {
    motor->ChangeDir();
  }

  return motor;
}

Radio* radio;
Motor* motor;

void setup() {
  Serial.begin(9600);

  pinMode(HOLL_PIN, INPUT);

  radio = new Radio;
  radio->SetReceiveMode();

  motor = InitMotor();
}

void loop() {
  if (Serial.available() <= 0) {
    return;
  }
  Serial.readString();

  for (int i = 0; i < STEP_NUM; ++i) {
    float curr_angle = i * (float)CIRCLE_DEG_NUM / STEP_NUM;

    radio->StartListening();

    while (!radio->IsAvailable());
    radio->Receive();

    while (!radio->IsAvailable());
    int measure = radio->Receive();
    radio->StopListening();

    Serial.print(curr_angle, 4);
    Serial.print(" ")
        Serial.println(measure)
            motor->MakeFullStep();
  }
  Serial.println(0);
}