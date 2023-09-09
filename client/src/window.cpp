#include <string>
#include <iostream>
#include <cstring>
#include <optional>
#include <algorithm>
#include <cassert>

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

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(22.0f, 22.0f));
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

    ImGui::TextColored(BLUEISH, "Connected to the server!");

    ImGui::Spacing();

    ImGui::PushItemWidth(175.0f);
    ImGui::InputText("Username", buffer_username, MAX_USERNAME_SIZE);
    ImGui::PopItemWidth();

    ImGui::Spacing();

    if (ImGui::Button("Sign In")) {
        if (*buffer_username != '\0' && std::strcmp(buffer_username, "SERVER") != 0) {
            client.ask_sign_in(buffer_username);

            state = State::Processing;
        } else {
            std::cout << "Invalid username\n";
        }
    }
}

void QuickMessWindow::menu() {
    if (!check_connection()) {
        return;
    }

    static constexpr float USERS_WIDTH = 175.0f;
    static constexpr float BUTTON_WIDTH = 100.0f;

    {
        ImGui::BeginTable("MenuLayout", 2, ImGuiTableFlags_BordersInnerV);

        ImGui::TableSetupColumn("MenuColumn1", ImGuiTableColumnFlags_WidthFixed, USERS_WIDTH);
        ImGui::TableSetupColumn("MenuColumn2", ImGuiTableColumnFlags_None);

        ImGui::TableNextRow();

        ImGui::TableNextColumn();

        menu_users();

        ImGui::TableNextColumn();

        menu_messages();

        ImGui::EndTable();
    }

    ImGui::Spacing();

    static char buffer[MAX_MESSYGE_SIZE] {};
    const auto size = ImVec2(ImGui::GetContentRegionAvail().x - BUTTON_WIDTH, ImGui::GetContentRegionAvail().y);

    ImGui::InputTextMultiline("##", buffer, MAX_MESSYGE_SIZE, size);

    ImGui::SameLine();

    if (ImGui::Button("Send", ImGui::GetContentRegionAvail())) {
        assert(!data.username.empty());

        client.messyge(data.username, buffer);
        std::memset(buffer, 0, MAX_MESSYGE_SIZE);
    }
}

void QuickMessWindow::menu_users() {
    ImGui::BeginChild("Users", ImVec2(0.0f, ImGui::GetContentRegionAvail().y - CHAT_HEIGHT));

    ImGui::Text("Active Users");
    ImGui::Separator();

    {
        ImGui::BeginChild("UsersInner");

        for (const auto& user : data.users) {
            ImGui::Text("%s", user.c_str());
        }

        ImGui::EndChild();
    }

    ImGui::EndChild();
}

void QuickMessWindow::menu_messages() {
    ImGui::BeginChild("Chat", ImVec2(0.0f, ImGui::GetContentRegionAvail().y - CHAT_HEIGHT));

    ImGui::TextColored(BLUEISH, "Messy Chat - %s", data.username.c_str());

    ImGui::Separator();

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(12.0f, 12.0f));

    {
        ImGui::BeginChild("ChatInner", {}, false, ImGuiWindowFlags_AlwaysUseWindowPadding);

        if (ImGui::Button("Load More")) {
            const unsigned int first_index = data.chat.messyges.at(0).index;

            if (first_index > 0) {
                client.ask_more_chat(first_index);
            }
        }

        ImGui::Spacing();
        ImGui::Spacing();

        for (const auto& message : data.chat.messyges) {
            if (message.username == std::nullopt) {
                static constexpr auto COLOR = ImVec4(0.4f, 0.4f, 0.8f, 1.0f);

                ImGui::TextColored(COLOR, "[SERVER]\n");
                ImGui::TextColored(COLOR, "%s", message.text.c_str());
            } else {
                ImGui::TextColored(ImVec4(0.75f, 0.75f, 0.75f, 1.0f), "[%s]\n", message.username->c_str());
                ImGui::Text("%s", message.text.c_str());
            }

            ImGui::Spacing();
            ImGui::Spacing();

            // Automatically scroll to the bottom
            if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY() - 5.0f) {
                ImGui::SetScrollHereY(1.0f);
            }
        }

        ImGui::EndChild();
    }

    ImGui::PopStyleVar();

    ImGui::EndChild();
}

void QuickMessWindow::accept_sign_in(rain_net::Message& message) {
    std::cout << "Server accepted sign in\n";

    data.username = buffer_username;

    unsigned int user_count;
    message >> user_count;

    for (unsigned int i = 0; i < user_count; i++) {
        StaticCString<MAX_USERNAME_SIZE> username;
        message >> username;

        data.users.push_back(username.data);
    }

    state = State::Menu;
}

void QuickMessWindow::deny_sign_in() {
    std::cout << "Server denied sign in\n";

    state = State::SignIn;
}

void QuickMessWindow::messyge(rain_net::Message& message) {
    if (state != State::Menu) {
        return;
    }

    unsigned int index;
    StaticCString<MAX_MESSYGE_SIZE> text;
    StaticCString<MAX_USERNAME_SIZE> username;

    message >> index;
    message >> text;
    message >> username;

    add_messyge_to_chat(username.data, text.data, index);
    sort_messages();
}

void QuickMessWindow::user_signed_in(rain_net::Message& message) {
    if (state != State::Menu) {
        return;
    }

    StaticCString<MAX_USERNAME_SIZE> username;
    message >> username;

    data.users.push_back(username.data);
}

void QuickMessWindow::user_signed_out(rain_net::Message& message) {
    if (state != State::Menu) {
        return;
    }

    StaticCString<MAX_USERNAME_SIZE> username;
    message >> username;

    // Nothing happens when there's nothing to remove
    data.users.erase(
        std::remove(data.users.begin(), data.users.end(), std::string(username.data)),
        data.users.cend()
    );
}

void QuickMessWindow::offer_more_chat(rain_net::Message& message) {
    if (state != State::Menu) {
        return;
    }

    unsigned int count;
    message >> count;

    for (unsigned int i = 0; i < count; i++) {
        unsigned int index;
        StaticCString<MAX_MESSYGE_SIZE> text;
        StaticCString<MAX_USERNAME_SIZE> username;

        message >> index;
        message >> text;
        message >> username;

        add_messyge_to_chat(username.data, text.data, index);
    }

    sort_messages();
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
            case MSG_SERVER_OFFER_MORE_CHAT:
                offer_more_chat(message);
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

        // Must clear all data
        data = {};

        return false;
    }

    return true;
}

void QuickMessWindow::add_messyge_to_chat(const std::string& username, const std::string& text, unsigned int index) {
    Messyge messyge;

    if (username == "SERVER") {
        messyge.username = std::nullopt;
    } else {
        messyge.username = std::make_optional(username);
    }

    messyge.text = text;
    messyge.index = index;

    data.chat.messyges.push_back(messyge);
}

void QuickMessWindow::sort_messages() {
    std::sort(data.chat.messyges.begin(), data.chat.messyges.end(), [](const Messyge& lhs, const Messyge& rhs) {
        return lhs.index < rhs.index;
    });
}
