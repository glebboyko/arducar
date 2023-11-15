#include "map/env-shot.hpp"
#include <iostream>

class Plus {
 public:
  int operator()(int left, int right) const {
    return left + right;
  }
  int GetNeutral() const {
    return 0;
  }
};

class Multiply {
 public:
  int operator()(int left, int right) const {
    return left * right;
  }
  int GetNeutral() const {
    return 1;
  }
};

int main() {
  Map::SegmentTree<int, Plus, Multiply> segment_tree(10);
  std::vector<int> array;
  for (int i = 1; i <= 10; ++i) {
    array.push_back(i);
  }
  segment_tree.SetData(array.begin(), array.end());
  while (true) {
    int mode;
    std::cin >> mode;
    switch (mode) {
      case 0:
        int left, right;
        std::cin >> left >> right;
        std::cout << segment_tree.Query(left, right) << "\n";
        break;
      case 1:
        int val, left1, right1;
        std::cin >> val;
        segment_tree.SegmUpdate(val, left1, right1);
    }
  }

}