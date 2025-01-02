#include <algorithm>
#include <ostream>
#include <string>
#include "kth-percentile.h"
#include "test.h"

struct Fish {
    int age;
    std::string name;
    
    bool operator==(const Fish& other) const {
        return other.age == age && other.name == name;
    }

    friend std::ostream& operator<<(std::ostream& out, const Fish& fish) {
        return out << "Fish{.age=" << fish.age << ", .name=\"" << fish.name << "\"}";
    }
};

void test_kth_percentile() {
    const auto by_age = [](const Fish& fish) { return fish.age; };
    order_statistics::KthPercentile<Fish, 50, decltype(by_age)> p50;
    order_statistics::KthPercentile<Fish, 70, decltype(by_age)> p70;
    order_statistics::KthPercentile<Fish, 95, decltype(by_age)> p95;
    std::vector<Fish> sorted;

    const Fish fishes[] = {
        {4, "salmon"},
        {4, "karp"},
        {100, "shark"},
        {4, "tilapia"},
        {3, "clownfish"},
        {15, "goldfish"},
        {20, "snapper"},
        {23, "marlin"},
        {6, "grouper"},
        {200, "rockfish"},
        {1, "minnow"},
        {2, "angelfish"},
    };

    for (const Fish& fish : fishes) {
        p50.insert(fish);
        p70.insert(fish);
        p95.insert(fish);
        sorted.push_back(fish);
        std::sort(sorted.begin(), sorted.end(),
            [](const Fish& left, const Fish& right) { return left.age < right.age; });

        ADD_CONTEXT(sorted.size());
        ADD_CONTEXT(fish);
        ADD_CONTEXT(sorted);

        ASSERT_EQUAL(p50.get().age, sorted[50 * sorted.size() / 100].age);
        ASSERT_EQUAL(p70.get().age, sorted[70 * sorted.size() / 100].age);
        ASSERT_EQUAL(p95.get().age, sorted[95 * sorted.size() / 100].age);
    }
}

int main() {
    test_kth_percentile();
}
