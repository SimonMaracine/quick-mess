#pragma once

#include <vector>
#include <string>

struct Message {
    std::string username;
    std::string text;
    bool remote {};
};

struct Chat {
    std::string self_username;
    std::string remote_username;

    std::vector<Message> messages;
};
