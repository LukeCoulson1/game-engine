#pragma once

#include <imgui.h>
#include <vector>
#include <string>
#include <memory>
#include <filesystem>

// Forward declarations
class Scene;
class GameEditor;

struct SceneInfo {
    std::string name;
    std::string filepath;
    std::shared_ptr<Scene> scene;
    bool isLoaded = false;
    bool isDirty = false;
    size_t lastModified = 0;
    
    SceneInfo(const std::string& sceneName, const std::string& scenePath) 
        : name(sceneName), filepath(scenePath) {}
};

class SceneManager {
public:
    SceneManager(GameEditor* editor);
    ~SceneManager();
    
    void show(bool* open);
    void refresh();    // Scene operations
    void createNewScene();
    void createUntitledScene();
    void createInitialScene();
    void loadScene(const std::string& filepath);
    void saveScene(SceneInfo& sceneInfo);
    void saveSceneAs(SceneInfo& sceneInfo, const std::string& newPath);
    void closeScene(SceneInfo& sceneInfo);
    void duplicateScene(const SceneInfo& sceneInfo);
    void renameScene(SceneInfo& sceneInfo, const std::string& newName);
    
    // Scene management
    void addSceneToList(std::shared_ptr<Scene> scene, const std::string& name, const std::string& filepath = "");
    void removeSceneFromList(const std::string& filepath);
    void markSceneDirty(const std::string& filepath);
    
    // Scene organization
    void createSceneFolder(const std::string& folderName);
    void moveSceneToFolder(SceneInfo& sceneInfo, const std::string& folderPath);
    
    // Getters
    const std::vector<SceneInfo>& getScenes() const { return m_scenes; }
    SceneInfo* findSceneByPath(const std::string& filepath);
    
private:
    void showSceneList();    void showSceneContextMenu(SceneInfo& sceneInfo);
    void showCreateSceneDialog();
    void showSaveAsDialog();
    void showNewFolderDialog();
    void showRenameDialog();
    
    void scanScenesDirectory();
    std::string getRelativePath(const std::string& fullPath);
    std::string getScenesDirectory();
    bool isValidSceneFile(const std::string& filepath);
      // UI helpers
    void drawSceneIcon(const SceneInfo& sceneInfo);
    ImVec4 getSceneStatusColor(const SceneInfo& sceneInfo);
    const char* getSceneStatusText(const SceneInfo& sceneInfo);
      // Scene serialization helpers
    bool loadSceneFromJson(std::shared_ptr<Scene> scene, const std::string& jsonData);
    bool saveSceneToJson(std::shared_ptr<Scene> scene, std::string& jsonData);
    
private:
    GameEditor* m_editor;
    std::vector<SceneInfo> m_scenes;
    std::string m_scenesDirectory;
      // UI state
    bool m_showCreateDialog = false;
    bool m_showSaveAsDialog = false;
    bool m_showNewFolderDialog = false;
    bool m_showRenameDialog = false;
    SceneInfo* m_selectedScene = nullptr;
      // Dialog buffers
    char m_newSceneNameBuffer[256] = {};
    char m_saveAsPathBuffer[512] = {};
    char m_newFolderNameBuffer[256] = {};
    char m_renameBuffer[256] = {};
    
    // View options
    bool m_showOnlyLoadedScenes = false;
    bool m_groupByFolder = true;
    int m_viewMode = 0; // 0 = List, 1 = Grid
    
    // Filtering
    char m_searchBuffer[256] = {};
    std::string m_currentFilter;
};
