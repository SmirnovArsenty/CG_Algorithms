#include <iostream>
#include <cstdint>
#include <array>
#include <fstream>
#include <algorithm>

namespace Hamurabi {

typedef int32_t Citizen_t;
typedef float Bushel_t;
typedef int32_t Acre_t;
typedef uint32_t Index_t;

#define SET_VALUE_MIN_MAX(type, value, min, max) \
    constexpr type g_##value##_min = min;        \
    constexpr type g_##value##_max = max;

constexpr Index_t g_round_count = 10;
SET_VALUE_MIN_MAX(Bushel_t, acre_price, 17.f, 26.f)
SET_VALUE_MIN_MAX(Bushel_t, wheat_per_acre, 1.f, 6.f)
SET_VALUE_MIN_MAX(Bushel_t, rats_ate_percent, 0.f, .07f)
constexpr Bushel_t g_citizen_bushel_needs = 20.f; // each citizen need <g_citizen_bushel_needs> bushel per year
constexpr int32_t g_citizen_can_work_on_acres = 10; // maximum acres citizen can work on
constexpr Bushel_t g_acre_need_to_sow = 0.5f; // each acre need <g_acre_need_to_sow> bushels per year
constexpr float g_citizens_dead_to_game_over_percent = 0.45f;
constexpr float g_plugue_probability = 0.15f;
constexpr char* g_save_file_name = "save.txt";

/********************************************\
 *                 Save file                 *
 *********************************************
 * - round_number (uint32_t)                 *
 * - citizens_count (int32_t)                *
 * - dead_citizens_count (int32_t)           *
 * - wheat_bushels_count (int32_t)           *
 * - area_owned_by_city (int32_t)            *
 * - area_used (int32_t)                     *
 * - death_percent (float[round_number - 1]) *
\*********************************************/

class City {
private:
    // use signed fields for exceptions handling
    Citizen_t citizens_{ 0 };
    Citizen_t starved_citizens_{ 0 };
    Bushel_t wheat_{ 0 };
    Acre_t area_{ 0 };
    Acre_t area_used_{ 0 };
public:
    City() = default;
    void set_start_params();

    void set_citizens(Citizen_t citizens) { citizens_ = citizens; }
    Citizen_t get_citizens() const noexcept { return citizens_; }

    void set_starved_citizens(Citizen_t starved_citizens) { starved_citizens_ = starved_citizens; }
    Citizen_t get_starved_citizens() const noexcept { return starved_citizens_; }

    void set_wheat(Bushel_t wheat) { wheat_ = wheat; }
    Bushel_t get_wheat() const noexcept { return wheat_; }

    void set_area(Acre_t area) { area_ = area; }
    Acre_t get_area() const noexcept { return area_; }

    void set_area_used(Acre_t area_used) { area_used_ = area_used; }
    Acre_t get_area_used() const noexcept { return area_used_; }

    bool is_valid() const;
};

class GameStats {
private:
    std::array<float, g_round_count> death_percent_{};
    float acre_per_citizen_{ 0.f };
public:
    GameStats() = default;
    void set_death_percent(Index_t index, float percent) { death_percent_[index] = percent; }
    float get_death_percent(Index_t index) const { return death_percent_[index]; }
    void set_acre_per_citizen(float acre_per_citizen) { acre_per_citizen_ = acre_per_citizen; }
    void print_game_over_message() const;
};

class GameInstance {
private:
    bool is_game_over_{ false };
    Index_t current_round_{ 0 };
    City city_state_; // place start params to 0 index and <g_round_count> rounds history
    GameStats stats_;

    static GameInstance& Instance(GameInstance&);
    GameInstance();
    GameInstance(const GameInstance&);
    GameInstance(GameInstance&&) = default;

    City perform_inputs(Acre_t acres_to_buy /* if sell: negative */,
                        Bushel_t wheat_to_eat,
                        Citizen_t citizen_came,
                        Acre_t acres_to_sow,
                        Bushel_t acre_price,
                        Bushel_t new_bushel_in_barns,
                        bool is_plugue) const;
public:
    static GameInstance& load_from_file(const std::string& path);
    void save_to_file(const std::string& path);

