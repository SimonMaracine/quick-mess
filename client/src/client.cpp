#include <string>

#include <rain_net/client.hpp>
#include <common.hpp>

#include "client.hpp"

void QuickMessClient::sign_up(const std::string& username) {
    auto message = rain_net::message(MSG_CLIENT_ASK_SIGN_UP, 32);

    StaticCString<32> self_username;
    std::strcpy(self_username.data, username.c_str());

    message << self_username;

    send_message(message);
}

void QuickMessClient::send_to(const std::string& username, const std::string& message_text) {
    rain_net::Message message;

    StaticCString<32> remote_username;
    std::strcpy(remote_username.data, username.c_str());

    message << remote_username;

    StaticCString<64> remote_message_text;
    std::strcpy(remote_message_text.data, message_text.c_str());

    message << remote_message_text;

    send_message(message);
}
