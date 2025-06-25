#include "SceneManager.h"
#include "GameEditor.h"
#include "../scene/Scene.h"
#include "../components/Components.h"
#include "../generation/ProceduralGeneration.h"
#include "../core/Engine.h"
#include "../utils/ResourceManager.h"
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
    
    // Count entities and components for user feedback
    auto entities = sceneInfo.scene->getAllLivingEntities();
    int componentCount = 0;
    for (EntityID entity : entities) {
        if (sceneInfo.scene->hasComponent<Transform>(entity)) componentCount++;
        if (sceneInfo.scene->hasComponent<Sprite>(entity)) componentCount++;
        if (sceneInfo.scene->hasComponent<Collider>(entity)) componentCount++;
        if (sceneInfo.scene->hasComponent<RigidBody>(entity)) componentCount++;
        if (sceneInfo.scene->hasComponent<PlayerController>(entity)) componentCount++;
        if (sceneInfo.scene->hasComponent<PlayerStats>(entity)) componentCount++;
        if (sceneInfo.scene->hasComponent<PlayerPhysics>(entity)) componentCount++;
        if (sceneInfo.scene->hasComponent<PlayerInventory>(entity)) componentCount++;
        if (sceneInfo.scene->hasComponent<PlayerAbilities>(entity)) componentCount++;
        if (sceneInfo.scene->hasComponent<PlayerState>(entity)) componentCount++;
    }
    std::cout << "Saving " << entities.size() << " entities with " << componentCount << " total components" << std::endl;
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
        
        int entitiesLoaded = 0;
        int componentsLoaded = 0;
        
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
                
                entitiesLoaded++;
                
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
                        componentsLoaded++;
                        componentsLoaded++;
                    }
                      // Load Sprite component
                    if (components.contains("Sprite")) {
                        auto spriteData = components["Sprite"];
                        Sprite sprite;
                        if (spriteData.contains("visible")) sprite.visible = spriteData["visible"];
                        if (spriteData.contains("layer")) sprite.layer = spriteData["layer"];
                        if (spriteData.contains("tintR")) sprite.tint.r = spriteData["tintR"];
                        if (spriteData.contains("tintG")) sprite.tint.g = spriteData["tintG"];
                        if (spriteData.contains("tintB")) sprite.tint.b = spriteData["tintB"];
                        if (spriteData.contains("tintA")) sprite.tint.a = spriteData["tintA"];
                        
                        if (spriteData.contains("sourceRectX") && spriteData.contains("sourceRectY") &&
                            spriteData.contains("sourceRectW") && spriteData.contains("sourceRectH")) {
                            sprite.sourceRect = Rect(spriteData["sourceRectX"], spriteData["sourceRectY"],
                                                   spriteData["sourceRectW"], spriteData["sourceRectH"]);
                        }
                        
                        // Load texture from saved path
                        if (spriteData.contains("texturePath")) {
                            std::string texturePath = spriteData["texturePath"];
                            if (!texturePath.empty()) {
                                auto& engine = Engine::getInstance();
                                auto resourceManager = engine.getResourceManager();
                                if (resourceManager) {
                                    auto texture = resourceManager->loadTexture(texturePath);
                                    if (texture) {
                                        sprite.texture = texture;
                                        std::cout << "Restored texture: " << texturePath << " for entity " << entityId << std::endl;
                                    } else {
                                        std::cerr << "Failed to load texture: " << texturePath << " for entity " << entityId << std::endl;
                                    }
                                }
                            }
                        }
                        
                        scene->addComponent<Sprite>(entityId, sprite);
                        componentsLoaded++;
                    }
                    
                    // Load Collider component
                    if (components.contains("Collider")) {
                        auto colliderData = components["Collider"];
                        Collider collider;
                        if (colliderData.contains("offsetX")) collider.offset.x = colliderData["offsetX"];
                        if (colliderData.contains("offsetY")) collider.offset.y = colliderData["offsetY"];
                        if (colliderData.contains("sizeX")) collider.size.x = colliderData["sizeX"];
                        if (colliderData.contains("sizeY")) collider.size.y = colliderData["sizeY"];
                        if (colliderData.contains("isTrigger")) collider.isTrigger = colliderData["isTrigger"];
                        if (colliderData.contains("isStatic")) collider.isStatic = colliderData["isStatic"];
                        scene->addComponent<Collider>(entityId, collider);
                        componentsLoaded++;
                    }
                    
                    // Load RigidBody component
                    if (components.contains("RigidBody")) {
                        auto rigidBodyData = components["RigidBody"];
                        RigidBody rigidBody;
                        if (rigidBodyData.contains("velocityX")) rigidBody.velocity.x = rigidBodyData["velocityX"];
                        if (rigidBodyData.contains("velocityY")) rigidBody.velocity.y = rigidBodyData["velocityY"];
                        if (rigidBodyData.contains("accelerationX")) rigidBody.acceleration.x = rigidBodyData["accelerationX"];
                        if (rigidBodyData.contains("accelerationY")) rigidBody.acceleration.y = rigidBodyData["accelerationY"];
                        if (rigidBodyData.contains("drag")) rigidBody.drag = rigidBodyData["drag"];
                        if (rigidBodyData.contains("mass")) rigidBody.mass = rigidBodyData["mass"];
                        if (rigidBodyData.contains("useGravity")) rigidBody.useGravity = rigidBodyData["useGravity"];
                        scene->addComponent<RigidBody>(entityId, rigidBody);
                        componentsLoaded++;
                    }
                    
                    // Load PlayerController component
                    if (components.contains("PlayerController")) {
                        auto controllerData = components["PlayerController"];
                        PlayerController controller;
                        if (controllerData.contains("controlScheme")) 
                            controller.controlScheme = static_cast<PlayerController::ControlScheme>(controllerData["controlScheme"]);
                        if (controllerData.contains("movementType")) 
                            controller.movementType = static_cast<PlayerController::MovementType>(controllerData["movementType"]);
                        if (controllerData.contains("moveSpeed")) controller.moveSpeed = controllerData["moveSpeed"];
                        if (controllerData.contains("runSpeedMultiplier")) controller.runSpeedMultiplier = controllerData["runSpeedMultiplier"];
                        if (controllerData.contains("acceleration")) controller.acceleration = controllerData["acceleration"];
                        if (controllerData.contains("deceleration")) controller.deceleration = controllerData["deceleration"];
                        if (controllerData.contains("jumpForce")) controller.jumpForce = controllerData["jumpForce"];
                        if (controllerData.contains("canDoubleJump")) controller.canDoubleJump = controllerData["canDoubleJump"];
                        if (controllerData.contains("maxJumps")) controller.maxJumps = controllerData["maxJumps"];
                        if (controllerData.contains("jumpsRemaining")) controller.jumpsRemaining = controllerData["jumpsRemaining"];
                        if (controllerData.contains("inputDirectionX")) controller.inputDirection.x = controllerData["inputDirectionX"];
                        if (controllerData.contains("inputDirectionY")) controller.inputDirection.y = controllerData["inputDirectionY"];
                        if (controllerData.contains("moveDirectionX")) controller.moveDirection.x = controllerData["moveDirectionX"];
                        if (controllerData.contains("moveDirectionY")) controller.moveDirection.y = controllerData["moveDirectionY"];
                        if (controllerData.contains("isRunning")) controller.isRunning = controllerData["isRunning"];
                        if (controllerData.contains("isGrounded")) controller.isGrounded = controllerData["isGrounded"];
                        if (controllerData.contains("jumpPressed")) controller.jumpPressed = controllerData["jumpPressed"];
                        if (controllerData.contains("jumpHeld")) controller.jumpHeld = controllerData["jumpHeld"];
                        scene->addComponent<PlayerController>(entityId, controller);
                        componentsLoaded++;
                    }
                    
                    // Load PlayerStats component
                    if (components.contains("PlayerStats")) {
                        auto statsData = components["PlayerStats"];
                        PlayerStats stats;
                        
                        if (statsData.contains("core")) {
                            auto coreData = statsData["core"];
                            if (coreData.contains("level")) stats.core.level = coreData["level"];
                            if (coreData.contains("experience")) stats.core.experience = coreData["experience"];
                            if (coreData.contains("experienceToNext")) stats.core.experienceToNext = coreData["experienceToNext"];
                            if (coreData.contains("strength")) stats.core.strength = coreData["strength"];
                            if (coreData.contains("dexterity")) stats.core.dexterity = coreData["dexterity"];
                            if (coreData.contains("intelligence")) stats.core.intelligence = coreData["intelligence"];
                            if (coreData.contains("vitality")) stats.core.vitality = coreData["vitality"];
                            if (coreData.contains("luck")) stats.core.luck = coreData["luck"];
                        }
                        
                        if (statsData.contains("derived")) {
                            auto derivedData = statsData["derived"];
                            if (derivedData.contains("maxHealth")) stats.derived.maxHealth = derivedData["maxHealth"];
                            if (derivedData.contains("currentHealth")) stats.derived.currentHealth = derivedData["currentHealth"];
                            if (derivedData.contains("maxMana")) stats.derived.maxMana = derivedData["maxMana"];
                            if (derivedData.contains("currentMana")) stats.derived.currentMana = derivedData["currentMana"];
                            if (derivedData.contains("maxStamina")) stats.derived.maxStamina = derivedData["maxStamina"];
                            if (derivedData.contains("currentStamina")) stats.derived.currentStamina = derivedData["currentStamina"];
                            if (derivedData.contains("physicalDamage")) stats.derived.physicalDamage = derivedData["physicalDamage"];
                            if (derivedData.contains("magicalDamage")) stats.derived.magicalDamage = derivedData["magicalDamage"];
                            if (derivedData.contains("defense")) stats.derived.defense = derivedData["defense"];
                            if (derivedData.contains("magicResistance")) stats.derived.magicResistance = derivedData["magicResistance"];
                            if (derivedData.contains("criticalChance")) stats.derived.criticalChance = derivedData["criticalChance"];
                            if (derivedData.contains("criticalMultiplier")) stats.derived.criticalMultiplier = derivedData["criticalMultiplier"];
                            if (derivedData.contains("moveSpeedModifier")) stats.derived.moveSpeedModifier = derivedData["moveSpeedModifier"];
                            if (derivedData.contains("attackSpeedModifier")) stats.derived.attackSpeedModifier = derivedData["attackSpeedModifier"];
                        }
                        
                        if (statsData.contains("status")) {
                            auto statusData = statsData["status"];
                            if (statusData.contains("poisoned")) stats.status.poisoned = statusData["poisoned"];
                            if (statusData.contains("burning")) stats.status.burning = statusData["burning"];
                            if (statusData.contains("frozen")) stats.status.frozen = statusData["frozen"];
                            if (statusData.contains("stunned")) stats.status.stunned = statusData["stunned"];
                            if (statusData.contains("invulnerable")) stats.status.invulnerable = statusData["invulnerable"];
                            if (statusData.contains("poisonDuration")) stats.status.poisonDuration = statusData["poisonDuration"];
                            if (statusData.contains("burnDuration")) stats.status.burnDuration = statusData["burnDuration"];
                            if (statusData.contains("freezeDuration")) stats.status.freezeDuration = statusData["freezeDuration"];
                            if (statusData.contains("stunDuration")) stats.status.stunDuration = statusData["stunDuration"];
                            if (statusData.contains("invulnerabilityDuration")) stats.status.invulnerabilityDuration = statusData["invulnerabilityDuration"];
                        }
                        
                        scene->addComponent<PlayerStats>(entityId, stats);
                        componentsLoaded++;
                    }
                    
                    // Load PlayerPhysics component
                    if (components.contains("PlayerPhysics")) {
                        auto physicsData = components["PlayerPhysics"];
                        PlayerPhysics physics;
                        if (physicsData.contains("velocityX")) physics.velocity.x = physicsData["velocityX"];
                        if (physicsData.contains("velocityY")) physics.velocity.y = physicsData["velocityY"];
                        if (physicsData.contains("accelerationX")) physics.acceleration.x = physicsData["accelerationX"];
                        if (physicsData.contains("accelerationY")) physics.acceleration.y = physicsData["accelerationY"];
                        if (physicsData.contains("externalForcesX")) physics.externalForces.x = physicsData["externalForcesX"];
                        if (physicsData.contains("externalForcesY")) physics.externalForces.y = physicsData["externalForcesY"];
                        if (physicsData.contains("maxSpeed")) physics.maxSpeed = physicsData["maxSpeed"];
                        if (physicsData.contains("friction")) physics.friction = physicsData["friction"];
                        if (physicsData.contains("airResistance")) physics.airResistance = physicsData["airResistance"];
                        if (physicsData.contains("mass")) physics.mass = physicsData["mass"];
                        if (physicsData.contains("isGrounded")) physics.isGrounded = physicsData["isGrounded"];
                        if (physicsData.contains("isOnSlope")) physics.isOnSlope = physicsData["isOnSlope"];
                        if (physicsData.contains("slopeAngle")) physics.slopeAngle = physicsData["slopeAngle"];
                        if (physicsData.contains("groundNormalX")) physics.groundNormal.x = physicsData["groundNormalX"];
                        if (physicsData.contains("groundNormalY")) physics.groundNormal.y = physicsData["groundNormalY"];
                        if (physicsData.contains("canJump")) physics.canJump = physicsData["canJump"];
                        if (physicsData.contains("jumpCooldown")) physics.jumpCooldown = physicsData["jumpCooldown"];
                        if (physicsData.contains("coyoteTime")) physics.coyoteTime = physicsData["coyoteTime"];
                        if (physicsData.contains("jumpBufferTime")) physics.jumpBufferTime = physicsData["jumpBufferTime"];
                        if (physicsData.contains("coyoteTimer")) physics.coyoteTimer = physicsData["coyoteTimer"];
                        if (physicsData.contains("jumpBufferTimer")) physics.jumpBufferTimer = physicsData["jumpBufferTimer"];
                        if (physicsData.contains("canWallJump")) physics.canWallJump = physicsData["canWallJump"];
                        if (physicsData.contains("isTouchingWall")) physics.isTouchingWall = physicsData["isTouchingWall"];
                        if (physicsData.contains("wallNormalX")) physics.wallNormal.x = physicsData["wallNormalX"];
                        if (physicsData.contains("wallNormalY")) physics.wallNormal.y = physicsData["wallNormalY"];
                        if (physicsData.contains("canDash")) physics.canDash = physicsData["canDash"];
                        if (physicsData.contains("isDashing")) physics.isDashing = physicsData["isDashing"];
                        if (physicsData.contains("dashForce")) physics.dashForce = physicsData["dashForce"];
                        if (physicsData.contains("dashDuration")) physics.dashDuration = physicsData["dashDuration"];
                        if (physicsData.contains("dashCooldown")) physics.dashCooldown = physicsData["dashCooldown"];
                        if (physicsData.contains("dashTimer")) physics.dashTimer = physicsData["dashTimer"];
                        if (physicsData.contains("dashCooldownTimer")) physics.dashCooldownTimer = physicsData["dashCooldownTimer"];
                        scene->addComponent<PlayerPhysics>(entityId, physics);
                        componentsLoaded++;
                    }
                    
                    // Load PlayerInventory component
                    if (components.contains("PlayerInventory")) {
                        auto inventoryData = components["PlayerInventory"];
                        PlayerInventory inventory;
                        if (inventoryData.contains("selectedHotbarSlot")) inventory.selectedHotbarSlot = inventoryData["selectedHotbarSlot"];
                        if (inventoryData.contains("currency")) inventory.currency = inventoryData["currency"];
                        
                        // Load inventory items
                        if (inventoryData.contains("items")) {
                            for (const auto& itemData : inventoryData["items"]) {
                                if (itemData.contains("slot")) {
                                    int slot = itemData["slot"];
                                    if (slot >= 0 && slot < PlayerInventory::MAX_INVENTORY_SLOTS) {
                                        auto item = std::make_shared<PlayerInventory::Item>();
                                        if (itemData.contains("id")) item->id = itemData["id"];
                                        if (itemData.contains("name")) item->name = itemData["name"];
                                        if (itemData.contains("description")) item->description = itemData["description"];
                                        if (itemData.contains("quantity")) item->quantity = itemData["quantity"];
                                        if (itemData.contains("maxStack")) item->maxStack = itemData["maxStack"];
                                        if (itemData.contains("consumable")) item->consumable = itemData["consumable"];
                                        if (itemData.contains("healthRestore")) item->healthRestore = itemData["healthRestore"];
                                        if (itemData.contains("manaRestore")) item->manaRestore = itemData["manaRestore"];
                                        if (itemData.contains("staminaRestore")) item->staminaRestore = itemData["staminaRestore"];
                                        if (itemData.contains("damageBonus")) item->damageBonus = itemData["damageBonus"];
                                        if (itemData.contains("defenseBonus")) item->defenseBonus = itemData["defenseBonus"];
                                        inventory.items[slot] = item;
                                    }
                                }
                            }
                        }
                        
                        // Load hotbar items
                        if (inventoryData.contains("hotbar")) {
                            for (const auto& itemData : inventoryData["hotbar"]) {
                                if (itemData.contains("slot")) {
                                    int slot = itemData["slot"];
                                    if (slot >= 0 && slot < PlayerInventory::HOTBAR_SLOTS) {
                                        auto item = std::make_shared<PlayerInventory::Item>();
                                        if (itemData.contains("id")) item->id = itemData["id"];
                                        if (itemData.contains("name")) item->name = itemData["name"];
                                        if (itemData.contains("quantity")) item->quantity = itemData["quantity"];
                                        inventory.hotbar[slot] = item;
                                    }
                                }
                            }
                        }
                        
                        scene->addComponent<PlayerInventory>(entityId, inventory);
                        componentsLoaded++;
                    }
                    
                    // Load PlayerAbilities component
                    if (components.contains("PlayerAbilities")) {
                        auto abilitiesData = components["PlayerAbilities"];
                        PlayerAbilities abilities;
                        if (abilitiesData.contains("skillPoints")) abilities.skillPoints = abilitiesData["skillPoints"];
                        
                        // Load abilities
                        if (abilitiesData.contains("abilities")) {
                            for (const auto& abilityData : abilitiesData["abilities"]) {
                                PlayerAbilities::Ability ability;
                                if (abilityData.contains("name")) ability.name = abilityData["name"];
                                if (abilityData.contains("description")) ability.description = abilityData["description"];
                                if (abilityData.contains("level")) ability.level = abilityData["level"];
                                if (abilityData.contains("maxLevel")) ability.maxLevel = abilityData["maxLevel"];
                                if (abilityData.contains("cooldown")) ability.cooldown = abilityData["cooldown"];
                                if (abilityData.contains("currentCooldown")) ability.currentCooldown = abilityData["currentCooldown"];
                                if (abilityData.contains("manaCost")) ability.manaCost = abilityData["manaCost"];
                                if (abilityData.contains("staminaCost")) ability.staminaCost = abilityData["staminaCost"];
                                if (abilityData.contains("unlocked")) ability.unlocked = abilityData["unlocked"];
                                if (abilityData.contains("passive")) ability.passive = abilityData["passive"];
                                if (abilityData.contains("damage")) ability.damage = abilityData["damage"];
                                if (abilityData.contains("range")) ability.range = abilityData["range"];
                                if (abilityData.contains("duration")) ability.duration = abilityData["duration"];
                                abilities.abilities.push_back(ability);
                            }
                        }
                        
                        // Load hotbar abilities
                        if (abilitiesData.contains("hotbarAbilities")) {
                            auto hotbarArray = abilitiesData["hotbarAbilities"];
                            for (int i = 0; i < 4 && i < hotbarArray.size(); ++i) {
                                abilities.hotbarAbilities[i] = hotbarArray[i];
                            }
                        }
                        
                        scene->addComponent<PlayerAbilities>(entityId, abilities);
                        componentsLoaded++;
                    }
                    
                    // Load PlayerState component
                    if (components.contains("PlayerState")) {
                        auto stateData = components["PlayerState"];
                        PlayerState state;
                        if (stateData.contains("currentState")) 
                            state.currentState = static_cast<PlayerState::State>(stateData["currentState"]);
                        if (stateData.contains("previousState")) 
                            state.previousState = static_cast<PlayerState::State>(stateData["previousState"]);
                        if (stateData.contains("stateTimer")) state.stateTimer = stateData["stateTimer"];
                        if (stateData.contains("currentFrame")) state.currentFrame = stateData["currentFrame"];
                        if (stateData.contains("frameTimer")) state.frameTimer = stateData["frameTimer"];
                        if (stateData.contains("frameRate")) state.frameRate = stateData["frameRate"];
                        if (stateData.contains("facingX")) state.facing.x = stateData["facingX"];
                        if (stateData.contains("facingY")) state.facing.y = stateData["facingY"];
                        if (stateData.contains("actionInProgress")) state.actionInProgress = stateData["actionInProgress"];
                        if (stateData.contains("actionDuration")) state.actionDuration = stateData["actionDuration"];                        scene->addComponent<PlayerState>(entityId, state);
                    }
                }
            }
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
                
                // Set the procedural map on the scene                scene->setProceduralMap(proceduralMap);
                
                std::cout << "Loaded procedural map: " << width << "x" << height 
                         << " with " << mapData["tiles"].size() << " tiles" << std::endl;
            }
        }
        
        std::cout << "Successfully loaded " << entitiesLoaded << " entities with their components from scene" << std::endl;
        
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
                const Sprite& sprite = scene->getComponent<Sprite>(entityId);
                json spriteData = {
                    {"visible", sprite.visible},
                    {"layer", sprite.layer},
                    {"tintR", sprite.tint.r},
                    {"tintG", sprite.tint.g},
                    {"tintB", sprite.tint.b},
                    {"tintA", sprite.tint.a},
                    {"sourceRectX", sprite.sourceRect.x},
                    {"sourceRectY", sprite.sourceRect.y},
                    {"sourceRectW", sprite.sourceRect.width},
                    {"sourceRectH", sprite.sourceRect.height}
                };
                
                // Save texture filepath if available
                if (sprite.texture && !sprite.texture->getFilePath().empty()) {
                    spriteData["texturePath"] = sprite.texture->getFilePath();
                }
                
                componentsData["Sprite"] = spriteData;
            }
            
            // Save Collider component
            if (scene->hasComponent<Collider>(entityId)) {
                const Collider& collider = scene->getComponent<Collider>(entityId);
                componentsData["Collider"] = {
                    {"offsetX", collider.offset.x},
                    {"offsetY", collider.offset.y},
                    {"sizeX", collider.size.x},
                    {"sizeY", collider.size.y},
                    {"isTrigger", collider.isTrigger},
                    {"isStatic", collider.isStatic}
                };
            }
            
            // Save RigidBody component
            if (scene->hasComponent<RigidBody>(entityId)) {
                const RigidBody& rigidBody = scene->getComponent<RigidBody>(entityId);
                componentsData["RigidBody"] = {
                    {"velocityX", rigidBody.velocity.x},
                    {"velocityY", rigidBody.velocity.y},
                    {"accelerationX", rigidBody.acceleration.x},
                    {"accelerationY", rigidBody.acceleration.y},
                    {"drag", rigidBody.drag},
                    {"mass", rigidBody.mass},
                    {"useGravity", rigidBody.useGravity}
                };
            }
            
            // Save PlayerController component
            if (scene->hasComponent<PlayerController>(entityId)) {
                const PlayerController& controller = scene->getComponent<PlayerController>(entityId);
                componentsData["PlayerController"] = {
                    {"controlScheme", static_cast<int>(controller.controlScheme)},
                    {"movementType", static_cast<int>(controller.movementType)},
                    {"moveSpeed", controller.moveSpeed},
                    {"runSpeedMultiplier", controller.runSpeedMultiplier},
                    {"acceleration", controller.acceleration},
                    {"deceleration", controller.deceleration},
                    {"jumpForce", controller.jumpForce},
                    {"canDoubleJump", controller.canDoubleJump},
                    {"maxJumps", controller.maxJumps},
                    {"jumpsRemaining", controller.jumpsRemaining},
                    {"inputDirectionX", controller.inputDirection.x},
                    {"inputDirectionY", controller.inputDirection.y},
                    {"moveDirectionX", controller.moveDirection.x},
                    {"moveDirectionY", controller.moveDirection.y},
                    {"isRunning", controller.isRunning},
                    {"isGrounded", controller.isGrounded},
                    {"jumpPressed", controller.jumpPressed},
                    {"jumpHeld", controller.jumpHeld}
                };
            }
            
            // Save PlayerStats component
            if (scene->hasComponent<PlayerStats>(entityId)) {
                const PlayerStats& stats = scene->getComponent<PlayerStats>(entityId);
                componentsData["PlayerStats"] = {
                    {"core", {
                        {"level", stats.core.level},
                        {"experience", stats.core.experience},
                        {"experienceToNext", stats.core.experienceToNext},
                        {"strength", stats.core.strength},
                        {"dexterity", stats.core.dexterity},
                        {"intelligence", stats.core.intelligence},
                        {"vitality", stats.core.vitality},
                        {"luck", stats.core.luck}
                    }},
                    {"derived", {
                        {"maxHealth", stats.derived.maxHealth},
                        {"currentHealth", stats.derived.currentHealth},
                        {"maxMana", stats.derived.maxMana},
                        {"currentMana", stats.derived.currentMana},
                        {"maxStamina", stats.derived.maxStamina},
                        {"currentStamina", stats.derived.currentStamina},
                        {"physicalDamage", stats.derived.physicalDamage},
                        {"magicalDamage", stats.derived.magicalDamage},
                        {"defense", stats.derived.defense},
                        {"magicResistance", stats.derived.magicResistance},
                        {"criticalChance", stats.derived.criticalChance},
                        {"criticalMultiplier", stats.derived.criticalMultiplier},
                        {"moveSpeedModifier", stats.derived.moveSpeedModifier},
                        {"attackSpeedModifier", stats.derived.attackSpeedModifier}
                    }},
                    {"status", {
                        {"poisoned", stats.status.poisoned},
                        {"burning", stats.status.burning},
                        {"frozen", stats.status.frozen},
                        {"stunned", stats.status.stunned},
                        {"invulnerable", stats.status.invulnerable},
                        {"poisonDuration", stats.status.poisonDuration},
                        {"burnDuration", stats.status.burnDuration},
                        {"freezeDuration", stats.status.freezeDuration},
                        {"stunDuration", stats.status.stunDuration},
                        {"invulnerabilityDuration", stats.status.invulnerabilityDuration}
                    }}
                };
            }
            
            // Save PlayerPhysics component
            if (scene->hasComponent<PlayerPhysics>(entityId)) {
                const PlayerPhysics& physics = scene->getComponent<PlayerPhysics>(entityId);
                componentsData["PlayerPhysics"] = {
                    {"velocityX", physics.velocity.x},
                    {"velocityY", physics.velocity.y},
                    {"accelerationX", physics.acceleration.x},
                    {"accelerationY", physics.acceleration.y},
                    {"externalForcesX", physics.externalForces.x},
                    {"externalForcesY", physics.externalForces.y},
                    {"maxSpeed", physics.maxSpeed},
                    {"friction", physics.friction},
                    {"airResistance", physics.airResistance},
                    {"mass", physics.mass},
                    {"isGrounded", physics.isGrounded},
                    {"isOnSlope", physics.isOnSlope},
                    {"slopeAngle", physics.slopeAngle},
                    {"groundNormalX", physics.groundNormal.x},
                    {"groundNormalY", physics.groundNormal.y},
                    {"canJump", physics.canJump},
                    {"jumpCooldown", physics.jumpCooldown},
                    {"coyoteTime", physics.coyoteTime},
                    {"jumpBufferTime", physics.jumpBufferTime},
                    {"coyoteTimer", physics.coyoteTimer},
                    {"jumpBufferTimer", physics.jumpBufferTimer},
                    {"canWallJump", physics.canWallJump},
                    {"isTouchingWall", physics.isTouchingWall},
                    {"wallNormalX", physics.wallNormal.x},
                    {"wallNormalY", physics.wallNormal.y},
                    {"canDash", physics.canDash},
                    {"isDashing", physics.isDashing},
                    {"dashForce", physics.dashForce},
                    {"dashDuration", physics.dashDuration},
                    {"dashCooldown", physics.dashCooldown},
                    {"dashTimer", physics.dashTimer},
                    {"dashCooldownTimer", physics.dashCooldownTimer}
                };
            }
            
            // Save PlayerInventory component
            if (scene->hasComponent<PlayerInventory>(entityId)) {
                const PlayerInventory& inventory = scene->getComponent<PlayerInventory>(entityId);
                json inventoryData = {
                    {"selectedHotbarSlot", inventory.selectedHotbarSlot},
                    {"currency", inventory.currency}
                };
                
                // Save inventory items
                json itemsArray = json::array();
                for (int i = 0; i < PlayerInventory::MAX_INVENTORY_SLOTS; ++i) {
                    if (inventory.items[i]) {
                        auto item = inventory.items[i];
                        itemsArray.push_back({
                            {"slot", i},
                            {"id", item->id},
                            {"name", item->name},
                            {"description", item->description},
                            {"quantity", item->quantity},
                            {"maxStack", item->maxStack},
                            {"consumable", item->consumable},
                            {"healthRestore", item->healthRestore},
                            {"manaRestore", item->manaRestore},
                            {"staminaRestore", item->staminaRestore},
                            {"damageBonus", item->damageBonus},
                            {"defenseBonus", item->defenseBonus}
                        });
                    }
                }
                inventoryData["items"] = itemsArray;
                
                // Save hotbar items
                json hotbarArray = json::array();
                for (int i = 0; i < PlayerInventory::HOTBAR_SLOTS; ++i) {
                    if (inventory.hotbar[i]) {
                        auto item = inventory.hotbar[i];
                        hotbarArray.push_back({
                            {"slot", i},
                            {"id", item->id},
                            {"name", item->name},
                            {"quantity", item->quantity}
                        });
                    }
                }
                inventoryData["hotbar"] = hotbarArray;
                
                componentsData["PlayerInventory"] = inventoryData;
            }
            
            // Save PlayerAbilities component
            if (scene->hasComponent<PlayerAbilities>(entityId)) {
                const PlayerAbilities& abilities = scene->getComponent<PlayerAbilities>(entityId);
                json abilitiesData = {
                    {"skillPoints", abilities.skillPoints}
                };
                
                // Save abilities
                json abilitiesArray = json::array();
                for (const auto& ability : abilities.abilities) {
                    abilitiesArray.push_back({
                        {"name", ability.name},
                        {"description", ability.description},
                        {"level", ability.level},
                        {"maxLevel", ability.maxLevel},
                        {"cooldown", ability.cooldown},
                        {"currentCooldown", ability.currentCooldown},
                        {"manaCost", ability.manaCost},
                        {"staminaCost", ability.staminaCost},
                        {"unlocked", ability.unlocked},
                        {"passive", ability.passive},
                        {"damage", ability.damage},
                        {"range", ability.range},
                        {"duration", ability.duration}
                    });
                }
                abilitiesData["abilities"] = abilitiesArray;
                
                // Save hotbar abilities
                json hotbarAbilitiesArray = json::array();
                for (int i = 0; i < 4; ++i) {
                    hotbarAbilitiesArray.push_back(abilities.hotbarAbilities[i]);
                }
                abilitiesData["hotbarAbilities"] = hotbarAbilitiesArray;
                
                componentsData["PlayerAbilities"] = abilitiesData;
            }
            
            // Save PlayerState component
            if (scene->hasComponent<PlayerState>(entityId)) {
                const PlayerState& state = scene->getComponent<PlayerState>(entityId);
                componentsData["PlayerState"] = {
                    {"currentState", static_cast<int>(state.currentState)},
                    {"previousState", static_cast<int>(state.previousState)},
                    {"stateTimer", state.stateTimer},
                    {"currentFrame", state.currentFrame},
                    {"frameTimer", state.frameTimer},
                    {"frameRate", state.frameRate},
                    {"facingX", state.facing.x},
                    {"facingY", state.facing.y},
                    {"actionInProgress", state.actionInProgress},
                    {"actionDuration", state.actionDuration}
                };
            }
            
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
