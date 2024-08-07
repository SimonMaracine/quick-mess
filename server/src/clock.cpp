#include "clock.hpp"

#include <thread>

Clock::Clock()
    : m_start(std::chrono::high_resolution_clock::now()) {}

void Clock::stop_and_wait(std::chrono::high_resolution_clock::duration target) const {
    const auto stop {std::chrono::high_resolution_clock::now()};
    const auto elapsed {stop - m_start};

    if (elapsed < target) {
        std::this_thread::sleep_for(target - elapsed);
    }
}
