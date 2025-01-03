#pragma once

#include <functional>
#include <vector>

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

 public:
    Tree();
    ~Tree();
    
    // Add the specified value to the tree.
    void insert(Value);

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

template <typename Value, typename Key>
Tree<Value, Key>::~Tree() {
    if (root == nullptr) {
        return;
    }

    std::vector<Node*> stack = {root};
    do {
        Node *const node = stack.back();
        stack.pop_back();
        if (node->left) {
            stack.push_back(node->left);
        }
        if (node->right) {
            stack.push_back(node->right);
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
void Tree<Value, Key>::insert(Value value) {
    // TODO
}

} // namespace order_statistics
