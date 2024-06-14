#include <gui_base/gui_base.hpp>

#include "window.hpp"

int main() {
    gui_base::WindowProperties properties;
    properties.width = 768;
    properties.height = 432;
    properties.title = "quick-mess";

    QuickMessWindow window {properties};
    window.run();
}
