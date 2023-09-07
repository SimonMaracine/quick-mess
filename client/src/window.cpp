#include <string>
#include <iostream>
#include <cstring>
#include <optional>

#include <gui_base/gui_base.hpp>
#include <common.hpp>
#include <rain_net/client.hpp>

#include "window.hpp"
#include "client.hpp"

void QuickMessWindow::start() {
    if (!try_connect()) {
        state = State::NoConnection;
    }
}

void QuickMessWindow::update() {
    process_incoming_messages();

    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(20.0f, 20.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);

    const ImGuiWindowFlags flags = (
        ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings
    );

    ImGui::Begin("Main", nullptr, flags);
    switch (state) {
        case State::NoConnection:
            no_connection();
            break;
        case State::Processing:
            processing();
            break;
        case State::SignIn:
            sign_in();
            break;
        case State::Menu:
            menu();
            break;
    }
    ImGui::End();

    ImGui::PopStyleVar(2);

    // ImGui::ShowDemoWindow();
}

void QuickMessWindow::dispose() {
    client.disconnect();
}

void QuickMessWindow::no_connection() {
    ImGui::Text("Disconnected from server.");

    ImGui::Spacing();

    if (ImGui::Button("Try To Reconnect")) {
        state = State::SignIn;

        if (!try_connect()) {
            state = State::NoConnection;
        }
    }
}

void QuickMessWindow::processing() {
    if (!check_connection()) {
        return;
    }

    ImGui::Text("Processing... Please wait.");
}

void QuickMessWindow::sign_in() {
    if (!check_connection()) {
        return;
    }

    ImGui::InputText("Username", buffer_username, MAX_USERNAME_SIZE);

    if (ImGui::Button("Sign In")) {
        client.sign_in(buffer_username);

        state = State::Processing;
    }
}

void QuickMessWindow::menu() {
    if (!check_connection()) {
        return;
    }

    static constexpr float CHATS_WIDTH = 150.0f;
    static constexpr float CHAT_HEIGHT = 90.0f;
    static constexpr float BUTTON_WIDTH = 100.0f;

    ImGui::Columns(2);

    ImGui::SetColumnWidth(0, CHATS_WIDTH);

    ImGui::BeginChild("Chats", ImVec2(0.0f, ImGui::GetContentRegionAvail().y - CHAT_HEIGHT));

    ImGui::Text("Active Users");

    ImGui::Separator();

    for (const auto& user : users) {
        ImGui::Text("%s", user.c_str());
    }

    ImGui::EndChild();

    ImGui::NextColumn();

    ImGui::BeginChild("Chat", ImVec2(0.0f, ImGui::GetContentRegionAvail().y - CHAT_HEIGHT));

    ImGui::TextColored(ImVec4(0.3f, 0.2f, 0.9f, 1.0f), "Messy Chat - %s", username.c_str());

    ImGui::Separator();

    for (const auto& message : chat.messyges) {
        if (message.username == std::nullopt) {
            static constexpr auto COLOR = ImVec4(0.4f, 0.25f, 0.75f, 1.0f);

            ImGui::TextColored(COLOR, "[SERVER]\n");
            ImGui::TextColored(COLOR, "%s", message.text.c_str());
        } else {
            ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.8f, 1.0f), "[%s]\n", message.username->c_str());
            ImGui::Text("%s", message.text.c_str());
        }

        ImGui::Spacing();
        ImGui::Spacing();
    }

    ImGui::EndChild();

    ImGui::Columns(1);

    ImGui::Spacing();

    static char buffer[MAX_MESSYGE_SIZE] {};
    const auto size = ImVec2(ImGui::GetContentRegionAvail().x - BUTTON_WIDTH, ImGui::GetContentRegionAvail().y);

    ImGui::InputTextMultiline("##", buffer, MAX_MESSYGE_SIZE, size);

    ImGui::SameLine();

    if (ImGui::Button("Send", ImGui::GetContentRegionAvail())) {
        client.messyge(username, buffer);
        std::memset(buffer, 0, MAX_MESSYGE_SIZE);
    }
}

void QuickMessWindow::accept_sign_in(rain_net::Message& message) {
    std::cout << "Server accepted sign in\n";

    username = buffer_username;

    unsigned int user_count;

    message >> user_count;

    for (unsigned int i = 0; i < user_count; i++) {
        StaticCString<MAX_USERNAME_SIZE> username;

        message >> username;

        users.push_back(username.data);
    }

    state = State::Menu;
}

void QuickMessWindow::deny_sign_in() {
    std::cout << "Server denied sign in\n";

    state = State::SignIn;
}

void QuickMessWindow::messyge(rain_net::Message& message) {
    StaticCString<MAX_MESSYGE_SIZE> source_text;
    StaticCString<MAX_USERNAME_SIZE> source_username;

    message >> source_text;
    message >> source_username;

    Messyge messyge;
    messyge.username = std::make_optional(std::string(source_username.data));
    messyge.text = source_text.data;

    chat.messyges.push_back(messyge);
}

void QuickMessWindow::user_signed_in(rain_net::Message& message) {
    StaticCString<MAX_USERNAME_SIZE> username;
    message >> username;

    users.push_back(username.data);
}

void QuickMessWindow::user_signed_out(rain_net::Message& message) {
    StaticCString<MAX_USERNAME_SIZE> username;
    message >> username;

    users.erase(std::find(users.cbegin(), users.cend(), std::string(username.data)));
}

void QuickMessWindow::process_incoming_messages() {
    while (true) {
        auto result = client.next_incoming_message();
        auto message = result.value_or(rain_net::Message());

        if (!result.has_value()) {
            break;
        }

        switch (message.id()) {
            case MSG_SERVER_ACCEPT_SIGN_IN:
                accept_sign_in(message);
                break;
            case MSG_SERVER_DENY_SIGN_IN:
                deny_sign_in();
                break;
            case MSG_SERVER_MESSYGE:
                messyge(message);
                break;
            case MSG_SERVER_USER_SIGNED_IN:
                user_signed_in(message);
                break;
            case MSG_SERVER_USER_SIGNED_OUT:
                user_signed_out(message);
                break;
        }
    }
}

bool QuickMessWindow::try_connect() {
    client.disconnect();

    return client.connect("localhost", 7021);
}

bool QuickMessWindow::check_connection() {
    if (!client.is_connected()) {
        state = State::NoConnection;

        return false;
    }

    return true;
}
