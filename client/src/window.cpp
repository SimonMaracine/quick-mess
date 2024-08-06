#include "window.hpp"

#include <iostream>
#include <cstring>
#include <optional>
#include <algorithm>
#include <cassert>
#include <cmath>

#include "font.hpp"

static constexpr ImVec4 BLUEISH {ImVec4(0.6f, 0.5f, 1.0f, 1.0f)};

void QuickMessWindow::start() {
    const DataFile data_file {load_data()};

    const unsigned int dpi {load_dpi(data_file)};
    create_sized_fonts(dpi);

    ImGuiStyle& style {ImGui::GetStyle()};
    style.ScaleAllSizes(static_cast<float>(dpi));

    CHAT_HEIGHT = rem(8.0f);

    ImGuiIO& io {ImGui::GetIO()};
    io.IniFilename = nullptr;

    client.connect(data_file.address, PORT);

    if (client.fail()) {
        std::cerr << client.fail_reason() << '\n';
        client.disconnect();

        state = State::NoConnection;
    }
}

void QuickMessWindow::update() {
    process_messages();

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

    if (client.fail()) {
        std::cerr << client.fail_reason() << '\n';
        client.disconnect();

        state = State::NoConnection;
        data = {};
    }
}

void QuickMessWindow::stop() {
    client.disconnect();
}

void QuickMessWindow::no_connection() {
    ImGui::Text("Disconnected from server.");

    ImGui::Spacing();

    if (ImGui::Button("Try To Reconnect")) {
        state = State::Connecting;

        const DataFile data_file {load_data()};

        client.connect(data_file.address, PORT);

        if (client.fail()) {
            std::cerr << client.fail_reason() << '\n';
            client.disconnect();

            state = State::NoConnection;
        }
    }
}

void QuickMessWindow::connecting() {
    if (client.connection_established()) {
        state = State::SignIn;
    }

    ImGui::Text("Connecting... Please wait.");
}

void QuickMessWindow::sign_in() {
    ImGui::TextColored(BLUEISH, "Connected to the server!");

    ImGui::Spacing();

    ImGui::PushItemWidth(rem(13.5f));
    ImGui::InputText("Username", buffer_username, MAX_USERNAME_SIZE);
    ImGui::PopItemWidth();

    ImGui::Spacing();

    if (ImGui::Button("Sign In")) {
        if (*buffer_username != '\0' && std::strcmp(buffer_username, "SERVER") != 0) {
            client.client_ask_sign_in(buffer_username);

            state = State::Processing;
        } else {
            std::cerr << "Invalid username\n";
        }
    }
}

void QuickMessWindow::processing() {
    ImGui::Text("Processing... Please wait.");
}

