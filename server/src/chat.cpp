#include "chat.hpp"

#include <fstream>
#include <string>

#include <nlohmann/json.hpp>

static const char* CHAT_FILE_NAME {"quick_mess_chat.json"};

bool load_chat(SavedChat& saved_chat) {
    std::ifstream stream {CHAT_FILE_NAME};

    if (!stream.is_open()) {
        return false;
    }

    try {
        nlohmann::json root {nlohmann::json::parse(stream)};

        nlohmann::json messages {root["chat"]};

        for (const nlohmann::json& message : messages) {
            Messyge messyge;
            messyge.username = message["username"].get<std::string>();
            messyge.text = message["text"].get<std::string>();
            messyge.index = message["index"].get<unsigned int>();

            saved_chat.chat.messyges.push_back(messyge);
        }

        saved_chat.chat.index_counter = root["index_counter"].get<unsigned int>();
    } catch (const nlohmann::json::exception&) {
        return false;
    }

    return true;
}

bool save_chat(const SavedChat& saved_chat) {
    std::ofstream stream {CHAT_FILE_NAME};

    if (!stream.is_open()) {
        return false;
    }

    nlohmann::json root;
    nlohmann::json messages {nlohmann::json::array()};

    for (const Messyge& messyge : saved_chat.chat.messyges) {
        nlohmann::json message;
        message["username"] = messyge.username.value_or("SERVER");
        message["text"] = messyge.text;
        message["index"] = messyge.index;

        messages.push_back(message);
    }

    root["chat"] = messages;
    root["index_counter"] = saved_chat.chat.index_counter;

    stream << root.dump(2);

    return true;
}
