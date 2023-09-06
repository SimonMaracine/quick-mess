#pragma once

#include <memory>
#include <cstdint>
#include <unordered_map>
#include <string>

#include <rain_net/server.hpp>

struct QuickMessServer : public rain_net::Server {
    QuickMessServer(std::uint16_t port)
        : rain_net::Server(port) {}

    virtual bool on_client_connected(std::shared_ptr<rain_net::Connection> client_connection) override;
    virtual void on_client_disconnected(std::shared_ptr<rain_net::Connection> client_connection) override;
    virtual void on_message_received(std::shared_ptr<rain_net::Connection> client_connection, rain_net::Message& message) override;

    void accept_sign_up(std::shared_ptr<rain_net::Connection> client_connection);
    void deny_sign_up(std::shared_ptr<rain_net::Connection> client_connection);

    void ask_sign_up(std::shared_ptr<rain_net::Connection> client_connection, rain_net::Message& message);

    std::unordered_map<std::string, std::shared_ptr<rain_net::Connection>> users;
};
