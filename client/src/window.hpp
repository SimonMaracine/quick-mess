#pragma once

#include <string>

#include <gui_base/gui_base.hpp>
#include <common.hpp>

#include "client.hpp"
#include "data.hpp"

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

    void process_incoming_messages();
    bool try_connect();
    bool check_connection();

    QuickMessClient client;
    State state = State::SignIn;

    char buffer_username[16] {};

    std::string username;
    Chat chat;
};