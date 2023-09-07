#pragma once

#include <string>

#include <rain_net/client.hpp>

struct QuickMessClient : public rain_net::Client {
    void sign_in(const std::string& username);
    void messyge(const std::string& username, const std::string& text);
};
