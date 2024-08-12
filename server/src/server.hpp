#pragma once

#include <memory>
#include <unordered_map>
#include <vector>
#include <string>

#include <rain_net/server.hpp>
#include <common.hpp>

#include "chat.hpp"

class QuickMessServer  {
public:
    QuickMessServer()
        : m_server(
            [this](rain_net::Server&, std::shared_ptr<rain_net::ClientConnection> connection) {
                return on_client_connected(connection);
            },
            [this](rain_net::Server&, std::shared_ptr<rain_net::ClientConnection> connection) {
                on_client_disconnected(connection);
            },
            on_log
        ) {}

    void process_messages();
    void update_disconnected_users();

    void import_chat(Chat&& chat);
    Chat export_chat() const;

    void start();
    void stop();
    void accept_connections();
private:
    bool on_client_connected(std::shared_ptr<rain_net::ClientConnection> connection);
    void on_client_disconnected(std::shared_ptr<rain_net::ClientConnection> connection);
    static void on_log(const std::string& message);

    // Server
    void server_accept_sign_in(std::shared_ptr<rain_net::ClientConnection> connection);
    void server_deny_sign_in(std::shared_ptr<rain_net::ClientConnection> connection);
    void server_user_signed_in(std::shared_ptr<rain_net::ClientConnection> connection, const UsernameStr& username);
    void server_user_signed_out(const std::string& username);
    void server_offer_more_chat(std::shared_ptr<rain_net::ClientConnection> connection, unsigned int from_index);
    void server_messyge(const std::string& text);

    // Client
    void client_ask_sign_in(std::shared_ptr<rain_net::ClientConnection> connection, const rain_net::Message& message);
    void client_ask_more_chat(std::shared_ptr<rain_net::ClientConnection> connection, const rain_net::Message& message);
    void client_messyge(const rain_net::Message& message);

    void send_messyge(const std::string& username, const std::string& text);
    void add_messyge_to_chat(const std::string& username, const std::string& text);

    rain_net::Server m_server;

    Chat m_chat;
    std::unordered_map<std::string, ServerUser> m_active_users;
    std::vector<std::string> m_disconnected_users;
};
