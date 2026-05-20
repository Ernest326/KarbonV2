#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>

namespace Karbon {

class KarbonImGUI {
public:
    static void init();
    static void begin();
    static void end();
    static void shutdown();

};

}