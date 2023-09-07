#pragma once

#include <cstddef>
#include <vector>
#include <string>
#include <optional>
#include <memory>

#include <rain_net/connection.hpp>

#define MSG_CLIENT_ASK_SIGN_IN 1
#define MSG_SERVER_ACCEPT_SIGN_IN 2
#define MSG_SERVER_DENY_SIGN_IN 3

#define MSG_CLIENT_MESSYGE 10
#define MSG_SERVER_MESSYGE 20

template<std::size_t Size>
struct StaticCString {
    char data[Size];
};

struct User {
    std::string username;
    std::shared_ptr<rain_net::Connection> connection;
};

struct Messyge {
    std::optional<std::string> username;
    std::string text;
};

struct Chat {
    std::vector<Messyge> messyges;
};
