#pragma once

#include <cstddef>
#include <memory_resource>
#include <utility>
#include <vector>

namespace order_statistics {
namespace detail {

// `T` is the value type
// `N` is the number of elements that can be stored inline.
template <typename T, std::size_t N>
class Stack {
  alignas(T) char storage[N * sizeof(T)];
  std::pmr::monotonic_buffer_resource memory;
  std::pmr::vector<T> stack;

 public:
  Stack();
  explicit Stack(std::pmr::memory_resource *upstream);

  bool empty() const;

  template <typename U>
  void push(U&& element);

  void pop();

  T& top();
  const T& top() const;    
};

template <typename T, std::size_t N>
Stack<T, N>::Stack()
: Stack(std::pmr::get_default_resource()) {}

template <typename T, std::size_t N>
Stack<T, N>::Stack(std::pmr::memory_resource *upstream)
: memory(storage, sizeof storage, upstream)
, stack(&memory) {
  stack.reserve(N);
}

template <typename T, std::size_t N>
bool Stack<T, N>::empty() const {
  return stack.empty();
}

template <typename T, std::size_t N>
template <typename U>
void Stack<T, N>::push(U&& element) {
  stack.push_back(std::forward<U>(element));
}

template <typename T, std::size_t N>
void Stack<T, N>::pop() {
  stack.pop_back();
}

template <typename T, std::size_t N>
T& Stack<T, N>::top() {
  return stack.back();
}
  
template <typename T, std::size_t N>
const T& Stack<T, N>::top() const {
  return stack.back();
}

} // namespace detail
} // namespace order_statistics
