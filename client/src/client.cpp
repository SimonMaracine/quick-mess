#include <string>
#include <cstring>

#include <rain_net/client.hpp>
#include <common.hpp>

#include "client.hpp"

void QuickMessClient::sign_in(const std::string& username) {
    auto message = rain_net::message(MSG_CLIENT_ASK_SIGN_IN, 16);

    StaticCString<16> c_username;
    std::strcpy(c_username.data, username.c_str());

    message << c_username;

    send_message(message);
}

void QuickMessClient::messyge(const std::string& username, const std::string& text) {
    auto message = rain_net::message(MSG_CLIENT_MESSYGE, username.size() + text.size());

    StaticCString<16> source_username;
    std::strcpy(source_username.data, username.c_str());

    StaticCString<64> source_text;
    std::strcpy(source_text.data, text.c_str());

    message << source_username;
    message << source_text;

    send_message(message);
}
