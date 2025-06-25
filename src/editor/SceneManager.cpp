#include "SceneManager.h"
#include "GameEditor.h"
#include "../scene/Scene.h"
#include "../components/Components.h"
#include "../generation/ProceduralGeneration.h"
#include <imgui.h>
#include <iostream>
#include <fstream>
#include <filesystem>
#include <algorithm>
#include <nlohmann/json.hpp>
#include <ctime>

namespace fs = std::filesystem;
using json = nlohmann::json;

SceneManager::SceneManager(GameEditor* editor) : m_editor(editor) {
    m_scenesDirectory = getScenesDirectory();
    
    // Create scenes directory if it doesn't exist
    if (!fs::exists(m_scenesDirectory)) {
        fs::create_directories(m_scenesDirectory);
    }
      // Scan for existing scenes
    scanScenesDirectory();
    
    // Create an initial scene only if no scene files exist at all
    if (m_scenes.empty()) {
        createInitialScene();
    }
}

SceneManager::~SceneManager() {
}

void SceneManager::show(bool* open) {
    ImGui::SetNextWindowSize(ImVec2(600, 400), ImGuiCond_FirstUseEver);
    if (!ImGui::Begin("🗂️ Scene Manager", open, ImGuiWindowFlags_MenuBar)) {
        ImGui::End();
        return;
    }
    
    // Menu bar
    if (ImGui::BeginMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("New Scene", "Ctrl+N")) {
                createNewScene();
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Refresh", "F5")) {
                refresh();
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Create Folder")) {
                m_showNewFolderDialog = true;
            }
            ImGui::EndMenu();
        }
        
        if (ImGui::BeginMenu("View")) {
            ImGui::Checkbox("Show Only Loaded Scenes", &m_showOnlyLoadedScenes);
            ImGui::Checkbox("Group By Folder", &m_groupByFolder);
            ImGui::Separator();
            if (ImGui::RadioButton("List View", m_viewMode == 0)) m_viewMode = 0;
            if (ImGui::RadioButton("Grid View", m_viewMode == 1)) m_viewMode = 1;
            ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
    }
      // Toolbar
    if (ImGui::Button("New Scene")) {
        createNewScene();
    }
    ImGui::SameLine();
    if (ImGui::Button("New Untitled")) {
        createUntitledScene();
    }
    ImGui::SameLine();
    if (ImGui::Button("Refresh")) {
        refresh();
    }    ImGui::SameLine();
    
    // Enable "Save Selected" button if a loaded scene is selected
    bool canSave = m_selectedScene && m_selectedScene->isLoaded && m_selectedScene->scene;
    if (!canSave) {
        ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.5f);
    }
    
    if (ImGui::Button("Save Selected")) {
        if (canSave) {
            // If scene doesn't have a filepath, show Save As dialog
            if (m_selectedScene->filepath.empty()) {
                m_showSaveAsDialog = true;
                // Pre-fill the save path with scene name
                std::string suggestedPath = m_scenesDirectory + "/" + m_selectedScene->name + ".scene";
                strncpy_s(m_saveAsPathBuffer, suggestedPath.c_str(), sizeof(m_saveAsPathBuffer) - 1);
                m_saveAsPathBuffer[sizeof(m_saveAsPathBuffer) - 1] = '\0';
            } else {
                // Scene already has a filepath, save directly
                saveScene(*m_selectedScene);
            }
        }
    }
    
    if (!canSave) {
        ImGui::PopStyleVar();
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled)) {
            ImGui::SetTooltip("Select a loaded scene to save");
        }
    } else if (m_selectedScene->filepath.empty()) {
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("Save '%s' (will prompt for location)", m_selectedScene->name.c_str());
        }
    } else if (m_selectedScene->isDirty) {
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("Save changes to '%s'", m_selectedScene->name.c_str());
        }
    } else {
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("Save '%s'", m_selectedScene->name.c_str());
        }
    }
    ImGui::SameLine();
    ImGui::Separator();
    ImGui::SameLine();
    
    // Search bar
    ImGui::SetNextItemWidth(200);
    if (ImGui::InputTextWithHint("##search", "Search scenes...", m_searchBuffer, sizeof(m_searchBuffer))) {
        m_currentFilter = std::string(m_searchBuffer);
    }
    
    ImGui::Separator();
    
    // Scene list
    showSceneList();
      // Dialogs
    if (m_showCreateDialog) {
        showCreateSceneDialog();
    }
    
    if (m_showSaveAsDialog) {
        showSaveAsDialog();
    }
    
    if (m_showNewFolderDialog) {
        showNewFolderDialog();
    }
    
    if (m_showRenameDialog) {
        showRenameDialog();
    }
    
    ImGui::End();
}

