#pragma once

#include <string>
#include <optional>

#include <rain_net/client.hpp>

struct QuickMessClient : public rain_net::Client {
    void client_ask_sign_in(const std::string& username);
    void client_ask_more_chat(unsigned int from_index);
    void client_messyge(const std::string& username, const std::string& text);

    std::optional<rain_net::Message> next_incoming_message();
};
