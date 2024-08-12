#pragma once

#include <string>
#include <vector>

#include <gui_base/gui_base.hpp>
#include <rain_net/client.hpp>
#include <common.hpp>

#include "data.hpp"

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

    // Client
    void client_ask_sign_in(const std::string& username);
    void client_ask_more_chat(unsigned int from_index);
    void client_messyge(const std::string& username, const std::string& text);

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
    void clear_data();

    static unsigned int load_dpi(const DataFile& data_file);
    static DataFile load_data();
    static void create_sized_fonts(unsigned int scale);
    static float rem(float size);

    rain_net::Client m_client;

    enum class State {
        NoConnection,
        Connecting,
        SignIn,
        Processing,
        Chat
    } m_state {State::Connecting};

    Chat m_chat;
    std::vector<ClientUser> m_active_users;

    float m_chat_height {};
    char m_buffer_username[MAX_USERNAME_SIZE] {};
    bool m_load_more {true};
};
