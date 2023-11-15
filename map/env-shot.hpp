#pragma once

#include <algorithm>
#include <vector>

namespace Map {
template <typename Iter>
concept Iterator =
    std::is_same_v<typename std::iterator_traits<Iter>::iterator_category,
                   std::random_access_iterator_tag>;

std::vector<int> GetNextLog(int max_num) {
  std::vector<int> nums;
  nums.push_back(1);

  while (max_num > nums.back()) {
    nums.push_back(nums.back() * 2);
  }

  return nums;
}

template <typename T, typename FuncF, typename FuncG>
  requires requires(T val, FuncF func_f, FuncG func_g) {
    func_f(val, val);
    func_g(val, val);
  }
class SegmentTree {
 public:
  SegmentTree(int num_of_elements) {
    logs_ = GetNextLog(num_of_elements);
    num_of_elements_ =
        *std::lower_bound(logs_.begin(), logs_.end(), num_of_elements);

    data_ = decltype(data_)(2 * num_of_elements_ - 1,
                            {func_f_.GetNeutral(), func_g_.GetNeutral()});

    for (int i = 0; i < num_of_elements_; ++i) {
      data_[i + num_of_elements_ - 1].segment = {i, i};
    }
    for (int i = num_of_elements_ - 2; i >= 0; --i) {
      data_[i].segment = UniteSegm(data_[GetLeftIndex(i)].segment,
                                   data_[GetRightIndex(i)].segment);
    }
  }
  ~SegmentTree() = default;

  template <Iterator Iter>
  void SetData(Iter begin, Iter end) {
    for (int i = num_of_elements_ - 1; begin != end && i < data_.size();
         ++begin, ++i) {
      data_[i].val = *begin;
    }

    for (int i = num_of_elements_ - 2; i >= 0; --i) {
      data_[i].val =
          func_f_(data_[GetLeftIndex(i)].val, data_[GetRightIndex(i)].val);
    }
  }

  T Query(int left, int right) const { return Query(0, left, right); }

  T GetElem(int index) const {
    UpdateElem(index);
    return data_[index].val;
  }

  void SegmUpdate(const T& val, int left, int right) {
    SegmUpdate(0, val, left, right);
  }

 private:
  struct Segment {
    int left;
    int right;
  };

  struct Node {
    T val;
    T debt;

    Segment segment;
  };
  enum EnumIntersect { In, Intersect, NoIntersect };

  int num_of_elements_;
  std::vector<int> logs_;

  mutable std::vector<Node> data_;

  FuncF func_f_;
  FuncG func_g_;

  int GetLeftIndex(int index) const { return 2 * index + 1; }
  int GetRightIndex(int index) const { return 2 * index + 2; }
  int GetParentIndex(int index) const { return (index - 1) / 2; }
  bool IsLeaf(int node_index) const {
    return node_index >= num_of_elements_ - 1;
  }

  EnumIntersect IntersectSegm(const Segment& first,
                              const Segment& second) const {
    if (first.left > second.right || first.right < second.left) {
      return NoIntersect;
    }
    if (first.left >= second.left && first.right <= second.right) {
      return In;
    }
    return Intersect;
  }
  Segment UniteSegm(const Segment& first, const Segment& second) {
    return {std::min(first.left, second.left),
            std::max(first.right, second.right)};
  }

  T Query(int node_index, int left, int right) const {
    PushDebt(node_index);

    int left_son_index = GetLeftIndex(node_index);
    int right_son_index = GetRightIndex(node_index);

    switch (IntersectSegm(data_[node_index].segment, {left, right})) {
      case NoIntersect:
        return func_f_.GetNeutral();
      case In:
        return data_[node_index].val;
      case Intersect:
        return func_f_(Query(left_son_index, left, right),
                       Query(right_son_index, left, right));
    }
  }

  void PushDebt(int node_index) const {
    if (IsLeaf(node_index)) {
      return;
    }

    int curr_debt = data_[node_index].debt;

    int left_son_index = GetLeftIndex(node_index);
    int right_son_index = GetRightIndex(node_index);

    if (IsLeaf(left_son_index)) {
      data_[left_son_index].val = func_g_(data_[left_son_index].val, curr_debt);
      data_[right_son_index].val =
          func_g_(data_[right_son_index].val, curr_debt);
    } else {
      data_[left_son_index].debt =
          func_g_(data_[left_son_index].debt, curr_debt);
      data_[right_son_index].debt =
          func_g_(data_[right_son_index].debt, curr_debt);
    }
    data_[node_index].debt = func_g_.GetNeutral();
  }

  void UpdateElem(int node_index) const {
    if (node_index != 0) {
      UpdateElem(GetParentIndex(node_index));
    }
    PushDebt(node_index);
  }

  void SegmUpdate(int curr_node, const T& val, int left, int right) {
    switch (IntersectSegm(data_[curr_node].segment, {left, right})) {
      case NoIntersect:
        return;
      case In:
        data_[curr_node].debt = func_g_(data_[curr_node].debt, val);
        return;
      case Intersect:
        SegmUpdate(GetLeftIndex(curr_node), val, left, right);
        SegmUpdate(GetRightIndex(curr_node), val, left, right);
    }
  }
};

}  // namespace Map