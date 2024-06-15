#pragma once

#include <stdexcept>
#include <string>

#include <common.hpp>

struct ChatError : public std::runtime_error {
    explicit ChatError(const std::string& message)
        : std::runtime_error(message) {}

    explicit ChatError(const char* message)
        : std::runtime_error(message) {}
};

Chat load_chat();
void save_chat(const Chat& chat);
