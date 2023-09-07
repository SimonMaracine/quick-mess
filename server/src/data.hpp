#pragma once

#include <unordered_map>
#include <vector>
#include <string>
#include <memory>

#include <common.hpp>
#include <rain_net/connection.hpp>

struct User {
    std::string username;
    std::string password;
    std::shared_ptr<rain_net::Connection> connection;
};

struct PendingMessage {
    StaticCString<16> destination_username;
    StaticCString<64> destination_text;
};

struct PendingMessages {
    std::unordered_map<std::string, std::vector<PendingMessage>> messages;
};
