#pragma once

#include "core/Engine.h"
#include "scene/Scene.h"
#include "generation/ProceduralGeneration.h"
#include "utils/ConfigManager.h"
#include "SceneWindow.h"
#include "NodeEditor.h"
#include "SceneManager.h"
#include <SDL2/SDL.h>
#include <memory>
#include <vector>

// ImGui includes
#include <imgui.h>
#include <imgui_impl_sdl2.h>
#include <imgui_impl_sdlrenderer2.h>

class GameEditor {
public:
    GameEditor();
    ~GameEditor();
    
    bool initialize();
    void run();
    void shutdown();

private:
    void update(float deltaTime);
    void render();
    void renderUI();
    
    // Window management
    void saveWindowState();    void loadWindowState();
    void updateWindowState();

public:
    // UI Panels
    void showMainMenuBar();
    void showSceneHierarchy();
    void showInspector();
    void showAssetBrowser();
    void showViewport();
    void showConsole();
    void showCameraControls();
    void showProceduralGeneration();
    void showNodeEditor();
    void showSceneManager();
    
    // Editor functionality
    void createNewScene();
    void openSceneInNewWindow();
    void openSceneInNewWindow(std::shared_ptr<Scene> scene, const std::string& title = "");
    void closeSceneWindow(SceneWindow* window);
    SceneWindow* getActiveSceneWindow();
    void setActiveSceneWindow(SceneWindow* window);
    
    void loadScene(const std::string& filepath);
    void saveScene(const std::string& filepath);
    void createEntity();
    void deleteEntity();
      // Scene window management
    std::vector<std::unique_ptr<SceneWindow>>& getSceneWindows() { return m_sceneWindows; }
    
    // Scene Manager access
    SceneManager* getSceneManager() { return m_sceneManager.get(); }
    
    // Internal helper methods
    void updateActiveSceneData();
    std::string openFolderDialog(const std::string& initialPath = "");
      // Editor state
    bool m_running = false;
    std::shared_ptr<Scene> m_currentScene;
    EntityID m_selectedEntity = 0;
    bool m_hasSelectedEntity = false;
    
    // Scene window management
    std::vector<std::unique_ptr<SceneWindow>> m_sceneWindows;
    SceneWindow* m_activeSceneWindow = nullptr;
    int m_nextSceneWindowId = 1;
    
    // UI state
    bool m_showSceneHierarchy = true;
    bool m_showInspector = true;
    bool m_showAssetBrowser = true;
    bool m_showConsole = true;
    bool m_showCameraControls = true;
    bool m_showProceduralGeneration = true;
    bool m_showNodeEditor = false;
    bool m_showSceneManager = false;
    bool m_showDemo = false;
      // Console
    std::vector<std::string> m_consoleMessages;
    
    // Asset Browser
    std::string m_currentAssetFolder;
    
    // Viewport
    Vector2 m_viewportSize{800, 600};
    bool m_viewportFocused = false;
    
    // Node Editor
    std::unique_ptr<NodeEditor::NodeEditorWindow> m_nodeEditor;
    
    // Scene Manager
    std::unique_ptr<SceneManager> m_sceneManager;
    
    // Rename dialog state
    bool m_showRenameDialog = false;
    EntityID m_renamingEntity = 0;
    std::string m_renameBuffer;
    
    // Procedural generation
    std::unique_ptr<ProceduralGenerationManager> m_proceduralManager;
    std::shared_ptr<ProceduralMap> m_currentMap;
};
