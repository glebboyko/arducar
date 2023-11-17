#pragma once

#include <algorithm>
#include <optional>
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

template <typename T, typename Y, typename FuncF, typename FuncG>
  requires requires(T t_val, Y y_val, FuncF func_f, FuncG func_g) {
    func_f(t_val, t_val);
    func_g(t_val, y_val);
    func_g(y_val, y_val);
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

  void SegmUpdate(const Y& val, int left, int right) {
    SegmUpdate(0, val, left, right);
  }

 private:
  struct Segment {
    int left;
    int right;
  };

  struct Node {
    T val;
    Y debt;

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
    Y curr_debt = data_[node_index].debt;
    if (curr_debt == func_g_.GetNeutral()) {
      return;
    }

    int left_son_index = GetLeftIndex(node_index);
    int right_son_index = GetRightIndex(node_index);

    if (IsLeaf(left_son_index)) {
      data_[left_son_index].val = func_g_(data_[left_son_index].val, curr_debt);
      data_[right_son_index].val =
          func_g_(data_[right_son_index].val, curr_debt);

      Update(node_index);
    } else {
      data_[left_son_index].debt =
          func_g_(data_[left_son_index].debt, curr_debt);
      data_[right_son_index].debt =
          func_g_(data_[right_son_index].debt, curr_debt);
    }
    data_[node_index].debt = func_g_.GetNeutral();
  }

  void SegmUpdate(int curr_node, const Y& val, int left, int right) {
    if (IsLeaf(curr_node)) {
      data_[curr_node].val = func_g_(data_[curr_node].val, val);
      Update(GetParentIndex(curr_node));
      return;
    }

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

  void Update(int node_ind) const {
    T left_son_val = data_[GetLeftIndex(node_ind)].val;
    T right_son_val = data_[GetRightIndex(node_ind)].val;
    data_[node_ind].val = func_f_(left_son_val, right_son_val);

    if (node_ind != 0) {
      Update(GetParentIndex(node_ind));
    }
  }
};

struct CircleSegment {
  int deg_left;
  int deg_right;
  int radius;
};
template <typename T>
struct NumCont {
  T data;
  int index;
};

auto NumerateSegments(const std::vector<CircleSegment>& env_shot) {
  std::vector<NumCont<CircleSegment>> num_env_shot(env_shot.size());
  for (int i = 0; i < num_env_shot.size(); ++i) {
    num_env_shot[i] = {env_shot[i], i};
  }
  return num_env_shot;
};

struct FuncF {
  CircleSegment operator()(const CircleSegment& left,
                           const CircleSegment& right) const {
    return {std::min(left.deg_left, right.deg_left),
            std::max(left.deg_right, right.deg_right),
            std::max(left.radius, right.radius)};
  }
  CircleSegment GetNeutral() const { return {INT32_MAX, INT32_MIN, 0}; }
};

struct DiffCircleSegment {
  std::optional<CircleSegment> left;
  std::optional<CircleSegment> right;
};
bool operator==(const DiffCircleSegment& left, const DiffCircleSegment& right) {
  return !(left.left.has_value() || left.right.has_value() ||
           right.left.has_value() || right.right.has_value());
}

struct FuncG {
  CircleSegment operator()(CircleSegment left,
                           const DiffCircleSegment& right) const {
    if (right.left.has_value() && right.left->radius > left.radius) {
      left = Cut(left, right.left.value());
    }
    if (right.right.has_value() && right.right->radius > left.radius) {
      left = Cut(left, right.right.value());
    }
    return left;
  }
  DiffCircleSegment operator()(DiffCircleSegment left,
                               DiffCircleSegment right) const {
    std::vector<CircleSegment> segments;
    if (left.left.has_value()) {
      segments.push_back(left.left.value());
    }
    if (left.right.has_value()) {
      segments.push_back(left.right.value());
    }

    if (right.left.has_value()) {
      segments.push_back(right.left.value());
    }
    if (right.right.has_value()) {
      segments.push_back(right.right.value());
    }

    std::sort(segments.begin(), segments.end(),
              [](const CircleSegment& left, const CircleSegment& right) {
                return left.deg_left <= right.deg_left;
              });

    for (int i = 1; i < segments.size(); ++i) {
      if (IsIntersect(segments[i - 1], segments[i])) {
        segments[i - 1] = Unite(segments[i - 1], segments[i]);
        segments.erase(segments.cbegin() + i);
        i -= 1;
      }
    }

    switch (segments.size()) {
      case 0:
        return {{}, {}};
      case 1:
        return {segments[0], {}};
      case 2:
        return {segments[0], segments[1]};
      default:
        throw std::logic_error("More than two segments!");
    }
  }
  DiffCircleSegment GetNeutral() const { return {{}, {}}; }

 private:
  CircleSegment Cut(const CircleSegment& from, const CircleSegment& del) const {
    // Возможен при технической ошибке
    if (from.deg_left >= del.deg_left && from.deg_right <= del.deg_right) {
      return from.deg_left == del.deg_left
                 ? CircleSegment{from.deg_left, from.deg_left, 0}
                 : CircleSegment{from.deg_right, from.deg_right, 0};
    }
    if (from.deg_left <= del.deg_left) {
      return {from.deg_left, del.deg_left, from.radius};
    }
    return {del.deg_right, from.deg_right, from.radius};
  }
  CircleSegment Unite(const CircleSegment& first,
                      const CircleSegment& second) const {
    return {std::min(first.deg_left, second.deg_left),
            std::max(first.deg_right, second.deg_right),
            std::max(first.radius, second.radius)};
  }
  bool IsIntersect(CircleSegment first, CircleSegment second) const {
    if (first.deg_left > second.deg_left) {
      std::swap(first, second);
    }
    return second.deg_left <= first.deg_right;
  }
};

std::vector<CircleSegment> GetEnvShot(
    std::vector<CircleSegment> circle_segments) {
  auto sorted_segments = NumerateSegments(circle_segments);
  std::sort(sorted_segments.begin(), sorted_segments.end(),
            [](const NumCont<CircleSegment>& left,
               const NumCont<CircleSegment>& right) {
              return left.data.radius == right.data.radius
                         ? left.data.deg_left < right.data.deg_left
                         : left.data.radius > right.data.radius;
            });

  std::vector<int> degs_left(circle_segments.size());
  std::vector<int> degs_right(circle_segments.size());
  for (int i = 0; i < circle_segments.size(); ++i) {
    degs_left[i] = circle_segments[i].deg_left;
    degs_right[i] = circle_segments[i].deg_right;
  }

  SegmentTree<CircleSegment, DiffCircleSegment, FuncF, FuncG> segment_tree(
      circle_segments.size());

  segment_tree.SetData(circle_segments.begin(), circle_segments.end());

  for (const auto& segment : sorted_segments) {
    auto real_segment = segment_tree.Query(segment.index, segment.index);
    if (real_segment.radius == 0) {
      continue;
    }

    int range_min = std::lower_bound(degs_right.begin(), degs_right.end(),
                                     real_segment.deg_left) -
                    degs_right.begin();
    int range_max = std::upper_bound(degs_left.begin(), degs_left.end(),
                                     real_segment.deg_right) -
                    degs_left.begin();

    if (range_max - range_min < 0) {
      continue;
    }

    DiffCircleSegment upd_val = {real_segment, {}};
    segment_tree.SegmUpdate(upd_val, range_min, range_max);
    segment_tree.Query(segment.index, segment.index);
  }

  std::vector<CircleSegment> result;
  result.reserve(circle_segments.size());
  for (int i = 0; i < circle_segments.size(); ++i) {
    auto segment = segment_tree.Query(i, i);
    if (segment.radius == 0) {
      continue;
    }
    result.push_back(segment);
  }
  return result;
}

}  // namespace Map