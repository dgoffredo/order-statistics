#pragma once

#include <functional>
#include <vector>

#include "stack.h"

namespace order_statistics {

template <typename Value, typename Key = std::identity>
class Tree {
    struct Node {
        std::size_t weight;
        Value value;
        Node *left;
        Node *right;
    };
    Node *root;

    void insert(Node*);

 public:
    Tree();
    ~Tree();

    // Add the specified value to the tree.
    void insert(const Value&);
    void insert(Value&&);

    std::size_t size() const;
    std::size_t empty() const;
    // `rank` in range `[0, size() - 1]`
    const Value& nth_element(std::size_t rank) const;
    // `percent` in range `[1, 100]`
    const Value& percentile(std::size_t percent) const;
    // Zero-based position of the `value` in `Key`-order sequence.
    // The behavior is undefined if `value` is not in the tree.
    std::size_t rank(const Value& value) const;
};

template <typename Value, typename Key>
Tree<Value, Key>::Tree()
: root(nullptr) {}

/* This would be fine too.
namespace detail {

template <typename Node>
void delete_node(Node *node) {
    if (node->left) {
        delete_node(node->left);
    }
    if (node->right) {
        delete_node(node->right);
    }
}

} // namespace detail

template <typename Value, typename Key>
Tree<Value, Key>::~Tree() {
    if (root) {
        detail::delete_node(root);
    }
}
*/

template <typename Value, typename Key>
Tree<Value, Key>::~Tree() {
    if (root == nullptr) {
        return;
    }

    detail::Stack<Node*, 32> stack;
    stack.push(root);
    do {
        Node *const node = stack.top();
        stack.pop();
        if (node->left) {
            stack.push(node->left);
        }
        if (node->right) {
            stack.push(node->right);
        }
        delete node;
    } while (!stack.empty());
}

template <typename Value, typename Key>
std::size_t Tree<Value, Key>::size() const {
    if (root) {
        return root->weight;
    }
    return 0;
}

template <typename Value, typename Key>
std::size_t Tree<Value, Key>::empty() const {
    return size() == 0;
}

template <typename Value, typename Key>
void Tree<Value, Key>::insert(const Value& value) {
    Node *newbie = new Node{.weight = 1, .value = value, .left = nullptr, .right = nullptr};
    insert(newbie);
}

template <typename Value, typename Key>
void Tree<Value, Key>::insert(Value&& value) {
    Node *newbie = new Node{.weight = 1, .value = std::move(value), .left = nullptr, .right = nullptr};
    insert(newbie);
}

template <typename Value, typename Key>
void Tree<Value, Key>::insert(Tree<Value, Key>::Node *node) {
    assert(node);
    // TODO
}

} // namespace order_statistics
