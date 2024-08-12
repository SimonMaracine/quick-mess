#include "window.hpp"

#include <iostream>
#include <cstring>
#include <optional>
#include <algorithm>
#include <cmath>
#include <cassert>

#include "font.hpp"

static constexpr ImVec4 BLUEISH {ImVec4(0.7f, 0.6f, 1.0f, 1.0f)};

void QuickMessWindow::start() {
    const DataFile data_file {load_data()};

    const unsigned int dpi {load_dpi(data_file)};
    create_sized_fonts(dpi);

    ImGuiStyle& style {ImGui::GetStyle()};
    style.ScaleAllSizes(static_cast<float>(dpi));

    m_chat_height = rem(8.0f);

    ImGuiIO& io {ImGui::GetIO()};
    io.IniFilename = nullptr;

    try {
        m_client.connect(data_file.address, PORT);
    } catch (const rain_net::ConnectionError& e) {
        std::cerr << e.what() << '\n';

        m_state = State::NoConnection;
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

    m_chat_height = rem(8.0f);

    ImGui::Begin("Main", nullptr, flags);
    switch (m_state) {
        case State::NoConnection:
            ui_no_connection();
            break;
        case State::Connecting:
            ui_connecting();
            break;
        case State::SignIn:
            ui_sign_in();
            break;
        case State::Processing:
            ui_processing();
            break;
        case State::Chat:
            ui_chat();
            break;
    }
    ImGui::End();

    ImGui::PopStyleVar(2);
}

void QuickMessWindow::stop() {
    m_client.disconnect();
}

void QuickMessWindow::ui_no_connection() {
    ImGui::Text("Disconnected from server.");

    ImGui::Spacing();

    if (ImGui::Button("Try To Reconnect")) {
        m_state = State::Connecting;

        const DataFile data_file {load_data()};

        try {
            m_client.connect(data_file.address, PORT);
        } catch (const rain_net::ConnectionError& e) {
            std::cerr << e.what() << '\n';

            m_state = State::NoConnection;
        }
    }
}

void QuickMessWindow::ui_connecting() {
    try {
        if (m_client.connection_established()) {
            m_state = State::SignIn;
        }
    } catch (const rain_net::ConnectionError& e) {
        std::cerr << e.what() << '\n';

        clear_data();
        m_state = State::NoConnection;
    }

    ImGui::Text("Connecting... Please wait.");
}

void QuickMessWindow::ui_sign_in() {
    ImGui::TextColored(BLUEISH, "Connected to the server!");

    ImGui::Spacing();

    ImGui::PushItemWidth(rem(13.5f));

    if (ImGui::InputText("Username", m_buffer_username, MAX_USERNAME_SIZE, ImGuiInputTextFlags_EnterReturnsTrue)) {
        sign_in();
    }

    ImGui::PopItemWidth();

    ImGui::Spacing();

    if (ImGui::Button("Sign In")) {
        sign_in();
    }
}

void QuickMessWindow::ui_processing() {
    ImGui::Text("Processing... Please wait.");
}

void QuickMessWindow::ui_chat() {
    const float USERS_WIDTH {rem(13.5f)};
    const float BUTTON_WIDTH {rem(8.5f)};

    {
        ImGui::BeginTable("MenuLayout", 2, ImGuiTableFlags_BordersInnerV);

        ImGui::TableSetupColumn("MenuColumn1", ImGuiTableColumnFlags_WidthFixed, USERS_WIDTH);
        ImGui::TableSetupColumn("MenuColumn2", ImGuiTableColumnFlags_None);

        ImGui::TableNextRow();

        ImGui::TableNextColumn();

        ui_chat_users();

        ImGui::TableNextColumn();

        ui_chat_messages();

        ImGui::EndTable();
    }

    ImGui::Spacing();

    static char buffer[MAX_MESSYGE_SIZE] {};
    const auto size {ImVec2(ImGui::GetContentRegionAvail().x - BUTTON_WIDTH, ImGui::GetContentRegionAvail().y)};
    bool reclaim_focus {false};

    const ImGuiInputTextFlags flags {ImGuiInputTextFlags_CtrlEnterForNewLine | ImGuiInputTextFlags_EnterReturnsTrue};

    if (ImGui::InputTextMultiline("##", buffer, MAX_MESSYGE_SIZE, size, flags)) {
        send_messyge(buffer);
        std::memset(buffer, 0, MAX_MESSYGE_SIZE);
        reclaim_focus = true;
    }

    // if (reclaim_focus) {  // FIXME
    //     ImGui::SetKeyboardFocusHere(-1);
    // }

    ImGui::SameLine();

    if (ImGui::Button("Send", ImGui::GetContentRegionAvail())) {
        send_messyge(buffer);
        std::memset(buffer, 0, MAX_MESSYGE_SIZE);
    }
}

void QuickMessWindow::ui_chat_users() {
    ImGui::BeginChild("Users", ImVec2(0.0f, ImGui::GetContentRegionAvail().y - m_chat_height));

    ImGui::Text("Active Users");
    ImGui::Separator();

    {
        ImGui::BeginChild("UsersInner");

        for (const auto& user : m_active_users) {
            ImGui::Text("%s", user.username.c_str());
        }

        ImGui::EndChild();
    }

    ImGui::EndChild();
}

void QuickMessWindow::ui_chat_messages() {
    ImGui::BeginChild("Chat", ImVec2(0.0f, ImGui::GetContentRegionAvail().y - m_chat_height));

    ImGui::TextColored(BLUEISH, "Messy Chat - %s", m_buffer_username);

    ImGui::Separator();

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(12.0f, 12.0f));

    {
        ImGui::BeginChild("ChatInner", {}, ImGuiChildFlags_None, ImGuiWindowFlags_AlwaysUseWindowPadding);

        if (!m_chat.messyges.empty()) {
            const unsigned int first_index {m_chat.messyges.at(0).index};

            if (first_index > 0) {
                if (m_load_more) {
                    if (ImGui::Button("Load More")) {
                        if (first_index > 0) {
                            m_load_more = false;
                            client_ask_more_chat(first_index);
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

        for (const auto& messyge : m_chat.messyges) {
            if (messyge.username == "SERVER") {
                static constexpr auto COLOR {ImVec4(0.5f, 0.5f, 0.9f, 1.0f)};

                ImGui::TextColored(COLOR, "[SERVER]");
                ImGui::TextColored(COLOR, "%s", messyge.text.c_str());
            } else {
                ImGui::TextColored(ImVec4(0.75f, 0.75f, 0.75f, 1.0f), "[%s]", messyge.username.c_str());
                ImGui::TextWrapped("%s", messyge.text.c_str());
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

void QuickMessWindow::client_ask_sign_in(const std::string& username) {
    rain_net::Message message {MSG_CLIENT_ASK_SIGN_IN};

    UsernameStr c_username;
    std::strncpy(c_username.data, username.c_str(), MAX_USERNAME_SIZE);

    message << c_username;

    m_client.send_message(message);
}

void QuickMessWindow::client_ask_more_chat(unsigned int from_index) {
    rain_net::Message message {MSG_CLIENT_ASK_MORE_CHAT};

    message << from_index;

    m_client.send_message(message);
}

void QuickMessWindow::client_messyge(const std::string& username, const std::string& text) {
    rain_net::Message message {MSG_CLIENT_MESSYGE};

    UsernameStr c_username;
    std::strncpy(c_username.data, username.c_str(), MAX_USERNAME_SIZE);

    message << c_username;
    message.write(text.data(), text.size());
    message << static_cast<unsigned short>(text.size());

    m_client.send_message(message);
}

void QuickMessWindow::server_accept_sign_in(const rain_net::Message& message) {
    rain_net::MessageReader reader;
    reader(message);

    unsigned short user_count;
    reader >> user_count;

    for (unsigned short i {0}; i < user_count; i++) {
        UsernameStr username;

        reader >> username;

        ClientUser user;
        user.username = username.data;

        m_active_users.push_back(user);
    }

    m_state = State::Chat;
}

void QuickMessWindow::server_deny_sign_in() {
    std::cerr << "Server denied sign in\n";

    m_state = State::SignIn;
}

void QuickMessWindow::server_user_signed_in(const rain_net::Message& message) {
    if (m_state != State::Chat) {
        return;
    }

    rain_net::MessageReader reader;
    reader(message);

    UsernameStr username;

    reader >> username;

    ClientUser user;
    user.username = username.data;

    m_active_users.push_back(user);
}

void QuickMessWindow::server_user_signed_out(const rain_net::Message& message) {
    if (m_state != State::Chat) {
        return;
    }

    rain_net::MessageReader reader;
    reader(message);

    UsernameStr username;
    reader >> username;

    // Nothing happens when there's nothing to remove
    m_active_users.erase(
        std::remove_if(m_active_users.begin(), m_active_users.end(), [&username](const auto& user) {
            return user.username == username.data;
        }),
        m_active_users.cend()
    );
}

void QuickMessWindow::server_offer_more_chat(const rain_net::Message& message) {
    if (m_state != State::Chat) {
        return;
    }

    rain_net::MessageReader reader;
    reader(message);

    unsigned int count;
    reader >> count;

    for (unsigned int i {0}; i < count; i++) {
        unsigned int index;
        unsigned short size;
        std::string text;
        UsernameStr username;

        reader >> index;
        reader >> size;

        text.resize(size);

        reader.read(text.data(), text.size());
        reader >> username;

        add_messyge_to_chat(username.data, text, index);
    }

    sort_messyges();

    m_load_more = true;
}

void QuickMessWindow::server_messyge(const rain_net::Message& message) {
    if (m_state != State::Chat) {
        return;
    }

    rain_net::MessageReader reader;
    reader(message);

    unsigned int index;
    unsigned short size;
    std::string text;
    UsernameStr username;

    reader >> index;
    reader >> size;

    text.resize(size);

    reader.read(text.data(), text.size());
    reader >> username;

    add_messyge_to_chat(username.data, text, index);
    sort_messyges();
}

void QuickMessWindow::process_messages() {
    while (m_client.available_messages()) {
        rain_net::Message message;

        try {
            message = m_client.next_message();  // FIXME this never throws, because it's never called when errors occur
        } catch (const rain_net::ConnectionError& e) {
            std::cerr << e.what() << '\n';

            clear_data();
            m_state = State::NoConnection;

            return;
        }

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

void QuickMessWindow::sign_in() {
    if (*m_buffer_username != '\0' && std::strcmp(m_buffer_username, "SERVER") != 0) {
        client_ask_sign_in(m_buffer_username);

        m_state = State::Processing;
    } else {
        std::cerr << "Invalid username\n";
    }
}

void QuickMessWindow::send_messyge(const char* buffer) {
    assert(buffer != nullptr);

    client_messyge(m_buffer_username, buffer);
}

void QuickMessWindow::add_messyge_to_chat(const std::string& username, const std::string& text, unsigned int index) {
    Messyge messyge;
    messyge.username = username;
    messyge.text = text;
    messyge.index = index;

    m_chat.messyges.push_back(messyge);
}

void QuickMessWindow::sort_messyges() {
    std::sort(m_chat.messyges.begin(), m_chat.messyges.end(), [](const Messyge& lhs, const Messyge& rhs) {
        return lhs.index < rhs.index;
    });
}

void QuickMessWindow::clear_data() {
    m_chat.index_counter = 0;
    m_chat.messyges.clear();
    m_active_users.clear();
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
