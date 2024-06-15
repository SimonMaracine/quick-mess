#include "data.hpp"

#include <fstream>

#include <nlohmann/json.hpp>

static const char* DATA_FILE_NAME {"quick_mess.json"};

DataFile load_data_file() {
    std::ifstream stream {DATA_FILE_NAME};

    if (!stream.is_open()) {
        throw DataError("Could not open data file");
    }

    DataFile data_file;

    // Do this stupid thing, because aggregate initialization breaks things
    nlohmann::json root;
    root = nlohmann::json::parse(stream);

    try {
        data_file.address = root["address"].get<std::string>();
        data_file.dpi_scale = root["dpi_scale"].get<unsigned int>();
    } catch (const nlohmann::json::exception& e) {
        throw DataError("Error loading data file: " + std::string(e.what()));
    }

    return data_file;
}

void create_data_file() {
    std::ofstream stream {DATA_FILE_NAME};

    if (!stream.is_open()) {
        throw DataError("Could not open data file for writing");
    }

    nlohmann::json root;
    root["address"] = "localhost";
    root["dpi_scale"] = 1;

    stream << root.dump(2);

    if (stream.fail()) {
        throw DataError("Error writing to data file");
    }
}
