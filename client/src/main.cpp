#include <iostream>
#include <cstring>
#include <string>
#include <thread>
#include <mutex>

#include <rain_net/client.hpp>
#include <common.hpp>

struct QuickMessClient : public rain_net::Client {
    void hello(const std::string& username) {
        rain_net::Message message;

        StaticCString<32> self_username;
        std::strcpy(self_username.data, username.c_str());

        message << self_username;

        send_message(message);
    }

    void send_to(const std::string& username, const std::string& message_text) {
        rain_net::Message message;

        StaticCString<32> remote_username;
        std::strcpy(remote_username.data, username.c_str());

        message << remote_username;

        StaticCString<64> remote_message_text;
        std::strcpy(remote_message_text.data, message_text.c_str());

        message << remote_message_text;

        send_message(message);
    }
};

class QuickMessSynchronized {
public:
    QuickMessClient* operator->() {
        std::lock_guard<std::mutex> guard {mutex};

        return &quick_mess;
    }
private:
    std::mutex mutex;
    QuickMessClient quick_mess;
};

enum class State {
    Joining,
    Menu,
    Chat
};

std::string input_string() {
    std::string string;
    std::cin >> string;

    return string;
}

/*
    Usernames consist only of ASCII letters, numbers and underscores

    The only thing that isn't a message is a too long message and this: "\e"
*/

std::string get_username_from_input(const std::string& input) {
    for (auto iter = std::next(input.cbegin()); iter != input.cend(); iter++) {
        const char character = *iter;

        const bool is_number = character >= 48 && character <= 57;
        const bool is_uppercase_letter = character >= 65 && character <= 90;
        const bool is_lowercase_letter = character >= 97 && character <= 122;
        const bool is_underscore = character == 95;

        if (is_number || is_uppercase_letter || is_lowercase_letter || is_underscore) {
            return std::string(iter, input.cend());
        }
    }

    return {};
}

void joining(QuickMessSynchronized& quick_mess, bool& running, State& state) {
    std::cout << "Enter your username (must be unique)\n";

    const auto username = input_string();
    quick_mess->hello(username);

    // TODO ask server for username

    state = State::Menu;
}

void menu(bool& running, State& state, std::string& chat_username) {
    std::cout << "Help:" << '\n' << "c <username> => enter a chat" << '\n' << "e => exit the application";

    std::cout << "menu>> " << '\n';

    const auto command = input_string();

    if (*command.cbegin() == 'c') {
        const auto username = get_username_from_input(command);

        if (username.empty()) {
            return;
        }

        // TODO ask server for username

        chat_username = username;
        state = State::Chat;
    } else if (*command.cbegin() == 'e') {
        running = false;
    } else {
        std::cout << "Unknown input\n";
    }
}

void chat(QuickMessSynchronized& quick_mess, bool& running, State& state, std::string& chat_username) {
    std::cout << "chat " << chat_username << ">> ";

    const auto message_text = input_string();

    if (message_text == "\\e") {
        state = State::Menu;
        chat_username = {};
        return;
    }

    quick_mess->send_to(chat_username, message_text);
}

void user_interface(QuickMessSynchronized& quick_mess, bool& running) {
    State state = State::Joining;
    std::string chat_username;

    while (running) {
        switch (state) {
            case State::Joining:
                joining(quick_mess, running, state);
                break;
            case State::Menu:
                menu(running, state, chat_username);
                break;
            case State::Chat:
                chat(quick_mess, running, state, chat_username);
                break;
        }
    }
}

int main() {
    QuickMessSynchronized quick_mess;

    if (!quick_mess->connect("localhost", 7021)) {
        std::cout << "Could not connect to the server\n";
        return 1;
    }

    bool running = true;

    std::thread cli_thread {user_interface, std::ref(quick_mess), std::ref(running)};

    while (running) {
        auto result = quick_mess->next_incoming_message();
        auto message = result.value_or(rain_net::Message());

        if (!result.has_value()) {
            continue;
        }

        switch (message.id()) {
            case MSG_SERVER_SENT_FROM:
                break;
            case MSG_SERVER_RESPOND_HELLO:
                break;
            case MSG_SERVER_RESPOND_USERNAME_FINE:
                break;
        }
    }

    quick_mess->disconnect();

    cli_thread.join();

    return 0;
}
