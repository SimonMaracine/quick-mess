#include <memory>
#include <cstdint>
#include <unordered_map>
#include <string>
#include <cstring>
#include <stdexcept>

#include <rain_net/server.hpp>
#include <common.hpp>

#include "server.hpp"

bool QuickMessServer::on_client_connected([[maybe_unused]] std::shared_ptr<rain_net::Connection> client_connection) {
    return true;
}

void QuickMessServer::on_client_disconnected(std::shared_ptr<rain_net::Connection> client_connection) {
    for (const auto& [username, user] : active_users) {
        if (user.connection == client_connection) {
            std::cout << "User `" << username << "` signed out\n";

            const auto user = username;

            active_users.erase(user);
            notify_user_signed_out(user);

            break;
        }
    }
}

void QuickMessServer::on_message_received(std::shared_ptr<rain_net::Connection> client_connection, rain_net::Message& message) {
    switch (message.id()) {
        case MSG_CLIENT_MESSYGE:
            messyge(message);
            break;
        case MSG_CLIENT_ASK_SIGN_IN:
            ask_sign_in(client_connection, message);
            break;
    }
}

void QuickMessServer::accept_sign_in(std::shared_ptr<rain_net::Connection> client_connection) {
    auto message = rain_net::message(MSG_SERVER_ACCEPT_SIGN_IN, active_users.size() * MAX_USERNAME_SIZE);

    for (const auto& [username, user] : active_users) {
        StaticCString<MAX_USERNAME_SIZE> c_username;
        std::strcpy( c_username.data, username.c_str());

        message << c_username;
    }

    message << static_cast<unsigned int>(active_users.size());

    send_message(client_connection, message);
}

void QuickMessServer::deny_sign_in(std::shared_ptr<rain_net::Connection> client_connection) {
    auto message = rain_net::message(MSG_SERVER_DENY_SIGN_IN, 0);

    send_message(client_connection, message);
}

void QuickMessServer::notify_user_signed_in(std::shared_ptr<rain_net::Connection> client_connection, const StaticCString<MAX_USERNAME_SIZE>& username) {
    auto message = rain_net::message(MSG_SERVER_USER_SIGNED_IN, MAX_USERNAME_SIZE);

    message << username;

    send_message_all(message, client_connection);
}

void QuickMessServer::notify_user_signed_out(const std::string& username) {
    auto message = rain_net::message(MSG_SERVER_USER_SIGNED_OUT, MAX_USERNAME_SIZE);

    StaticCString<MAX_USERNAME_SIZE> c_username;
    std::strcpy(c_username.data, username.c_str());

    message << c_username;

    send_message_all(message);
}

void QuickMessServer::ask_sign_in(std::shared_ptr<rain_net::Connection> client_connection, rain_net::Message& message) {
    StaticCString<MAX_USERNAME_SIZE> username;

    message >> username;

    if (active_users.find(std::string(username.data)) != active_users.end()) {
        std::cout << "Denied user `" << username.data << "` sign in\n";

        deny_sign_in(client_connection);

        return;
    }

    User user;
    user.username = username.data;
    user.connection = client_connection;

    active_users[user.username] = user;

    accept_sign_in(client_connection);
    notify_user_signed_in(client_connection, username);

    std::cout << "User `" << username.data << "` signed in\n";
}

void QuickMessServer::messyge(rain_net::Message& message) {
    StaticCString<MAX_MESSYGE_SIZE> destination_text;
    StaticCString<MAX_USERNAME_SIZE> destination_username;

    message >> destination_text;
    message >> destination_username;

    auto destination_message = rain_net::message(MSG_SERVER_MESSYGE, message.size());
    destination_message << destination_username;
    destination_message << destination_text;

    send_message_all(destination_message);
}
