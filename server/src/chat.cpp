#include "chat.hpp"

#include <fstream>

#include <nlohmann/json.hpp>

static const char* CHAT_FILE_NAME {"quick_mess_chat.json"};

Chat load_chat() {
    std::ifstream stream {CHAT_FILE_NAME};

    if (!stream.is_open()) {
        throw ChatError("Could not open chat file");
    }

    Chat chat;

    try {
        const nlohmann::json root {nlohmann::json::parse(stream)};

        const nlohmann::json& messages {root["chat"]};

        for (const nlohmann::json& message : messages) {
            Messyge messyge;
            messyge.username = message["username"].get<std::string>();
            messyge.text = message["text"].get<std::string>();
            messyge.index = message["index"].get<unsigned int>();

            chat.messyges.push_back(messyge);
        }

        chat.index_counter = root["index_counter"].get<unsigned int>();
    } catch (const nlohmann::json::exception& e) {
        throw ChatError("Error loading chat file: " + std::string(e.what()));
    }

    return chat;
}

void save_chat(const Chat& chat) {
    std::ofstream stream {CHAT_FILE_NAME};

    if (!stream.is_open()) {
        throw ChatError("Could not open chat file for writing");
    }

    nlohmann::json root;
    nlohmann::json messages {nlohmann::json::array()};

    for (const Messyge& messyge : chat.messyges) {
        nlohmann::json message;
        message["username"] = messyge.username.value_or("SERVER");
        message["text"] = messyge.text;
        message["index"] = messyge.index;

        messages.push_back(message);
    }

    root["chat"] = messages;
    root["index_counter"] = chat.index_counter;

    stream << root.dump(2);

    if (stream.fail()) {
        throw ChatError("Error saving chat file");
    }
}
