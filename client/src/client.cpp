#include <string>
#include <cstring>

#include <rain_net/client.hpp>
#include <common.hpp>

#include "client.hpp"

void QuickMessClient::sign_up(const std::string& username) {
    auto message = rain_net::message(MSG_CLIENT_ASK_SIGN_UP, 16);

    StaticCString<16> self_username;
    std::strcpy(self_username.data, username.c_str());

    message << self_username;

    send_message(message);
}

void QuickMessClient::is_registered(const std::string& username) {
    auto message = rain_net::message(MSG_CLIENT_ASK_IS_REGISTERED, 16);

    StaticCString<16> remote_username;
    std::strcpy(remote_username.data, username.c_str());

    message << remote_username;

    send_message(message);
}

void QuickMessClient::send_to(const std::string& username, const std::string& message_text) {
    auto message = rain_net::message(MSG_CLIENT_SEND_TO, username.size() + message_text.size());

    StaticCString<16> remote_username;
    std::strcpy(remote_username.data, username.c_str());

    message << remote_username;

    StaticCString<64> remote_message_text;
    std::strcpy(remote_message_text.data, message_text.c_str());

    message << remote_message_text;

    send_message(message);
}
