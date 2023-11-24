#pragma once
#include <list>

namespace Primitives {

struct Coord {
  int x;
  int y;

  Coord operator*(float coef) const {
    Coord ret = *this;
    ret.x *= coef;
    ret.y *= coef;
    return ret;
  }
};

class Primitive {
 public:
  virtual std::list<Coord> GetGraphic(float px_per_mm,
                                      int line_width) const = 0;
};

class Segment : public Primitive {
 public:
  Segment() = default;
  Segment(const Coord& a_point, const Coord& b_point);

  Coord& GetA();
  Coord& GetB();
  const Coord& GetA() const;
  const Coord& GetB() const;

  std::list<Coord> GetGraphic(float px_per_mm,
                              int line_width = 1) const override;

 private:
  Coord a_point_;
  Coord b_point_;

  float GetKCoefficient() const;
  int GetBCoefficient() const;

  std::list<Coord> GetGraphic() const;

  std::list<Coord> GetGraphic(const Coord& a_point, int lenth,
                              float k_coefficient, int b_coefficient) const;
};

}  // namespace Primitives