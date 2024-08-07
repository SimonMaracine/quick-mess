#pragma once

#include <string>
#include <vector>

#include <gui_base/gui_base.hpp>
#include <common.hpp>

#include "client.hpp"
#include "data.hpp"

enum class State {
    NoConnection,
    Connecting,
    SignIn,
    Processing,
    Chat
};

class QuickMessWindow : public gui_base::GuiApplication {
public:
    explicit QuickMessWindow(const gui_base::WindowProperties& properties)
        : gui_base::GuiApplication(properties) {}
private:
    void start() override;
    void update() override;
    void stop() override;

    // UI states
    void ui_no_connection();
    void ui_connecting();
    void ui_sign_in();
    void ui_processing();
    void ui_chat();

    void ui_chat_users();
    void ui_chat_messages();

    // Server
    void server_accept_sign_in(const rain_net::Message& message);
    void server_deny_sign_in();
    void server_user_signed_in(const rain_net::Message& message);
    void server_user_signed_out(const rain_net::Message& message);
    void server_offer_more_chat(const rain_net::Message& message);
    void server_messyge(const rain_net::Message& message);

    // Helpers
    void process_messages();
    void sign_in();
    void send_messyge(const char* buffer);
    void add_messyge_to_chat(const std::string& username, const std::string& text, unsigned int index);
    void sort_messyges();

    static unsigned int load_dpi(const DataFile& data_file);
    static DataFile load_data();
    static void create_sized_fonts(unsigned int scale);
    static float rem(float size);

    QuickMessClient client;
    State state {State::Connecting};

    struct {
        std::string username;
        Chat chat;
        std::vector<std::string> users;
    } data;

    float chat_height {};
    char buffer_username[MAX_USERNAME_SIZE] {};
    bool load_more {true};
};
