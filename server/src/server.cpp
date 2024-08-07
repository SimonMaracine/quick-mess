#include "server.hpp"

#include <cstring>
#include <utility>
#include <cstddef>
#include <iostream>

static const char* server {"server: "};

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
                client_ask_sign_in(connection, message);
                break;
            case MSG_CLIENT_ASK_MORE_CHAT:
                client_ask_more_chat(connection, message);
                break;
            case MSG_CLIENT_MESSYGE:
                client_messyge(message);
                break;
        }
    }
}

void QuickMessServer::update_disconnected_users() {
    check_connections();

    for (const auto& username : m_disconnected_users) {
        server_user_signed_out(username);
        server_messyge(username + " left the chat.");
    }

    m_disconnected_users.clear();
}

void QuickMessServer::import_chat(Chat&& chat) {
    m_chat = std::move(chat);
}

Chat QuickMessServer::export_chat() const {
    return m_chat;
}

bool QuickMessServer::on_client_connected(std::shared_ptr<rain_net::ClientConnection>) {
    return true;
}

void QuickMessServer::on_client_disconnected(std::shared_ptr<rain_net::ClientConnection> connection) {
    // The disconnected client might not be in the active_users list

    for (const auto& [username, user] : m_active_users) {
        if (user.connection->get_id() != connection->get_id()) {
            continue;
        }

        std::cout << server << "User `" << username << "` (" << connection->get_id() << ") signed out\n";

        m_disconnected_users.push_back(username);
        m_active_users.erase(username);

        break;
    }
}

void QuickMessServer::server_accept_sign_in(std::shared_ptr<rain_net::ClientConnection> connection) {
    rain_net::Message message {MSG_SERVER_ACCEPT_SIGN_IN};

    // Send all current users including this one that we're messaging
    for (const auto& [username, user] : m_active_users) {
        UsernameStr c_username;
        std::strncpy(c_username.data, username.c_str(), MAX_USERNAME_SIZE);

        message << c_username;
    }

    message << static_cast<unsigned short>(m_active_users.size());

    send_message(connection, message);
}

void QuickMessServer::server_deny_sign_in(std::shared_ptr<rain_net::ClientConnection> connection) {
    rain_net::Message message {MSG_SERVER_DENY_SIGN_IN};

    send_message(connection, message);
}

void QuickMessServer::server_user_signed_in(std::shared_ptr<rain_net::ClientConnection> connection, const UsernameStr& username) {
    rain_net::Message message {MSG_SERVER_USER_SIGNED_IN};

    message << username;

    send_message_broadcast(message, connection);
}

void QuickMessServer::server_user_signed_out(const std::string& username) {
    rain_net::Message message {MSG_SERVER_USER_SIGNED_OUT};

    UsernameStr c_username;
    std::strncpy(c_username.data, username.c_str(), MAX_USERNAME_SIZE);

    message << c_username;

    send_message_broadcast(message);
}

void QuickMessServer::server_offer_more_chat(std::shared_ptr<rain_net::ClientConnection> connection, unsigned int from_index) {
    static constexpr std::size_t MAX_MESSAGES {8};

    rain_net::Message message {MSG_SERVER_OFFER_MORE_CHAT};

    unsigned int count {0};

    for (std::size_t i {from_index}; i > 0; i--) {
        const std::size_t I {i - 1};

        UsernameStr username;
        std::strncpy(username.data, m_chat.messyges.at(I).username.c_str(), MAX_USERNAME_SIZE);

        message << username;
        message.write(m_chat.messyges.at(I).text.data(), m_chat.messyges.at(I).text.size());
        message << static_cast<unsigned short>(m_chat.messyges.at(I).text.size());
        message << m_chat.messyges.at(I).index;

        if (++count == MAX_MESSAGES) {
            break;
        }
    }

    message << count;

    send_message(connection, message);
}

void QuickMessServer::server_messyge(const std::string& text) {
    send_messyge("SERVER", text);

    add_messyge_to_chat("SERVER", text);
}

void QuickMessServer::client_ask_sign_in(std::shared_ptr<rain_net::ClientConnection> connection, const rain_net::Message& message) {
    rain_net::MessageReader reader;
    reader(message);

    UsernameStr username;
    reader >> username;

    if (m_active_users.find(std::string(username.data)) != m_active_users.end()) {
        std::cout << server << "Denied user `" << username.data << "` (" << connection->get_id() << ") sign in\n";

        server_deny_sign_in(connection);

        return;
    }

    ServerUser user;
    user.username = username.data;
    user.connection = connection;

    m_active_users[user.username] = user;

    server_accept_sign_in(connection);
    server_user_signed_in(connection, username);
    server_messyge(std::string(username.data) + " entered the chat.");

    std::cout << server << "User `" << username.data << "` (" << connection->get_id() << ") signed in\n";
}

void QuickMessServer::client_ask_more_chat(std::shared_ptr<rain_net::ClientConnection> connection, const rain_net::Message& message) {
    rain_net::MessageReader reader;
    reader(message);

    unsigned int from_index;
    reader >> from_index;

    server_offer_more_chat(connection, from_index);

    std::cout << server << "Offered more chat to user " << connection->get_id() << " from chat index " << from_index << '\n';
}

void QuickMessServer::client_messyge(const rain_net::Message& message) {
    rain_net::MessageReader reader;
    reader(message);

    unsigned short size;
    std::string text;
    UsernameStr username;

    reader >> size;

    text.resize(size);

    reader.read(text.data(), text.size());
    reader >> username;

    send_messyge(username.data, text);

    add_messyge_to_chat(username.data, text);
}

void QuickMessServer::send_messyge(const std::string& username, const std::string& text) {
    rain_net::Message message {MSG_SERVER_MESSYGE};

    UsernameStr c_username;
    std::strncpy(c_username.data, username.c_str(), MAX_USERNAME_SIZE);

    message << c_username;
    message.write(text.data(), text.size());
    message << static_cast<unsigned short>(text.size());
    message << m_chat.index_counter;

    send_message_broadcast(message);
}

void QuickMessServer::add_messyge_to_chat(const std::string& username, const std::string& text) {
    // Note that index_counter must be read before increment

    Messyge messyge;
    messyge.username = username;
    messyge.text = text;
    messyge.index = m_chat.index_counter;

    m_chat.messyges.push_back(messyge);

    m_chat.index_counter++;
}
