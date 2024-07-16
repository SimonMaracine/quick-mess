#pragma once

#include <memory>
#include <unordered_map>
#include <vector>
#include <string>

#include <rain_net/server.hpp>
#include <common.hpp>

#include "chat.hpp"

void on_log_message(std::string&& message);

class QuickMessServer : public rain_net::Server {
public:
    QuickMessServer()
        : rain_net::Server(on_log_message) {}

    void process_messages();
    void update_disconnected_users();

    void import_chat(Chat&& chat_);
    Chat export_chat() const;
private:
    bool on_client_connected(std::shared_ptr<rain_net::ClientConnection> connection) override;
    void on_client_disconnected(std::shared_ptr<rain_net::ClientConnection> connection) override;

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

    void add_messyge_to_chat(const std::string& username, const std::string& text);

    std::unordered_map<std::string, User> active_users;
    std::vector<std::string> disconnected_users;

    Chat chat;
};