void SceneManager::showSceneList() {
    ImGui::BeginChild("SceneList", ImVec2(0, 0), true);
    
    // Filter scenes
    std::vector<SceneInfo*> filteredScenes;
    for (auto& scene : m_scenes) {
        // Apply loaded filter
        if (m_showOnlyLoadedScenes && !scene.isLoaded) continue;
        
        // Apply search filter
        if (!m_currentFilter.empty()) {
            std::string lowerName = scene.name;
            std::string lowerFilter = m_currentFilter;
            std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(), ::tolower);
            std::transform(lowerFilter.begin(), lowerFilter.end(), lowerFilter.begin(), ::tolower);
            if (lowerName.find(lowerFilter) == std::string::npos) continue;
        }
        
        filteredScenes.push_back(&scene);
    }
    
    // Display scenes
    for (auto* sceneInfo : filteredScenes) {
        ImGui::PushID(sceneInfo);
        
        // Scene icon and status
        drawSceneIcon(*sceneInfo);
        ImGui::SameLine();
        
        // Scene name (clickable)
        ImVec4 statusColor = getSceneStatusColor(*sceneInfo);
        ImGui::PushStyleColor(ImGuiCol_Text, statusColor);
        
        bool isSelected = (m_selectedScene == sceneInfo);
        if (ImGui::Selectable(sceneInfo->name.c_str(), isSelected, ImGuiSelectableFlags_AllowDoubleClick)) {
            m_selectedScene = sceneInfo;
            
            // Double-click to open/load scene
            if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
                if (!sceneInfo->isLoaded) {
                    loadScene(sceneInfo->filepath);
                }
                if (sceneInfo->scene) {
                    m_editor->openSceneInNewWindow(sceneInfo->scene, sceneInfo->name);
                }
            }
        }
        
        ImGui::PopStyleColor();
        
        // Context menu
        if (ImGui::BeginPopupContextItem()) {
            showSceneContextMenu(*sceneInfo);
            ImGui::EndPopup();
        }
        
        // Status and path info
        ImGui::SameLine();
        ImGui::TextDisabled("(%s)", getSceneStatusText(*sceneInfo));
        
        if (!sceneInfo->filepath.empty()) {
            ImGui::Indent();
            ImGui::TextDisabled("%s", getRelativePath(sceneInfo->filepath).c_str());
            ImGui::Unindent();
        }
        
        ImGui::PopID();
    }
    
    if (filteredScenes.empty()) {
        ImGui::TextDisabled("No scenes found");
        if (!m_currentFilter.empty()) {
            ImGui::TextDisabled("Try adjusting your search filter");
        }
    }
    
    ImGui::EndChild();
}

