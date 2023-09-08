#pragma once

#include <string>
#include <vector>

#include <gui_base/gui_base.hpp>
#include <common.hpp>

#include "client.hpp"

enum class State {
    NoConnection,
    Processing,
    SignIn,
    Menu
};

struct QuickMessWindow : public gui_base::GuiApplication {
    QuickMessWindow()
        : gui_base::GuiApplication(768, 432, "quick-mess") {}

    virtual void start() override;
    virtual void update() override;
    virtual void dispose() override;

    void no_connection();
    void processing();
    void sign_in();
    void menu();

    void menu_users();
    void menu_messages();

    void accept_sign_in(rain_net::Message& message);
    void deny_sign_in();
    void messyge(rain_net::Message& message);
    void user_signed_in(rain_net::Message& message);
    void user_signed_out(rain_net::Message& message);

    void process_incoming_messages();
    bool try_connect();
    bool check_connection();

    QuickMessClient client;
    State state = State::SignIn;

    struct Data {
        std::string username;
        Chat chat;
        std::vector<std::string> users;
    } data;

    char buffer_username[MAX_USERNAME_SIZE] {};

    static constexpr float CHAT_HEIGHT = 75.0f;
    static constexpr ImVec4 BLUEISH = ImVec4(0.6f, 0.5f, 1.0f, 1.0f);
};
