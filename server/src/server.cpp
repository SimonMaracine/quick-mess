#include "server.hpp"

#include <cstring>
#include <stdexcept>
#include <utility>
#include <cstddef>
#include <iostream>

void on_log_message(std::string&& message) {
    std::cerr << message << '\n';
}

void QuickMessServer::process_messages() {
    while (true) {
        const auto result {next_incoming_message()};

        if (!result) {
            break;
        }

        const auto& [connection, message] {*result};

        switch (message.id()) {
            case MSG_CLIENT_ASK_SIGN_IN:
                ask_sign_in(connection, message);
                break;
            case MSG_CLIENT_ASK_MORE_CHAT:
                ask_more_chat(connection, message);
                break;
            case MSG_CLIENT_MESSYGE:
                messyge(message);
                break;
        }
    }
}

void QuickMessServer::update_disconnected_users() {
    check_connections();

    for (const auto& username : disconnected_users) {
        user_signed_out(username);
        messyge(username + " left the chat.");
    }

    disconnected_users.clear();
}

void QuickMessServer::import_chat(Chat&& chat) {
    this->chat = std::move(chat);
}

Chat QuickMessServer::export_chat() const {
    return chat;
}

bool QuickMessServer::on_client_connected([[maybe_unused]] std::shared_ptr<rain_net::ClientConnection> connection) {
    return true;
}

void QuickMessServer::on_client_disconnected(std::shared_ptr<rain_net::ClientConnection> connection) {
    // The disconnected client might not be in the active_users list

    for (const auto& [username, user] : active_users) {
        if (user.connection->get_id() != connection->get_id()) {
            continue;
        }

        std::cerr << "User `" << username << "` signed out\n";

        disconnected_users.push_back(username);
        active_users.erase(username);

        break;
    }
}

void QuickMessServer::accept_sign_in(std::shared_ptr<rain_net::ClientConnection> connection) {
    rain_net::Message message {MSG_SERVER_ACCEPT_SIGN_IN};

    for (const auto& [username, user] : active_users) {
        UsernameString c_username;
        std::strcpy(c_username.data, username.c_str());

        message << c_username;
    }

    message << static_cast<unsigned int>(active_users.size());

    // Send all current users including this one that we're messaging
    send_message(connection, message);
}

void QuickMessServer::deny_sign_in(std::shared_ptr<rain_net::ClientConnection> connection) {
    rain_net::Message message {MSG_SERVER_DENY_SIGN_IN};

    send_message(connection, message);
}

void QuickMessServer::user_signed_in(std::shared_ptr<rain_net::ClientConnection> connection, const UsernameString& username) {
    rain_net::Message message {MSG_SERVER_USER_SIGNED_IN};

    message << username;

    send_message_broadcast(message, connection);
}

void QuickMessServer::user_signed_out(const std::string& username) {
    rain_net::Message message {MSG_SERVER_USER_SIGNED_OUT};

    UsernameString c_username;
    std::strcpy(c_username.data, username.c_str());

    message << c_username;

    send_message_broadcast(message);
}

void QuickMessServer::offer_more_chat(std::shared_ptr<rain_net::ClientConnection> connection, unsigned int from_index) {
    static constexpr std::size_t MAX_MESSAGES {10};

    rain_net::Message message {MSG_SERVER_OFFER_MORE_CHAT};

    unsigned int count {0};

    for (std::size_t i {from_index}; i > 0; i--) {
        const std::size_t I {i - 1};

        UsernameString username;
        MessygeString text;

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

    send_message(connection, message);
}

void QuickMessServer::messyge(const std::string& text) {
    rain_net::Message broadcast_message {MSG_SERVER_MESSYGE};

    UsernameString username;
    MessygeString c_text;

    std::strcpy(username.data, "SERVER");
    std::strcpy(c_text.data, text.c_str());

    broadcast_message << username;
    broadcast_message << c_text;
    broadcast_message << chat.index_counter;

    send_message_broadcast(broadcast_message);

    add_messyge_to_chat("SERVER", text);
}

void QuickMessServer::ask_sign_in(std::shared_ptr<rain_net::ClientConnection> connection, const rain_net::Message& message) {
    rain_net::MessageReader reader;
    reader(message);

    UsernameString username;
    reader >> username;

    if (active_users.find(std::string(username.data)) != active_users.end()) {
        std::cerr << "Denied user `" << username.data << "` sign in\n";

        deny_sign_in(connection);

        return;
    }

    User user;
    user.username = username.data;
    user.connection = connection;

    active_users[user.username] = user;

    accept_sign_in(connection);
    user_signed_in(connection, username);
    messyge(std::string(username.data) + " entered the chat.");

    std::cerr << "User `" << username.data << "` signed in\n";
}

void QuickMessServer::ask_more_chat(std::shared_ptr<rain_net::ClientConnection> connection, const rain_net::Message& message) {
    rain_net::MessageReader reader;
    reader(message);

    unsigned int from_index;
    reader >> from_index;

    offer_more_chat(connection, from_index);

    std::cerr << "Offered more chat to user `" << connection->get_id() << "` from index `" << from_index << "`\n";
}

void QuickMessServer::messyge(const rain_net::Message& message) {
    rain_net::MessageReader reader;
    reader(message);

    MessygeString text;
    UsernameString username;

    reader >> text;
    reader >> username;

    rain_net::Message broadcast_message {MSG_SERVER_MESSYGE};
    broadcast_message << username;
    broadcast_message << text;
    broadcast_message << chat.index_counter;

    send_message_broadcast(broadcast_message);

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
