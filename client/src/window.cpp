#include <string>
#include <iostream>
#include <cstring>
#include <optional>

#include <gui_base/gui_base.hpp>
#include <common.hpp>
#include <rain_net/client.hpp>

#include "window.hpp"
#include "client.hpp"
#include "data.hpp"

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

    ImGui::InputText("Username", buffer_username, 16);

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

    ImGui::Text("chat1");
    ImGui::Text("chat2");
    ImGui::Text("chat3");

    ImGui::EndChild();

    ImGui::NextColumn();

    ImGui::BeginChild("Chat", ImVec2(0.0f, ImGui::GetContentRegionAvail().y - CHAT_HEIGHT));

    ImGui::TextColored(ImVec4(0.3f, 0.2f, 0.9f, 1.0f), "Messy Chat");

    ImGui::Separator();

    for (const auto& message : chat.messyges) {
        if (message.username == std::nullopt) {
            static constexpr auto COLOR = ImVec4(0.4f, 0.25f, 0.75f, 1.0f);

            ImGui::TextColored(COLOR, "[SERVER]\n%s", message.text.c_str());
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

    static char buffer[256] {};
    const auto size = ImVec2(ImGui::GetContentRegionAvail().x - BUTTON_WIDTH, ImGui::GetContentRegionAvail().y);

    ImGui::InputTextMultiline("##", buffer, 256, size);

    ImGui::SameLine();

    if (ImGui::Button("Send", ImGui::GetContentRegionAvail())) {
        client.messyge(username, buffer);
        std::memset(buffer, 0, 256);
    }
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
                std::cout << "Server accepted sign in\n";

                username = buffer_username;
                state = State::Menu;

                break;
            case MSG_SERVER_DENY_SIGN_IN:
                std::cout << "Server denied sign in\n";

                state = State::SignIn;

                break;
            case MSG_SERVER_MESSYGE:
                StaticCString<64> source_text;
                StaticCString<16> source_username;

                message >> source_text;
                message >> source_username;

                std::cout << source_username.data << ": " << source_text.data << '\n';

                Messyge messyge;
                messyge.username = std::make_optional(std::string(source_username.data));
                messyge.text = source_text.data;

                chat.messyges.push_back(messyge);

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
