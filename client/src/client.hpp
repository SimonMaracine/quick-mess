#pragma once

#include <string>

#include <rain_net/client.hpp>
#include <common.hpp>

struct QuickMessClient : public rain_net::Client {
    void sign_up(const std::string& username);
    void is_registered(const std::string& username);
    void send_to(const std::string& username, const std::string& message_text);
};
