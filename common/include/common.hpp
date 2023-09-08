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

#define MSG_SERVER_USER_SIGNED_IN 4
#define MSG_SERVER_USER_SIGNED_OUT 5

#define MSG_CLIENT_MESSYGE 10
#define MSG_SERVER_MESSYGE 20

inline constexpr std::size_t MAX_USERNAME_SIZE = 16;
inline constexpr std::size_t MAX_MESSYGE_SIZE = 256;

template<std::size_t Size>
struct StaticCString {
    char data[Size] {};
};

struct User {
    std::string username;
    std::shared_ptr<rain_net::Connection> connection;
};

struct Messyge {
    std::optional<std::string> username;
    std::string text;
    unsigned int index {};
};

struct Chat {
    std::vector<Messyge> messyges;
    unsigned int index_counter = 0;
};
