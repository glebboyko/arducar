class Motor {
 public:
  Motor(int pin_a, int pin_b)
      : pin_a_(pin_a), pin_b_(pin_b) {
    pinMode(pin_a_, OUTPUT);
    pinMode(pin_b_, OUTPUT);
  }

  ~Motor() { Release(); }

  // speed should be in range [0,1023]
  void SetSpeed(int speed) {
    speed_ = speed;
    if (speed > 0) {
      analogWrite(pin_b_, 0);
      analogWrite(pin_a_, speed / 4);
    } else {
      analogWrite(pin_a_, 0);
      analogWrite(pin_b_, (-speed) / 4);
    }
  }

  void Stop() {
    speed_ = 1024;
    analogWrite(pin_a_, 255);
    analogWrite(pin_b_, 255);
  }
  void Release() {
    speed_ = 0;
    analogWrite(pin_a_, 0);
    analogWrite(pin_b_, 0);
  }

  int GetSpeed() const { return speed_; }

 private:
  int pin_a_;
  int pin_b_;

  int speed_ = 0;
};

class Car {
 public:
  Car(int left_pin_a, int left_pin_b, int right_pin_a, int right_pin_b)
      : left_(Motor(left_pin_a, left_pin_b)),
        right_(Motor(right_pin_a, right_pin_b)) {}

  // speed should be in range [0,1023]
  void SetSpeed(int speed) { SetSpeed(speed, angle_speed_); }
  void SetAngleSpeed(int angle_speed) { SetSpeed(speed_, angle_speed); }

  void Stop() {
    speed_ = 1024;
    angle_speed_ = 0;

    left_.Stop();
    right_.Stop();
  }
  void Release() {
    speed_ = 0;
    angle_speed_ = 0;

    left_.Release();
    right_.Release();
  }

  int GetSpeed() const { return speed_; }
  int GetAngleSpeed() const { return angle_speed_; }

 private:
  Motor left_;
  Motor right_;

  int speed_ = 0;
  int angle_speed_ = 0;

  void SetSpeed(int speed, int angle_speed) {
    angle_speed_ = angle_speed;

    int left_speed = speed + (angle_speed_ / 2) + (angle_speed_ % 2);
    int right_speed = speed - (angle_speed_ / 2);

    int exceeding_left = GetExceeding(left_speed, 1023);
    int exceeding_right = GetExceeding(right_speed, 1023);
    left_speed -= exceeding_left + exceeding_right;
    right_speed -= exceeding_left + exceeding_right;

    left_.SetSpeed(left_speed);
    right_.SetSpeed(right_speed);

    speed_ = (left_speed + right_speed) / 2;
  }

  static int GetExceeding(int val, int threshold) {
    if (val > threshold) {
      return val - threshold;
    }
    if (val < -threshold) {
      return val + threshold;
    }
    return 0;
  }
};

void setup() {

}

void loop() {

}