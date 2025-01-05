#pragma once

#include <bit>
#include <cassert>
#include <cstdint>
#include <concepts>
#include <functional>
#include <limits>
#include <memory>
#include <type_traits>
#include <span>
#include <utility>
#include <vector>

namespace order_statistics {
namespace detail {

// Return the smallest power of two that is greater than or equal to `value`,
// or return zero if that power of two cannot be expressed as a `Uint`.
template <std::unsigned_integral Uint>
Uint enclosing_power_of_2(Uint value) {
    if (!value) {
        return 1;
    }
    constexpr int width = std::numeric_limits<Uint>::digits;
    const int left_zeros = std::countl_zero(value);
    const Uint high_only = Uint(1) << (width - left_zeros - 1);
    if (high_only == value) {
        return value;
    }
    return high_only << 1;
}

template <typename Func>
class ScopeExitGuard {
    Func func;

 public:
    template <typename U>
    explicit ScopeExitGuard(U&& func)
    : func(std::forward<U>(func)) {}
    
    ~ScopeExitGuard() {
        func();
    }
};

template <typename Func>
ScopeExitGuard<Func> on_scope_exit(Func&& func) {
    return ScopeExitGuard<Func>(std::forward<Func>(func));
} 

} // namespace detail

// The move constructor of the node's value type `T` must not throw exceptions.
// This is needed to ensure the strong exception guarantee of
// `TreeNode<T>::insert`.
template <typename T>
concept TreeNodeValue = std::is_nothrow_move_constructible_v<T>;

template <TreeNodeValue T>
class TreeNode {
 public:
    // Note that `weight` must be listed first in order for MSVC to pack the
    // bit fields as tightly as possible. 57 + 6 + 1 = 64.
    std::uint64_t weight : 57;
    std::uint8_t height : 6;
    enum { IN_PLACE, ALLOCATED } storage : 1;

    TreeNode *left;
    TreeNode *right;

 private:
    // If this node has only one element, then it might be stored as
    // `in_place`. Otherwise, `allocated` points to storage for an array of
    // elements. It's possible that this node has only one element, but that
    // it's nonetheless in storage pointed to by `allocated` -- it's a
    // necessary edge case to preserve the strong exception guarantee for
    // `insert`. See `insert`.
    // The selected union field is indicated by `storage`.
    union {
        T in_place;
        char *allocated;
    };

 public:
    explicit TreeNode(const T&);
    explicit TreeNode(T&&);
    ~TreeNode();

    TreeNode() = delete;
    TreeNode& operator=(const TreeNode&) = delete;
    TreeNode& operator=(TreeNode&&) = delete;

    std::span<const T> values() const;

    std::size_t left_height() const;
    std::size_t right_height() const;

    std::size_t left_weight() const;
    std::size_t right_weight() const;

    std::size_t size() const;

    void insert(const T&);
    void insert(T&&);

    void replace_children(TreeNode *new_left, TreeNode *new_right);
    
    // TODO: document
    static std::pair<const TreeNode*, std::size_t> get(const TreeNode&, std::size_t rank);

