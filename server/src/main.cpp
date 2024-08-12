#include <iostream>
#include <csignal>
#include <utility>

#include "server.hpp"
#include "chat.hpp"
#include "clock.hpp"

static void load_chat(QuickMessServer& server) {
    Chat chat;

    try {
        chat = load_chat();
    } catch (const ChatError& e) {
        std::cerr << e.what() << '\n';
        return;
    }

    server.import_chat(std::move(chat));
}

static void save_chat(const QuickMessServer& server) {
    const Chat chat {server.export_chat()};

    try {
        save_chat(chat);
    } catch (const ChatError& e) {
        std::cerr << e.what() << '\n';
    }
}

static volatile bool running {true};

int main() {
    const auto handler {
        [](int) { running = false; }
    };

    if (std::signal(SIGINT, handler) == SIG_ERR) {
        std::cerr << "Could not setup signal handler\n";
        return 1;
    }

    QuickMessServer server;

    load_chat(server);

    try {
        server.start();
    } catch (const rain_net::ConnectionError& e) {
        return 1;
    }

    int exit_code {0};

    while (running) {
        Clock clock;

        try {
            server.process_messages();
            server.accept_connections();
            server.update_disconnected_users();
        } catch (const rain_net::ConnectionError& e) {
            exit_code = 1;
            break;
        }

        using namespace std::chrono_literals;

        clock.stop_and_wait(32ms);
    }

    server.stop();
    save_chat(server);

    return exit_code;
}
