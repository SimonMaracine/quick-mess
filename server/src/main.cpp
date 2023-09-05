#include <iostream>
#include <cstdint>
#include <unordered_map>
#include <string>

#include <rain_net/server.hpp>
#include <common.hpp>

struct QuickMessServer : public rain_net::Server {
    QuickMessServer(std::uint16_t port)
        : rain_net::Server(port) {}

    virtual bool on_client_connected(std::shared_ptr<rain_net::Connection> client_connection) override {

    }

    virtual void on_client_disconnected(std::shared_ptr<rain_net::Connection> client_connection) override {

    }

    virtual void on_message_received(std::shared_ptr<rain_net::Connection> client_connection, rain_net::Message& message) override {
        switch (message.id()) {
            case MSG_CLIENT_SEND_TO: {
                StaticCString<64> message_text;
                StaticCString<32> username;

                message >> message_text;
                message >> username;

                rain_net::Message destination_message;
                // TODO

                send_message(usernames.at(std::string(username.data)), destination_message);

                break;
            }
            case MSG_CLIENT_ASK_HELLO: {
                StaticCString<32> username;
                message >> username;

                if (usernames.find(std::string(username.data)) != usernames.end()) {
                    std::cout << "Username already in use\n";
                    return;
                }

                usernames[std::string(username.data)] = client_connection;

                break;
            }
            case MSG_CLIENT_ASK_USERNAME_FINE: {
                break;
            }
        }
    }

    std::unordered_map<std::string, std::shared_ptr<rain_net::Connection>> usernames;
};

int main() {
    QuickMessServer server {7021};
    server.start();

    while (true) {
        server.update(rain_net::Server::MAX, true);
    }

    server.stop();
}
