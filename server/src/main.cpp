#include <rain_net/server.hpp>

#include "server.hpp"

int main() {
    QuickMessServer server {7021};
    server.start();

    while (true) {
        server.update(rain_net::Server::MAX_MSG, true);
    }

    server.stop();
}
