#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>
#include <string>
#include <optional>
#include <memory>

namespace rain_net {
    class ClientConnection;
}

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
inline constexpr std::size_t MAX_MESSYGE_SIZE {512};

inline constexpr std::uint16_t PORT {7021};

template<std::size_t Size>
struct StaticCStr {
    char data[Size] {};
};

using UsernameStr = StaticCStr<MAX_USERNAME_SIZE>;

struct ServerUser {
    std::string username;
    std::shared_ptr<rain_net::ClientConnection> connection;
};

struct ClientUser {
    std::string username;
};

struct Messyge {
    std::string username;
    std::string text;
    unsigned int index {};
};

struct Chat {
    std::vector<Messyge> messyges;
    unsigned int index_counter {};
};
