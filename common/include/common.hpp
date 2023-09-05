#pragma once

#include <cstddef>

#define MSG_CLIENT_SEND_TO 10
#define MSG_SERVER_SENT_FROM 20
#define MSG_CLIENT_ASK_HELLO 0
#define MSG_SERVER_RESPOND_HELLO 1
#define MSG_CLIENT_ASK_USERNAME_FINE 2
#define MSG_SERVER_RESPOND_USERNAME_FINE 3

template<std::size_t Size>
struct StaticCString {
    char data[Size];
};
