#include <fstream>
#include <string>

#include <common.hpp>
#include <nlohmann/json.hpp>

#include "chat.hpp"

inline constexpr const char* FILE_NAME = "quick_mess_chat.json";

bool load_chat(SavedChat& saved_chat) {
    std::ifstream file {FILE_NAME};

    if (!file.is_open()) {
        return false;
    }

    try {
        nlohmann::json root = nlohmann::json::parse(file);

        nlohmann::json messages = root["chat"];

        for (const nlohmann::json& message : messages) {
            Messyge messyge;
            messyge.username = message["username"].get<std::string>();
            messyge.text = message["text"].get<std::string>();
            messyge.index = message["index"].get<unsigned int>();

            saved_chat.chat.messyges.push_back(messyge);
        }

        saved_chat.chat.index_counter = root["index_counter"].get<unsigned int>();
    } catch (const nlohmann::json::exception& e) {
        return false;
    }

    return true;
}

bool save_chat(const SavedChat& saved_chat) {
    std::ofstream file {FILE_NAME};

    if (!file.is_open()) {
        return false;
    }

    nlohmann::json root;
    nlohmann::json messages = nlohmann::json::array();

    for (const Messyge& messyge : saved_chat.chat.messyges) {
        nlohmann::json message;
        message["username"] = messyge.username.value_or("SERVER");
        message["text"] = messyge.text;
        message["index"] = messyge.index;

        messages.push_back(message);
    }

    root["chat"] = messages;
    root["index_counter"] = saved_chat.chat.index_counter;

    file << root.dump(2);

    return true;
}
