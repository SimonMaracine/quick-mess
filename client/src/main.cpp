#include <iostream>

#include <gui_base/gui_base.hpp>

#include "window.hpp"

int main() {
    gui_base::WindowProperties properties;
    properties.width = 768;
    properties.height = 432;
    properties.min_width = 512;
    properties.min_height = 288;
    properties.title = "quick-mess";

    try {
        QuickMessWindow window {properties};
        window.run();
    } catch (const gui_base::InitializationError& e) {
        std::cerr << e.what() << '\n';
        return 1;
    }

    return 0;
}
