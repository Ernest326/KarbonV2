#include <iostream>
#include "window.h"

namespace Karbon {

class Application {
public:
    void run();
    Application(const char* title);
    ~Application();

private:
    Window* m_window;
};

}