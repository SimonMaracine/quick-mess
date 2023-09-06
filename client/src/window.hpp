#pragma once

#include <string>

#include <gui_base/gui_base.hpp>

#include "client.hpp"
#include "chat.hpp"

enum class State {
    NoConnection,
    Processing,
    SignUp,
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

    void sign_up();
    void sign_in();
    void menu();

    void could_not_connect_to_server();

    void process_incoming_messages();
    bool check_connection();

    QuickMessClient client;
    State state = State::SignUp;

    char buffer_chat_with[16] {};
    std::string chat_with = "";

    Chat chat;
};
