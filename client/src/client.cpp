#include "client.hpp"

#include <cstring>

#include <common.hpp>

void QuickMessClient::client_ask_sign_in(const std::string& username) {
    rain_net::Message message {MSG_CLIENT_ASK_SIGN_IN};

    UsernameStr c_username;
    std::strncpy(c_username.data, username.c_str(), MAX_USERNAME_SIZE);

    message << c_username;

    send_message(message);
}

void QuickMessClient::client_ask_more_chat(unsigned int from_index) {
    rain_net::Message message {MSG_CLIENT_ASK_MORE_CHAT};

    message << from_index;

    send_message(message);
}

void QuickMessClient::client_messyge(const std::string& username, const std::string& text) {
    rain_net::Message message {MSG_CLIENT_MESSYGE};

    UsernameStr c_username;
    std::strncpy(c_username.data, username.c_str(), MAX_USERNAME_SIZE);

    message << c_username;
    message.write(text.data(), text.size());
    message << static_cast<unsigned int>(text.size());

    send_message(message);
}

std::optional<rain_net::Message> QuickMessClient::next_incoming_message() {
    return rain_net::Client::next_incoming_message();
}
