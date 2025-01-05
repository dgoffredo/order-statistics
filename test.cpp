#include <algorithm>
#include <cmath>
#include <cstdint>
#include <iterator>
#include <limits>
#include <ostream>
#include <string>
#include <string_view>
#include "kth-percentile.h"
#include "tree.h"
#include "test.h"

struct Fish {
    std::string name;
    int age;
    
    bool operator==(const Fish& other) const {
        return other.age == age && other.name == name;
    }

    friend std::ostream& operator<<(std::ostream& out, const Fish& fish) {
        return out << "Fish{.age=" << fish.age << ", .name=\"" << fish.name << "\"}";
    }
};

void test_kth_percentile() {
    const auto by_age = [](const Fish& fish) { return fish.age; };
    order_statistics::KthPercentile<Fish, 10, decltype(by_age)> p10;
    order_statistics::KthPercentile<Fish, 50, decltype(by_age)> p50;
    order_statistics::KthPercentile<Fish, 70, decltype(by_age)> p70;
    order_statistics::KthPercentile<Fish, 95, decltype(by_age)> p95;
    std::vector<Fish> sorted;

    // <https://www.thesprucepets.com/lifespans-of-aquarium-fish-1378340>
    const Fish fishes[] = {
        {"Adolfos cory (Corydoras adolfoi)", 5},
        {"Angelfish (Pterophyllum scalare)", 10},
        {"Apistogramma (Apistogramma cacatuoides)", 7},
        {"Archer fish (Toxotes spp.)", 5},
        {"Armored catfish (Corydoras spp., Brochis spp.)", 10},
        {"Australian Lungfish (Neoceratodus forsteri)", 90},
        {"Axelrod's rainbowfish (Melantaenia herbertaxelrodi )", 5},
        {"Bala shark (Balantiocheilos melanopterus)", 10},
        {"Bandit cory (Corydoras metae)", 5},
        {"Banjo catfish (Acanthobunocephalus nicoi)", 8},
        {"Betta (Betta splendens)", 3},
        {"Black-banded leporinus (Leporinus fasciatus)", 7},
        {"Black neon tetra (Hyphessobrycon herbertaxelrodi)", 5},
        {"Black phantom tetra (Megalamphodus megalopterus)", 5},
        {"Black shark (Labeo chrysophekadion)", 7},
        {"Black widow tetra (Gymnocorymbus ternetzi)", 5},
        {"Blackfin cory (Corydoras leucomelas)", 5},
        {"Blackskirt tetra (Gymnocorymbus ternetzi)", 5},
        {"Bloodfin tetra (Aphyocharax anisitsi)", 6},
        {"Blue gourami (Trichopodus trichopterus)", 4},
        {"Boeseman's rainbowfish (Melanotaenia boesemani)", 5},
        {"Bolivian ram (Mikrogeophagus altispinosus)", 4},
        {"Bronze cory (Corydoras aeneus)", 5},
        {"Cardinal tetra (Paracheirodon axelrodi)", 4},
        {"Cherry barb (Puntius titteya)", 6},
        {"Chocolate gourami (Sphaerichthys osphromenoides)", 5},
        {"Clown loach (Chromobotia macracanthus)", 15},
        {"Columbian tetra (Hyphessobrycon columbianus)", 5},
        {"Congo tetra (Phenacogrammus interruptus)", 5},
        {"Convict cichlid (Amatitlania nigrofasciata)", 10},
        {"Debauwi catfish (Pareutropius debauwi)", 8},
        {"Diamond tetra (Moenkhausia pittier)", 5},
        {"Discus (Symphysodon aequifasciata)", 14},
        {"Dojo loach (Misgurnus anguillicaudatus)", 10},
        {"Dwarf gourami (Colisa lalia)", 5},
        {"Festivum cichlid (Mesonauta festivus)", 10},
        {"Figure 8 puffer (Tetraodon biocellatus)", 15},
        {"Firemouth cichlid (Thorichthys meeki)", 13},
        {"Frontosa cichlid (Cyphotilapia frontosa)", 11},
        {"Giant danio (Devario aequipinnatus)", 6},
        {"Glassfish (Parambassis ranga)", 8},
        {"Glowlight tetra (Hemigrammus erthrozonus)", 5},
        {"Gold barb (Puntius semifasciolatus)", 6},
        {"Goldfish (Carassius auratus auratus)", 20},
        {"Guppy (Poecilia reticulata)", 4},
        {"Harlequin rasbora (Trigonostigma heteromorpha)", 6},
        {"Hatchetfish (Gasteropelecus sternicla))", 5},
        {"Hognose brochis (Brochis multiradiatus)", 10},
        {"Honey gourami (Colisa chuna, Colisa sota)", 6},
        {"Jack Dempsey cichlid (Rocio octofasciata)", 14},
        {"Jordan's Catfish (Arius seemani)", 10},
        {"Killifish (Fundulopanchax  spp.)", 2},
        {"Kissing gourami (Helostoma temmincki)", 7},
        {"Koi (Cyprinus rubrofuscus)",  60},
        {"Kribensis cichlid (Pelvicachromis pulcher)", 5},
        {"Lake Kutubu rainbowfish (Melanotaenia lacustris)", 5},
        {"Lake Madagascar rainbowfish (Bedotia geayi )", 5},
        {"Lake Wanam rainbowfish (Glossolepis wanamensis)", 5},
        {"Lemon cichlid (Neolamprologus leleupi)", 8},
        {"Lemon tetra (Hyphessobrycon pulchripinnis)", 5},
        {"Leopard danio (Danio frankei)", 4},
        {"Leporinus (Leporinus sp.)", 5},
        {"Livingstoni cichlid (Nimbochromis livingstonii)", 10},
        {"Marigold swordtail (Xiphophorus helleri)", 4},
        {"Midas cichlid (Amphilophus citrinellus)", 15},
        {"Molly (Poecilia latipinna)", 5},
        {"Moonlight gourami (Trichogaster microlepis)", 4},
        {"Neon rainbowfish (Melanotaenia praecox)", 4},
        {"Neon tetra (Paracheirodon innesi )", 5},
        {"Oscar cichlid (Astronotus ocellatus)", 15},
        {"Otocinclus (Otocinculus sp.)", 5},
        {"Pacu (Colossoma sp.)", 30},
        {"Pearl danio (Danio albolineatus)", 5},
        {"Pearl gourami (Trichogaster leeri)", 8},
        {"Pictus catfish (Pimelodus pictus)", 8},
        {"Piranha (Serrasalmus piraya)", 10},
        {"Platy (Xiphorphorus maculatus)", 5},
        {"Plecostomus (Hypostomus plecostomus)", 15},
        {"Powder blue gourami (Colisa lalia)", 4},
        {"Rafael catfish (Platydoras costatus)", 15},
        {"Rainbow shark (Epalzeorhynchos frenatum)", 10},
        {"Ram cichlid (Mikrogeophagus ramirezi)", 4},
        {"Rasbora (Trigonostigma heteromorpha)", 10},
        {"Red bellied piranha (Pygocentrus nattereri)", 10},
        {"Red eye tetra (Moenkhausia sanctaefilomenae)", 5},
        {"Red fin shark (Epalzeorhynchos frenatum)", 8},
        {"Red hook silver dollar (Metynnis argenteus)", 10},
        {"Red rainbowfish (Glossolepis incisus)", 5},
        {"Red-tailed catfish (Phractocephalus hemioliopterus)", 15},
        {"Red tuxedo platy (Xiphophorus maculatus)", 4},
        {"Red wagtail platy (Xiphophorus maculatus)", 4},
        {"Rosy barb (Puntius conchonius)", 5},
        {"Royal pleco (Panaque nigrolineatus)", 10},
        {"Rummy nose tetra (Hemigrammus rhodostomus)", 7},
        {"Sailfin molly (Poecilia latipinna)", 5},
        {"Serpae Tetra (Hyphessobrycon eques)", 7},
        {"Severum cichlid (Heros efasciatus)", 10},
        {"Silver pacu (Colossoma brachypomus)", 20},
        {"Silvertip tetra (Hasemania nana)", 5},
        {"Swordtail (Xiphorphorus hellerii)", 5},
        {"Texas cichlid (Herichthys cyanoguttatus)", 10},
        {"Tiger barb (Puntius tetrazona)", 6},
        {"Tinfoil barb (Barbonymus schwanenfeldii)", 10},
        {"Upside-down catfish (Synodontis nigriventris)", 10},
        {"Weather loach (Misgurnus anguillicaudatus)", 10},
        {"Whiptail catfish(Rineloricaria fallax)", 10},
        {"White cloud mountain minnow (Tanichthys albonubes)", 7},
        {"Yellow tetra (Hyphessobrycon bifasciatus)", 5},
        {"Zebra cichlid (Metriaclima callainos)", 10},
        {"Zebra danio (Danio rerio)", 5},  
    };

    for (const Fish& fish : fishes) {
        p10.insert(fish);
        p50.insert(fish);
        p70.insert(fish);
        p95.insert(fish);
        sorted.push_back(fish);
        std::sort(sorted.begin(), sorted.end(),
            [](const Fish& left, const Fish& right) { return left.age < right.age; });

        ADD_CONTEXT(sorted.size());
        ADD_CONTEXT(fish);
        ADD_CONTEXT(sorted);

        ASSERT_EQUAL(p10.get().age, sorted[10 * sorted.size() / 100].age);
        ASSERT_EQUAL(p50.get().age, sorted[50 * sorted.size() / 100].age);
        ASSERT_EQUAL(p70.get().age, sorted[70 * sorted.size() / 100].age);
        ASSERT_EQUAL(p95.get().age, sorted[95 * sorted.size() / 100].age);
    }
}

