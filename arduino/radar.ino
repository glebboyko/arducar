#include <Adafruit_VL53L0X.h>

#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"

#define CE_PIN 8
#define CSN_PIN 10

#define PIPE 111156789
#define CHANNEL 0x6f

class LaserRange {
public:
    LaserRange() {
        if (!laser_.begin()) {
            active_ = false;
        }
        laser_.setMeasurementTimingBudgetMicroSeconds(200000);
    }

    int GetDist() {
        if (!active_) {
            return -2;
        }

        VL53L0X_RangingMeasurementData_t raw_dist;
        laser_.rangingTest(&raw_dist, false);
        if (laser_.timeoutOccurred()) {
            return -1;
        }

        int dist = min(raw_dist.RangeMilliMeter, 2000);
        return dist;
    }

    Adafruit_VL53L0X laser_ = Adafruit_VL53L0X();
    bool active_ = true;
};

class Radio {
public:
    Radio() {
        radio_.begin();
        radio_.setDataRate (RF24_250KBPS); // Скорость обмена данными 1 Мбит/сек
        radio_.setCRCLength(RF24_CRC_8);
        radio_.setChannel(CHANNEL);
        radio_.setAutoAck(false);
        radio_.setPALevel(RF24_PA_LOW);
        radio_.powerUp();
    }

    void SetReceiveMode() {
        radio_.openReadingPipe(1, PIPE);
    }
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

LaserRange* laser;
Radio* radio;


void setup() {
    radio = new Radio;
    laser = new LaserRange;

    radio->SetSendMode();
}

void loop() {
    int dist = laser->GetDist();
    radio->Send(dist);
}