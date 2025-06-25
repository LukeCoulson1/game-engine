#include "editor/GameEditor.h"
#include <iostream>

int main(int argc, char* argv[]) {
    GameEditor editor;
    
    if (!editor.initialize()) {
        std::cerr << "Failed to initialize editor!" << std::endl;
        return -1;
    }
    
    editor.run();
    editor.shutdown();
    
    return 0;
}
