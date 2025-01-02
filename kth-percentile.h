#pragma once

#include <cassert>
#include <cmath>
#include <cstddef>
#include <functional>
#include <queue>
#include <vector>

namespace order_statistics {

template <typename Value, std::size_t percentile /* "k" */, typename Key = std::identity>
class KthPercentile {
  static_assert(percentile > 0);
  static_assert(percentile <= 100);

  struct KeyLess {
    bool operator()(const Value&, const Value&) const;
  };
  struct KeyGreater {
    bool operator()(const Value&, const Value&) const;
  };

  // max-heap of all elements less than or equal to the k'th percentile value.
  std::priority_queue<Value, std::vector<Value>, KeyLess> lower;
  // min-heap of all elements greater than the k'th percentile value.
  std::priority_queue<Value, std::vector<Value>, KeyGreater> higher;

  // Move elements between `lower` and `higher` until
  // `lower.size() == percentile * (lower.size() + higher.size()) / 100`.
  void rebalance();

 public:
  // Return the kth-percentile value.
  // The behavior is undefined if this object is empty.
  const Value& get() const;

  void insert(const Value&);
  void insert(Value&&);
};

template <typename Value, std::size_t percentile, typename Key>
bool KthPercentile<Value, percentile, Key>::KeyLess::operator()(const Value& left, const Value& right) const {
  return Key()(left) < Key()(right);
}

template <typename Value, std::size_t percentile, typename Key>
bool KthPercentile<Value, percentile, Key>::KeyGreater::operator()(const Value& left, const Value& right) const {
  return Key()(left) > Key()(right);
}

template <typename Value, std::size_t percentile, typename Key>
const Value& KthPercentile<Value, percentile, Key>::get() const {
  return lower.top();
}

template <typename Value, std::size_t percentile, typename Key>
void KthPercentile<Value, percentile, Key>::insert(const Value& value) {
  if (lower.empty()) {
    lower.push(value);
    return;
  }

  if (KeyGreater()(value, lower.top())) {
    higher.push(value);
  } else {
    lower.push(value);
  }

  rebalance();
}

template <typename Value, std::size_t percentile, typename Key>
void KthPercentile<Value, percentile, Key>::insert(Value&& value) {
  if (lower.empty()) {
    lower.push(std::move(value));
    return;
  }

  if (KeyGreater()(value, lower.top())) {
    higher.push(std::move(value));
  } else {
    lower.push(std::move(value));
  }

  rebalance();
}

template <typename Value, std::size_t percentile, typename Key>
void KthPercentile<Value, percentile, Key>::rebalance() {
  const std::size_t n = lower.size() + higher.size();
  const std::size_t lower_target = std::round(percentile / 100.0 * n);
  const std::size_t higher_target = std::round((100 - percentile) / 100.0 * n);
  
  while (lower.size() < lower_target) {
    assert(!higher.empty());
    lower.push(higher.top());
    higher.pop();
  }

  while (higher.size() < higher_target) {
    assert(!lower.empty());
    higher.push(lower.top());
    lower.pop();
  }
}

} // namespace order_statistics
