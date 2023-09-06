#include <iostream>

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
            break;
        case State::Menu:
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

    static char username[32] {};

    ImGui::InputText("Username", username, 32);

    if (ImGui::Button("Sign Up")) {
        client.sign_up(username);

        state = State::Processing;
    }
}

void QuickMessWindow::sign_in() {

}

void QuickMessWindow::menu() {

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