void test_enclosing_power_of_2() {
    // `oracle` calculates the expected answer in a different way than the true
    // implementation.
    const auto oracle = [](auto integer) {
        using Uint = decltype(integer);
        if (integer == 0) {
           return Uint(1);
        }
        const int power = std::ceil(std::log2(integer));
        if (power >= std::numeric_limits<Uint>::digits) {
            return Uint(0);
        }
        return Uint(Uint(1) << power);
    };

    // Try all 65,536 values of `uint16_t`.
    std::uint16_t i = 0;
    do {
        ADD_CONTEXT(i);
        ASSERT_EQUAL(order_statistics::detail::enclosing_power_of_2(i), oracle(i));
        ++i;
    } while (i != 0);
}

template <typename T>
void debug_print(std::ostream& out, order_statistics::TreeNode<T> *node, int indent = 0) {
    static const auto tabstop = std::string(2, ' ');
    for (int i = 0; i < indent; ++i) {
        out << tabstop;
    }

    if (!node) {
        out << "()";
        return;
    }

    out << "([";
    const auto values = node->values();
    auto iter = values.begin();
    const auto end = values.end();
    if (iter != end) {
        out << *iter;
        for (++iter; iter != end; ++iter) {
            out << ' ' << *iter;
        }
    }
    // `std::uint8_t TreeNode::height` might print as a `char` because it might
    // be a `typedef` for `unsigned char`. 😾
    out << "] {h:" << int(node->height) << " w:" << node->weight << '}';

    if (!node->left && !node->right) {
        out << ')';
        return;
    }

    out << '\n';
    debug_print(out, node->left, indent + 1);
    out << '\n';
    debug_print(out, node->right, indent + 1);
    out << ')';
}