 private:
    template <typename U>
    void generic_insert(U&& value);
};

template <TreeNodeValue T>
TreeNode<T>::TreeNode(const T& value)
: weight(1)
, height(1)
, storage(IN_PLACE)
, left()
, right()
, in_place(value) {}

template <TreeNodeValue T>
TreeNode<T>::TreeNode(T&& value)
: weight(1)
, height(1)
, storage(IN_PLACE)
, left()
, right()
, in_place(std::move(value)) {}

template <TreeNodeValue T>
TreeNode<T>::~TreeNode() {
    if (storage == IN_PLACE) {
        in_place.~T();
        return;
    }
    assert(storage == ALLOCATED);
    for (const T& value : values()) {
        value.~T();
    }
    delete[] allocated;
}

template <TreeNodeValue T>
std::span<const T> TreeNode<T>::values() const {
    if (storage == IN_PLACE) {
        return std::span<const T>(&in_place, 1);
    }
    assert(storage == ALLOCATED);
    return std::span<const T>(std::launder(reinterpret_cast<const T*>(allocated)), size());
}

template <TreeNodeValue T>
std::size_t TreeNode<T>::left_weight() const {
    return left ? left->weight : 0;
}

template <TreeNodeValue T>
std::size_t TreeNode<T>::right_weight() const {
    return right ? right->weight : 0;
}

template <TreeNodeValue T>
std::size_t TreeNode<T>::size() const {
    return weight - left_weight() - right_weight();
}

template <TreeNodeValue T>
std::size_t TreeNode<T>::left_height() const {
    return left ? left->height : 0;
}

template <TreeNodeValue T>
std::size_t TreeNode<T>::right_height() const {
    return right ? right->height : 0;
}

template <TreeNodeValue T>
void TreeNode<T>::insert(const T& value) {
    generic_insert(value);
}

template <TreeNodeValue T>
void TreeNode<T>::insert(T&& value) {
    generic_insert(std::move(value));
}

template <TreeNodeValue T>
void TreeNode<T>::replace_children(TreeNode *new_left, TreeNode *new_right) {
    const std::size_t my_size = size();
    left = new_left;
    right = new_right;
    weight = my_size + left_weight() + right_weight();
    height = 1 + std::max(left_height(), right_height());
}

template <TreeNodeValue T>
std::pair<const TreeNode<T>*, std::size_t> TreeNode<T>::get(const TreeNode<T>& node, std::size_t rank) {
   if (rank < node.left_weight()) {
       // It's an element to our left.
       return get(*node.left, rank);
   } 
   if (rank - node.left_weight() < node.size()) {
       // It's one of our elements.
       return {&node, rank - node.left_weight()};
   }
   // It's an element to our right.
   return get(*node.right, rank - node.left_weight() - node.size());
}

// Note that in order for `generic_insert` to provide the strong exception
// guarantee, the order of statements in its implementation is a bit subtle.
template <TreeNodeValue T>
template <typename U>
void TreeNode<T>::generic_insert(U&& value) {
    if (storage == IN_PLACE) {
        // We need to allocate `allocated` and then move `in_place` into it and
        // append `value`.
        char *const new_storage = new char[2 * sizeof(T)];
        new (new_storage) T(std::move(in_place));
        // Maybe at the `return` below, and maybe if `~T()` throws.
        // In the latter case, I think it's undefined behavior to assign to
        // `allocated`, but I bet it's fine.
        const auto guard = detail::on_scope_exit([&, this]() {
            storage = ALLOCATED;
            allocated = new_storage;
        });
        in_place.~T();
        // copy/move the new element
        new (new_storage + sizeof(T)) T(std::forward<U>(value));
        ++weight;
        return;
    }

    // Append to `allocated`.
    assert(storage == ALLOCATED);
    const std::size_t size = values().size();
    const std::size_t capacity = detail::enclosing_power_of_2(size);
    char *destination;
    if (size == capacity) {
        // We have to reallocate to larger storage and move the elements over.
        std::unique_ptr<char[]> new_storage(new char[2 * capacity * sizeof(T)]);
        char *const old_storage = allocated;
        T *const begin = std::launder(reinterpret_cast<T*>(old_storage));
        const T *const end = begin + size;
        destination = new_storage.get();
        for (auto iter = begin; iter != end; ++iter, destination += sizeof(T)) {
            new (destination) T(std::move(*iter));
        }
        allocated = new_storage.release();
        // Now `allocated` is just a higher-capacity version of what we started
        // with. Before we append `value`, first destroy the old elements that
        // were just moved-from. This way, if any of the destructors throw, the
        // sequence of elements appears unchanged (strong exception guarnatee),
        // even though we leak `old_storage`.
        for (auto iter = begin; iter != end; ++iter) {
            iter->~T();
        }
        delete[] old_storage;
    } else {
        // There's already room for `value`.
        destination = allocated + size * sizeof(T);
    }
    new (destination) T(std::forward<U>(value));
    ++weight;
}

template <typename T, typename GetKey = std::identity>
class Tree {
    using Node = TreeNode<T>;
    Node *root;

 public:
    Tree();
    ~Tree();

    Tree(const Tree&) = delete;
    Tree(Tree&&) = delete;
    Tree& operator=(const Tree&) = delete;
    Tree& operator=(Tree&&) = delete;

