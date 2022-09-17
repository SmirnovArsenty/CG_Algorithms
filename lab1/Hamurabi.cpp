#include <iostream>
#include <cstdint>
#include <array>
#include <fstream>

namespace Hamurabi {

// types size is important for save file
typedef int16_t Citizen_t;
typedef float Bushel_t;
typedef int16_t Acre_t;

#define SET_VALUE_MIN_MAX(type, value, min, max) \
    constexpr type g_##value##_min = min;        \
    constexpr type g_##value##_max = max;

constexpr uint16_t g_round_count = 10;
SET_VALUE_MIN_MAX(Bushel_t, acre_price, 17.f, 26.f)
SET_VALUE_MIN_MAX(Bushel_t, wheat_per_acre, 1.f, 6.f)
SET_VALUE_MIN_MAX(Bushel_t, rats_ate_percent, 0.f, .07f)
constexpr Bushel_t g_citizen_bushel_needs = 20.f; // each citizen need <g_citizen_bushel_needs> bushel per year
constexpr uint16_t g_citizen_can_work_on_acres = 10; // maximum acres citizen can work on
constexpr Bushel_t g_acre_need_to_sow = 0.5f; // each acre need <g_acre_need_to_sow> bushels per year

class City {
private:
    // use signed fields for exceptions handling
    Citizen_t citizens_{ 0 }; // count
    Bushel_t wheat_{ 0 }; // in bushels
    Acre_t area_{ 0 }; // acres
public:
    City() = default;
    void set_start_params();

    void set_citizens(Citizen_t citizens) { citizens_ = citizens; }
    Citizen_t get_citizens() const noexcept { return citizens_; }

    void set_wheat(Bushel_t wheat) { wheat_ = wheat; }
    Bushel_t get_wheat() const noexcept { return wheat_; }

    void set_area(Acre_t area) { area_ = area; }
    Acre_t get_area() const noexcept { return area_; }
};

void City::set_start_params()
{
    citizens_ = 100;
    wheat_ = 2800;
    area_ = 1000;
}

typedef uint16_t Index_t;

class GameInstance {
private:
    Index_t current_round_{ 0 };
    /// TODO: use only one state
    std::array<City, g_round_count + 1> city_history_; // place start params to 0 index and <g_round_count> rounds history
    static GameInstance& Instance(GameInstance&);
    GameInstance() = default;
    GameInstance(const GameInstance&) = default;
    GameInstance(GameInstance&&) = default;

    City perform_inputs(Acre_t acres_to_buy /* if sell: negative */,
                        Bushel_t wheat_to_eat,
                        Acre_t acres_to_sow,
                        Bushel_t acre_price);
public:
    static GameInstance& load_from_file(const std::string& path);
    void save_to_file(const std::string& path);

    Index_t get_current_round() const noexcept { return current_round_; }
    void move_to_next_round();

};

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
        return GameInstance::Instance({});
    }

    GameInstance init{}; // try fill init data
    try {
        Index_t saved_rounds_count;
        f >> saved_rounds_count;
        init.current_round_ = saved_rounds_count; /// TODO: +1?
        for (Index_t i = 0; i < saved_rounds_count; ++i) {
            City& city_round_i = init.city_history_[i];

            Citizen_t citizens{ 0 };
            Bushel_t bushels{ 0 };
            Acre_t arces{ 0 };

            f >> citizens >> bushels >> arces;
            city_round_i.set_citizens(citizens);
            city_round_i.set_wheat(bushels);
            city_round_i.set_area(arces);
        }
    } catch(...) {
        return GameInstance::Instance({});
    }

    f.close();

    // init filled successfuly, use it to initialize GameInstance
    return GameInstance::Instance(init);
}

void GameInstance::save_to_file(const std::string& path)
{
    std::fstream f(path, std::fstream::out);
    f << current_round_;
    for (Index_t i = 0; i < current_round_; ++i) {
        City& city_round_i = city_history_[i];
        f << city_round_i.get_citizens() << " "
          << city_round_i.get_wheat() << " "
          << city_round_i.get_area() << " ";
    }
}

void GameInstance::move_to_next_round() {
    // calculate round values
    Bushel_t acre_price =
        ((1.f * rand()) / RAND_MAX) // [0.f, 1.f]
        * (g_acre_price_max - g_acre_price_min + 1)
        + g_acre_price_min;
    Bushel_t wheat_per_acre = 
        ((1.f * rand()) / RAND_MAX) // [0.f, 1.f]
        * (g_wheat_per_acre_max - g_wheat_per_acre_min + 1)
        + g_wheat_per_acre_min;
    Bushel_t rats_ate_percent = 


    // print current status
    printf("My lord, deign to tell you:\n");
    printf("in year %d of your exalted reign\n", current_round_);

    Citizen_t dead_citizens{ 0 };
    if (dead_citizens > 0) {
        printf("%d people starved to death\n", dead_citizens);
    }
    Citizen_t new_citizens{ 0 };
    if (new_citizens > 0) {
        printf("%d people arrived in our great city\n", new_citizens); 
    }

    bool is_plugue{ false };
    if (is_plugue) {
        printf("The plague wiped out half the population\n");
    }
    printf("The city's population is now %d\n", 0);
    printf("We harvested %d bushels of wheat, %d bushels per acre\n", 0, 0);
    printf("Rats ate %d bushels of wheat, leaving %d bushels in barns\n", 0, 0);
    printf("The city now covers %d acres\n", 0);

    printf("1 acre of land is now worth %d bushels\n", acre_price);

    // ask player for input
    do {
        printf("What do you wish, lord?\n");
        printf("How many acres of land do you command to buy?");
        Acre_t acres_to_buy{ 0 };
        std::cin >> acres_to_buy;

        if (acres_to_buy == 0) {
            printf("How many acres of land do you command to sell?");
            std::cin >> acres_to_buy;
            acres_to_buy = -acres_to_buy;
        }

        Bushel_t bushels_to_eat{ 0 };
        printf("How many bushels of wheat do you command to eat?");
        std::cin >> bushels_to_eat;

        Acre_t arces_to_sow{ 0 };
        printf("How many acres of land do you command to sow?");
        std::cin >> arces_to_sow;

        City new_city = perform_inputs(acres_to_buy, bushels_to_eat,
                                       arces_to_sow, acre_price);

    } while(true);
    // perform checks

    // apply inputs to history

    ++current_round_;
}

City GameInstance::perform_inputs(Acre_t acres_to_buy,
                             Bushel_t wheat_to_eat,
                             Acre_t acres_to_sow,
                             Bushel_t acre_price)
{
    const City& current_city{ city_history_[current_round_] };
    City new_city{};


}

int run_game(GameInstance& game)
{
    for (uint16_t i = game.get_current_round(); i < g_round_count; ++i) {
        printf("Round #%d\n", i + 1);
        fflush(stdout);
    }
    return 0;
}

} // namespace Hamurabi

int main()
{
    // check game saves
    return Hamurabi::run_game(Hamurabi::GameInstance::load_from_file(""));
}
