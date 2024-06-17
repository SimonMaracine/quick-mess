#include "clock.hpp"

#include <thread>

Clock::Clock()
    : start(std::chrono::high_resolution_clock::now()) {}

void Clock::stop_and_wait(std::chrono::high_resolution_clock::duration target) const {
    const auto stop {std::chrono::high_resolution_clock::now()};
    const auto elapsed {stop - start};

    if (elapsed < target) {
        std::this_thread::sleep_for(target - elapsed);
    }
}
