#include <memory>
#include <cstdint>
#include <unordered_map>
#include <string>

#include <rain_net/server.hpp>
#include <common.hpp>

#include "server.hpp"

bool QuickMessServer::on_client_connected([[maybe_unused]] std::shared_ptr<rain_net::Connection> client_connection) {
    return true;
}

void QuickMessServer::on_client_disconnected([[maybe_unused]] std::shared_ptr<rain_net::Connection> client_connection) {

}

void QuickMessServer::on_message_received(std::shared_ptr<rain_net::Connection> client_connection, rain_net::Message& message) {
    switch (message.id()) {
        case MSG_CLIENT_SEND_TO:
            send_to(client_connection, message);
            break;
        case MSG_CLIENT_ASK_SIGN_UP:
            ask_sign_up(client_connection, message);
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
    message >> username;

    if (users.find(std::string(username.data)) != users.end()) {
        std::cout << "Username `" << username.data << "` already in use\n";

        deny_sign_up(client_connection);

        return;
    }

    users[std::string(username.data)] = client_connection;
    std::cout << "Registered user with name `" << username.data << "`\n";

    accept_sign_up(client_connection);
}

void QuickMessServer::ask_is_registered(std::shared_ptr<rain_net::Connection> client_connection, rain_net::Message& message) {
    StaticCString<16> username;
    message >> username;

    if (users.find(std::string(username.data)) != users.end()) {
        positive_is_registered(client_connection);

        return;
    }

    negative_is_registered(client_connection);
}

void QuickMessServer::send_to(std::shared_ptr<rain_net::Connection> client_connection, rain_net::Message& message) {
    StaticCString<64> text;
    StaticCString<16> username;

    message >> text;
    message >> username;

    auto destination_message = rain_net::message(MSG_SERVER_SENT_FROM, message.size());

    destination_message << username;
    destination_message << text;

    send_message(users.at(std::string(username.data)), destination_message);
}
