#include <iostream>
#include <cstdlib>
#include <csignal>

#include <rain_net/server.hpp>
#include <common.hpp>

#include "server.hpp"
#include "chat.hpp"

static volatile bool running {true};

int main() {
    const auto handler {
        [](int) { running = false; }
    };

    if (std::signal(SIGINT, handler) == SIG_ERR) {
        std::cerr << "Could not setup signal handler\n";
        std::exit(1);
    }

    QuickMessServer server;

    {
        SavedChat saved_chat;

        if (load_chat(saved_chat)) {
            server.import_chat(saved_chat);
        } else {
            std::cout << "Could not read chat from file\n";
        }
    }

    server.start();

    while (running) {
        server.update(rain_net::Server::MAX_MSG, true);
        server.update_disconnected_users();
    }

    server.stop();

    {
        SavedChat saved_chat;

        server.export_chat(saved_chat);

        if (!save_chat(saved_chat)) {
            std::cout << "Could not write chat to file\n";
        }
    }
}
