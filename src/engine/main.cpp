#include <iostream>
#include "core/application.h"

int main() {
    Karbon::Application app("Karbon");
    try {
        app.run();
    } catch (const std::exception& e) {
        std::cerr << "Application error: " << e.what() << std::endl;
    }
    return 0;
}