#include <string>
#include <iostream>
#include <cstring>

#include <gui_base/gui_base.hpp>
#include <rain_net/client.hpp>
#include <rain_net/message.hpp>

#include "window.hpp"
#include "client.hpp"

void QuickMessWindow::start() {
    if (!client.connect("localhost", 7021)) {
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
        case State::SignUp:
            sign_up();
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
}

void QuickMessWindow::processing() {
    if (!check_connection()) {
        return;
    }

    ImGui::Text("Processing... Please wait.");
}

void QuickMessWindow::sign_up() {
    if (!check_connection()) {
        return;
    }

    static char username[16] {};

    ImGui::InputText("Username", username, 16);

    if (ImGui::Button("Sign Up")) {
        client.sign_up(username);

        state = State::Processing;
    }
}

void QuickMessWindow::sign_in() {

}

void QuickMessWindow::menu() {
    static constexpr float CHATS_WIDTH = 150.0f;
    static constexpr float CHAT_HEIGHT = 90.0f;
    static constexpr float BUTTON_WIDTH = 100.0f;

    ImGui::Columns(2);

    ImGui::SetColumnWidth(0, CHATS_WIDTH);

    ImGui::BeginChild("Chats", ImVec2(0.0f, ImGui::GetContentRegionAvail().y - CHAT_HEIGHT));

    ImGui::InputText("##", buffer_chat_with, 16);

    ImGui::SameLine();

    if (ImGui::Button("Chat")) {
        client.is_registered(buffer_chat_with);
    }

    ImGui::Separator();

    ImGui::Text("chat1");
    ImGui::Text("chat2");
    ImGui::Text("chat3");

    ImGui::EndChild();

    ImGui::NextColumn();

    ImGui::BeginChild("Chat", ImVec2(0.0f, ImGui::GetContentRegionAvail().y - CHAT_HEIGHT));

    ImGui::TextColored(ImVec4(0.3f, 0.2f, 0.9f, 1.0f), "Chat With: %s", chat_with.c_str());

    ImGui::Separator();

    for (const auto& message : chat.messages) {
        if (message.remote) {
            ImGui::Text("%s", message.text.c_str());
        } else {
            ImGui::TextColored(ImVec4(0.4f, 0.25f, 0.75f, 1.0f), "%s", message.text.c_str());
        }

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
        if (!chat_with.empty()) {
            client.send_to(chat_with, buffer);

            std::memset(buffer, 0, 256);
        }
    }
}

void QuickMessWindow::could_not_connect_to_server() {

}

void QuickMessWindow::process_incoming_messages() {
    while (true) {
        auto result = client.next_incoming_message();
        auto message = result.value_or(rain_net::Message());

        if (!result.has_value()) {
            break;
        }

        switch (message.id()) {
            case MSG_SERVER_ACCEPT_SIGN_UP:
                std::cout << "Server accepted sign up\n";

                state = State::Menu;

                break;
            case MSG_SERVER_DENY_SIGN_UP:
                std::cout << "Server denied sign up\n";

                state = State::SignUp;

                break;
            case MSG_SERVER_POSITIVE_IS_REGISTERED:
                std::cout << "Server found this user\n";

                chat_with = buffer_chat_with;

                break;
            case MSG_SERVER_NEGATIVE_IS_REGISTERED:
                std::cout << "Server didn't find this user\n";

                break;
            case MSG_SERVER_SENT_FROM:
                std::cout << "Got a message\n";

                StaticCString<64> text;
                StaticCString<16> username;

                message >> text;
                message >> username;

                std::cout << username.data << ": " << text.data << '\n';

                break;
        }
    }
}

bool QuickMessWindow::check_connection() {
    if (!client.is_connected()) {
        state = State::NoConnection;
        return false;
    }

    return true;
}
