#define DIR1 4
#define DIR2 5
#define STEP 12
#define MS1 7
#define MS2 8
#define MS3 9

class Car {
 public:
  Car(int pin_dir1, int pin_dir2, int pin_step, int pin_ms1, int pin_ms2,
      int pin_ms3)
      : pin_dir1_(pin_dir1),
        pin_dir2_(pin_dir2),
        pin_step_(pin_step),
        pin_ms1_(pin_ms1),
        pin_ms2_(pin_ms2),
        pin_ms3_(pin_ms3) {
    pinMode(pin_dir1_, OUTPUT);
    pinMode(pin_dir2_, OUTPUT);
    pinMode(pin_step_, OUTPUT);
    pinMode(pin_ms1_, OUTPUT);
    pinMode(pin_ms2_, OUTPUT);
    pinMode(pin_ms3_, OUTPUT);

    digitalWrite(pin_step_, LOW);

    SetDir(1);
    SetSpeed(1);
  }

  /* 1 = full;
     2 = 1/2;
     4 = 1/4;
     8 = 1/8;
     16 = 1/16
  */
  void SetSpeed(int speed) {
    switch (speed) {
      case 1:
        digitalWrite(pin_ms1_, LOW);
        digitalWrite(pin_ms2_, LOW);
        digitalWrite(pin_ms3_, LOW);
        break;
      case 2:
        digitalWrite(pin_ms1_, HIGH);
        digitalWrite(pin_ms2_, LOW);
        digitalWrite(pin_ms3_, LOW);
        break;
      case 4:
        digitalWrite(pin_ms1_, LOW);
        digitalWrite(pin_ms2_, HIGH);
        digitalWrite(pin_ms3_, LOW);
        break;
      case 8:
        digitalWrite(pin_ms1_, HIGH);
        digitalWrite(pin_ms2_, HIGH);
        digitalWrite(pin_ms3_, LOW);
        break;
      case 16:
        digitalWrite(pin_ms1_, HIGH);
        digitalWrite(pin_ms2_, HIGH);
        digitalWrite(pin_ms3_, HIGH);
        break;
    }
  }

  /* 0 = left;
     1 = forward;
     2 = right
  */
  void SetDir(int dir) {
    switch(dir) {
      case 0:
        digitalWrite(pin_dir1_, LOW);
        digitalWrite(pin_dir2_, LOW);
        break;
      case 1:
        digitalWrite(pin_dir1_, LOW);
        digitalWrite(pin_dir2_, HIGH);
        break;
      case 2:
        digitalWrite(pin_dir1_, HIGH);
        digitalWrite(pin_dir2_, HIGH);
        break;
    }
  }

  void MakeStep() {
    if (millis() - last_step_ < 5) {
      delay(5 - (millis() - last_step_));
    }

    digitalWrite(pin_step_, HIGH);
    digitalWrite(pin_step_, LOW);

    last_step_ = millis();
  }

 private:
  int pin_dir1_;
  int pin_dir2_;
  int pin_step_;
  int pin_ms1_;
  int pin_ms2_;
  int pin_ms3_;

  long last_step_ = 0;
};

Car* car;

void setup() {
  Serial.begin(9600);

  car = new Car(DIR1, DIR2, STEP, MS1, MS2, MS3);
}

void loop() {
  if (Serial.available() <= 0) {
    return;
  }

  int steps = Serial.parseInt();
  int dir = Serial.parseInt();
  int speed = Serial.parseInt();

  car->SetDir(dir);
  car->SetSpeed(speed);

  for (int i = 0; i < steps; ++i) {
    car->MakeStep();
    Serial.println(i + 1);
  }
}