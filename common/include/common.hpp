#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>
#include <string>
#include <optional>
#include <memory>

#include <rain_net/internal/connection.hpp>

enum MessygeType {
    MSG_CLIENT_ASK_SIGN_IN = 1,
    MSG_SERVER_ACCEPT_SIGN_IN,
    MSG_SERVER_DENY_SIGN_IN,

    MSG_SERVER_USER_SIGNED_IN,
    MSG_SERVER_USER_SIGNED_OUT,

    MSG_CLIENT_ASK_MORE_CHAT,
    MSG_SERVER_OFFER_MORE_CHAT,

    MSG_CLIENT_MESSYGE,
    MSG_SERVER_MESSYGE
};

inline constexpr std::size_t MAX_USERNAME_SIZE {16};
inline constexpr std::size_t MAX_MESSYGE_SIZE {256};

inline constexpr std::uint16_t PORT {7021};

template<std::size_t Size>
struct StaticCString {
    char data[Size] {};
};

using UsernameString = StaticCString<MAX_USERNAME_SIZE>;
using MessygeString = StaticCString<MAX_MESSYGE_SIZE>;

struct User {
    std::string username;
    std::shared_ptr<rain_net::ClientConnection> connection;
};

struct Messyge {
    std::optional<std::string> username;
    std::string text;
    unsigned int index {};
};

struct Chat {
    std::vector<Messyge> messyges;
    unsigned int index_counter {};
};
