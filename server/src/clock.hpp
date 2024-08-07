#pragma once

#include <chrono>

class Clock {
public:
    Clock();
    void stop_and_wait(std::chrono::high_resolution_clock::duration target) const;
private:
    std::chrono::high_resolution_clock::time_point m_start;
};