void SceneManager::showSceneContextMenu(SceneInfo& sceneInfo) {
    if (!sceneInfo.isLoaded && ImGui::MenuItem("Load Scene")) {
        loadScene(sceneInfo.filepath);
    }
    
    if (sceneInfo.isLoaded && ImGui::MenuItem("Open in New Window")) {
        if (sceneInfo.scene) {
            m_editor->openSceneInNewWindow(sceneInfo.scene, sceneInfo.name);
        }
    }
    
    ImGui::Separator();
    
    if (sceneInfo.isLoaded && sceneInfo.isDirty && ImGui::MenuItem("Save")) {
        saveScene(sceneInfo);
    }
    
    if (ImGui::MenuItem("Save As...")) {
        m_selectedScene = &sceneInfo;
        m_showSaveAsDialog = true;
        strcpy_s(m_saveAsPathBuffer, sceneInfo.filepath.c_str());
    }
      ImGui::Separator();
    
    if (ImGui::MenuItem("Rename...")) {
        m_selectedScene = &sceneInfo;
        m_showRenameDialog = true;
        strcpy_s(m_renameBuffer, sceneInfo.name.c_str());
    }
    
    if (ImGui::MenuItem("Duplicate")) {
        duplicateScene(sceneInfo);
    }
    
    if (sceneInfo.isLoaded && ImGui::MenuItem("Close")) {
        closeScene(sceneInfo);
    }
    
    ImGui::Separator();
    
    if (ImGui::MenuItem("Remove from List")) {
        removeSceneFromList(sceneInfo.filepath);
    }
    
    ImGui::Separator();
    
    if (ImGui::MenuItem("Show in Explorer")) {
        if (fs::exists(sceneInfo.filepath)) {
            std::string command = "explorer /select,\"" + sceneInfo.filepath + "\"";
            system(command.c_str());
        }
    }
}

