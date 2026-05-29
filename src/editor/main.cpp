#include "karbon.h"
#include "core/editor_layer.h"

int main() {
    Karbon::Application app("Karbon Editor");
    app.pushOverlay(new Karbon::EditorLayer(&app.getActiveScene()));
    app.run();
    return 0;
}
