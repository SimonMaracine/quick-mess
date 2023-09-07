#include <string>
#include <cstring>

#include <rain_net/client.hpp>
#include <common.hpp>

#include "client.hpp"

void QuickMessClient::sign_in(const std::string& username) {
    auto message = rain_net::message(MSG_CLIENT_ASK_SIGN_IN, MAX_USERNAME_SIZE);

    StaticCString<MAX_USERNAME_SIZE> c_username;
    std::strcpy(c_username.data, username.c_str());

    message << c_username;

    send_message(message);
}

void QuickMessClient::messyge(const std::string& username, const std::string& text) {
    auto message = rain_net::message(MSG_CLIENT_MESSYGE, username.size() + text.size());

    StaticCString<MAX_USERNAME_SIZE> source_username;
    std::strcpy(source_username.data, username.c_str());

    StaticCString<MAX_MESSYGE_SIZE> source_text;
    std::strcpy(source_text.data, text.c_str());

    message << source_username;
    message << source_text;

    send_message(message);
}