void QuickMessWindow::chat() {
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

    ImGui::InputTextMultiline("##", buffer, MAX_MESSYGE_SIZE, size);  // TODO Ctrl+Enter new line

    ImGui::SameLine();

    if (ImGui::Button("Send", ImGui::GetContentRegionAvail())) {  // TODO Enter key event
        assert(!data.username.empty());

        client.client_messyge(data.username, buffer);
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
        ImGui::BeginChild("ChatInner", {}, ImGuiChildFlags_None, ImGuiWindowFlags_AlwaysUseWindowPadding);

        if (!data.chat.messyges.empty()) {
            const unsigned int first_index {data.chat.messyges.at(0).index};

            if (first_index > 0) {
                if (load_more) {
                    if (ImGui::Button("Load More")) {
                        if (first_index > 0) {
                            load_more = false;
                            client.client_ask_more_chat(first_index);
                        }
                    }
                } else {
                    ImGui::BeginDisabled();
                    ImGui::Button("Load More");
                    ImGui::EndDisabled();
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
            if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY() - 6.0f) {
                ImGui::SetScrollHereY(1.0f);
            }
        }

        ImGui::EndChild();
    }

    ImGui::PopStyleVar();

    ImGui::EndChild();
}

void QuickMessWindow::server_accept_sign_in(const rain_net::Message& message) {
    data.username = buffer_username;

    rain_net::MessageReader reader;
    reader(message);

    unsigned int user_count;
    reader >> user_count;

    for (unsigned int i {0}; i < user_count; i++) {
        UsernameStr username;
        reader >> username;

        data.users.push_back(username.data);
    }

    state = State::Chat;
}

void QuickMessWindow::server_deny_sign_in() {
    std::cerr << "Server denied sign in\n";

    state = State::SignIn;
}

void QuickMessWindow::server_user_signed_in(const rain_net::Message& message) {
    if (state != State::Chat) {
        return;
    }

    rain_net::MessageReader reader;
    reader(message);

    UsernameStr username;
    reader >> username;

    data.users.push_back(username.data);
}

void QuickMessWindow::server_user_signed_out(const rain_net::Message& message) {
    if (state != State::Chat) {
        return;
    }

    rain_net::MessageReader reader;
    reader(message);

    UsernameStr username;
    reader >> username;

    // Nothing happens when there's nothing to remove
    data.users.erase(
        std::remove(data.users.begin(), data.users.end(), std::string(username.data)),
        data.users.cend()
    );
}

void QuickMessWindow::server_offer_more_chat(const rain_net::Message& message) {
    if (state != State::Chat) {
        return;
    }

    rain_net::MessageReader reader;
    reader(message);

    unsigned int count;
    reader >> count;

    for (unsigned int i {0}; i < count; i++) {
        unsigned int index;
        MessygeStr text;
        UsernameStr username;

        reader >> index;
        reader >> text;
        reader >> username;

        add_messyge_to_chat(username.data, text.data, index);
    }

    sort_messages();

    load_more = true;
}

void QuickMessWindow::server_messyge(const rain_net::Message& message) {
    if (state != State::Chat) {
        return;
    }

    rain_net::MessageReader reader;
    reader(message);

    unsigned int index;
    MessygeStr text;
    UsernameStr username;

    reader >> index;
    reader >> text;
    reader >> username;

    add_messyge_to_chat(username.data, text.data, index);
    sort_messages();
}

void QuickMessWindow::process_messages() {
    while (true) {
        const auto result {client.next_incoming_message()};

        if (!result.has_value()) {
            break;
        }

        const rain_net::Message& message {*result};

        switch (message.id()) {
            case MSG_SERVER_ACCEPT_SIGN_IN:
                server_accept_sign_in(message);
                break;
            case MSG_SERVER_DENY_SIGN_IN:
                server_deny_sign_in();
                break;
            case MSG_SERVER_USER_SIGNED_IN:
                server_user_signed_in(message);
                break;
            case MSG_SERVER_USER_SIGNED_OUT:
                server_user_signed_out(message);
                break;
            case MSG_SERVER_OFFER_MORE_CHAT:
                server_offer_more_chat(message);
                break;
            case MSG_SERVER_MESSYGE:
                server_messyge(message);
                break;
        }
    }
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

unsigned int QuickMessWindow::load_dpi(const DataFile& data_file) {
    return std::clamp(data_file.dpi_scale, 1u, 3u);
}

DataFile QuickMessWindow::load_data() {
    DataFile data_file;

    try {
        data_file = load_data_file();
    } catch (const DataError& e) {
        std::cerr << e.what() << '\n';

        try {
            create_data_file();
        } catch (const DataError& e) {
            std::cerr << e.what() << '\n';
        }
    }

    return data_file;
}

void QuickMessWindow::create_sized_fonts(unsigned int scale) {
    const float SCALE {std::floor(13.0f * static_cast<float>(scale))};

    ImGuiIO& io {ImGui::GetIO()};

    ImFontGlyphRangesBuilder builder;
    builder.AddRanges(io.Fonts->GetGlyphRangesDefault());
    builder.AddText(u8"ăîâșțĂÎÂȘȚ„”");
    ImVector<ImWchar> ranges;
    builder.BuildRanges(&ranges);

    // This should make the next const_cast safe
    ImFontConfig config;
    config.FontDataOwnedByAtlas = false;

    const auto font {
        io.Fonts->AddFontFromMemoryTTF(
            const_cast<unsigned char*>(LiberationMono_Regular_ttf),
            LiberationMono_Regular_ttf_len,
            SCALE,
            &config,
            ranges.Data
        )
    };

    if (font == nullptr) {
        return;
    }

    io.FontDefault = font;
    io.Fonts->Build();
}

float QuickMessWindow::rem(float size) {
    return ImGui::GetFontSize() * size;
}