template <typename T, typename K>
void debug_print(const order_statistics::Tree<T, K>& tree) {
    debug_print(std::cerr, tree.get_root_for_testing());
    std::cerr << "\n\n";
}

void test_tree() {
    const auto by_length = [](const std::string& str) { return str.size(); };
    order_statistics::Tree<std::string, decltype(by_length)> tree;

    debug_print(tree);
    ASSERT_EQUAL(tree.size(), 0u);

    const char *values[] = {"a", "b", "ab", "abc", "abcde", "abcd", "abcdef"};

    for (std::size_t i = 0; i < std::size(values); ++i) {
        ADD_CONTEXT(i);
        ADD_CONTEXT(values[i]);
        tree.insert(values[i]);
        debug_print(tree);
        ASSERT_EQUAL(tree.size(), i + 1);
    }

    // Sort the input values so we can test `nth_element`.
    const auto length_less = [](std::string_view left, std::string_view right) {
        return left.size() < right.size();
    };
    std::stable_sort(std::begin(values), std::end(values), length_less);
    
    // `nth_element`
    for (std::size_t i = 0; i < std::size(values); ++i) {
        ADD_CONTEXT(i);
        ADD_CONTEXT(values[i]);
        ASSERT_EQUAL(tree.nth_element(i), values[i]);
    }

    // `nth_elements`
    for (std::size_t i = 0; i < std::size(values); ++i) {
        ADD_CONTEXT(i);
        const std::span<const std::string> elements = tree.nth_elements(i);
        ASSERT_EQUAL(elements.empty(), false);
        const auto [begin, end] =
            std::equal_range(std::begin(values), std::end(values), elements[0], length_less);
        ASSERT_EQUAL(std::equal(begin, end, elements.begin(), elements.end()), true);
    }
}

int main() {
    test_kth_percentile();
    test_enclosing_power_of_2();
    test_tree();
}
