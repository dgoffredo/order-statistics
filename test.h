#pragma once

#include <cstdlib>
#include <iostream>
#include <ostream>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#define TEST_PREAMBLE \
    std::vector<std::pair<std::string, std::string>> test_context

#define ASSERT_EQUAL(LEFT, RIGHT) \
    ASSERT_EQUAL_IMPL(test_context, LEFT, RIGHT)

#define ASSERT_EQUAL_IMPL(CONTEXT, LEFT, RIGHT) \
    assert_equal_impl(__LINE__, CONTEXT, #LEFT, #RIGHT, LEFT, RIGHT)

template <typename Left, typename Right>
void assert_equal_impl(int line, const std::vector<std::pair<std::string, std::string>>& context, const char *left_expr, const char *right_expr, const Left& left_value, const Right& right_value) {
    if (left_value == right_value) {
        return;
    }

    std::cerr << "Assertion failed on line " << line << ":\n\t" << left_expr << " == " << right_expr
        << "\nwith left side:\n\t" << left_value << "\nand right side\n\t" << right_value << '\n';
    if (!context.empty()) {
        std::cerr << "Context:\n";
        for (const auto& [name, value] : context) {
            std::cerr << '\t' << name << ":\t" << value << '\n';
        }
    }

    std::exit(1);
}

#define ADD_CONTEXT(VALUE) \
    ADD_CONTEXT_IMPL(__LINE__, test_context, VALUE)

#define ADD_CONTEXT_IMPL(LINE, CONTEXT, VALUE) \
ADD_CONTEXT_IMPL2(LINE, test_context, VALUE)

#define ADD_CONTEXT_IMPL2(LINE, CONTAINER, VALUE) \
    ContextGuard context_guard_##LINE(CONTAINER, #VALUE, VALUE)

class ContextGuard {
    std::vector<std::pair<std::string, std::string>>& context;

 public:
    template <typename Value>
   ContextGuard(std::vector<std::pair<std::string, std::string>>& context, const char *value_name, const Value& value)
   : context(context) {
       std::ostringstream stream;
       stream << value;
       context.emplace_back(value_name, stream.str());
   }

   ~ContextGuard() {
       context.pop_back();
   }
};

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
