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
    QuickMessServer(std::uint16_t port)
        : rain_net::Server(port) {}

    virtual bool on_client_connected(std::shared_ptr<rain_net::Connection> client_connection) override;
    virtual void on_client_disconnected(std::shared_ptr<rain_net::Connection> client_connection) override;
    virtual void on_message_received(std::shared_ptr<rain_net::Connection> client_connection, rain_net::Message& message) override;

    void accept_sign_in(std::shared_ptr<rain_net::Connection> client_connection);
    void deny_sign_in(std::shared_ptr<rain_net::Connection> client_connection);

    void notify_user_signed_in(std::shared_ptr<rain_net::Connection> client_connection, const StaticCString<MAX_USERNAME_SIZE>& username);
    void notify_user_signed_out(const std::string& username);

    void messyge(const std::string& text);

    void ask_sign_in(std::shared_ptr<rain_net::Connection> client_connection, rain_net::Message& message);
    void messyge(rain_net::Message& message);

    void add_messyge_to_chat(const std::string& username, const std::string& text);
    void import_chat(SavedChat& saved_chat);
    void export_chat(SavedChat& saved_chat);
    void update_disconnected_users();

    std::unordered_map<std::string, User> active_users;
    std::vector<std::string> disconnected_users;

    Chat chat;
};
