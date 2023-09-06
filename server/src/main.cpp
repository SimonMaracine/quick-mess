#include <rain_net/server.hpp>

#include "server.hpp"

int main() {
    QuickMessServer server {7021};
    server.start();

    while (true) {
        server.update(rain_net::Server::MAX, true);
    }

    server.stop();
}