    Index_t get_current_round() const noexcept { return current_round_; }
    void next_round();
};

//////
/// City methods implementation
//////
void City::set_start_params()
{
    citizens_ = 100;
    wheat_ = 2800;
    area_ = 1000;
    area_used_ = 0;
}

bool City::is_valid() const
{
    if (wheat_ < 0) {
        std::cout << "You are out of wheat in barns\n";
        return false;
    }
    if (citizens_ <= 0) {
        std::cout << "No citizens left in the city\n";
        return false;
    }
    if (area_ < area_used_) {
        std::cout << "Area used more than owns\n";
        return false;
    }
    if (citizens_ * g_citizen_can_work_on_acres < area_used_) {
        std::cout << "Not enough citizens for work on this area\n";
        return false;
    }
    return true;
}

//////
/// GameStats methods implementation
//////
void GameStats::print_game_over_message() const
{
    float P{ 0 }; // average annual death percent
    for (Index_t i = 0; i < g_round_count; ++i) {
        P += death_percent_[i];
    }
    P /= g_round_count;

    float L{ acre_per_citizen_ };

    if (P > 0.33 && L < 7) {
        std::cout << "D\n"; // very bad
    } else if (P > 0.1 && L < 9) {
        std::cout << "C\n"; //satisfactorily
    } else if (P > 0.03 && L < 10) {
        std::cout << "B"; // not bad
    } else {
        std::cout << "A";
    }
}

//////
/// GameInstance methods implementation
//////
GameInstance::GameInstance()
    : current_round_{ 0 },
      city_state_{}
{
    city_state_.set_start_params();
}

GameInstance::GameInstance(const GameInstance& other)
{
    is_game_over_ = other.is_game_over_;
    current_round_ = other.current_round_;
    city_state_ = other.city_state_;
    stats_ = other.stats_;
}

// static
GameInstance& GameInstance::Instance(GameInstance& init_game_instance)
{
    static GameInstance instance{ init_game_instance };
    return instance;
}

// static
GameInstance& GameInstance::load_from_file(const std::string& path)
{
    std::fstream f(path, std::fstream::in);
    if (!f.is_open()) {
        return GameInstance::Instance(GameInstance{});
    }

    GameInstance init{}; // try fill init data
    try {
        Index_t round_index{ 0 };
        Citizen_t citizens{ 0 };
        Bushel_t bushels{ 0 };
        Acre_t acres{ 0 };
        Acre_t acres_used{ 0 };

        f >> round_index;

        if (round_index == 0) {
            f.close();
            return GameInstance::Instance(GameInstance{});
        }

        f >> citizens >> bushels >> acres >> acres_used;

        init.current_round_ = round_index; /// TODO: +1?
        init.city_state_.set_citizens(citizens);
        init.city_state_.set_wheat(bushels);
        init.city_state_.set_area(acres);
        init.city_state_.set_area_used(acres_used);

        for (uint32_t i = 0; i < round_index; ++i) {
            float death_percent;
            f >> death_percent;
            init.stats_.set_death_percent(i, death_percent);
        }
    } catch(...) {
        return GameInstance::Instance(GameInstance{});
    }

    f.close();

    // init filled successfuly, use it to initialize GameInstance
    return GameInstance::Instance(init);
}

void GameInstance::save_to_file(const std::string& path)
{
    std::fstream f(path, std::fstream::out);
    f << current_round_ << '\n'
      << city_state_.get_citizens() << '\n'
      << city_state_.get_wheat() << '\n'
      << city_state_.get_area() << '\n'
      << city_state_.get_area_used() << '\n';
    for (Index_t i = 0; i <= current_round_; ++i) {
        f << stats_.get_death_percent(i) << '\n';
    }
    f.close();
}

void GameInstance::next_round()
{
    if (is_game_over_) {
        return;
    }

    std::cout << "Do you want to leave the game? (Y/N)";
    char yes_no{ 0 };
    std::cin >> yes_no;
    if (yes_no == 'y') {
        save_to_file(g_save_file_name);
        is_game_over_ = true;
        return;
    }

    // rest rand
    srand(static_cast<unsigned int>(clock()));

    // calculate round values
    Bushel_t acre_price =
        ((1.f * rand()) / RAND_MAX) // [0.f, 1.f]
        * (g_acre_price_max - g_acre_price_min)
        + g_acre_price_min;
    Bushel_t wheat_per_acre =
        ((1.f * rand()) / RAND_MAX) // [0.f, 1.f]
        * (g_wheat_per_acre_max - g_wheat_per_acre_min)
        + g_wheat_per_acre_min;
    Bushel_t rats_ate_percent =
        ((1.f * rand()) / RAND_MAX) // [0.f, 1.f]
        * (g_rats_ate_percent_max - g_rats_ate_percent_min)
        + g_rats_ate_percent_min;
    float plugue = (1.f * rand()) / RAND_MAX; // [0.f, 1.f]

    Bushel_t harvested_wheat = wheat_per_acre * city_state_.get_area_used();

    Bushel_t rats_ate_bushels = (city_state_.get_wheat() + harvested_wheat) * rats_ate_percent;

    Bushel_t new_bushel_in_barns = city_state_.get_wheat() + harvested_wheat - rats_ate_bushels;

    // print current status
    std::cout << "I beg to report to you:\n";
    std::cout << "In year " << current_round_ + 1 << '\n';

    if (city_state_.get_starved_citizens() > 0) {
        std::cout << city_state_.get_starved_citizens() << " people starved\n";
    }

    Citizen_t new_citizens = static_cast<Citizen_t>(city_state_.get_starved_citizens() / 2 + (5 - wheat_per_acre) * new_bushel_in_barns / 600 + 1);
    if (new_citizens > 50) { // kinda std::clamp from stdc++17
        new_citizens = 50;
    }
    if (new_citizens < 0) {
        new_citizens = 0;
    }
    if (new_citizens > 0) {
        std::cout << new_citizens << " people came to the city\n";
    }

    bool is_plugue{ false };
    if (plugue < g_plugue_probability) {
        std::cout << "The plague wiped out half the population\n";
        is_plugue = true;
    }
    std::cout << "The city population is now " << city_state_.get_citizens() / (is_plugue + 1) << '\n';
    std::cout << "The city now owns " << city_state_.get_area() << " acres.\n";
    std::cout << "We harvested " << harvested_wheat << " bushels of wheat, " << wheat_per_acre << " bushels per acre\n";
    std::cout << "Rats ate " << rats_ate_bushels << " bushels\n";
    std::cout << "Now you have " << new_bushel_in_barns << " bushels in barns\n";

    std::cout << "1 acre of land is now worth " << acre_price << " bushels\n";

    // ask player for input
    do {
        std::cout << "What do you wish, lord?\n";

        std::cout << "How many acres of land do you command to buy? (0 to ask for sell) ";
        Acre_t acres_to_buy{ 0 };
        std::cin >> acres_to_buy;

        if (acres_to_buy == 0) {
            std::cout << "How many acres of land do you command to sell? ";
            std::cin >> acres_to_buy;
            acres_to_buy = -acres_to_buy;
        }

        Bushel_t bushels_to_eat{ 0 };
        std::cout << "How many bushels of wheat do you command to eat? ";
        std::cin >> bushels_to_eat;

        Acre_t acres_to_sow{ 0 };
        std::cout << "How many acres of land do you command to sow? ";
        std::cin >> acres_to_sow;

        City new_city = perform_inputs(acres_to_buy, bushels_to_eat,
                                       new_citizens, acres_to_sow,
                                       acre_price, new_bushel_in_barns, is_plugue);

        if (new_city.is_valid()) {
            // check less than <g_citizens_dead_to_game_over_percent> citizens dead
            if (static_cast<float>(new_city.get_starved_citizens()) / city_state_.get_citizens() >
                g_citizens_dead_to_game_over_percent) {
                std::cout << static_cast<uint32_t>(g_citizens_dead_to_game_over_percent * 100)
                          << "% of citizens dead. Game Over.\n";
                is_game_over_ = true;
                break;
            }

            stats_.set_acre_per_citizen(1.f * new_city.get_area() / new_city.get_citizens());
            stats_.set_death_percent(current_round_, (1.f * new_city.get_starved_citizens() +
                                                      (is_plugue * (city_state_.get_citizens() / 2.f))) /
                                                        city_state_.get_citizens());
            city_state_ = new_city;
            break;
        }
        std::cout << "Unable to apply your inputs for current city\n";
    } while(true);

    ++current_round_;
}

City GameInstance::perform_inputs(Acre_t acres_to_buy,
                             Bushel_t wheat_to_eat,
                             Citizen_t citizen_came,
                             Acre_t acres_to_sow,
                             Bushel_t acre_price,
                             Bushel_t bushel_count,
                             bool is_plugue) const
{
    // buy acres (or sell)
    bushel_count -= acres_to_buy * acre_price;

    // feed citizens
    bushel_count -= wheat_to_eat;

    // sow acres
    bushel_count -= (acres_to_sow) * g_acre_need_to_sow;

    City new_city{};
    new_city.set_wheat(bushel_count);
    new_city.set_area(city_state_.get_area() + acres_to_buy);
    new_city.set_area_used(acres_to_sow);

    is_plugue = !!is_plugue; // only 0 or 1

    Citizen_t starved_citizens = city_state_.get_citizens() / (1 + is_plugue) - std::min(static_cast<Citizen_t>(wheat_to_eat / g_citizen_bushel_needs),
                                                                       city_state_.get_citizens() / (1 + is_plugue));
    new_city.set_starved_citizens(starved_citizens);

    Citizen_t new_citizens = std::min<Citizen_t>(static_cast<Citizen_t>(wheat_to_eat / g_citizen_bushel_needs),
                                                  city_state_.get_citizens() / (1 + is_plugue))
                             + citizen_came;
    new_city.set_citizens(new_citizens);

    return new_city;
}

int run_game(GameInstance& game)
{
    for (Index_t i = game.get_current_round(); i < g_round_count; ++i) {
        game.next_round();
    }
    return 0;
}

} // namespace Hamurabi

int main()
{
    // check game saves
    return Hamurabi::run_game(Hamurabi::GameInstance::load_from_file(Hamurabi::g_save_file_name));
}
