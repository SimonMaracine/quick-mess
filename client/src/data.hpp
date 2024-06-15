#pragma once

#include <string>
#include <stdexcept>

struct DataError : public std::runtime_error {
    explicit DataError(const std::string& message)
        : std::runtime_error(message) {}

    explicit DataError(const char* message)
        : std::runtime_error(message) {}
};

struct DataFile {
    std::string address {"localhost"};
    unsigned int dpi_scale {1};
};

DataFile load_data_file();
void create_data_file();
