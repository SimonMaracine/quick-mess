#pragma once

#include <string>

#include <rain_net/client.hpp>

struct QuickMessClient : public rain_net::Client {
    void ask_sign_in(const std::string& username);
    void ask_more_chat(unsigned int from_index);
    void messyge(const std::string& username, const std::string& text);
};
