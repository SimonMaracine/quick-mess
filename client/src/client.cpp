#include "client.hpp"

#include <cstring>

#include <common.hpp>

void QuickMessClient::ask_sign_in(const std::string& username) {
    rain_net::Message message {MSG_CLIENT_ASK_SIGN_IN};

    UsernameString c_username;
    std::strcpy(c_username.data, username.c_str());  // TODO use safer version

    message << c_username;

    send_message(message);
}

void QuickMessClient::ask_more_chat(unsigned int from_index) {
    rain_net::Message message {MSG_CLIENT_ASK_MORE_CHAT};

    message << from_index;

    send_message(message);
}

void QuickMessClient::messyge(const std::string& username, const std::string& text) {
    rain_net::Message message {MSG_CLIENT_MESSYGE};

    UsernameString source_username;
    MessygeString source_text;

    std::strcpy(source_username.data, username.c_str());
    std::strcpy(source_text.data, text.c_str());

    message << source_username;
    message << source_text;

    send_message(message);
}

std::optional<rain_net::Message> QuickMessClient::next_incoming_message() {
    return rain_net::Client::next_incoming_message();
}
