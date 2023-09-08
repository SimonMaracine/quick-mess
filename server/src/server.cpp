#include <memory>
#include <cstdint>
#include <unordered_map>
#include <string>
#include <cstring>
#include <stdexcept>
#include <iostream>
#include <utility>
#include <cstddef>

#include <rain_net/server.hpp>
#include <common.hpp>

#include "server.hpp"
#include "chat.hpp"

bool QuickMessServer::on_client_connected([[maybe_unused]] std::shared_ptr<rain_net::Connection> client_connection) {
    return true;
}

void QuickMessServer::on_client_disconnected(std::shared_ptr<rain_net::Connection> client_connection) {
    // The disconnected client might not be in the active_users list

    for (const auto& [username, user] : active_users) {
        if (user.connection->get_id() != client_connection->get_id()) {
            continue;
        }

        std::cout << "User `" << username << "` signed out\n";

        disconnected_users.push_back(username);
        active_users.erase(username);

        break;
    }
}

void QuickMessServer::on_message_received(std::shared_ptr<rain_net::Connection> client_connection, rain_net::Message& message) {
    switch (message.id()) {
        case MSG_CLIENT_ASK_SIGN_IN:
            ask_sign_in(client_connection, message);
            break;
        case MSG_CLIENT_ASK_MORE_CHAT:
            ask_more_chat(client_connection, message);
            break;
        case MSG_CLIENT_MESSYGE:
            messyge(message);
            break;
    }
}

void QuickMessServer::accept_sign_in(std::shared_ptr<rain_net::Connection> client_connection) {
    auto message = rain_net::message(MSG_SERVER_ACCEPT_SIGN_IN, active_users.size() * MAX_USERNAME_SIZE);

    for (const auto& [username, user] : active_users) {
        StaticCString<MAX_USERNAME_SIZE> c_username;
        std::strcpy(c_username.data, username.c_str());

        message << c_username;
    }

    message << static_cast<unsigned int>(active_users.size());

    // Send all current users including this one that we're messaging
    send_message(client_connection, message);
}

void QuickMessServer::deny_sign_in(std::shared_ptr<rain_net::Connection> client_connection) {
    auto message = rain_net::message(MSG_SERVER_DENY_SIGN_IN, 0);

    send_message(client_connection, message);
}

void QuickMessServer::user_signed_in(std::shared_ptr<rain_net::Connection> client_connection, const StaticCString<MAX_USERNAME_SIZE>& username) {
    auto message = rain_net::message(MSG_SERVER_USER_SIGNED_IN, MAX_USERNAME_SIZE);

    message << username;

    send_message_all(message, client_connection);
}

void QuickMessServer::user_signed_out(const std::string& username) {
    auto message = rain_net::message(MSG_SERVER_USER_SIGNED_OUT, MAX_USERNAME_SIZE);

    StaticCString<MAX_USERNAME_SIZE> c_username;
    std::strcpy(c_username.data, username.c_str());

    message << c_username;

    send_message_all(message);
}

void QuickMessServer::offer_more_chat(std::shared_ptr<rain_net::Connection> client_connection, unsigned int from_index) {
    static constexpr std::size_t MAX_MESSAGES = 10;

    auto message = rain_net::message(MSG_SERVER_OFFER_MORE_CHAT, MAX_MESSYGE_SIZE * MAX_MESSAGES);

    unsigned int count = 0;

    for (std::size_t i = from_index; i > 0; i--) {
        const std::size_t I = i - 1;

        StaticCString<MAX_USERNAME_SIZE> username;
        StaticCString<MAX_MESSYGE_SIZE> text;

        std::strcpy(username.data, chat.messyges.at(I).username.value_or("SERVER").c_str());
        std::strcpy(text.data, chat.messyges.at(I).text.c_str());

        message << username;
        message << text;
        message << chat.messyges.at(I).index;

        if (++count == MAX_MESSAGES) {
            break;
        }
    }

    message << count;

    send_message(client_connection, message);
}

void QuickMessServer::messyge(const std::string& text) {
    auto broadcast_message = rain_net::message(MSG_SERVER_MESSYGE, MAX_MESSYGE_SIZE);

    StaticCString<MAX_USERNAME_SIZE> username;
    StaticCString<MAX_MESSYGE_SIZE> c_text;

    std::strcpy(username.data, "SERVER");
    std::strcpy(c_text.data, text.c_str());

    broadcast_message << username;
    broadcast_message << c_text;
    broadcast_message << chat.index_counter;

    send_message_all(broadcast_message);

    add_messyge_to_chat("SERVER", text);
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
    user_signed_in(client_connection, username);
    messyge(std::string(username.data) + " entered the chat.");

    std::cout << "User `" << username.data << "` signed in\n";
}

void QuickMessServer::ask_more_chat(std::shared_ptr<rain_net::Connection> client_connection, rain_net::Message& message) {
    unsigned int from_index;
    message >> from_index;

    offer_more_chat(client_connection, from_index);

    std::cout << "A user asked for more chat from index `" << from_index << "`\n";
}

void QuickMessServer::messyge(rain_net::Message& message) {
    StaticCString<MAX_MESSYGE_SIZE> text;
    StaticCString<MAX_USERNAME_SIZE> username;

    message >> text;
    message >> username;

    auto broadcast_message = rain_net::message(MSG_SERVER_MESSYGE, message.size());
    broadcast_message << username;
    broadcast_message << text;
    broadcast_message << chat.index_counter;

    send_message_all(broadcast_message);

    add_messyge_to_chat(username.data, text.data);
}

void QuickMessServer::add_messyge_to_chat(const std::string& username, const std::string& text) {
    // Note that index_counter must be read before increment

    Messyge messyge;
    messyge.username = username;
    messyge.text = text;
    messyge.index = chat.index_counter;

    chat.messyges.push_back(messyge);

    chat.index_counter++;
}

void QuickMessServer::import_chat(SavedChat& saved_chat) {
    chat = std::move(saved_chat.chat);
}

void QuickMessServer::export_chat(SavedChat& saved_chat) {
    saved_chat.chat = std::move(chat);
}

void QuickMessServer::update_disconnected_users() {
    check_connections();

    for (const auto& username : disconnected_users) {
        user_signed_out(username);
        messyge(username + " left the chat.");
    }

    disconnected_users.clear();
}
