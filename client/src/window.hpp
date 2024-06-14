#pragma once

#include <string>
#include <vector>

#include <gui_base/gui_base.hpp>
#include <common.hpp>

#include "client.hpp"

enum class State {
    NoConnection,
    Connecting,
    SignIn,
    Processing,
    Chat
};

struct QuickMessWindow : public gui_base::GuiApplication {
    explicit QuickMessWindow(const gui_base::WindowProperties& properties)
        : gui_base::GuiApplication(properties) {}

    void start() override;
    void update() override;
    void stop() override;

    void no_connection();
    void connecting();
    void sign_in();
    void processing();
    void chat();

    void chat_users();
    void chat_messages();

    void accept_sign_in(rain_net::Message& message);
    void deny_sign_in();
    void messyge(rain_net::Message& message);
    void user_signed_in(rain_net::Message& message);
    void user_signed_out(rain_net::Message& message);
    void offer_more_chat(rain_net::Message& message);

    void process_incoming_messages();
    bool try_connect();
    bool check_connection();
    void add_messyge_to_chat(const std::string& username, const std::string& text, unsigned int index);
    void sort_messages();
    unsigned int load_dpi();
    void create_sized_fonts(unsigned int scale);
    static float rem(float size);

    QuickMessClient client;
    State state {State::Connecting};
    bool connection_flag {false};

    struct Data {
        std::string username;
        Chat chat;
        std::vector<std::string> users;
    } data;

    char buffer_username[MAX_USERNAME_SIZE] {};

    float CHAT_HEIGHT {};
};
