#pragma once

#include <cstdlib>
#include <iostream>
#include <ostream>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#define ADD_CONTEXT(VALUE) \
    ADD_CONTEXT_IMPL(__LINE__, VALUE)

#define ADD_CONTEXT_IMPL(LINE, VALUE) \
ADD_CONTEXT_IMPL2(LINE, VALUE)

#define ADD_CONTEXT_IMPL2(LINE, VALUE) \
    ContextGuard context_guard_##LINE(#VALUE, VALUE)

struct ContextGuard {
    static thread_local std::vector<std::pair<std::string, std::string>> context;

    template <typename Value>
   ContextGuard(const char *value_name, const Value& value) {
       std::ostringstream stream;
       stream << value;
       context.emplace_back(value_name, stream.str());
   }

   ~ContextGuard() {
       context.pop_back();
   }
};

inline thread_local std::vector<std::pair<std::string, std::string>> ContextGuard::context;

#define ASSERT_EQUAL(LEFT, RIGHT) \
    ASSERT_EQUAL_IMPL(LEFT, RIGHT)

#define ASSERT_EQUAL_IMPL(LEFT, RIGHT) \
    assert_equal_impl(__LINE__, #LEFT, #RIGHT, LEFT, RIGHT)

template <typename Left, typename Right>
void assert_equal_impl(int line, const char *left_expr, const char *right_expr, const Left& left_value, const Right& right_value) {
    if (left_value == right_value) {
        return;
    }

    std::cerr << "Assertion failed on line " << line << ":\n\t" << left_expr << " == " << right_expr
        << "\nwith left side:\n\t" << left_value << "\nand right side\n\t" << right_value << '\n';
    if (!ContextGuard::context.empty()) {
        std::cerr << "Context:\n";
        for (const auto& [name, value] : ContextGuard::context) {
            std::cerr << '\t' << name << ":\t" << value << '\n';
        }
    }

    std::exit(1);
}

template <typename Value>
std::ostream& operator<<(std::ostream& stream, const std::vector<Value>& values) {
    stream << '[';
    auto iter = values.begin();
    const auto end = values.end();
    if (iter != end) {
        stream << *iter;
        for (++iter; iter != end; ++iter) {
            stream << ", " << *iter;
        }
    }
    return stream << ']';
}
