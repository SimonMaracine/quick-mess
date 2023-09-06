#pragma once

#include <cstddef>

#define MSG_CLIENT_ASK_SIGN_UP 0
#define MSG_SERVER_ACCEPT_SIGN_UP 1
#define MSG_SERVER_DENY_SIGN_UP 2

#define MSG_CLIENT_ASK_IS_REGISTERED 3
#define MSG_SERVER_POSITIVE_IS_REGISTERED 4
#define MSG_SERVER_NEGATIVE_IS_REGISTERED 5

#define MSG_CLIENT_SEND_TO 10
#define MSG_SERVER_SENT_FROM 20

template<std::size_t Size>
struct StaticCString {
    char data[Size];
};