    // Add the specified value to the tree.
    void insert(const T&);
    void insert(T&&);

    // Remove all values from the tree.
    void clear();

    std::size_t size() const;
    std::size_t empty() const;
    
    // Return the element whose zero-based `GetKey`-order index is the
    // specified `rank`, where elements having the same key are ordered by
    // their order of insertion. `rank` is between 0 and `size() - 1`, inclusive.
    const T& nth_element(std::size_t rank) const;
    
    // Return all elements whose zero-based `GetKey`-order index could be the
    // specified `rank`. `nth_elements` might return more than one element
    // because elements can have the same key. `rank` is between 0 and
    // `size() - 1`, inclusive.
    //
    // For example, consider the following sorted sequence of elements, where
    // each element is identified by its key and its insertion order relative
    // to other elements having its key:
    //
    //     [A0, B0, B1, C0, D0, D1, D2, D3, E0, F0] (10 elements)
    //
    // That is, there is 1 "A", 2 "B"s, 1 "C", 4 "D"s, and so on. Then:
    //
    // - `nth_elements(0) is [A0]`
    // - `nth_elements(k) is [B0, B1] for k in 1, 2`
    // - `nth_elements(3) is [C0]`
    // - `nth_elements(k) is [D0, D1, D2, D3] for k in 4, 5, 6, 7`
    // - `nth_elements(8) is [E0]`
    // - `nth_elements(9) is [F0]`
    std::span<const T> nth_elements(std::size_t rank) const;

    // Return all elements whose `GetKey` key is in the specified percentile.
    // `percent` is between 1 and 100, inclusive.
    // The n'th percentile is the smallest key `k` such that the keys of at
    // least n% of elements are less than or equal to `k`.
    std::span<const T> percentile(std::size_t percent) const;

    // TODO
    // {min, max} of possible zero-based position of the `value` in `GetKey`-order
    // sequence.
    std::pair<std::size_t, std::size_t> rank(const T& value) const; // TODO
    
    // Return all elements whose `GetKey` key is the same as the key of the
    // specified `value`.
    std::span<const T> equal_range(const T& value) const; // TODO
    
    // Please don't.
    Node *get_root_for_testing() const;

 private:
    template <typename U>
    void generic_insert(U&& value);
    
    template <typename U>
    static Node *generic_insert(Node *into, U&& value);
    
    static Node *balance(Node*);
    static Node *rotate_left(Node*);
    static Node *rotate_right(Node*);
    static void dispose(Node*);

