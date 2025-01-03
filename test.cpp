#include <algorithm>
#include <concepts>
#include <cstdint>
#include <limits>
#include <memory_resource>
#include <new>
#include <ostream>
#include <string>
#include "kth-percentile.h"
#include "stack.h"
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
    // Otherwise, return the next power of two.
    return Uint(1) << (width - left_zeros);
}

void test_enclosing_power_of_2() {
    std::uint8_t i = 0;
    do {
        std::cout << "i = " << int(i) << "\tenclosing_power_of_2 = " << int(enclosing_power_of_2(i)) << '\n';
        ++i;
    } while (i != 0);
}

template <std::size_t N>
void test_stack_for_size() {
    // Make sure it doesn't allocate beyond the inline buffer, except when necessary.
    order_statistics::detail::Stack<int, N> stack(std::pmr::null_memory_resource());

    for (std::size_t i = 0; i < N; ++i) {
        stack.push(i);
    }

    // An additional element will trigger reallocation, which will fail due to
    // the backing `null_memory_resource`.
    bool threw = false;
    try {
        stack.push(N);
    } catch (const std::bad_alloc&) {
        threw = true;
    }
    ADD_CONTEXT(N);
    ASSERT_EQUAL(threw, true);
}
    
void test_stack() {
    test_stack_for_size<1>();
    test_stack_for_size<2>();
    test_stack_for_size<3>();
    test_stack_for_size<4>();
    test_stack_for_size<5>();
    test_stack_for_size<6>();
    test_stack_for_size<7>();
    test_stack_for_size<8>();
    test_stack_for_size<9>();
    test_stack_for_size<10>();
    test_stack_for_size<11>();
    test_stack_for_size<12>();
    test_stack_for_size<13>();
    test_stack_for_size<14>();
    test_stack_for_size<15>();
    test_stack_for_size<16>();
    test_stack_for_size<17>();
}

int main() {
    test_kth_percentile();
    // test_enclosing_power_of_2();
    test_stack();
}
