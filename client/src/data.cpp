#include <string>
#include <fstream>

#include <nlohmann/json.hpp>

#include "data.hpp"

// using json = nlohmann::json;

// static const char* FILE_NAME = "user_data.json";

// bool export_user_data(const UserData& data) {
//     std::ofstream stream {FILE_NAME};

//     if (!stream.is_open()) {
//         return false;
//     }

//     json root;
//     root["username"] = data.username;
//     root["password"] = data.password;

//     stream << root;

//     return true;
// }

// bool import_user_data(UserData& data) {
//     std::ifstream stream {FILE_NAME};

//     if (!stream.is_open()) {
//         return false;
//     }

//     json root = json::parse(stream);
//     data.username = root["username"];
//     data.password = root["password"];

//     return true;
// }
