#include <string>
#include <fstream>

#include <nlohmann/json.hpp>

#include "data.hpp"

static const char* FILE_NAME = "quick_mess.json";

bool load_data_file(DataFile& data) {
    std::ifstream file {FILE_NAME};

    if (!file.is_open()) {
        return false;
    }

    nlohmann::json root = nlohmann::json::parse(file);

    try {
        data.host_address = root["address"].get<std::string>();
        data.dpi_scale = root["dpi_scale"].get<unsigned int>();
    } catch (const nlohmann::json::exception&) {
        return false;
    }

    return true;
}

bool create_data_file() {
    std::ofstream file {FILE_NAME};

    if (!file.is_open()) {
        return false;
    }

    nlohmann::json root;
    root["address"] = "";
    root["dpi_scale"] = 1;

    file << root.dump(2);

    return true;
}
