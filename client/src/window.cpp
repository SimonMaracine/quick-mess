#include "window.hpp"

#include <iostream>
#include <cstring>
#include <optional>
#include <algorithm>
#include <cassert>
#include <cmath>

#include <rain_net/client.hpp>

#include "data.hpp"

static constexpr ImVec4 BLUEISH {ImVec4(0.6f, 0.5f, 1.0f, 1.0f)};

void QuickMessWindow::start() {
    const unsigned int dpi {load_dpi()};
    create_sized_fonts(dpi);

    ImGuiStyle& style {ImGui::GetStyle()};
    style.ScaleAllSizes(static_cast<float>(dpi));

    CHAT_HEIGHT = rem(8.0f);

    ImGuiIO& io {ImGui::GetIO()};
    io.IniFilename = nullptr;

    if (!try_connect()) {
        state = State::NoConnection;
    }
}

void QuickMessWindow::update() {
    process_incoming_messages();

    const ImGuiViewport* viewport {ImGui::GetMainViewport()};
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(22.0f, 22.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);

    const ImGuiWindowFlags flags {
        ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings
    };

    CHAT_HEIGHT = rem(8.0f);

    ImGui::Begin("Main", nullptr, flags);
    switch (state) {
        case State::NoConnection:
            no_connection();
            break;
        case State::Connecting:
            connecting();
            break;
        case State::SignIn:
            sign_in();
            break;
        case State::Processing:
            processing();
            break;
        case State::Chat:
            chat();
            break;
    }
    ImGui::End();

    ImGui::PopStyleVar(2);
}

void QuickMessWindow::stop() {
    client.disconnect();
}

void QuickMessWindow::no_connection() {
    ImGui::Text("Disconnected from server.");

    ImGui::Spacing();

    if (ImGui::Button("Try To Reconnect")) {
        state = State::Connecting;

        if (!try_connect()) {
            state = State::NoConnection;
        }
    }
}

void QuickMessWindow::connecting() {
    if (!check_connection()) {
        return;
    }

    if (connection_flag) {
        state = State::SignIn;

        connection_flag = false;
    }

    ImGui::Text("Connecting... Please wait.");
}

void QuickMessWindow::sign_in() {
    if (!check_connection()) {
        return;
    }

    ImGui::TextColored(BLUEISH, "Connected to the server!");

    ImGui::Spacing();

    ImGui::PushItemWidth(rem(13.5f));
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

void QuickMessWindow::processing() {
    if (!check_connection()) {
        return;
    }

    ImGui::Text("Processing... Please wait.");
}

void QuickMessWindow::chat() {
    if (!check_connection()) {
        return;
    }

    const float USERS_WIDTH {rem(13.5f)};
    const float BUTTON_WIDTH {rem(8.5f)};

    {
        ImGui::BeginTable("MenuLayout", 2, ImGuiTableFlags_BordersInnerV);

        ImGui::TableSetupColumn("MenuColumn1", ImGuiTableColumnFlags_WidthFixed, USERS_WIDTH);
        ImGui::TableSetupColumn("MenuColumn2", ImGuiTableColumnFlags_None);

        ImGui::TableNextRow();

        ImGui::TableNextColumn();

        chat_users();

        ImGui::TableNextColumn();

        chat_messages();

        ImGui::EndTable();
    }

    ImGui::Spacing();

    static char buffer[MAX_MESSYGE_SIZE] {};
    const auto size {ImVec2(ImGui::GetContentRegionAvail().x - BUTTON_WIDTH, ImGui::GetContentRegionAvail().y)};

    ImGui::InputTextMultiline("##", buffer, MAX_MESSYGE_SIZE, size);

    ImGui::SameLine();

    if (ImGui::Button("Send", ImGui::GetContentRegionAvail())) {
        assert(!data.username.empty());

        client.messyge(data.username, buffer);
        std::memset(buffer, 0, MAX_MESSYGE_SIZE);
    }
}

void QuickMessWindow::chat_users() {
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

void QuickMessWindow::chat_messages() {
    ImGui::BeginChild("Chat", ImVec2(0.0f, ImGui::GetContentRegionAvail().y - CHAT_HEIGHT));

    ImGui::TextColored(BLUEISH, "Messy Chat - %s", data.username.c_str());

    ImGui::Separator();

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(12.0f, 12.0f));

    {
        ImGui::BeginChild("ChatInner", {}, false, ImGuiWindowFlags_AlwaysUseWindowPadding);

        if (!data.chat.messyges.empty() && data.chat.messyges.at(0).index > 0) {
            if (ImGui::Button("Load More")) {
                const unsigned int first_index = data.chat.messyges.at(0).index;

                if (first_index > 0) {
                    client.ask_more_chat(first_index);
                }
            }
        }

        ImGui::Spacing();
        ImGui::Spacing();

        for (const auto& message : data.chat.messyges) {
            if (!message.username) {
                static constexpr auto COLOR {ImVec4(0.4f, 0.4f, 0.8f, 1.0f)};

                ImGui::TextColored(COLOR, "[SERVER]\n");
                ImGui::TextColored(COLOR, "%s", message.text.c_str());
            } else {
                ImGui::TextColored(ImVec4(0.75f, 0.75f, 0.75f, 1.0f), "[%s]\n", message.username->c_str());
                ImGui::TextWrapped("%s", message.text.c_str());
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

    for (unsigned int i {0}; i < user_count; i++) {
        StaticCString<MAX_USERNAME_SIZE> username;
        message >> username;

        data.users.push_back(username.data);
    }

    state = State::Chat;
}

void QuickMessWindow::deny_sign_in() {
    std::cout << "Server denied sign in\n";

    state = State::SignIn;
}

void QuickMessWindow::messyge(rain_net::Message& message) {
    if (state != State::Chat) {
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
    if (state != State::Chat) {
        return;
    }

    StaticCString<MAX_USERNAME_SIZE> username;
    message >> username;

    data.users.push_back(username.data);
}

void QuickMessWindow::user_signed_out(rain_net::Message& message) {
    if (state != State::Chat) {
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
    if (state != State::Chat) {
        return;
    }

    unsigned int count;
    message >> count;

    for (unsigned int i {0}; i < count; i++) {
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
        auto result {client.next_incoming_message()};
        auto message {result.value_or(rain_net::Message())};

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

    DataFile data_file;

    if (!load_data_file(data_file)) {
        std::cout << "Could not load host address from file\n";

        if (!create_data_file()) {
            std::cout << "Could not create data file\n";
        }

        data_file.host_address = "localhost";
    }

    const bool result {client.connect(data_file.host_address, PORT, [this]() {
        connection_flag = true;
    })};

    return result;
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

unsigned int QuickMessWindow::load_dpi() {
    DataFile data_file;

    if (!load_data_file(data_file)) {
        std::cout << "Could not load DPI scale from file\n";

        if (!create_data_file()) {
            std::cout << "Could not create data file\n";
        }
    }

    return std::clamp(data_file.dpi_scale, 1u, 3u);
}

void QuickMessWindow::create_sized_fonts(unsigned int scale) {
    const char* FONT_FILE {"LiberationMono-Regular.ttf"};
    const float SCALE {std::floor(13.0f * static_cast<float>(scale))};

    ImGuiIO& io {ImGui::GetIO()};
    const auto font {io.Fonts->AddFontFromFileTTF(FONT_FILE, SCALE)};  // FIXME

    if (font == nullptr) {
        return;
    }

    io.FontDefault = font;
    io.Fonts->Build();
}

float QuickMessWindow::rem(float size) {
    return ImGui::GetFontSize() * size;
}
