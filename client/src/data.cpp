#include <string>
#include <fstream>

#include <nlohmann/json.hpp>

#include "data.hpp"

static const char* FILE_NAME = "quick_mess.json";

bool load_host_address(std::string& address) {
    std::ifstream file {FILE_NAME};

    if (!file.is_open()) {
        return false;
    }

    nlohmann::json root = nlohmann::json::parse(file);

    try {
        address = root["address"].get<std::string>();
    } catch (const nlohmann::json::exception&) {
        return false;
    }

    return true;
}

bool create_host_address_file() {
    std::ofstream file {FILE_NAME};

    if (!file.is_open()) {
        return false;
    }

    nlohmann::json root;
    root["address"] = "";

    file << root.dump(2);

    return true;
}