    // TODO: document
    std::pair<std::span<const T>, std::size_t> get(std::size_t rank) const;
};

template <typename T, typename GetKey>
Tree<T, GetKey>::Tree()
: root(nullptr) {}

template <typename T, typename GetKey>
void Tree<T, GetKey>::dispose(Node *node) {
    Node *left = node->left;
    Node *right = node->right;
    delete node;
    if (left) {
        dispose(left);
    }
    if (right) {
        dispose(right);
    }
}

template <typename T, typename GetKey>
Tree<T, GetKey>::~Tree() {
    if (root) {
        dispose(root);
    }
}

template <typename T, typename GetKey>
std::size_t Tree<T, GetKey>::size() const {
    return root ? root->weight : 0;
}

template <typename T, typename GetKey>
std::size_t Tree<T, GetKey>::empty() const {
    return size() == 0;
}

template <typename T, typename GetKey>
void Tree<T, GetKey>::insert(const T& value) {
    generic_insert(value);
}

template <typename T, typename GetKey>
void Tree<T, GetKey>::insert(T&& value) {
    generic_insert(std::move(value));
}

template <typename T, typename GetKey>
template <typename U>
void Tree<T, GetKey>::generic_insert(U&& value) {
    root = generic_insert(root, std::forward<U>(value));
}

template <typename T, typename GetKey>
void Tree<T, GetKey>::clear() {
    if (root) {
        dispose(root);
        root = nullptr;
    }
}

template <typename T, typename GetKey>
template <typename U>
TreeNode<T> *Tree<T, GetKey>::generic_insert(TreeNode<T> *into, U&& value) {
    if (into == nullptr) {
        return new Node(std::forward<U>(value));
    }

    const auto& value_key = GetKey()(value);
    const auto& node_key = GetKey()(into->values()[0]);
    if (value_key < node_key) {
        const std::size_t size = into->size();
        into->left = generic_insert(into->left, std::forward<U>(value));
        into->weight = size + into->left_weight() + into->right_weight();
        into->height = 1 + std::max(into->left_height(), into->right_height());
    } else if (node_key < value_key) {
        const std::size_t size = into->size();
        into->right = generic_insert(into->right, std::forward<U>(value));
        into->weight = size + into->left_weight() + into->right_weight();
        into->height = 1 + std::max(into->left_height(), into->right_height());
    } else {
        into->insert(std::forward<U>(value));
        // `insert` takes care of increasing `weight`, and `height` doesn't
        // change.
    }

    return balance(into);
}

template <typename T, typename GetKey>
TreeNode<T> *Tree<T, GetKey>::balance(TreeNode<T> *node) {
    assert(node);
    switch (const int diff = node->right_height() - node->left_height()) {
    case 2: {
      // right-heavy: rotate left.
    
      // If the right side is left-heavy, then rotating left will just make us
      // left-heavy. First we might have to rotate the right side to the right.
      if (const int diff = node->right->left_height() - node->right->right_height(); diff > 0) {
          assert(diff == 1);
          node->right = rotate_right(node->right);
      }
      return rotate_left(node);
    }
    case -2:
      // left-heavy: rotate right.
    
      // If the left side is right-heavy, then rotating right will just make us
      // right-heavy. First we might have to rotate the left side to the left.
      if (const int diff = node->left->left_height() - node->left->right_height(); diff < 0) {
          assert(diff == -1);
          node->left = rotate_left(node->left);
      }
      return rotate_right(node);
    default:
        assert(diff == 0 || diff == 1 || diff == -1);
        // already balanced enough (rotating wouldn't help)
        return node;
    }
}

template <typename T, typename GetKey>
TreeNode<T> *Tree<T, GetKey>::rotate_left(TreeNode<T> *node) {
    //         B                     A
    //       ./ \.                 ./ \.
    //     low   A        →        B  high
    //         ./ \.             ./ \.
    //     middle  high        low  middle
    //
    Node* B = node;
    assert(B);
    Node* low = B->left;
    Node* A   = B->right;
    assert(A); // because we're right-heavy
    Node* middle = A->left;
    Node* high   = A->right;

    B->replace_children(low, middle);
    A->replace_children(B, high);

    return A;
}

template <typename T, typename GetKey>
TreeNode<T> *Tree<T, GetKey>::rotate_right(TreeNode<T> *node) {
    //
    //           A                 B
    //         ./ \.             ./ \.
    //         B  high    →    low   A
    //       ./ \.                 ./ \.
    //     low  middle         middle  high
    //
    Node* A = node;
    assert(A);
    Node* B    = A->left;
    Node* high = A->right;
    assert(B); // because we're left-heavy
    Node* low    = B->left;
    Node* middle = B->right;
    
    A->replace_children(middle, high);
    B->replace_children(low, A);

    return B;
}

template <typename T, typename GetKey>
TreeNode<T> *Tree<T, GetKey>::get_root_for_testing() const {
    return root;
}

template <typename T, typename GetKey>
std::pair<std::span<const T>, std::size_t> Tree<T, GetKey>::get(std::size_t rank) const {
    assert(root);
    const auto [node, offset] = Node::get(*root, rank);
    return {node->values(), offset};
}

template <typename T, typename GetKey>
const T& Tree<T, GetKey>::nth_element(std::size_t rank) const {
    const auto [values, offset] = get(rank);
    return values[offset];
}
    
template <typename T, typename GetKey>
std::span<const T> Tree<T, GetKey>::nth_elements(std::size_t rank) const {
    const auto [values, _] = get(rank);
    return values;
}

template <typename T, typename GetKey>
std::span<const T> Tree<T, GetKey>::percentile(std::size_t percent) const {
    const std::size_t rank = std::min(percent * size() / 100, size() - 1);
    return nth_elements(rank);
}

} // namespace order_statistics
