#pragma once

#include <common.hpp>

struct SavedChat {
    Chat chat;
};

bool load_chat(SavedChat& saved_chat);
bool save_chat(const SavedChat& saved_chat);
