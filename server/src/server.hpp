#pragma once

#include <memory>
#include <cstdint>
#include <unordered_map>
#include <vector>
#include <string>

#include <rain_net/server.hpp>
#include <common.hpp>

#include "chat.hpp"

struct QuickMessServer : public rain_net::Server {
    QuickMessServer()
        : rain_net::Server() {}

    bool on_client_connected(std::shared_ptr<rain_net::ClientConnection> connection) override;
    void on_client_disconnected(std::shared_ptr<rain_net::ClientConnection> connection) override;

    void accept_sign_in(std::shared_ptr<rain_net::ClientConnection> connection);
    void deny_sign_in(std::shared_ptr<rain_net::ClientConnection> connection);

    void user_signed_in(std::shared_ptr<rain_net::ClientConnection> connection, const UsernameString& username);
    void user_signed_out(const std::string& username);

    void offer_more_chat(std::shared_ptr<rain_net::ClientConnection> connection, unsigned int from_index);

    void messyge(const std::string& text);

    void ask_sign_in(std::shared_ptr<rain_net::ClientConnection> connection, const rain_net::Message& message);
    void ask_more_chat(std::shared_ptr<rain_net::ClientConnection> connection, const rain_net::Message& message);
    void messyge(const rain_net::Message& message);

    void add_messyge_to_chat(const std::string& username, const std::string& text);
    void import_chat(SavedChat& saved_chat);
    void export_chat(SavedChat& saved_chat);
    void update_disconnected_users();

    std::unordered_map<std::string, User> active_users;
    std::vector<std::string> disconnected_users;

    Chat chat;
};
