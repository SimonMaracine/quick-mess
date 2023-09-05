#include <iostream>
#include <cstdint>

#include <rain_net/server.hpp>

struct QuickMessServer : public rain_net::Server {
    QuickMessServer(std::uint16_t port)
        : rain_net::Server(port) {}

    virtual bool on_client_connected(std::shared_ptr<rain_net::Connection> client_connection) override {

    }

    virtual void on_client_disconnected(std::shared_ptr<rain_net::Connection> client_connection) override {

    }

    virtual void on_message_received(std::shared_ptr<rain_net::Connection> client_connection, rain_net::Message& message) override {

    }
};

int main() {
    std::cout << "Starting server...\n";

    QuickMessServer server {7021};
}
