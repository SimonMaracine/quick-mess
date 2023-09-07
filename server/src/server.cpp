#include <memory>
#include <cstdint>
#include <unordered_map>
#include <string>
#include <stdexcept>

#include <rain_net/server.hpp>
#include <common.hpp>

#include "server.hpp"
#include "data.hpp"

bool QuickMessServer::on_client_connected([[maybe_unused]] std::shared_ptr<rain_net::Connection> client_connection) {
    return true;
}

void QuickMessServer::on_client_disconnected(std::shared_ptr<rain_net::Connection> client_connection) {
    for (const auto& [username, user] : active_users) {
        if (user.connection == client_connection) {
            std::cout << "User `" << username << "` signed out\n";

            active_users.erase(username);

            break;
        }
    }
}

void QuickMessServer::on_message_received(std::shared_ptr<rain_net::Connection> client_connection, rain_net::Message& message) {
    switch (message.id()) {
        case MSG_CLIENT_MESSYGE:
            messyge(client_connection, message);
            break;
        case MSG_CLIENT_ASK_SIGN_IN:
            ask_sign_in(client_connection, message);
            break;
    }
}

void QuickMessServer::accept_sign_in(std::shared_ptr<rain_net::Connection> client_connection) {
    auto message = rain_net::message(MSG_SERVER_ACCEPT_SIGN_IN, 0);

    send_message(client_connection, message);
}

void QuickMessServer::deny_sign_in(std::shared_ptr<rain_net::Connection> client_connection) {
    auto message = rain_net::message(MSG_SERVER_DENY_SIGN_IN, 0);

    send_message(client_connection, message);
}

void QuickMessServer::ask_sign_in(std::shared_ptr<rain_net::Connection> client_connection, rain_net::Message& message) {
    StaticCString<16> username;

    message >> username;

    if (active_users.find(std::string(username.data)) != active_users.end()) {
        std::cout << "User `" << username.data << "` already exists\n";

        deny_sign_in(client_connection);

        return;
    }

    User user;
    user.username = username.data;
    user.connection = client_connection;

    active_users[user.username] = user;

    accept_sign_in(client_connection);

    std::cout << "User `" << username.data << "` signed in\n";
}

void QuickMessServer::messyge(std::shared_ptr<rain_net::Connection> client_connection, rain_net::Message& message) {
    StaticCString<64> destination_text;
    StaticCString<16> destination_username;

    message >> destination_text;
    message >> destination_username;

    auto destination_message = rain_net::message(MSG_SERVER_MESSYGE, message.size());
    destination_message << destination_username;
    destination_message << destination_text;

    send_message_all(destination_message);
}