void SceneManager::showCreateSceneDialog() {
    ImGui::OpenPopup("Create New Scene");
    
    if (ImGui::BeginPopupModal("Create New Scene", &m_showCreateDialog, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Enter scene name:");
        ImGui::InputText("##scenename", m_newSceneNameBuffer, sizeof(m_newSceneNameBuffer));
        
        ImGui::Separator();
          if (ImGui::Button("Create")) {
            if (strlen(m_newSceneNameBuffer) > 0) {
                std::string sceneName = m_newSceneNameBuffer;
                std::string sceneFile = m_scenesDirectory + "/" + sceneName + ".scene";
                
                // Create new scene
                auto newScene = std::make_shared<Scene>();
                newScene->initialize();
                
                // Add to list
                addSceneToList(newScene, sceneName, sceneFile);
                
                // Immediately save the new scene to disk so it persists through refresh
                auto* sceneInfo = findSceneByPath(sceneFile);
                if (sceneInfo) {
                    saveScene(*sceneInfo);
                }
                
                // Open in editor
                m_editor->openSceneInNewWindow(newScene, sceneName);
                
                // Clear buffer
                memset(m_newSceneNameBuffer, 0, sizeof(m_newSceneNameBuffer));
                m_showCreateDialog = false;
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel")) {
            memset(m_newSceneNameBuffer, 0, sizeof(m_newSceneNameBuffer));
            m_showCreateDialog = false;
        }
        
        ImGui::EndPopup();
    }
}

void SceneManager::showSaveAsDialog() {
    ImGui::OpenPopup("Save Scene As");
    
    if (ImGui::BeginPopupModal("Save Scene As", &m_showSaveAsDialog, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Save scene to:");
        ImGui::InputText("##savepath", m_saveAsPathBuffer, sizeof(m_saveAsPathBuffer));
        
        ImGui::Separator();
        
        if (ImGui::Button("Save")) {
            if (strlen(m_saveAsPathBuffer) > 0 && m_selectedScene) {
                saveSceneAs(*m_selectedScene, std::string(m_saveAsPathBuffer));
                m_showSaveAsDialog = false;
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel")) {
            m_showSaveAsDialog = false;
        }
        
        ImGui::EndPopup();
    }
}

void SceneManager::showNewFolderDialog() {
    ImGui::OpenPopup("Create Folder");
    
    if (ImGui::BeginPopupModal("Create Folder", &m_showNewFolderDialog, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Folder name:");
        ImGui::InputText("##foldername", m_newFolderNameBuffer, sizeof(m_newFolderNameBuffer));
        
        ImGui::Separator();
        
        if (ImGui::Button("Create")) {
            if (strlen(m_newFolderNameBuffer) > 0) {
                createSceneFolder(m_newFolderNameBuffer);
                memset(m_newFolderNameBuffer, 0, sizeof(m_newFolderNameBuffer));
                m_showNewFolderDialog = false;
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel")) {
            memset(m_newFolderNameBuffer, 0, sizeof(m_newFolderNameBuffer));
            m_showNewFolderDialog = false;
        }
        
        ImGui::EndPopup();
    }
}

void SceneManager::showRenameDialog() {
    ImGui::OpenPopup("Rename Scene");
    
    if (ImGui::BeginPopupModal("Rename Scene", &m_showRenameDialog, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Enter new scene name:");
        
        // Auto-focus the input field when dialog opens
        if (ImGui::IsWindowAppearing()) {
            ImGui::SetKeyboardFocusHere();
        }
        
        if (ImGui::InputText("##renamescene", m_renameBuffer, sizeof(m_renameBuffer), ImGuiInputTextFlags_EnterReturnsTrue)) {
            // Enter pressed - confirm rename
            if (strlen(m_renameBuffer) > 0 && m_selectedScene) {
                renameScene(*m_selectedScene, std::string(m_renameBuffer));
            }
            m_showRenameDialog = false;
        }
        
        ImGui::Separator();
        
        if (ImGui::Button("Rename")) {
            if (strlen(m_renameBuffer) > 0 && m_selectedScene) {
                renameScene(*m_selectedScene, std::string(m_renameBuffer));
            }
            m_showRenameDialog = false;
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel")) {
            m_showRenameDialog = false;
        }
        
        ImGui::EndPopup();
    }
}

void SceneManager::createNewScene() {
    m_showCreateDialog = true;
}

void SceneManager::createUntitledScene() {
    // Generate unique name for untitled scene
    static int untitledCounter = 1;
    std::string sceneName = "Untitled Scene " + std::to_string(untitledCounter++);
    
    // Create new scene without filepath (empty string)
    auto newScene = std::make_shared<Scene>();
    newScene->initialize();
    
    // Add to list with empty filepath - this makes it an unsaved scene
    addSceneToList(newScene, sceneName, "");
    
    // Open in editor
    m_editor->openSceneInNewWindow(newScene, sceneName);
    
    std::cout << "Created untitled scene: " << sceneName << std::endl;
}

void SceneManager::createInitialScene() {
    std::string sceneName = "Initial Scene";
    std::string sceneFile = m_scenesDirectory + "/" + sceneName + ".scene";
    
    // Create new scene
    auto newScene = std::make_shared<Scene>();
    newScene->initialize();
    
    // Add to list
    addSceneToList(newScene, sceneName, sceneFile);
    
    // Save the initial scene to disk
    auto* sceneInfo = findSceneByPath(sceneFile);
    if (sceneInfo) {
        saveScene(*sceneInfo);
    }
}

void SceneManager::loadScene(const std::string& filepath) {
    // Find scene in list
    auto* sceneInfo = findSceneByPath(filepath);
    if (!sceneInfo) return;
    
    if (sceneInfo->isLoaded) return; // Already loaded
    
    // Try to load scene from JSON file
    std::ifstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "Failed to open scene file: " << filepath << std::endl;
        // Fall back to creating new scene
        auto scene = std::make_shared<Scene>();
        scene->initialize();
        sceneInfo->scene = scene;
        sceneInfo->isLoaded = true;
        sceneInfo->isDirty = true; // Mark as dirty since it's a new scene
        return;
    }
    
    try {        json sceneData;
        file >> sceneData;
        file.close();
        
        // Create new scene
        auto scene = std::make_shared<Scene>();
        scene->initialize();
          // Load scene data from JSON
        if (loadSceneFromJson(scene, sceneData.dump())) {
            sceneInfo->scene = scene;
            sceneInfo->isLoaded = true;
            sceneInfo->isDirty = false;
            
            // Don't automatically open scene window - let user open manually via Scene Manager
            std::cout << "Scene loaded successfully: " << filepath << std::endl;
        } else {
            std::cerr << "Failed to parse scene data from: " << filepath << std::endl;
            // Fall back to new scene
            sceneInfo->scene = scene;
            sceneInfo->isLoaded = true;
            sceneInfo->isDirty = true;
        }
    } catch (const std::exception& e) {
        std::cerr << "Error loading scene from " << filepath << ": " << e.what() << std::endl;
        // Fall back to creating new scene
        auto scene = std::make_shared<Scene>();
        scene->initialize();
        sceneInfo->scene = scene;
        sceneInfo->isLoaded = true;
        sceneInfo->isDirty = true;
    }
}

void SceneManager::saveScene(SceneInfo& sceneInfo) {
    if (!sceneInfo.scene) {
        std::cerr << "Cannot save scene: scene is null" << std::endl;
        return;
    }
    
    if (sceneInfo.filepath.empty()) {
        std::cerr << "Cannot save scene: filepath is empty" << std::endl;
        return;
    }
    
    std::cout << "Saving scene: " << sceneInfo.name << " to " << sceneInfo.filepath << std::endl;
      try {
        // Create JSON object from scene
        std::string jsonData;
        if (saveSceneToJson(sceneInfo.scene, jsonData)) {
            // Ensure directory exists
            fs::path filepath(sceneInfo.filepath);
            if (filepath.has_parent_path()) {
                fs::create_directories(filepath.parent_path());
            }
              // Write to file
            std::ofstream file(sceneInfo.filepath);
            if (file.is_open()) {
                file << jsonData;
                file.close();
                
                sceneInfo.isDirty = false;
                sceneInfo.lastModified = fs::last_write_time(filepath).time_since_epoch().count();
                
                std::cout << "Scene saved successfully: " << sceneInfo.filepath << std::endl;
            } else {
                std::cerr << "Failed to open file for writing: " << sceneInfo.filepath << std::endl;
            }
        } else {
            std::cerr << "Failed to serialize scene data for: " << sceneInfo.name << std::endl;
        }
    } catch (const std::exception& e) {
        std::cerr << "Error saving scene to " << sceneInfo.filepath << ": " << e.what() << std::endl;
    }
}

void SceneManager::saveSceneAs(SceneInfo& sceneInfo, const std::string& newPath) {
    sceneInfo.filepath = newPath;
    sceneInfo.name = fs::path(newPath).stem().string();
    saveScene(sceneInfo);
}

void SceneManager::closeScene(SceneInfo& sceneInfo) {
    // Check if scene has unsaved changes
    if (sceneInfo.isDirty) {
        // TODO: Show save dialog
    }
    
    sceneInfo.scene.reset();
    sceneInfo.isLoaded = false;
}

void SceneManager::renameScene(SceneInfo& sceneInfo, const std::string& newName) {
    if (newName.empty() || newName == sceneInfo.name) return;
    
    std::string oldName = sceneInfo.name;
    sceneInfo.name = newName;
    
    // Mark as dirty since the name has changed
    sceneInfo.isDirty = true;
    
    // If the scene has an associated file path, update it to match the new name
    if (!sceneInfo.filepath.empty()) {
        std::string directory = sceneInfo.filepath.substr(0, sceneInfo.filepath.find_last_of("/\\"));
        std::string extension = ".scene";
        sceneInfo.filepath = directory + "/" + newName + extension;
    }
    
    // Update any open scene windows that reference this scene
    if (m_editor && sceneInfo.scene) {
        auto& sceneWindows = m_editor->getSceneWindows();
        for (auto& window : sceneWindows) {
            if (window->getScene() == sceneInfo.scene) {
                window->setTitle(newName);
                window->setDirty(true);
            }
        }
    }
    
    std::cout << "Renamed scene from '" << oldName << "' to '" << newName << "'" << std::endl;
}

void SceneManager::duplicateScene(const SceneInfo& sceneInfo) {
    if (!sceneInfo.scene) return;
    
    // Create duplicate
    auto duplicateScene = std::make_shared<Scene>();
    duplicateScene->initialize();
    // TODO: Copy scene data
    
    std::string duplicateName = sceneInfo.name + " Copy";
    std::string duplicatePath = m_scenesDirectory + "/" + duplicateName + ".scene";
    
    addSceneToList(duplicateScene, duplicateName, duplicatePath);
}

void SceneManager::addSceneToList(std::shared_ptr<Scene> scene, const std::string& name, const std::string& filepath) {
    SceneInfo newScene(name, filepath);
    newScene.scene = scene;
    newScene.isLoaded = true;
    newScene.isDirty = filepath.empty(); // New scenes are dirty until saved
    
    m_scenes.push_back(newScene);
}

void SceneManager::removeSceneFromList(const std::string& filepath) {
    m_scenes.erase(
        std::remove_if(m_scenes.begin(), m_scenes.end(),
            [&filepath](const SceneInfo& scene) {
                return scene.filepath == filepath;
            }),
        m_scenes.end()
    );
}

void SceneManager::markSceneDirty(const std::string& filepath) {
    auto* sceneInfo = findSceneByPath(filepath);
    if (sceneInfo) {
        sceneInfo->isDirty = true;
    }
}

void SceneManager::createSceneFolder(const std::string& folderName) {
    std::string folderPath = m_scenesDirectory + "/" + folderName;
    fs::create_directories(folderPath);
    refresh();
}

void SceneManager::moveSceneToFolder(SceneInfo& sceneInfo, const std::string& folderPath) {
    if (sceneInfo.filepath.empty()) return;
    
    std::string filename = fs::path(sceneInfo.filepath).filename().string();
    std::string newPath = folderPath + "/" + filename;
    
    if (fs::exists(sceneInfo.filepath)) {
        fs::rename(sceneInfo.filepath, newPath);
        sceneInfo.filepath = newPath;
    }
}

void SceneManager::refresh() {
    scanScenesDirectory();
}

void SceneManager::scanScenesDirectory() {
    if (!fs::exists(m_scenesDirectory)) return;
    
    // Keep existing loaded scenes
    std::vector<SceneInfo> existingScenes = m_scenes;
    m_scenes.clear();
    
    try {
        for (const auto& entry : fs::recursive_directory_iterator(m_scenesDirectory)) {
            if (entry.is_regular_file() && isValidSceneFile(entry.path().string())) {
                std::string filepath = entry.path().string();
                std::string name = entry.path().stem().string();
                
                // Check if this scene was already loaded
                auto existingIt = std::find_if(existingScenes.begin(), existingScenes.end(),
                    [&filepath](const SceneInfo& scene) {
                        return scene.filepath == filepath;
                    });
                  if (existingIt != existingScenes.end()) {
                    // Keep existing scene info (preserves loaded state and scene object)
                    std::cout << "DEBUG: Preserving existing scene '" << name << "' - isLoaded: " << existingIt->isLoaded << std::endl;
                    m_scenes.push_back(*existingIt);                } else {
                    // Add new scene entry (unloaded by default)
                    SceneInfo newScene(name, filepath);
                    newScene.lastModified = fs::last_write_time(entry).time_since_epoch().count();
                    std::cout << "DEBUG: Found new scene file '" << name << "' - adding as unloaded" << std::endl;
                    m_scenes.push_back(newScene);
                }
            }
        }
    } catch (const fs::filesystem_error& ex) {
        std::cerr << "Error scanning scenes directory: " << ex.what() << std::endl;
    }
}

std::string SceneManager::getRelativePath(const std::string& fullPath) {
    fs::path full(fullPath);
    fs::path base(m_scenesDirectory);
    fs::path relative = fs::relative(full, base);
    return relative.string();
}

std::string SceneManager::getScenesDirectory() {
    return "./assets/scenes"; // Default scenes directory
}

bool SceneManager::isValidSceneFile(const std::string& filepath) {
    std::string extension = fs::path(filepath).extension().string();
    return extension == ".scene" || extension == ".json";
}

SceneInfo* SceneManager::findSceneByPath(const std::string& filepath) {
    auto it = std::find_if(m_scenes.begin(), m_scenes.end(),
        [&filepath](const SceneInfo& scene) {
            return scene.filepath == filepath;
        });
    
    return (it != m_scenes.end()) ? &(*it) : nullptr;
}

void SceneManager::drawSceneIcon(const SceneInfo& sceneInfo) {
    const char* icon = "📄"; // Default scene icon
    
    if (sceneInfo.isLoaded) {
        icon = sceneInfo.isDirty ? "📝" : "📋"; // Loaded/dirty vs loaded/clean
    }
    
    ImGui::Text("%s", icon);
}

ImVec4 SceneManager::getSceneStatusColor(const SceneInfo& sceneInfo) {
    if (!sceneInfo.isLoaded) {
        return ImVec4(0.6f, 0.6f, 0.6f, 1.0f); // Gray for unloaded
    } else if (sceneInfo.isDirty) {
        return ImVec4(1.0f, 0.8f, 0.2f, 1.0f); // Yellow for dirty
    } else {
        return ImVec4(0.2f, 1.0f, 0.2f, 1.0f); // Green for clean
    }
}

const char* SceneManager::getSceneStatusText(const SceneInfo& sceneInfo) {
    if (!sceneInfo.isLoaded) {
        return "unloaded";
    } else if (sceneInfo.isDirty) {
        return "modified";
    } else {
        return "loaded";
    }
}

// Scene serialization implementation
bool SceneManager::loadSceneFromJson(std::shared_ptr<Scene> scene, const std::string& jsonData) {
    if (!scene) return false;
    
    try {
        json sceneJsonData = json::parse(jsonData);        // Load scene metadata
        if (sceneJsonData.contains("metadata")) {
            auto metadata = sceneJsonData["metadata"];
            // Could load scene name, version, etc. here
        }
        
        // Load entities
        if (sceneJsonData.contains("entities")) {
            auto entities = sceneJsonData["entities"];
            
            for (const auto& entityData : entities) {
                if (!entityData.contains("id")) continue;
                
                EntityID entityId = scene->createEntity();
                
                // Load entity name
                if (entityData.contains("name")) {
                    scene->setEntityName(entityId, entityData["name"]);
                }
                
                // Load components
                if (entityData.contains("components")) {
                    auto components = entityData["components"];
                    
                    // Load Transform component
                    if (components.contains("Transform")) {
                        auto transformData = components["Transform"];
                        Transform transform;
                        if (transformData.contains("x")) transform.position.x = transformData["x"];
                        if (transformData.contains("y")) transform.position.y = transformData["y"];
                        if (transformData.contains("rotation")) transform.rotation = transformData["rotation"];
                        if (transformData.contains("scaleX")) transform.scale.x = transformData["scaleX"];
                        if (transformData.contains("scaleY")) transform.scale.y = transformData["scaleY"];
                        scene->addComponent<Transform>(entityId, transform);
                    }
                    
                    // Load Sprite component
                    if (components.contains("Sprite")) {
                        auto spriteData = components["Sprite"];
                        Sprite sprite;
                        // For now, just create empty sprite - would need to load texture path
                        scene->addComponent<Sprite>(entityId, sprite);
                    }
                    
                    // Could add other components here (Rigidbody, Collider, etc.)
                }            }
        }
        
        // Load procedural map data if present
        if (sceneJsonData.contains("proceduralMap")) {
            auto mapData = sceneJsonData["proceduralMap"];
            
            if (mapData.contains("width") && mapData.contains("height")) {
                int width = mapData["width"];
                int height = mapData["height"];
                
                // Create procedural map
                auto proceduralMap = std::make_shared<ProceduralMap>(width, height);
                
                // Load tiles
                if (mapData.contains("tiles")) {
                    auto tilesData = mapData["tiles"];
                    for (const auto& tileData : tilesData) {
                        if (tileData.contains("x") && tileData.contains("y") && tileData.contains("type")) {
                            int x = tileData["x"];
                            int y = tileData["y"];
                            int typeInt = tileData["type"];
                            
                            if (proceduralMap->isValidPosition(x, y)) {
                                auto& tile = proceduralMap->getTile(x, y);
                                tile.type = static_cast<TileType>(typeInt);
                                
                                if (tileData.contains("sprite")) {
                                    tile.spriteName = tileData["sprite"];
                                }
                            }
                        }
                    }
                }
                
                // Create and set sprite manager if needed
                if (mapData.contains("hasSpriteManager") && mapData["hasSpriteManager"]) {
                    // Create a basic sprite manager - in a real implementation,
                    // you'd want to save/restore the theme and settings
                    auto spriteManager = std::make_shared<TileSpriteManager>();
                    // Try to infer theme from tile types or default to Dungeon
                    spriteManager->setTheme(GenerationTheme::Dungeon);
                    proceduralMap->setSpriteManager(spriteManager);
                }
                
                // Set the procedural map on the scene
                scene->setProceduralMap(proceduralMap);
                
                std::cout << "Loaded procedural map: " << width << "x" << height 
                         << " with " << mapData["tiles"].size() << " tiles" << std::endl;
            }
        }
        
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error parsing scene JSON: " << e.what() << std::endl;
        return false;
    }
}

bool SceneManager::saveSceneToJson(std::shared_ptr<Scene> scene, std::string& jsonData) {
    if (!scene) return false;
    
    try {
        // Save scene metadata
        json sceneJsonData;
        sceneJsonData["metadata"] = {
            {"version", "1.0"},
            {"created", std::time(nullptr)},
            {"engine", "2D Game Engine"}
        };
        
        // Save entities
        json entitiesArray = json::array();
        auto allEntities = scene->getAllLivingEntities();
        
        for (EntityID entityId : allEntities) {
            json entityData;
            entityData["id"] = entityId;
            
            // Save entity name
            std::string entityName = scene->getEntityName(entityId);
            if (!entityName.empty()) {
                entityData["name"] = entityName;
            }
            
            // Save components
            json componentsData;
            
            // Save Transform component
            if (scene->hasComponent<Transform>(entityId)) {
                const Transform& transform = scene->getComponent<Transform>(entityId);
                componentsData["Transform"] = {
                    {"x", transform.position.x},
                    {"y", transform.position.y},
                    {"rotation", transform.rotation},
                    {"scaleX", transform.scale.x},
                    {"scaleY", transform.scale.y}
                };
            }
            
            // Save Sprite component
            if (scene->hasComponent<Sprite>(entityId)) {
                // For now, just mark that sprite exists
                // Would need to save texture path and other sprite data
                componentsData["Sprite"] = {
                    {"hasSprite", true}
                };
            }
            
            // Could add other components here
            
            if (!componentsData.empty()) {
                entityData["components"] = componentsData;
            }
            
            entitiesArray.push_back(entityData);        }
        
        sceneJsonData["entities"] = entitiesArray;
        
        // Save procedural map data if present
        if (scene->hasProceduralMap()) {
            auto proceduralMap = scene->getProceduralMap();
            json proceduralMapData;
            
            proceduralMapData["width"] = proceduralMap->getWidth();
            proceduralMapData["height"] = proceduralMap->getHeight();
            
            // Save tile data
            json tilesArray = json::array();
            for (int y = 0; y < proceduralMap->getHeight(); ++y) {
                for (int x = 0; x < proceduralMap->getWidth(); ++x) {
                    const auto& tile = proceduralMap->getTile(x, y);
                    if (tile.type != TileType::Empty) {
                        json tileData;
                        tileData["x"] = x;
                        tileData["y"] = y;
                        tileData["type"] = static_cast<int>(tile.type);
                        if (!tile.spriteName.empty()) {
                            tileData["sprite"] = tile.spriteName;
                        }
                        tilesArray.push_back(tileData);
                    }
                }
            }
            proceduralMapData["tiles"] = tilesArray;
            
            // Save sprite manager theme if available
            if (proceduralMap->getSpriteManager()) {
                // Note: Would need to add getTheme() method to TileSpriteManager
                // For now, we'll try to infer or just save that it exists
                proceduralMapData["hasSpriteManager"] = true;
            }
            
            sceneJsonData["proceduralMap"] = proceduralMapData;
        }
        
        // Convert to string
        jsonData = sceneJsonData.dump(2); // Pretty print with 2 spaces
        return true;
        
    } catch (const std::exception& e) {
        std::cerr << "Error creating scene JSON: " << e.what() << std::endl;
        return false;
    }
}
