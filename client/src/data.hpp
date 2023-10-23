#pragma once

#include <string>

struct DataFile {
    std::string host_address;
    unsigned int dpi_scale = 1;
};

bool load_data_file(DataFile& data);
bool create_data_file();
