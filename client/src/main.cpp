#include "window.hpp"

/*
    Usernames consist only of ASCII letters, numbers and underscores

    The only thing that isn't a message is a too long message and this: "\e"
*/

int main() {
    QuickMessWindow window;
    window.run();
}
