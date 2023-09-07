#pragma once

#include <string>

struct UserData {
    std::string username;
    std::string password;
};

bool export_user_data(const UserData& data);
bool import_user_data(UserData& data);
