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
            active_users.erase(username);

            std::cout << "User `" << username << "` became inactive\n";

            break;
        }
    }
}

void QuickMessServer::on_message_received(std::shared_ptr<rain_net::Connection> client_connection, rain_net::Message& message) {
    switch (message.id()) {
        case MSG_CLIENT_SEND_TO:
            send_to(client_connection, message);
            break;
        case MSG_CLIENT_ASK_SIGN_UP:
            ask_sign_up(client_connection, message);
            break;
        case MSG_CLIENT_ASK_SIGN_IN:
            ask_sign_in(client_connection, message);
            break;
        case MSG_CLIENT_ASK_IS_REGISTERED:
            ask_is_registered(client_connection, message);
            break;
    }
}

void QuickMessServer::accept_sign_up(std::shared_ptr<rain_net::Connection> client_connection) {
    auto message = rain_net::message(MSG_SERVER_ACCEPT_SIGN_UP, 0);

    send_message(client_connection, message);
}

void QuickMessServer::deny_sign_up(std::shared_ptr<rain_net::Connection> client_connection) {
    auto message = rain_net::message(MSG_SERVER_DENY_SIGN_UP, 0);

    send_message(client_connection, message);
}

void QuickMessServer::accept_sign_in(std::shared_ptr<rain_net::Connection> client_connection) {
    auto message = rain_net::message(MSG_SERVER_ACCEPT_SIGN_IN, 0);

    send_message(client_connection, message);
}

void QuickMessServer::deny_sign_in(std::shared_ptr<rain_net::Connection> client_connection) {
    auto message = rain_net::message(MSG_SERVER_DENY_SIGN_IN, 0);

    send_message(client_connection, message);
}

void QuickMessServer::positive_is_registered(std::shared_ptr<rain_net::Connection> client_connection) {
    auto message = rain_net::message(MSG_SERVER_POSITIVE_IS_REGISTERED, 0);

    send_message(client_connection, message);
}

void QuickMessServer::negative_is_registered(std::shared_ptr<rain_net::Connection> client_connection) {
    auto message = rain_net::message(MSG_SERVER_NEGATIVE_IS_REGISTERED, 0);

    send_message(client_connection, message);
}

void QuickMessServer::ask_sign_up(std::shared_ptr<rain_net::Connection> client_connection, rain_net::Message& message) {
    StaticCString<16> username;
    StaticCString<16> password;

    message >> password;
    message >> username;

    if (registered_users.find(std::string(username.data)) != registered_users.end()) {
        std::cout << "Username `" << username.data << "` already in use\n";

        deny_sign_up(client_connection);

        return;
    }

    User user;
    user.username = username.data;
    user.password = password.data;
    user.connection = client_connection;

    // Create a new user

    registered_users[user.username] = user;
    active_users[user.username] = user;  // Make them directly active

    std::cout << "Registered user with name `" << user.username << "`\n";

    accept_sign_up(client_connection);
}

void QuickMessServer::ask_sign_in(std::shared_ptr<rain_net::Connection> client_connection, rain_net::Message& message) {
    StaticCString<16> username;
    StaticCString<16> password;

    message >> password;
    message >> username;

    try {
        auto registered_user = registered_users.at(std::string(username.data));

        if (registered_user.password != std::string(password.data)) {
            std::cout << "Denied sign in: passwords don't match\n";

            deny_sign_in(client_connection);

            return;
        }

        registered_user.connection = client_connection;

        active_users[registered_user.username] = registered_user;

        accept_sign_in(client_connection);

        std::cout << "User `" << username.data << "` signed in\n";
    } catch (const std::out_of_range&) {
        std::cout << "Denied sign in: username `" << username.data << "` doesn't exist\n";

        deny_sign_in(client_connection);
    }
}

void QuickMessServer::ask_is_registered(std::shared_ptr<rain_net::Connection> client_connection, rain_net::Message& message) {
    StaticCString<16> username;
    message >> username;

    if (registered_users.find(std::string(username.data)) != registered_users.end()) {
        positive_is_registered(client_connection);

        return;
    }

    negative_is_registered(client_connection);
}

void QuickMessServer::send_to(std::shared_ptr<rain_net::Connection> client_connection, rain_net::Message& message) {
    StaticCString<64> destination_text;
    StaticCString<16> destination_username;

    message >> destination_text;
    message >> destination_username;

    std::shared_ptr<rain_net::Connection> remote_connection;

    try {
        remote_connection = active_users.at(std::string(destination_username.data)).connection;
    } catch (const std::out_of_range& e) {
        std::cout << "Inactive user: " << e.what() << '\n';

        PendingMessage message;
        message.destination_username = destination_username;
        message.destination_text = destination_text;

        store_pending_message(message);

        return;
    }

    auto destination_message = rain_net::message(MSG_SERVER_SENT_FROM, message.size());
    destination_message << destination_username;
    destination_message << destination_text;

    send_message(remote_connection, destination_message);
}

void QuickMessServer::store_pending_message(const PendingMessage& pending_message) {
    pending_messages.messages[pending_message.destination_username.data].push_back(pending_message);
}
