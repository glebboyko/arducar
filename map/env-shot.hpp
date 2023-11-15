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

    data_ = decltype(data_)(2 * num_of_elements_ - 1);
  }
  ~SegmentTree() = default;

  template <Iterator Iter>
  void SetData(Iter begin, Iter end) {
    for (int i = num_of_elements_ - 1; begin != end && i < data_.size();
         ++begin, ++i) {
      data_[i].val = *begin;
    }

    for (int i = num_of_elements_ - 2; i >= 0; --i) {
      data_[i] = func_f_(data_[2 * i + 1].val, data_[2 * i + 2].val);
    }
  }

  T Query(T left, T right) const { return Query(0, left, right); }

  T GetElem(int index) const {
    UpdateElem(index);
    return data_[index].val;
  }

 private:
  struct Node {
    mutable T val = T();
    mutable T debt = T();
  };

  int num_of_elements_;
  std::vector<int> logs_;

  std::vector<Node> data_;

  FuncF func_f_;
  FuncG func_g_;

  int GetLeftIndex(int index) const { return 2 * index + 1; }
  int GetRightIndex(int index) const { return 2 * index + 2; }
  int GetParentIndex(int index) const { return (index - 1) / 2; }

  T Query(int node_index, T left, T right) const {
    PushDebt(node_index);

    int left_son_index = GetLeftIndex(node_index);
    int right_son_index = GetRightIndex(node_index);

    if (left > data_[right_son_index].val ||
        right < data_[left_son_index].val) {
      return T();
    }
    if (left <= data_[left_son_index].val &&
        right >= data_[right_son_index].val) {
      return data_[node_index].val;
    }
    return func_f_(Query(left_son_index, left, right),
                   Query(right_son_index, left, right));
  }

  void PushDebt(int node_index) const {
    if (IsLeaf(node_index)) {
      return;
    }

    int left_son_index = GetLeftIndex(node_index);
    int right_son_index = GetRightIndex(node_index);

    if (IsLeaf(left_son_index)) {
      data_[left_son_index].val =
          func_g_(data_[left_son_index].val, data_[node_index].debt);
      data_[right_son_index].val =
          func_g_(data_[right_son_index].val, data_[node_index].debt);
    } else {
      data_[left_son_index].debt =
          func_g_(data_[left_son_index].debt, data_[node_index].debt);
      data_[right_son_index].debt =
          func_g_(data_[right_son_index].debt, data_[node_index].debt);
    }
    data_[node_index].debt = T();
  }

  bool IsLeaf(int node_index) const {
    return node_index >= num_of_elements_ - 1;
  }

  void UpdateElem(int node_index) const {
    if (node_index != 0) {
      UpdateElem(GetParentIndex(node_index));
    }
    PushDebt(node_index);
  }
};

}  // namespace Map