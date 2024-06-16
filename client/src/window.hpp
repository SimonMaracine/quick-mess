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
    void no_connection();
    void connecting();
    void sign_in();
    void processing();
    void chat();

    void chat_users();
    void chat_messages();

    // Server
    void accept_sign_in(const rain_net::Message& message);
    void deny_sign_in();
    void user_signed_in(const rain_net::Message& message);
    void user_signed_out(const rain_net::Message& message);
    void offer_more_chat(const rain_net::Message& message);
    void messyge(const rain_net::Message& message);

    void process_messages();
    void add_messyge_to_chat(const std::string& username, const std::string& text, unsigned int index);
    void sort_messages();

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

    char buffer_username[MAX_USERNAME_SIZE] {};
    float CHAT_HEIGHT {};
};
