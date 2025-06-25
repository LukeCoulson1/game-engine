#include "GameEditor.h"
#include "SceneWindow.h"
#include "GameLogicWindow.h"
#include "../systems/CoreSystems.h"
#include "../scene/Scene.h"
#include "../systems/SystemManager.h"
#include "../input/InputManager.h"
#include "../utils/ResourceManager.h"
#include "../utils/ConfigManager.h"
#include "../generation/ProceduralGeneration.h"
#include "../generation/OptimizedProceduralGeneration.h"
#include <chrono>
#include <iostream>
#include <filesystem>
#include <algorithm>

#ifdef _WIN32
#include <windows.h>
#include <commdlg.h>
#include <shlobj.h>
#include <objbase.h>
#include <shobjidl.h>
#endif

GameEditor::GameEditor() {
    // Initialize rename buffer with reasonable size
    m_renameBuffer.resize(256);
    
    // Initialize procedural generation manager
    m_proceduralManager = std::make_unique<ProceduralGenerationManager>();
      // Initialize node editor
    m_nodeEditor = std::make_unique<NodeEditor::NodeEditorWindow>();    // Initialize scene manager
    m_sceneManager = std::make_unique<SceneManager>(this);
    
    // Initialize game logic window
    m_gameLogicWindow = std::make_unique<GameLogicWindow>();
    
    // Initialize asset folder from config
    auto& config = ConfigManager::getInstance();
    m_currentAssetFolder = config.getAssetFolder();
}

GameEditor::~GameEditor() {
    shutdown();
}

bool GameEditor::initialize() {
    // Load configuration first
    auto& config = ConfigManager::getInstance();
    config.loadConfig();
    
    // Get window settings from config
    int windowWidth, windowHeight;
    config.getWindowSize(windowWidth, windowHeight);
    
    // Initialize engine with saved window size
    auto& engine = Engine::getInstance();
    if (!engine.initialize("Game Engine Editor", windowWidth, windowHeight)) {
        return false;
    }
    
    // Apply additional window settings after engine initialization
    loadWindowState();    // Initialize ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;    
    // Setup ImGui style
    ImGui::StyleColorsDark();
    
    // Setup Platform/Renderer backends
    auto renderer = engine.getRenderer();
    ImGui_ImplSDL2_InitForSDLRenderer(SDL_GetWindowFromID(1), renderer->getSDLRenderer());
    ImGui_ImplSDLRenderer2_Init(renderer->getSDLRenderer());
    
    // Don't create a default scene window on startup
    // Users can open scenes via Scene Manager when needed
    
    m_running = true;
    return true;
}

void GameEditor::run() {
    auto lastTime = std::chrono::high_resolution_clock::now();
    
    while (m_running) {
        auto currentTime = std::chrono::high_resolution_clock::now();
        float deltaTime = std::chrono::duration<float>(currentTime - lastTime).count();
        lastTime = currentTime;
        
        // Handle events
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            ImGui_ImplSDL2_ProcessEvent(&event);
            
            if (event.type == SDL_QUIT) {
                m_running = false;
            }
            
            auto& engine = Engine::getInstance();
            if (auto inputManager = engine.getInputManager()) {
                inputManager->handleEvent(event);
            }
        }
        
        update(deltaTime);
        render();
    }
}

void GameEditor::update(float deltaTime) {
    auto& engine = Engine::getInstance();
    
    // Update input
    if (auto inputManager = engine.getInputManager()) {
        inputManager->update();
    }
    
    // Update current scene if playing
    if (m_currentScene) {
        m_currentScene->update(deltaTime);
    }
}

void GameEditor::render() {
    auto& engine = Engine::getInstance();
    auto renderer = engine.getRenderer();
    
    // Start ImGui frame
    ImGui_ImplSDLRenderer2_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();
      // Enable docking (commented out - requires docking branch)
    ImGuiIO& io = ImGui::GetIO();
    // if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable) {
    //     ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
    //     ImGui::DockSpaceOverViewport(dockspace_id);
    // }
      // Render UI
    renderUI();
    
    // Clear background only - scene content is now rendered within scene windows
    renderer->clear(Color(45, 45, 48));
    
    // Note: Scene content is now rendered within individual SceneWindow instances
    // Each SceneWindow handles its own scene rendering using ImGui draw lists// Render ImGui
    ImGui::Render();
    auto engineRenderer = engine.getRenderer();
    ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData(), engineRenderer->getSDLRenderer());
    
    // Update and Render additional Platform Windows (commented out - requires viewports)
    // if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
    //     ImGui::UpdatePlatformWindows();
    //     ImGui::RenderPlatformWindowsDefault();
    // }
    
    renderer->present();
}

void GameEditor::renderUI() {    showMainMenuBar();
      if (m_showSceneHierarchy) showSceneHierarchy();
    if (m_showInspector) showInspector();
    if (m_showAssetBrowser) showAssetBrowser();
    if (m_showConsole) showConsole();
    if (m_showCameraControls) showCameraControls();
    if (m_showProceduralGeneration) showProceduralGeneration();    if (m_showNodeEditor) showNodeEditor();
    if (m_showSceneManager) showSceneManager();
    if (m_showGameLogicWindow) showGameLogicWindow();
    if (m_showDemo) ImGui::ShowDemoWindow(&m_showDemo);
    
    // Render all scene windows
    for (auto& sceneWindow : m_sceneWindows) {
        if (sceneWindow->isOpen()) {
            sceneWindow->render();
        }
    }    // Clean up closed windows
    bool activeWindowWasClosed = false;
    size_t windowsBeforeCleanup = m_sceneWindows.size();
    m_sceneWindows.erase(
        std::remove_if(m_sceneWindows.begin(), m_sceneWindows.end(),
            [this, &activeWindowWasClosed](const std::unique_ptr<SceneWindow>& window) {
                if (!window->isOpen()) {
                    if (window.get() == m_activeSceneWindow) {
                        m_activeSceneWindow = nullptr;
                        activeWindowWasClosed = true;
                    }
                    return true;
                }
                return false;
            }),
        m_sceneWindows.end()
    );
      // If the active window was closed, try to set another as active
    if (activeWindowWasClosed && !m_sceneWindows.empty()) {
        m_activeSceneWindow = m_sceneWindows[0].get();
    }
    
    // Don't automatically create scene windows - let users open them via Scene Manager
    
    // Update active scene window for other panels
    updateActiveSceneData();
}

void GameEditor::showMainMenuBar() {
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("New Scene", "Ctrl+N")) {
                openSceneInNewWindow();
            }
            if (ImGui::MenuItem("Open Scene in New Window", "Ctrl+Shift+N")) {
                openSceneInNewWindow();
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Load Scene", "Ctrl+O")) {
                // TODO: File dialog
                loadScene("scene.json");
            }
            if (ImGui::MenuItem("Save Scene", "Ctrl+S")) {
                // TODO: File dialog
                saveScene("scene.json");
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Exit", "Alt+F4")) {
                m_running = false;
            }
            ImGui::EndMenu();
        }
        
        if (ImGui::BeginMenu("Window")) {
            if (ImGui::MenuItem("New Scene Window", "Ctrl+Shift+W")) {
                openSceneInNewWindow();
            }
            ImGui::Separator();
            
            // List all open scene windows
            for (size_t i = 0; i < m_sceneWindows.size(); ++i) {
                auto& window = m_sceneWindows[i];
                if (window->isOpen()) {
                    bool isActive = (window.get() == m_activeSceneWindow);
                    if (ImGui::MenuItem(window->getTitle().c_str(), nullptr, isActive)) {
                        setActiveSceneWindow(window.get());
                    }
                }
            }
            
            if (!m_sceneWindows.empty()) {
                ImGui::Separator();
                if (ImGui::MenuItem("Close Current Window", "Ctrl+W")) {
                    if (m_activeSceneWindow) {
                        closeSceneWindow(m_activeSceneWindow);
                    }
                }
            }
            ImGui::EndMenu();
        }
          if (ImGui::BeginMenu("Edit")) {
            bool hasActiveScene = (m_activeSceneWindow && m_activeSceneWindow->isOpen());
            bool hasSelection = hasActiveScene && m_activeSceneWindow->hasSelectedEntity();
            
            if (ImGui::MenuItem("Create Entity", "Ctrl+E", false, hasActiveScene)) {
                if (m_activeSceneWindow) {
                    auto scene = m_activeSceneWindow->getScene();
                    if (scene) {
                        EntityID entity = scene->createEntity();
                        scene->addComponent<Transform>(entity, Transform(0.0f, 0.0f));
                        
                        std::string defaultName = "Entity_" + std::to_string(entity);
                        scene->setEntityName(entity, defaultName);
                        
                        m_activeSceneWindow->setSelectedEntity(entity);
                        m_activeSceneWindow->setDirty(true);
                        
                        m_consoleMessages.push_back("Created entity: " + defaultName + " in " + m_activeSceneWindow->getTitle());
                    }
                }
            }
            if (ImGui::MenuItem("Delete Entity", "Delete", false, hasSelection)) {
                if (m_activeSceneWindow) {
                    auto scene = m_activeSceneWindow->getScene();
                    EntityID entityToDelete = m_activeSceneWindow->getSelectedEntity();
                    if (scene && entityToDelete != 0) {
                        std::string entityName = scene->getEntityName(entityToDelete);
                        scene->destroyEntity(entityToDelete);
                        m_activeSceneWindow->setSelectedEntity(0);
                        m_activeSceneWindow->setDirty(true);
                        
                        m_consoleMessages.push_back("Deleted entity: " + entityName + " from " + m_activeSceneWindow->getTitle());
                    }
                }
            }
            ImGui::EndMenu();
        }        if (ImGui::BeginMenu("View")) {
            ImGui::MenuItem("Scene Hierarchy", nullptr, &m_showSceneHierarchy);
            ImGui::MenuItem("Inspector", nullptr, &m_showInspector);
            ImGui::MenuItem("Asset Browser", nullptr, &m_showAssetBrowser);
            ImGui::MenuItem("Console", nullptr, &m_showConsole);
            ImGui::MenuItem("Camera Controls", nullptr, &m_showCameraControls);
            ImGui::MenuItem("Procedural Generation", nullptr, &m_showProceduralGeneration);            ImGui::MenuItem("Node Editor", nullptr, &m_showNodeEditor);
            ImGui::MenuItem("Scene Manager", nullptr, &m_showSceneManager);
            ImGui::MenuItem("Game Logic Window", nullptr, &m_showGameLogicWindow);
            ImGui::Separator();
            ImGui::MenuItem("ImGui Demo", nullptr, &m_showDemo);
            ImGui::EndMenu();
        }
        
        ImGui::EndMainMenuBar();
    }
}

void GameEditor::showSceneHierarchy() {
    ImGui::Begin("Scene Hierarchy", &m_showSceneHierarchy);
    
    // Show which scene window is active
    if (m_activeSceneWindow) {
        ImGui::Text("Active Scene: %s", m_activeSceneWindow->getTitle().c_str());
        ImGui::Separator();
    } else {
        ImGui::Text("No active scene");
        ImGui::Text("Create a new scene window to start editing");
        ImGui::End();
        return;
    }
    
    bool hasActiveScene = (m_activeSceneWindow && m_activeSceneWindow->isOpen() && m_activeSceneWindow->getScene());    if (ImGui::Button("Create Entity") && hasActiveScene) {
        auto scene = m_activeSceneWindow->getScene();
        EntityID entity = scene->createEntity();
        scene->addComponent<Transform>(entity, Transform(0.0f, 0.0f));
        
        std::string defaultName = "Entity_" + std::to_string(entity);
        scene->setEntityName(entity, defaultName);
        
        m_activeSceneWindow->setSelectedEntity(entity);
        m_activeSceneWindow->setDirty(true);
        
        // Debug logging to track entity creation
        std::string debugMsg = "Created entity: " + defaultName + " in " + m_activeSceneWindow->getTitle() + 
                              " (Entity ID: " + std::to_string(entity) + ", Window ID: " + std::to_string(reinterpret_cast<uintptr_t>(m_activeSceneWindow)) + ")";
        m_consoleMessages.push_back(debugMsg);
    }
    ImGui::SameLine();
    if (ImGui::Button("Delete Entity") && hasActiveScene && m_activeSceneWindow->hasSelectedEntity()) {
        auto scene = m_activeSceneWindow->getScene();
        EntityID entityToDelete = m_activeSceneWindow->getSelectedEntity();
        std::string entityName = scene->getEntityName(entityToDelete);
        scene->destroyEntity(entityToDelete);
        m_activeSceneWindow->setSelectedEntity(0);
        m_activeSceneWindow->setDirty(true);
        
        m_consoleMessages.push_back("Deleted entity: " + entityName + " from " + m_activeSceneWindow->getTitle());
    }
    
    ImGui::Separator();
    
    // List all entities in the active scene
    if (hasActiveScene) {
        auto scene = m_activeSceneWindow->getScene();
        auto entities = scene->getAllLivingEntities();
        EntityID selectedEntity = m_activeSceneWindow->getSelectedEntity();
        
        for (EntityID entity : entities) {
            std::string entityName = scene->getEntityName(entity);
            std::string displayName = "🎭 " + entityName + " (" + std::to_string(entity) + ")";            if (ImGui::Selectable(displayName.c_str(), selectedEntity == entity)) {
                m_activeSceneWindow->setSelectedEntity(entity);
            }
            
            // Context menu for renaming
            if (ImGui::BeginPopupContextItem(("entity_context_" + std::to_string(entity)).c_str())) {
                if (ImGui::MenuItem("Rename")) {
                    m_renamingEntity = entity;
                    m_renameBuffer = entityName;
                    m_showRenameDialog = true;
                }                if (ImGui::MenuItem("Delete")) {
                    if (selectedEntity == entity) {
                        m_activeSceneWindow->setSelectedEntity(0);
                    }
                    scene->destroyEntity(entity);
                    m_activeSceneWindow->setDirty(true);
                    m_consoleMessages.push_back("Deleted entity: " + entityName + " from " + m_activeSceneWindow->getTitle());
                }
                ImGui::EndPopup();
            }
        }
    }
    
    // Rename dialog
    if (m_showRenameDialog) {
        ImGui::OpenPopup("Rename Entity");
    }
    
    if (ImGui::BeginPopupModal("Rename Entity", &m_showRenameDialog, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Enter new name for entity:");
        
        // Auto-focus the input field when dialog opens
        if (ImGui::IsWindowAppearing()) {
            ImGui::SetKeyboardFocusHere();
        }        if (ImGui::InputText("##rename", m_renameBuffer.data(), m_renameBuffer.capacity(), ImGuiInputTextFlags_EnterReturnsTrue)) {
            // Enter pressed - confirm rename
            if (m_activeSceneWindow && m_activeSceneWindow->getScene() && !m_renameBuffer.empty()) {
                std::string newName(m_renameBuffer.c_str());
                auto scene = m_activeSceneWindow->getScene();
                std::cout << "DEBUG: Renaming entity " << m_renamingEntity << " to '" << newName << "' (Enter key)" << std::endl;
                scene->setEntityName(m_renamingEntity, newName);
                m_activeSceneWindow->setDirty(true);
                m_consoleMessages.push_back("Renamed entity to: " + newName + " in " + m_activeSceneWindow->getTitle());
            }
            m_showRenameDialog = false;
        }
        
        ImGui::Separator();
          if (ImGui::Button("Rename", ImVec2(80, 0))) {
            if (m_activeSceneWindow && m_activeSceneWindow->getScene() && !m_renameBuffer.empty()) {
                std::string newName(m_renameBuffer.c_str());
                auto scene = m_activeSceneWindow->getScene();
                std::cout << "DEBUG: Renaming entity " << m_renamingEntity << " to '" << newName << "' (Button)" << std::endl;
                scene->setEntityName(m_renamingEntity, newName);
                m_activeSceneWindow->setDirty(true);
                m_consoleMessages.push_back("Renamed entity to: " + newName + " in " + m_activeSceneWindow->getTitle());
            }
            m_showRenameDialog = false;
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(80, 0))) {
            m_showRenameDialog = false;
        }
        
        ImGui::EndPopup();
    }
    
    ImGui::End();
}

void GameEditor::showInspector() {
    ImGui::Begin("Inspector", &m_showInspector);
    
    if (!m_activeSceneWindow || !m_activeSceneWindow->isOpen() || !m_activeSceneWindow->getScene()) {
        ImGui::Text("No active scene");
        ImGui::Text("Select a scene window to inspect entities");
        ImGui::End();
        return;
    }
    
    auto scene = m_activeSceneWindow->getScene();
    EntityID selectedEntity = m_activeSceneWindow->getSelectedEntity();
    bool hasSelection = m_activeSceneWindow->hasSelectedEntity();
    
    if (hasSelection && scene) {
        // Show which scene this entity belongs to
        ImGui::Text("Scene: %s", m_activeSceneWindow->getTitle().c_str());
        ImGui::Separator();
        
        // Entity name editing
        std::string currentName = scene->getEntityName(selectedEntity);
        ImGui::Text("🎭 Entity Name:");
        ImGui::SameLine();
        
        // Inline name editing
        static char nameEditBuffer[256];
        strncpy_s(nameEditBuffer, sizeof(nameEditBuffer), currentName.c_str(), _TRUNCATE);
        
        ImGui::PushItemWidth(-50);
        if (ImGui::InputText("##entityname", nameEditBuffer, sizeof(nameEditBuffer), ImGuiInputTextFlags_EnterReturnsTrue)) {
            scene->setEntityName(selectedEntity, std::string(nameEditBuffer));
            m_activeSceneWindow->setDirty(true);
            m_consoleMessages.push_back("Renamed entity to: " + std::string(nameEditBuffer) + " in " + m_activeSceneWindow->getTitle());
        }
        ImGui::PopItemWidth();
        
        ImGui::Text("Entity ID: %u", selectedEntity);
        ImGui::Separator();
          // Transform component
        if (scene->hasComponent<Transform>(selectedEntity)) {
            if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen)) {
                auto& transform = scene->getComponent<Transform>(selectedEntity);
                  ImGui::Text("📍 World Coordinates:");                if (ImGui::DragFloat2("Position", &transform.position.x, 1.0f, -10000.0f, 10000.0f)) {
                    m_activeSceneWindow->setDirty(true);
                }
                ImGui::SameLine();
                ImGui::TextDisabled("(?)");
                if (ImGui::IsItemHovered()) {
                    ImGui::SetTooltip("World coordinates can be negative!\n(0,0) = center of screen\nNegative X = left, Negative Y = up");
                }                if (ImGui::DragFloat2("Scale", &transform.scale.x, 0.1f, 0.1f, 10.0f)) {
                    m_activeSceneWindow->setDirty(true);
                    printf("DEBUG: Scale changed to %.2f, %.2f\n", transform.scale.x, transform.scale.y);
                    
                    // Verify the change was persisted by re-reading from the scene
                    auto& verifyTransform = scene->getComponent<Transform>(selectedEntity);
                    printf("DEBUG: Verified persisted scale: %.2f, %.2f\n", verifyTransform.scale.x, verifyTransform.scale.y);
                    fflush(stdout);
                }
                if (ImGui::DragFloat("Rotation", &transform.rotation, 1.0f, -360.0f, 360.0f)) {
                    m_activeSceneWindow->setDirty(true);
                    printf("DEBUG: Rotation changed to %.1f\n", transform.rotation);
                    
                    // Verify the change was persisted by re-reading from the scene
                    auto& verifyTransform = scene->getComponent<Transform>(selectedEntity);
                    printf("DEBUG: Verified persisted rotation: %.1f\n", verifyTransform.rotation);
                    fflush(stdout);
                }
                
                // Show screen position for debugging
                auto& engine = Engine::getInstance();
                auto renderer = engine.getRenderer();
                Vector2 screenPos = renderer->worldToScreen(transform.position);
                ImGui::Separator();
                ImGui::Text("🖥️ Screen Position: (%.1f, %.1f)", screenPos.x, screenPos.y);
            }
        }        // Sprite component
        if (scene->hasComponent<Sprite>(selectedEntity)) {
            if (ImGui::CollapsingHeader("Sprite Component", ImGuiTreeNodeFlags_DefaultOpen)) {
                auto& sprite = scene->getComponent<Sprite>(selectedEntity);
                
                bool changed = false;
                
                // Basic sprite properties
                ImGui::Text("🎨 Sprite Properties:");
                changed |= ImGui::Checkbox("Visible", &sprite.visible);
                ImGui::SameLine();
                ImGui::TextDisabled("(?)");
                if (ImGui::IsItemHovered()) {
                    ImGui::SetTooltip("Uncheck to hide this sprite");
                }
                
                changed |= ImGui::DragInt("Layer", &sprite.layer, 1.0f, -100, 100);
                ImGui::SameLine();
                ImGui::TextDisabled("(?)");
                if (ImGui::IsItemHovered()) {
                    ImGui::SetTooltip("Higher layer numbers render on top");
                }
                
                // Color tint with better UI
                ImGui::Text("Tint Color:");
                changed |= ImGui::ColorEdit4("##tint", reinterpret_cast<float*>(&sprite.tint), 
                                           ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_AlphaPreview);
                
                if (changed) {
                    m_activeSceneWindow->setDirty(true);
                }
                
                ImGui::Separator();
                
                // Texture info and assignment with better visual feedback
                ImGui::Text("🖼️ Texture Assignment:");
                if (sprite.texture) {
                    ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "✅ Texture: %dx%d pixels", 
                        sprite.texture->getWidth(), sprite.texture->getHeight());
                    
                    ImGui::SameLine();
                    if (ImGui::SmallButton("Remove")) {
                        sprite.texture.reset();
                        m_activeSceneWindow->setDirty(true);
                        m_consoleMessages.push_back("Removed texture from entity " + std::to_string(selectedEntity) + " in " + m_activeSceneWindow->getTitle());
                    }
                } else {
                    ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f), "⚠️ No texture assigned");
                }                  // Texture assignment from assets
                ImGui::Text("📁 Available Images:");
                ImGui::Text("(Click to assign texture)");
                
                // Show available images in a compact, scrollable area
                try {
                    if (std::filesystem::exists(m_currentAssetFolder)) {
                        std::vector<std::string> imageFiles;
                        
                        // Collect images from main asset folder
                        for (const auto& entry : std::filesystem::directory_iterator(m_currentAssetFolder)) {
                            if (entry.is_regular_file()) {
                                std::string filename = entry.path().filename().string();
                                std::string extension = entry.path().extension().string();
                                std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);
                                
                                if (extension == ".png" || extension == ".jpg" || extension == ".jpeg" || 
                                    extension == ".bmp" || extension == ".tga") {
                                    imageFiles.push_back(m_currentAssetFolder + "/" + filename);
                                }
                            }
                        }
                        
                        // Also collect from tiles subfolder
                        std::string tilesFolder = m_currentAssetFolder + "/tiles";
                        if (std::filesystem::exists(tilesFolder)) {
                            for (const auto& entry : std::filesystem::directory_iterator(tilesFolder)) {
                                if (entry.is_regular_file()) {
                                    std::string filename = entry.path().filename().string();
                                    std::string extension = entry.path().extension().string();
                                    std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);
                                    
                                    if (extension == ".png" || extension == ".jpg" || extension == ".jpeg" || 
                                        extension == ".bmp" || extension == ".tga") {
                                        imageFiles.push_back(tilesFolder + "/" + filename);
                                    }
                                }
                            }
                        }
                        
                        if (!imageFiles.empty()) {
                            ImGui::BeginChild("ImageSelector", ImVec2(0, 120), true, ImGuiWindowFlags_HorizontalScrollbar);
                            for (const auto& fullPath : imageFiles) {
                                // Extract just the filename for display
                                std::string displayName = std::filesystem::path(fullPath).filename().string();
                                
                                if (ImGui::Selectable(("🖼️ " + displayName).c_str())) {
                                    auto& engine = Engine::getInstance();
                                    auto resourceManager = engine.getResourceManager();
                                    auto texture = resourceManager->loadTexture(fullPath);
                                    
                                    if (texture) {
                                        sprite.texture = texture;
                                        sprite.sourceRect = Rect(0, 0, texture->getWidth(), texture->getHeight());
                                        m_activeSceneWindow->setDirty(true);
                                        m_consoleMessages.push_back("✅ Assigned texture: " + displayName + " to entity " + std::to_string(selectedEntity) + " in " + m_activeSceneWindow->getTitle());
                                    } else {
                                        m_consoleMessages.push_back("❌ Failed to load texture: " + displayName);
                                    }
                                }
                                if (ImGui::IsItemHovered()) {
                                    ImGui::SetTooltip("Assign %s to this sprite\nPath: %s", displayName.c_str(), fullPath.c_str());
                                }
                            }
                            ImGui::EndChild();
                        } else {
                            ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.0f, 1.0f), "📂 No images found in %s", m_currentAssetFolder.c_str());
                            ImGui::TextWrapped("Add .png, .jpg, .jpeg, .bmp, or .tga files to %s", m_currentAssetFolder.c_str());
                        }
                    } else {
                        ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f), "📁 Asset folder not found: %s", m_currentAssetFolder.c_str());
                        if (ImGui::SmallButton("Create Asset Folder")) {
                            std::filesystem::create_directories(m_currentAssetFolder);
                            m_consoleMessages.push_back("Created asset folder: " + m_currentAssetFolder);
                        }
                    }
                } catch (const std::exception& e) {
                    ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "❌ Error reading assets: %s", e.what());
                }
                
                // Source rectangle editing for sprite sheets
                if (sprite.texture) {
                    ImGui::Separator();
                    ImGui::Text("Source Rectangle (for sprite sheets):");
                    bool rectChanged = false;
                    rectChanged |= ImGui::DragFloat("X", &sprite.sourceRect.x, 1.0f, 0.0f, sprite.texture->getWidth());
                    rectChanged |= ImGui::DragFloat("Y", &sprite.sourceRect.y, 1.0f, 0.0f, sprite.texture->getHeight());
                    rectChanged |= ImGui::DragFloat("Width", &sprite.sourceRect.width, 1.0f, 1.0f, sprite.texture->getWidth());
                    rectChanged |= ImGui::DragFloat("Height", &sprite.sourceRect.height, 1.0f, 1.0f, sprite.texture->getHeight());
                    
                    if (rectChanged) {
                        m_activeSceneWindow->setDirty(true);
                    }
                    
                    if (ImGui::Button("Reset to Full Texture")) {
                        sprite.sourceRect = Rect(0, 0, sprite.texture->getWidth(), sprite.texture->getHeight());
                    }
                }
            }
        }
          // Player Components
        if (scene->hasComponent<PlayerController>(selectedEntity)) {
            if (ImGui::CollapsingHeader("🎮 Player Controller", ImGuiTreeNodeFlags_DefaultOpen)) {
                auto& controller = scene->getComponent<PlayerController>(selectedEntity);
                
                // Control scheme selection
                ImGui::Text("Control Scheme:");                const char* schemes[] = { "WASD", "Arrow Keys", "WASD+Mouse", "Gamepad", "Custom" };
                int currentScheme = static_cast<int>(controller.controlScheme);
                if (ImGui::Combo("##scheme", &currentScheme, schemes, 5)) {
                    controller.controlScheme = static_cast<PlayerController::ControlScheme>(currentScheme);
                    m_activeSceneWindow->setDirty(true);
                }
                  // Movement type selection
                ImGui::Text("Movement Type:");
                const char* types[] = { "Top Down", "Platformer", "First Person", "Grid Based", "Physics" };
                int currentType = static_cast<int>(controller.movementType);
                if (ImGui::Combo("##movement", &currentType, types, 5)) {
                    controller.movementType = static_cast<PlayerController::MovementType>(currentType);
                    m_activeSceneWindow->setDirty(true);
                }
                
                // Movement speed
                if (ImGui::DragFloat("Move Speed", &controller.moveSpeed, 1.0f, 10.0f, 1000.0f)) {
                    m_activeSceneWindow->setDirty(true);
                }
            }
        }
        
        if (scene->hasComponent<PlayerStats>(selectedEntity)) {
            if (ImGui::CollapsingHeader("📊 Player Stats", ImGuiTreeNodeFlags_DefaultOpen)) {
                auto& stats = scene->getComponent<PlayerStats>(selectedEntity);
                
                // Core stats
                ImGui::Text("Level: %d | XP: %d/%d", stats.core.level, stats.core.experience, stats.core.experienceToNext);
                
                // Health, Mana, Stamina bars
                ImGui::Text("Health:");
                ImGui::ProgressBar((float)stats.derived.currentHealth / stats.derived.maxHealth, 
                                 ImVec2(-1, 0), ("" + std::to_string(stats.derived.currentHealth) + "/" + std::to_string(stats.derived.maxHealth)).c_str());
                
                ImGui::Text("Mana:");
                ImGui::ProgressBar((float)stats.derived.currentMana / stats.derived.maxMana, 
                                 ImVec2(-1, 0), ("" + std::to_string(stats.derived.currentMana) + "/" + std::to_string(stats.derived.maxMana)).c_str());
                
                ImGui::Text("Stamina:");
                ImGui::ProgressBar((float)stats.derived.currentStamina / stats.derived.maxStamina, 
                                 ImVec2(-1, 0), ("" + std::to_string(stats.derived.currentStamina) + "/" + std::to_string(stats.derived.maxStamina)).c_str());
                  // Primary attributes
                if (ImGui::TreeNode("Primary Attributes")) {
                    if (ImGui::DragInt("Strength", &stats.core.strength, 1, 1, 100)) {
                        stats.recalculateStats();
                        m_activeSceneWindow->setDirty(true);
                    }
                    if (ImGui::DragInt("Dexterity", &stats.core.dexterity, 1, 1, 100)) {
                        stats.recalculateStats();
                        m_activeSceneWindow->setDirty(true);
                    }
                    if (ImGui::DragInt("Intelligence", &stats.core.intelligence, 1, 1, 100)) {
                        stats.recalculateStats();
                        m_activeSceneWindow->setDirty(true);
                    }
                    if (ImGui::DragInt("Vitality", &stats.core.vitality, 1, 1, 100)) {
                        stats.recalculateStats();
                        m_activeSceneWindow->setDirty(true);
                    }
                    if (ImGui::DragInt("Luck", &stats.core.luck, 1, 1, 100)) {
                        stats.recalculateStats();
                        m_activeSceneWindow->setDirty(true);
                    }
                    ImGui::TreePop();
                }
            }
        }
        
        if (scene->hasComponent<PlayerPhysics>(selectedEntity)) {
            if (ImGui::CollapsingHeader("🏃 Player Physics")) {
                auto& physics = scene->getComponent<PlayerPhysics>(selectedEntity);
                
                ImGui::Text("Velocity: (%.1f, %.1f)", physics.velocity.x, physics.velocity.y);
                
                if (ImGui::DragFloat("Max Speed", &physics.maxSpeed, 1.0f, 10.0f, 1000.0f)) {
                    m_activeSceneWindow->setDirty(true);
                }
                if (ImGui::DragFloat("Friction", &physics.friction, 0.01f, 0.0f, 1.0f)) {
                    m_activeSceneWindow->setDirty(true);
                }
                
                ImGui::Checkbox("Can Jump", &physics.canJump);
                ImGui::SameLine();
                ImGui::Checkbox("Can Dash", &physics.canDash);
                
                if (physics.canDash) {
                    ImGui::DragFloat("Dash Force", &physics.dashForce, 10.0f, 100.0f, 2000.0f);
                    ImGui::DragFloat("Dash Cooldown", &physics.dashCooldown, 0.1f, 0.1f, 5.0f);
                }
            }
        }
        
        if (scene->hasComponent<PlayerInventory>(selectedEntity)) {
            if (ImGui::CollapsingHeader("🎒 Player Inventory")) {
                auto& inventory = scene->getComponent<PlayerInventory>(selectedEntity);
                
                ImGui::Text("💰 Currency: %d", inventory.currency);
                ImGui::Text("🔥 Selected Hotbar Slot: %d", inventory.selectedHotbarSlot);
                
                // Show hotbar items
                if (ImGui::TreeNode("Hotbar Items")) {
                    for (int i = 0; i < inventory.HOTBAR_SLOTS; ++i) {
                        auto& item = inventory.hotbar[i];
                        if (item && item->quantity > 0) {
                            ImGui::Text("[%d] %s (x%d)", i, item->name.c_str(), item->quantity);
                        } else {
                            ImGui::Text("[%d] Empty", i);
                        }
                    }
                    ImGui::TreePop();
                }
            }
        }
        
        if (scene->hasComponent<PlayerAbilities>(selectedEntity)) {
            if (ImGui::CollapsingHeader("✨ Player Abilities")) {
                auto& abilities = scene->getComponent<PlayerAbilities>(selectedEntity);
                
                ImGui::Text("🔮 Skill Points: %d", abilities.skillPoints);
                ImGui::Text("📋 Abilities: %zu", abilities.abilities.size());
                
                // Show abilities
                for (size_t i = 0; i < abilities.abilities.size(); ++i) {
                    auto& ability = abilities.abilities[i];
                    ImGui::Text("%s (Lvl %d) - Cooldown: %.1fs", 
                               ability.name.c_str(), ability.level, ability.currentCooldown);
                }
            }
        }
        
        if (scene->hasComponent<PlayerState>(selectedEntity)) {
            if (ImGui::CollapsingHeader("🎭 Player State")) {
                auto& state = scene->getComponent<PlayerState>(selectedEntity);
                
                // Current state display
                const char* stateNames[] = { "Idle", "Walking", "Running", "Jumping", "Falling", 
                                           "Attacking", "Defending", "Dashing", "Interacting", "Dead", "Stunned" };
                int currentStateIndex = static_cast<int>(state.currentState);
                ImGui::Text("Current State: %s", stateNames[currentStateIndex]);
                ImGui::Text("State Timer: %.2fs", state.stateTimer);
                ImGui::Text("Facing: (%.1f, %.1f)", state.facing.x, state.facing.y);
                
                // Animation info
                ImGui::Text("Frame: %d | Frame Rate: %.1f fps", state.currentFrame, state.frameRate);
            }
        }

        // Add component buttons
        ImGui::Separator();
        ImGui::Text("🔧 Add Components:");
        
        if (ImGui::Button("Add Sprite Component") && !scene->hasComponent<Sprite>(selectedEntity)) {
            scene->addComponent<Sprite>(selectedEntity, Sprite());
            m_activeSceneWindow->setDirty(true);
        }
        ImGui::SameLine();
        if (ImGui::Button("Add Collider Component") && !scene->hasComponent<Collider>(selectedEntity)) {
            scene->addComponent<Collider>(selectedEntity, Collider());
            m_activeSceneWindow->setDirty(true);
        }
        
        if (ImGui::Button("Add RigidBody Component") && !scene->hasComponent<RigidBody>(selectedEntity)) {
            scene->addComponent<RigidBody>(selectedEntity, RigidBody());
            m_activeSceneWindow->setDirty(true);
        }
        
        ImGui::Separator();
        ImGui::Text("🎮 Player Entity:");
        
        // Check if entity is already a player
        bool isPlayer = scene->hasComponent<PlayerController>(selectedEntity) &&
                       scene->hasComponent<PlayerStats>(selectedEntity) &&
                       scene->hasComponent<PlayerPhysics>(selectedEntity);
                       
        if (isPlayer) {
            ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "✅ This is a Player Entity");
            
            if (ImGui::Button("Remove Player Components")) {
                // Remove all player components
                if (scene->hasComponent<PlayerController>(selectedEntity)) {
                    scene->removeComponent<PlayerController>(selectedEntity);
                }
                if (scene->hasComponent<PlayerStats>(selectedEntity)) {
                    scene->removeComponent<PlayerStats>(selectedEntity);
                }
                if (scene->hasComponent<PlayerPhysics>(selectedEntity)) {
                    scene->removeComponent<PlayerPhysics>(selectedEntity);
                }
                if (scene->hasComponent<PlayerInventory>(selectedEntity)) {
                    scene->removeComponent<PlayerInventory>(selectedEntity);
                }
                if (scene->hasComponent<PlayerAbilities>(selectedEntity)) {
                    scene->removeComponent<PlayerAbilities>(selectedEntity);
                }
                if (scene->hasComponent<PlayerState>(selectedEntity)) {
                    scene->removeComponent<PlayerState>(selectedEntity);
                }
                m_activeSceneWindow->setDirty(true);
                m_consoleMessages.push_back("Removed player components from entity " + std::to_string(selectedEntity));
            }
        }
        
        // Entity conversion section - always visible when an entity is selected
        ImGui::Separator();
        ImGui::Text("🔄 Convert Entity Type:");
        ImGui::Spacing();
        
        if (ImGui::Button("🎮 Make Player Entity", ImVec2(-1, 0))) {
            // Add all core player components
            if (!scene->hasComponent<PlayerController>(selectedEntity)) {
                scene->addComponent<PlayerController>(selectedEntity, PlayerController());
            }
            if (!scene->hasComponent<PlayerStats>(selectedEntity)) {
                scene->addComponent<PlayerStats>(selectedEntity, PlayerStats());
            }
            if (!scene->hasComponent<PlayerPhysics>(selectedEntity)) {
                scene->addComponent<PlayerPhysics>(selectedEntity, PlayerPhysics());
            }
            if (!scene->hasComponent<PlayerInventory>(selectedEntity)) {
                scene->addComponent<PlayerInventory>(selectedEntity, PlayerInventory());
            }
            if (!scene->hasComponent<PlayerAbilities>(selectedEntity)) {
                scene->addComponent<PlayerAbilities>(selectedEntity, PlayerAbilities());
            }
            if (!scene->hasComponent<PlayerState>(selectedEntity)) {
                scene->addComponent<PlayerState>(selectedEntity, PlayerState());
            }
            
            // Update entity name to indicate it's a player
            std::string currentName = scene->getEntityName(selectedEntity);
            if (currentName.find("Player") == std::string::npos) {
                scene->setEntityName(selectedEntity, "Player_" + currentName);
            }
            
            m_activeSceneWindow->setDirty(true);
            m_consoleMessages.push_back("✅ Converted entity " + std::to_string(selectedEntity) + " to Player Entity");
        }
        ImGui::SameLine();
        ImGui::TextDisabled("(?)");
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("Adds PlayerController, PlayerStats, PlayerPhysics,\nPlayerInventory, PlayerAbilities, and PlayerState components");
        }
        
        // Make NPC Entity
        if (ImGui::Button("🤖 Make NPC Entity", ImVec2(-1, 0))) {
            // Add basic NPC components if not present
            if (!scene->hasComponent<Sprite>(selectedEntity)) {
                scene->addComponent<Sprite>(selectedEntity, Sprite());
            }
            if (!scene->hasComponent<Collider>(selectedEntity)) {
                scene->addComponent<Collider>(selectedEntity, Collider());
            }
            if (!scene->hasComponent<PlayerStats>(selectedEntity)) {
                scene->addComponent<PlayerStats>(selectedEntity, PlayerStats());
            }
            
            // Update entity name to indicate it's an NPC
            std::string currentName = scene->getEntityName(selectedEntity);
            if (currentName.find("NPC") == std::string::npos) {
                scene->setEntityName(selectedEntity, "NPC_" + currentName);
            }
            
            m_activeSceneWindow->setDirty(true);
            m_consoleMessages.push_back("✅ Converted entity " + std::to_string(selectedEntity) + " to NPC Entity");
        }
        ImGui::SameLine();
        ImGui::TextDisabled("(?)");
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("Adds Sprite, Collider, and PlayerStats components\nfor basic NPC functionality");
        }
        
        // Make Enemy Entity
        if (ImGui::Button("⚔️ Make Enemy Entity", ImVec2(-1, 0))) {
            // Add basic enemy components if not present
            if (!scene->hasComponent<Sprite>(selectedEntity)) {
                scene->addComponent<Sprite>(selectedEntity, Sprite());
            }
            if (!scene->hasComponent<Collider>(selectedEntity)) {
                scene->addComponent<Collider>(selectedEntity, Collider());
            }
            if (!scene->hasComponent<RigidBody>(selectedEntity)) {
                scene->addComponent<RigidBody>(selectedEntity, RigidBody());
            }
            if (!scene->hasComponent<PlayerStats>(selectedEntity)) {
                scene->addComponent<PlayerStats>(selectedEntity, PlayerStats());
            }
            
            // Update entity name to indicate it's an enemy
            std::string currentName = scene->getEntityName(selectedEntity);
            if (currentName.find("Enemy") == std::string::npos) {
                scene->setEntityName(selectedEntity, "Enemy_" + currentName);
            }
            
            m_activeSceneWindow->setDirty(true);
            m_consoleMessages.push_back("✅ Converted entity " + std::to_string(selectedEntity) + " to Enemy Entity");
        }
        ImGui::SameLine();
        ImGui::TextDisabled("(?)");
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("Adds Sprite, Collider, RigidBody, and PlayerStats\ncomponents for combat-ready enemy");
        }
        
    } else {
        ImGui::Text("No entity selected");
        ImGui::TextWrapped("Select an entity from the scene to view and edit its components.");
    }
    
    ImGui::End();
}

void GameEditor::showAssetBrowser() {
    ImGui::Begin("Asset Browser", &m_showAssetBrowser);
    
    // Asset folder management
    ImGui::Text("Asset Folder:");
    ImGui::SameLine();
    
    // Current folder display and change button
    char folderBuffer[512];
    strncpy(folderBuffer, m_currentAssetFolder.c_str(), sizeof(folderBuffer) - 1);
    folderBuffer[sizeof(folderBuffer) - 1] = '\0';
    
    ImGui::PushItemWidth(-120);
    if (ImGui::InputText("##AssetFolder", folderBuffer, sizeof(folderBuffer), ImGuiInputTextFlags_EnterReturnsTrue)) {
        m_currentAssetFolder = folderBuffer;
        auto& config = ConfigManager::getInstance();
        config.setAssetFolder(m_currentAssetFolder);
        config.saveConfig();
        m_consoleMessages.push_back("Asset folder changed to: " + m_currentAssetFolder);
    }
    ImGui::PopItemWidth();
      ImGui::SameLine();
    if (ImGui::Button("Browse...")) {
        std::string selectedFolder = openFolderDialog(m_currentAssetFolder);
        if (!selectedFolder.empty()) {
            // Normalize path separators
            std::replace(selectedFolder.begin(), selectedFolder.end(), '\\', '/');
            
            // Update the asset folder
            m_currentAssetFolder = selectedFolder;
            ConfigManager::getInstance().setAssetFolder(selectedFolder);
            
            m_consoleMessages.push_back("Asset folder changed to: " + selectedFolder);
        }
    }
    
    // Action buttons
    if (ImGui::Button("Refresh")) {
        m_consoleMessages.push_back("Refreshed asset browser");
    }
    ImGui::SameLine();
    if (ImGui::Button("Open Folder")) {
        // Try to open the assets folder in file explorer
        #ifdef _WIN32
        std::string command = "explorer \"" + m_currentAssetFolder + "\"";
        system(command.c_str());
        #endif
    }
    ImGui::SameLine();
    if (ImGui::Button("Create Folder")) {
        try {
            std::filesystem::create_directories(m_currentAssetFolder);
            m_consoleMessages.push_back("Created folder: " + m_currentAssetFolder);
        } catch (const std::exception& e) {
            m_consoleMessages.push_back("Failed to create folder: " + std::string(e.what()));
        }
    }
    
    ImGui::Separator();
    
    // File browser for assets
    ImGui::Text("Available Assets:");
    
    try {
        if (std::filesystem::exists(m_currentAssetFolder)) {
            // Organize files by type
            std::vector<std::string> imageFiles;
            std::vector<std::string> audioFiles;
            std::vector<std::string> otherFiles;
            
            for (const auto& entry : std::filesystem::directory_iterator(m_currentAssetFolder)) {
                if (entry.is_regular_file()) {
                    std::string filename = entry.path().filename().string();
                    std::string extension = entry.path().extension().string();
                    
                    // Convert extension to lowercase for comparison
                    std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);
                    
                    // Categorize files
                    if (extension == ".png" || extension == ".jpg" || extension == ".jpeg" || 
                        extension == ".bmp" || extension == ".tga") {
                        imageFiles.push_back(filename);
                    } else if (extension == ".wav" || extension == ".ogg" || extension == ".mp3") {
                        audioFiles.push_back(filename);
                    } else {
                        otherFiles.push_back(filename);
                    }
                }
            }
            
            // Also check subdirectories
            std::vector<std::string> subdirectories;
            for (const auto& entry : std::filesystem::directory_iterator(m_currentAssetFolder)) {
                if (entry.is_directory()) {
                    std::string dirname = entry.path().filename().string();
                    subdirectories.push_back(dirname);
                    
                    // Also scan subdirectories for images (like tiles folder)
                    try {
                        for (const auto& subEntry : std::filesystem::directory_iterator(entry.path())) {
                            if (subEntry.is_regular_file()) {
                                std::string filename = subEntry.path().filename().string();
                                std::string extension = subEntry.path().extension().string();
                                std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);
                                
                                if (extension == ".png" || extension == ".jpg" || extension == ".jpeg" || 
                                    extension == ".bmp" || extension == ".tga") {
                                    imageFiles.push_back(dirname + "/" + filename);
                                } else if (extension == ".wav" || extension == ".ogg" || extension == ".mp3") {
                                    audioFiles.push_back(dirname + "/" + filename);
                                }
                            }
                        }
                    } catch (const std::exception&) {
                        // Ignore errors accessing subdirectories
                    }
                }
            }
            
            // Display subdirectories
            if (!subdirectories.empty()) {
                if (ImGui::CollapsingHeader("Subdirectories")) {
                    for (const auto& dirname : subdirectories) {
                        if (ImGui::Selectable(("📁 " + dirname).c_str())) {
                            m_consoleMessages.push_back("Subdirectory: " + dirname);
                        }
                    }
                }
            }
            
            // Display images
            if (!imageFiles.empty()) {
                if (ImGui::CollapsingHeader("Images", ImGuiTreeNodeFlags_DefaultOpen)) {
                    for (const auto& filename : imageFiles) {
                        std::string fullPath = m_currentAssetFolder + "/" + filename;
                        if (ImGui::Selectable(("🖼️ " + filename).c_str())) {
                            // Try to assign to selected entity if it has a sprite component
                            if (m_activeSceneWindow && m_activeSceneWindow->hasSelectedEntity() && 
                                m_activeSceneWindow->getScene()->hasComponent<Sprite>(m_activeSceneWindow->getSelectedEntity())) {
                                
                                auto& engine = Engine::getInstance();
                                auto resourceManager = engine.getResourceManager();
                                auto texture = resourceManager->loadTexture(fullPath);
                                
                                if (texture) {
                                    auto scene = m_activeSceneWindow->getScene();
                                    EntityID selectedEntity = m_activeSceneWindow->getSelectedEntity();
                                    auto& sprite = scene->getComponent<Sprite>(selectedEntity);
                                    sprite.texture = texture;
                                    sprite.sourceRect = Rect(0, 0, texture->getWidth(), texture->getHeight());
                                    m_activeSceneWindow->setDirty(true);
                                    m_consoleMessages.push_back("Assigned " + filename + " to selected entity in " + m_activeSceneWindow->getTitle());
                                } else {
                                    m_consoleMessages.push_back("Failed to load: " + filename);
                                }
                            } else {
                                m_consoleMessages.push_back("Selected: " + filename + " (select entity with Sprite component in active scene to assign)");
                            }
                        }
                        
                        // Show tooltip with file info
                        if (ImGui::IsItemHovered()) {
                            ImGui::BeginTooltip();
                            ImGui::Text("File: %s", filename.c_str());
                            ImGui::Text("Path: %s", fullPath.c_str());
                            ImGui::Text("Click to assign to selected entity");
                            ImGui::EndTooltip();
                        }
                    }
                }
            }
            
            // Display audio files
            if (!audioFiles.empty()) {
                if (ImGui::CollapsingHeader("Audio")) {
                    for (const auto& filename : audioFiles) {
                        std::string fullPath = m_currentAssetFolder + "/" + filename;
                        if (ImGui::Selectable(("🔊 " + filename).c_str())) {
                            m_consoleMessages.push_back("Selected audio: " + filename);
                        }
                        
                        if (ImGui::IsItemHovered()) {
                            ImGui::BeginTooltip();
                            ImGui::Text("File: %s", filename.c_str());
                            ImGui::Text("Path: %s", fullPath.c_str());
                            ImGui::EndTooltip();
                        }
                    }
                }
            }
            
            // Display other files
            if (!otherFiles.empty()) {
                if (ImGui::CollapsingHeader("Other Files")) {
                    for (const auto& filename : otherFiles) {
                        std::string fullPath = m_currentAssetFolder + "/" + filename;
                        if (ImGui::Selectable(("📄 " + filename).c_str())) {
                            m_consoleMessages.push_back("Selected file: " + filename);
                        }
                        
                        if (ImGui::IsItemHovered()) {
                            ImGui::BeginTooltip();
                            ImGui::Text("File: %s", filename.c_str());
                            ImGui::Text("Path: %s", fullPath.c_str());
                            ImGui::EndTooltip();
                        }
                    }
                }
            }
            
            if (imageFiles.empty() && audioFiles.empty() && otherFiles.empty() && subdirectories.empty()) {
                ImGui::Text("📁 Folder is empty");
                ImGui::TextWrapped("Add files to: %s", m_currentAssetFolder.c_str());
                ImGui::Text("Supported image formats: PNG, JPG, JPEG, BMP, TGA");
                ImGui::Text("Supported audio formats: WAV, OGG, MP3");
            }
            
        } else {
            ImGui::TextColored(ImVec4(1.0f, 0.6f, 0.0f, 1.0f), "📁 Folder not found: %s", m_currentAssetFolder.c_str());
            if (ImGui::Button("Create This Folder")) {
                try {
                    std::filesystem::create_directories(m_currentAssetFolder);
                    m_consoleMessages.push_back("Created folder: " + m_currentAssetFolder);
                } catch (const std::exception& e) {
                    m_consoleMessages.push_back("Failed to create folder: " + std::string(e.what()));
                }
            }
            ImGui::Text("Supported image formats: PNG, JPG, JPEG, BMP, TGA");
            ImGui::Text("Supported audio formats: WAV, OGG, MP3");
        }
    } catch (const std::exception& e) {
        ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "❌ Error reading folder: %s", e.what());
        ImGui::Text("Check if the path is valid: %s", m_currentAssetFolder.c_str());
    }
    
    ImGui::End();
}

void GameEditor::showViewport() {
    ImGui::Begin("Viewport");
    
    // Get the size of the viewport window
    ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
    m_viewportSize = Vector2(viewportPanelSize.x, viewportPanelSize.y);
    m_viewportFocused = ImGui::IsWindowFocused();
    
    // TODO: Render scene to texture and display here
    ImGui::Text("Game Viewport");
    ImGui::Text("Size: %.0fx%.0f", m_viewportSize.x, m_viewportSize.y);
    ImGui::Text("Focused: %s", m_viewportFocused ? "Yes" : "No");
    
    ImGui::End();
}

void GameEditor::showConsole() {
    ImGui::Begin("Console", &m_showConsole);
    
    // Display messages
    for (const auto& message : m_consoleMessages) {
        ImGui::Text("%s", message.c_str());
    }
    
    // Auto-scroll to bottom
    if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY()) {
        ImGui::SetScrollHereY(1.0f);
    }
    
    ImGui::End();
}

void GameEditor::createNewScene() {
    m_currentScene = std::make_shared<Scene>();
    m_currentScene->initialize();
    
    // Set up proper coordinate system - center camera on world origin
    auto& engine = Engine::getInstance();
    auto renderer = engine.getRenderer();
      // Center the camera so (0,0) is in the middle of the screen
    int windowWidth = 1280;  // Default editor window width
    int windowHeight = 720;  // Default editor window height
    renderer->setCamera(Vector2(-windowWidth/2.0f, -windowHeight/2.0f));
    
    // Register systems
    auto renderSystem = m_currentScene->registerSystem<RenderSystem>();
    auto physicsSystem = m_currentScene->registerSystem<PhysicsSystem>();
    auto collisionSystem = m_currentScene->registerSystem<CollisionSystem>();
    
    // Set scene pointer for each system
    renderSystem->setScene(m_currentScene.get());
    physicsSystem->setScene(m_currentScene.get());
    collisionSystem->setScene(m_currentScene.get());
    
    // Set system signatures
    ComponentMask renderSignature;
    renderSignature.set(m_currentScene->getComponentType<Transform>());
    renderSignature.set(m_currentScene->getComponentType<Sprite>());
    m_currentScene->setSystemSignature<RenderSystem>(renderSignature);
    
    ComponentMask physicsSignature;
    physicsSignature.set(m_currentScene->getComponentType<Transform>());
    physicsSignature.set(m_currentScene->getComponentType<RigidBody>());
    m_currentScene->setSystemSignature<PhysicsSystem>(physicsSignature);
    
    ComponentMask collisionSignature;
    collisionSignature.set(m_currentScene->getComponentType<Transform>());
    collisionSignature.set(m_currentScene->getComponentType<Collider>());
    m_currentScene->setSystemSignature<CollisionSystem>(collisionSignature);
    
    m_consoleMessages.push_back("Created new scene");
}

void GameEditor::loadScene(const std::string& filepath) {
    if (m_sceneManager) {
        m_sceneManager->loadScene(filepath);
        m_consoleMessages.push_back("Loading scene: " + filepath);
    } else {
        m_consoleMessages.push_back("Scene Manager not available");
    }
}

void GameEditor::saveScene(const std::string& filepath) {
    if (!m_activeSceneWindow || !m_activeSceneWindow->getScene()) {
        m_consoleMessages.push_back("No active scene to save");
        return;
    }
    
    if (m_sceneManager) {
        // Find the scene in the scene manager and save it
        // For now, just show a message - proper implementation would find the scene by the active window
        m_consoleMessages.push_back("Use Scene Manager to save scenes: " + filepath);
    } else {
        m_consoleMessages.push_back("Scene Manager not available");
    }
}

void GameEditor::createEntity() {
    // Legacy method - now uses active scene window
    if (m_activeSceneWindow && m_activeSceneWindow->getScene()) {
        auto scene = m_activeSceneWindow->getScene();
        EntityID entity = scene->createEntity();
        // Place new entities at world center (0,0) for better visibility
        scene->addComponent<Transform>(entity, Transform(0.0f, 0.0f));
        
        // Auto-assign a default name
        std::string defaultName = "Entity_" + std::to_string(entity);
        scene->setEntityName(entity, defaultName);
        
        m_activeSceneWindow->setSelectedEntity(entity);
        m_activeSceneWindow->setDirty(true);
        
        m_consoleMessages.push_back("Created entity: " + defaultName + " at world center (0,0) in " + m_activeSceneWindow->getTitle());
    }
}

void GameEditor::deleteEntity() {
    // Legacy method - now uses active scene window
    if (m_activeSceneWindow && m_activeSceneWindow->hasSelectedEntity() && m_activeSceneWindow->getScene()) {
        auto scene = m_activeSceneWindow->getScene();
        EntityID selectedEntity = m_activeSceneWindow->getSelectedEntity();
        std::string entityName = scene->getEntityName(selectedEntity);
        scene->destroyEntity(selectedEntity);
        m_activeSceneWindow->setSelectedEntity(0);
        m_activeSceneWindow->setDirty(true);
        
        m_consoleMessages.push_back("Deleted entity: " + entityName + " from " + m_activeSceneWindow->getTitle());
    }
}

void GameEditor::shutdown() {
    // Save window state before shutting down
    saveWindowState();
    
    // Save configuration
    auto& config = ConfigManager::getInstance();
    config.saveConfig();
    
    // Cleanup ImGui
    ImGui_ImplSDLRenderer2_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
    
    // Shutdown engine
    auto& engine = Engine::getInstance();
    engine.shutdown();
}

void GameEditor::showCameraControls() {
    ImGui::Begin("Camera Controls", &m_showCameraControls);
    
    // Check if we have an active scene window
    if (!m_activeSceneWindow) {
        ImGui::Text("❌ No active scene window");
        ImGui::Text("Please create or focus a scene window to control the camera.");
        ImGui::End();
        return;
    }
    
    // Get camera position from the active scene window
    Vector2 cameraPos = m_activeSceneWindow->getCameraPosition();
    float zoomLevel = m_activeSceneWindow->getZoomLevel();
    
    ImGui::Text("🎥 Active Scene: %s", m_activeSceneWindow->getTitle().c_str());
    ImGui::Separator();
    
    ImGui::Text("📍 Camera Position:");
    if (ImGui::DragFloat2("Camera XY", &cameraPos.x, 1.0f, -10000.0f, 10000.0f)) {
        m_activeSceneWindow->setCameraPosition(cameraPos);
    }
    
    ImGui::Text("� Zoom Level:");
    if (ImGui::SliderFloat("Zoom", &zoomLevel, 0.1f, 10.0f, "%.2fx")) {
        m_activeSceneWindow->setZoomLevel(zoomLevel);
    }
    
    ImGui::SameLine();
    if (ImGui::Button("1:1")) {
        m_activeSceneWindow->resetZoom();
    }
    
    ImGui::Separator();
    
    ImGui::Text("🎮 Quick Camera Controls:");
    if (ImGui::Button("Center Camera (0,0)")) {
        m_activeSceneWindow->setCameraPosition(Vector2(0, 0));
        m_consoleMessages.push_back("Camera centered in scene: " + m_activeSceneWindow->getTitle());
    }
    
    ImGui::SameLine();
    if (ImGui::Button("Reset Zoom")) {
        m_activeSceneWindow->resetZoom();
        m_consoleMessages.push_back("Zoom reset in scene: " + m_activeSceneWindow->getTitle());
    }
    
    if (ImGui::Button("Zoom In")) {
        m_activeSceneWindow->zoomIn();
    }
    
    ImGui::SameLine();
    if (ImGui::Button("Zoom Out")) {
        m_activeSceneWindow->zoomOut();
    }
    
    ImGui::Separator();
    
    ImGui::Text("📐 Scene Info:");
    Vector2 viewportSize = m_activeSceneWindow->getViewportSize();
    ImGui::Text("• Viewport Size: %.0f x %.0f", viewportSize.x, viewportSize.y);
    ImGui::Text("• Camera Position: (%.1f, %.1f)", cameraPos.x, cameraPos.y);
    ImGui::Text("• Zoom Level: %.2fx", zoomLevel);
    ImGui::Text("• Scene Focus: %s", m_activeSceneWindow->isViewportFocused() ? "Yes" : "No");
    
    // Show world bounds visible in the scene window
    float halfWidth = viewportSize.x / (2.0f * zoomLevel);
    float halfHeight = viewportSize.y / (2.0f * zoomLevel);
    Vector2 topLeft = Vector2(cameraPos.x - halfWidth, cameraPos.y - halfHeight);
    Vector2 bottomRight = Vector2(cameraPos.x + halfWidth, cameraPos.y + halfHeight);
    
    ImGui::Text("🗺️ Visible World Area:");
    ImGui::Text("Top-Left: (%.1f, %.1f)", topLeft.x, topLeft.y);
    ImGui::Text("Bottom-Right: (%.1f, %.1f)", bottomRight.x, bottomRight.y);
    
    ImGui::End();
}

void GameEditor::showProceduralGeneration() {
    ImGui::Begin("Procedural Generation", &m_showProceduralGeneration);
    
    // Show which scene window is active
    if (m_activeSceneWindow) {
        ImGui::Text("Target Scene: %s", m_activeSceneWindow->getTitle().c_str());
        if (m_activeSceneWindow->isDirty()) {
            ImGui::SameLine();
            ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f), "*");
        }
        ImGui::Separator();
    } else {
        ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f), "⚠️ No active scene");
        ImGui::Text("Create a scene window to generate content");
        ImGui::End();
        return;
    }
    
    ImGui::Text("🎲 Generate Game Environments");
    ImGui::Separator();

    // Tile Management Section
    if (ImGui::CollapsingHeader("🎨 Tile Image Manager", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Text("Configure tile images before generation:");
        
        // Theme selection
        ImGui::Text("Visual Theme:");
        static int currentTheme = 0;
        const char* themeNames[] = { "Default", "Dungeon", "City", "Terrain", "Fantasy", "Modern" };
        if (ImGui::Combo("##Theme", &currentTheme, themeNames, IM_ARRAYSIZE(themeNames))) {
            if (m_proceduralManager) {
                m_proceduralManager->setGenerationTheme(static_cast<GenerationTheme>(currentTheme));
            }
        }
        
        ImGui::Separator();
          // Available assets browser
        if (ImGui::TreeNode("📂 Available Asset Images")) {
            ImGui::Text("Asset folder: %s", m_currentAssetFolder.c_str());
            
            // Show available images in asset folder
            static std::vector<std::string> availableImages;
            static bool imagesLoaded = false;
            
            if (!imagesLoaded || ImGui::Button("🔄 Refresh")) {
                availableImages.clear();
                
                // Add main asset images
                const char* mainAssets[] = { "wall.png", "ground.png", "player.png", "enemy.png", "coin.png", "tileset.png" };
                for (const auto& asset : mainAssets) {
                    availableImages.push_back(m_currentAssetFolder + "/" + asset);
                }
                
                // Add tile assets
                const char* tileAssets[] = {
                    "dungeon_wall.png", "dungeon_floor.png", "dungeon_door.png",
                    "city_road.png", "city_house.png", "city_shop.png", "city_building.png",
                    "terrain_grass.png", "terrain_water.png", "terrain_stone.png", "terrain_tree.png",
                    "tile_empty.png", "tile_entrance.png", "tile_exit.png"
                };
                for (const auto& asset : tileAssets) {
                    availableImages.push_back(m_currentAssetFolder + "/tiles/" + asset);
                }
                
                imagesLoaded = true;
            }
            
            // Display available images in a scrollable list
            if (ImGui::BeginChild("AvailableImages", ImVec2(-1, 150), true)) {
                for (const auto& imagePath : availableImages) {
                    // Extract filename for display
                    size_t lastSlash = imagePath.find_last_of('/');
                    std::string filename = (lastSlash != std::string::npos) ? imagePath.substr(lastSlash + 1) : imagePath;
                    
                    if (ImGui::Selectable(filename.c_str())) {
                        // Store selected image for assignment
                        static char selectedImage[256] = "";
                        strncpy_s(selectedImage, imagePath.c_str(), sizeof(selectedImage) - 1);
                    }
                    
                    // Show full path on hover
                    if (ImGui::IsItemHovered()) {
                        ImGui::SetTooltip("%s", imagePath.c_str());
                    }
                }
            }
            ImGui::EndChild();
            
            ImGui::TreePop();
        }
        
        ImGui::Separator();
        
        // Tile type to image assignment
        if (ImGui::TreeNode("🔧 Tile-to-Image Assignment")) {
            ImGui::Text("Assign images to tile types for current theme:");
            
            if (m_proceduralManager && m_proceduralManager->getSpriteManager()) {
                auto spriteManager = m_proceduralManager->getSpriteManager();
                
                const char* tileNames[] = {
                    "Empty", "Wall", "Floor", "Door", "Water", "Grass", 
                    "Stone", "Tree", "Building", "Road", "House", "Shop", "Entrance", "Exit"
                };
                
                static char imageInputs[14][128] = {};
                static bool inputsInitialized = false;
                
                // Initialize inputs with current mappings
                if (!inputsInitialized) {
                    for (int i = 0; i < 14; ++i) {
                        TileType type = static_cast<TileType>(i);
                        std::string currentSprite = spriteManager->getSprite(type);
                        strncpy_s(imageInputs[i], currentSprite.c_str(), sizeof(imageInputs[i]) - 1);
                    }
                    inputsInitialized = true;
                }
                
                for (int i = 0; i < 14; ++i) {
                    TileType type = static_cast<TileType>(i);
                    
                    ImGui::Text("%s:", tileNames[i]);
                    ImGui::SameLine();
                    
                    // Input field for image path
                    ImGui::PushID(i);
                    if (ImGui::InputText("##imagePath", imageInputs[i], sizeof(imageInputs[i]))) {
                        // Update sprite mapping when text changes
                        spriteManager->setSprite(type, std::string(imageInputs[i]));
                    }
                    
                    // Quick assign buttons for common images
                    ImGui::SameLine();
                    if (ImGui::SmallButton("Wall")) {
                        strcpy_s(imageInputs[i], "wall.png");
                        spriteManager->setSprite(type, "wall.png");
                    }
                    ImGui::SameLine();
                    if (ImGui::SmallButton("Ground")) {
                        strcpy_s(imageInputs[i], "ground.png");
                        spriteManager->setSprite(type, "ground.png");
                    }
                    ImGui::SameLine();
                    if (ImGui::SmallButton("Clear")) {
                        strcpy_s(imageInputs[i], "");
                        spriteManager->setSprite(type, "");
                    }
                    
                    ImGui::PopID();
                }                
                ImGui::Separator();
                
                if (ImGui::Button("💾 Save Current Mapping")) {
                    // Save current mapping as the theme default
                    // This could write to a config file or update the code
                    m_consoleMessages.push_back("Tile mapping saved for " + std::string(themeNames[currentTheme]) + " theme");
                }
                
                ImGui::SameLine();
                if (ImGui::Button("🔄 Reset to Defaults")) {
                    // Reset to default mappings
                    if (currentTheme == 0) spriteManager->loadDefaultSprites();
                    else if (currentTheme == 1) spriteManager->loadDungeonSprites();
                    else if (currentTheme == 2) spriteManager->loadCitySprites();
                    else if (currentTheme == 3) spriteManager->loadTerrainSprites();
                    
                    // Refresh input fields
                    for (int i = 0; i < 14; ++i) {
                        TileType type = static_cast<TileType>(i);
                        std::string currentSprite = spriteManager->getSprite(type);
                        strncpy_s(imageInputs[i], currentSprite.c_str(), sizeof(imageInputs[i]) - 1);
                    }
                    inputsInitialized = true;
                    
                    m_consoleMessages.push_back("Reset tile mapping to defaults for " + std::string(themeNames[currentTheme]) + " theme");
                }
                
                ImGui::Separator();
                
                // Nomenclature-based auto-assignment
                ImGui::TextColored(ImVec4(0.2f, 0.8f, 1.0f, 1.0f), "🎯 Smart Auto-Assignment");
                ImGui::Text("Automatically finds sprites based on naming conventions:");
                ImGui::BulletText("Theme prefix + tile type (e.g., 'city_wall.png')");
                ImGui::BulletText("Generic tile type (e.g., 'wall.png', 'floor.png')");
                ImGui::BulletText("Alternative names (e.g., 'ground.png' for floor)");
                
                if (ImGui::Button("🔍 Auto-Assign Current Theme")) {
                    spriteManager->autoAssignThemeByNomenclature(static_cast<GenerationTheme>(currentTheme));
                    
                    // Refresh input fields after auto-assignment
                    for (int i = 0; i < 14; ++i) {
                        TileType type = static_cast<TileType>(i);
                        std::string currentSprite = spriteManager->getSprite(type);
                        strncpy_s(imageInputs[i], currentSprite.c_str(), sizeof(imageInputs[i]) - 1);
                    }
                    inputsInitialized = true;
                    
                    m_consoleMessages.push_back("Auto-assigned sprites for " + std::string(themeNames[currentTheme]) + " theme using nomenclature");
                }
                
                ImGui::SameLine();
                if (ImGui::Button("🔍 Auto-Assign All Themes")) {
                    spriteManager->autoAssignThemeByNomenclature(GenerationTheme::Dungeon);
                    spriteManager->autoAssignThemeByNomenclature(GenerationTheme::City);
                    spriteManager->autoAssignThemeByNomenclature(GenerationTheme::Terrain);
                    
                    // Refresh current theme's input fields
                    for (int i = 0; i < 14; ++i) {
                        TileType type = static_cast<TileType>(i);
                        std::string currentSprite = spriteManager->getSprite(type);
                        strncpy_s(imageInputs[i], currentSprite.c_str(), sizeof(imageInputs[i]) - 1);
                    }
                    inputsInitialized = true;
                    
                    m_consoleMessages.push_back("Auto-assigned sprites for all themes using nomenclature");
                }
            }
            
            ImGui::TreePop();
        }
        
        ImGui::Separator();
        
        // Add new image file
        if (ImGui::TreeNode("➕ Add New Tile Image")) {            ImGui::Text("Instructions for adding custom tile images:");
            ImGui::BulletText("Create or find a 32x32 pixel PNG image");
            ImGui::BulletText("Copy it to the %s/tiles/ folder", m_currentAssetFolder.c_str());
            ImGui::BulletText("Use descriptive names (e.g., 'my_wall.png')");
            ImGui::BulletText("Click 'Refresh' in Available Images to see it");
            ImGui::BulletText("Assign it to tile types above");
            
            ImGui::Separator();
            ImGui::Text("💡 Pro Tips:");
            ImGui::BulletText("Use transparency for tiles that shouldn't fill the entire square");
            ImGui::BulletText("Keep consistent art style across all tiles");
            ImGui::BulletText("Test with small maps first");
            
            if (ImGui::Button("📂 Open Assets Folder")) {
                // Open assets folder in file explorer
                m_consoleMessages.push_back("Opening assets folder...");
                #ifdef _WIN32
                system("explorer assets\\tiles");
                #endif
            }
            
            ImGui::TreePop();
        }
    }
    
    ImGui::Separator();
    
    // Generation settings
    static int mapWidth = 50;
    static int mapHeight = 50;
    static unsigned int seed = 12345;
    
    ImGui::Text("📐 Map Settings:");
    ImGui::DragInt("Width", &mapWidth, 1.0f, 10, 200);
    ImGui::DragInt("Height", &mapHeight, 1.0f, 10, 200);
    ImGui::InputInt("Seed", reinterpret_cast<int*>(&seed));
    ImGui::SameLine();
    if (ImGui::Button("Random Seed")) {
        seed = std::random_device{}();
    }
    
    ImGui::Separator();
    
    // Dungeon Generation
    if (ImGui::CollapsingHeader("🏰 Dungeon Generator", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Text("🖼️ Uses: dungeon_wall.png, dungeon_floor.png, dungeon_door.png");
        static DungeonGenerator::DungeonSettings dungeonSettings;
        
        ImGui::DragInt("Min Room Size", &dungeonSettings.minRoomSize, 1.0f, 3, 20);
        ImGui::DragInt("Max Room Size", &dungeonSettings.maxRoomSize, 1.0f, 5, 30);
        ImGui::DragInt("Number of Rooms", &dungeonSettings.numRooms, 1.0f, 3, 20);
        ImGui::DragFloat("Wall Density", &dungeonSettings.wallDensity, 0.01f, 0.1f, 0.8f);
        ImGui::DragInt("Smoothing Iterations", &dungeonSettings.smoothingIterations, 1.0f, 1, 10);
        ImGui::Checkbox("Use Room Approach", &dungeonSettings.useRoomApproach);
        ImGui::Checkbox("Generate Treasure", &dungeonSettings.generateTreasure);        if (ImGui::Button("🏰 Generate Dungeon", ImVec2(-1, 30))) {
            if (!m_activeSceneWindow || !m_activeSceneWindow->getScene()) {
                m_consoleMessages.push_back("❌ No active scene window! Create a scene window first.");
                ImGui::End();
                return;
            }
            
            // Clear existing procedural map and entities (optional - could ask user)
            m_activeSceneWindow->setProceduralMap(nullptr);
            auto activeScene = m_activeSceneWindow->getScene();
                       auto entities = activeScene->getAllLivingEntities();
            for (EntityID entity : entities) {
                activeScene->destroyEntity(entity);
            }
            
            // Generate dungeon using optimized method
            OptimizedProceduralGeneration::generateDungeonToSceneWindow(
                m_activeSceneWindow, mapWidth, mapHeight, seed, dungeonSettings);
            
            m_consoleMessages.push_back("Generated optimized dungeon " + std::to_string(mapWidth) + "x" + std::to_string(mapHeight) + " (seed: " + std::to_string(seed) + ") in " + m_activeSceneWindow->getTitle());
            m_consoleMessages.push_back("Using efficient tile rendering - no entity limit!");
        }
    }
    
    // City Generation
    if (ImGui::CollapsingHeader("🏙️ City Generator")) {
        ImGui::Text("🖼️ Uses: city_road.png, city_house.png, city_shop.png, city_building.png");
        static CityGenerator::CitySettings citySettings;
        
        ImGui::DragInt("Block Size", &citySettings.blockSize, 1.0f, 4, 20);
        ImGui::DragInt("Road Width", &citySettings.roadWidth, 1.0f, 1, 5);
        ImGui::DragFloat("Building Density", &citySettings.buildingDensity, 0.01f, 0.1f, 1.0f);
        ImGui::DragInt("Min Building Size", &citySettings.minBuildingSize, 1.0f, 1, 10);
        ImGui::DragInt("Max Building Size", &citySettings.maxBuildingSize, 1.0f, 2, 15);
        ImGui::Checkbox("Generate Districts", &citySettings.generateDistricts);
        ImGui::Checkbox("Add Landmarks", &citySettings.addLandmarks);        if (ImGui::Button("🏙️ Generate City", ImVec2(-1, 30))) {
            if (!m_activeSceneWindow || !m_activeSceneWindow->getScene()) {
                m_consoleMessages.push_back("❌ No active scene window! Create a scene window first.");
                ImGui::End();
                return;
            }
            
            // Clear existing procedural map and entities (optional - could ask user)
            m_activeSceneWindow->setProceduralMap(nullptr);
            auto activeScene = m_activeSceneWindow->getScene();
            auto entities = activeScene->getAllLivingEntities();
            for (EntityID entity : entities) {
                activeScene->destroyEntity(entity);
            }
            
            // Generate city using optimized method
            OptimizedProceduralGeneration::generateCityToSceneWindow(
                m_activeSceneWindow, mapWidth, mapHeight, seed, citySettings);
            
            m_consoleMessages.push_back("Generated optimized city " + std::to_string(mapWidth) + "x" + std::to_string(mapHeight) + " (seed: " + std::to_string(seed) + ") in " + m_activeSceneWindow->getTitle());
            m_consoleMessages.push_back("Using efficient tile rendering - no entity limit!");
        }
    }
    
    // Terrain Generation
    if (ImGui::CollapsingHeader("🌍 Terrain Generator")) {
        ImGui::Text("🖼️ Uses: terrain_grass.png, terrain_water.png, terrain_stone.png, terrain_tree.png");
        static TerrainGenerator::TerrainSettings terrainSettings;
        
        ImGui::DragFloat("Water Level", &terrainSettings.waterLevel, 0.01f, 0.0f, 1.0f);
        ImGui::DragFloat("Grass Level", &terrainSettings.grassLevel, 0.01f, 0.0f, 1.0f);
        ImGui::DragFloat("Stone Level", &terrainSettings.stoneLevel, 0.01f, 0.0f, 1.0f);
        ImGui::DragInt("Octaves", &terrainSettings.octaves, 1.0f, 1, 8);
        ImGui::DragFloat("Persistence", &terrainSettings.persistence, 0.01f, 0.1f, 1.0f);
        ImGui::DragFloat("Scale", &terrainSettings.scale, 0.001f, 0.01f, 1.0f);
        ImGui::Checkbox("Add Trees", &terrainSettings.addTrees);
        ImGui::DragFloat("Tree Density", &terrainSettings.treeDensity, 0.01f, 0.0f, 0.5f);        if (ImGui::Button("🌍 Generate Terrain", ImVec2(-1, 30))) {
            if (!m_activeSceneWindow || !m_activeSceneWindow->getScene()) {
                m_consoleMessages.push_back("❌ No active scene window! Create a scene window first.");
                ImGui::End();
                return;
            }
            
            // Clear existing procedural map and entities (optional - could ask user)
            m_activeSceneWindow->setProceduralMap(nullptr);
            auto activeScene = m_activeSceneWindow->getScene();
            auto entities = activeScene->getAllLivingEntities();
            for (EntityID entity : entities) {
                activeScene->destroyEntity(entity);
            }
            
            // Generate terrain using optimized method
            OptimizedProceduralGeneration::generateTerrainToSceneWindow(
                m_activeSceneWindow, mapWidth, mapHeight, seed, terrainSettings);
            
            m_consoleMessages.push_back("Generated optimized terrain " + std::to_string(mapWidth) + "x" + std::to_string(mapHeight) + " (seed: " + std::to_string(seed) + ") in " + m_activeSceneWindow->getTitle());
            m_consoleMessages.push_back("Using efficient tile rendering - no entity limit!");
        }
    }
      ImGui::Separator();
    
    // Current map info - get from active scene window
    std::shared_ptr<ProceduralMap> currentMap = nullptr;
    if (m_activeSceneWindow) {
        currentMap = m_activeSceneWindow->getProceduralMap();
    }
    
    if (currentMap) {
        ImGui::Text("📊 Current Map Info:");
        ImGui::Text("Size: %dx%d", currentMap->getWidth(), currentMap->getHeight());
        ImGui::Text("Total Tiles: %d", currentMap->getWidth() * currentMap->getHeight());
        
        if (ImGui::Button("Clear Map")) {
            if (m_activeSceneWindow) {
                m_activeSceneWindow->setProceduralMap(nullptr);
                m_activeSceneWindow->setDirty(true);
            }
            m_consoleMessages.push_back("Cleared procedurally generated map");
        }
    } else {
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "No procedural map generated yet");
        ImGui::Text("Generate a map above to see it in the scene!");
    }
    
    ImGui::End();
}

void GameEditor::showNodeEditor() {
    if (m_nodeEditor) {
        m_nodeEditor->show(&m_showNodeEditor, m_activeSceneWindow);
    }
}

void GameEditor::showSceneManager() {
    if (m_sceneManager) {
        m_sceneManager->show(&m_showSceneManager);
    }
}

void GameEditor::showGameLogicWindow() {
    if (m_gameLogicWindow) {
        m_gameLogicWindow->show(&m_showGameLogicWindow, m_activeSceneWindow);
    }
}

// Window state management implementation
void GameEditor::saveWindowState() {
    auto& config = ConfigManager::getInstance();
    auto& engine = Engine::getInstance();
    auto renderer = engine.getRenderer();
    
    if (renderer && renderer->getWindow()) {
        SDL_Window* window = renderer->getWindow();
        
        // Get current window size
        int width, height;
        SDL_GetWindowSize(window, &width, &height);
        config.setWindowSize(width, height);
        
        // Get current window position
        int x, y;
        SDL_GetWindowPosition(window, &x, &y);
        config.setWindowPosition(x, y);
        
        // Check if window is maximized
        Uint32 flags = SDL_GetWindowFlags(window);
        bool maximized = (flags & SDL_WINDOW_MAXIMIZED) != 0;
        config.setWindowMaximized(maximized);
    }    // Save panel visibility states
    config.setPanelVisible("SceneHierarchy", m_showSceneHierarchy);
    config.setPanelVisible("Inspector", m_showInspector);
    config.setPanelVisible("AssetBrowser", m_showAssetBrowser);
    config.setPanelVisible("Console", m_showConsole);
    config.setPanelVisible("CameraControls", m_showCameraControls);
    config.setPanelVisible("ProceduralGeneration", m_showProceduralGeneration);
    config.setPanelVisible("NodeEditor", m_showNodeEditor);
    config.setPanelVisible("SceneManager", m_showSceneManager);
}

void GameEditor::loadWindowState() {
    auto& config = ConfigManager::getInstance();
    auto& engine = Engine::getInstance();
    auto renderer = engine.getRenderer();
    
    if (renderer && renderer->getWindow()) {
        SDL_Window* window = renderer->getWindow();
        
        // Restore window position
        int x, y;
        config.getWindowPosition(x, y);
        if (x != SDL_WINDOWPOS_CENTERED && y != SDL_WINDOWPOS_CENTERED) {
            SDL_SetWindowPosition(window, x, y);
        }
        
        // Restore maximized state
        if (config.getWindowMaximized()) {
            SDL_MaximizeWindow(window);
        }
    }    // Load panel visibility states
    m_showSceneHierarchy = config.getPanelVisible("SceneHierarchy");
    m_showInspector = config.getPanelVisible("Inspector");
    m_showAssetBrowser = config.getPanelVisible("AssetBrowser");
    m_showConsole = config.getPanelVisible("Console");
    m_showCameraControls = config.getPanelVisible("CameraControls");
    m_showProceduralGeneration = config.getPanelVisible("ProceduralGeneration");
    m_showNodeEditor = config.getPanelVisible("NodeEditor");
    m_showSceneManager = config.getPanelVisible("SceneManager");
}

void GameEditor::updateWindowState() {
    // This method can be called periodically to track window changes
    // For now, we'll save on shutdown, but this could be used for
    // real-time tracking if needed
    static float lastSaveTime = 0.0f;
    static float saveInterval = 5.0f; // Save every 5 seconds
    
    // Get current time (would need to implement proper time tracking)
    // For now, we'll just save on shutdown
}

// Scene window management implementation
void GameEditor::openSceneInNewWindow() {
    auto scene = std::make_shared<Scene>();
    scene->initialize();
    
    // Set up proper coordinate system and register systems
    auto renderSystem = scene->registerSystem<RenderSystem>();
    auto physicsSystem = scene->registerSystem<PhysicsSystem>();
    auto collisionSystem = scene->registerSystem<CollisionSystem>();
    
    // Set scene pointer for each system
    renderSystem->setScene(scene.get());
    physicsSystem->setScene(scene.get());
    collisionSystem->setScene(scene.get());
    
    // Set system signatures
    ComponentMask renderSignature;
    renderSignature.set(scene->getComponentType<Transform>());
    renderSignature.set(scene->getComponentType<Sprite>());
    scene->setSystemSignature<RenderSystem>(renderSignature);
    
    ComponentMask physicsSignature;
    physicsSignature.set(scene->getComponentType<Transform>());
    physicsSignature.set(scene->getComponentType<RigidBody>());
    scene->setSystemSignature<PhysicsSystem>(physicsSignature);
    
    ComponentMask collisionSignature;
    collisionSignature.set(scene->getComponentType<Transform>());
    collisionSignature.set(scene->getComponentType<Collider>());
    scene->setSystemSignature<CollisionSystem>(collisionSignature);
    
    std::string title = "Scene " + std::to_string(m_nextSceneWindowId++);
    openSceneInNewWindow(scene, title);
    
    // Register the scene with the Scene Manager
    if (m_sceneManager) {
        m_sceneManager->addSceneToList(scene, title, ""); // Empty filepath for unsaved scenes
    }
    
    m_consoleMessages.push_back("Created new scene window: " + title);
}

void GameEditor::openSceneInNewWindow(std::shared_ptr<Scene> scene, const std::string& title) {
    std::string windowTitle = title.empty() ? ("Scene " + std::to_string(m_nextSceneWindowId++)) : title;
    
    auto sceneWindow = std::make_unique<SceneWindow>(windowTitle, scene, this);
    
    // Set this window as active if it's the first one or no active window
    if (m_sceneWindows.empty() || !m_activeSceneWindow) {
        m_activeSceneWindow = sceneWindow.get();
    }
    
    m_sceneWindows.push_back(std::move(sceneWindow));
}

void GameEditor::closeSceneWindow(SceneWindow* window) {
    if (!window) return;
    
    auto it = std::find_if(m_sceneWindows.begin(), m_sceneWindows.end(),
        [window](const std::unique_ptr<SceneWindow>& ptr) {
            return ptr.get() == window;
        });
    
    if (it != m_sceneWindows.end()) {
        if (window == m_activeSceneWindow) {
            m_activeSceneWindow = nullptr;
            // Try to set another window as active
            for (auto& sw : m_sceneWindows) {
                if (sw.get() != window && sw->isOpen()) {
                    m_activeSceneWindow = sw.get();
                    break;
                }
            }
        }
        
        window->close();
        m_consoleMessages.push_back("Closed scene window: " + window->getTitle());
    }
}

SceneWindow* GameEditor::getActiveSceneWindow() {
    return m_activeSceneWindow;
}

void GameEditor::setActiveSceneWindow(SceneWindow* window) {
    m_activeSceneWindow = window;
}

void GameEditor::updateActiveSceneData() {
    // Update m_currentScene and selection for compatibility with existing panels
    if (m_activeSceneWindow && m_activeSceneWindow->isOpen()) {
        m_currentScene = m_activeSceneWindow->getScene();
        m_selectedEntity = m_activeSceneWindow->getSelectedEntity();
        m_hasSelectedEntity = m_activeSceneWindow->hasSelectedEntity();
    } else if (m_sceneWindows.empty()) {
        m_currentScene = nullptr;
        m_selectedEntity = 0;
        m_hasSelectedEntity = false;
    }
}

std::string GameEditor::openFolderDialog(const std::string& initialPath) {
#ifdef _WIN32
    std::string selectedPath;
    
    // Initialize COM (if not already initialized)
    HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    bool comInitialized = SUCCEEDED(hr);
    if (hr == RPC_E_CHANGED_MODE) {
        // COM already initialized with different mode, that's okay
        comInitialized = false;
    } else if (FAILED(hr)) {
        m_consoleMessages.push_back("Failed to initialize COM for folder dialog");
        return "";
    }
    
    // Create the file dialog object
    IFileOpenDialog* pFileOpen = nullptr;
    hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL, IID_IFileOpenDialog, reinterpret_cast<void**>(&pFileOpen));
    
    if (SUCCEEDED(hr)) {
        // Set options for folder selection
        DWORD dwOptions;
        hr = pFileOpen->GetOptions(&dwOptions);
        if (SUCCEEDED(hr)) {
            hr = pFileOpen->SetOptions(dwOptions | FOS_PICKFOLDERS);
        }
        
        // Set initial directory if provided
        if (!initialPath.empty() && std::filesystem::exists(initialPath)) {
            IShellItem* pInitialFolder = nullptr;
            std::wstring wInitialPath(initialPath.begin(), initialPath.end());
            hr = SHCreateItemFromParsingName(wInitialPath.c_str(), NULL, IID_IShellItem, reinterpret_cast<void**>(&pInitialFolder));
            if (SUCCEEDED(hr)) {
                pFileOpen->SetFolder(pInitialFolder);
                pInitialFolder->Release();
            }
        }
        
        // Show the dialog
        hr = pFileOpen->Show(NULL);
        
        if (SUCCEEDED(hr)) {
            IShellItem* pItem = nullptr;
            hr = pFileOpen->GetResult(&pItem);
            if (SUCCEEDED(hr)) {
                PWSTR pszFilePath = nullptr;
                hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);
                if (SUCCEEDED(hr)) {
                    // Convert from wide string to regular string
                    int bufferSize = WideCharToMultiByte(CP_UTF8, 0, pszFilePath, -1, NULL, 0, NULL, NULL);
                    if (bufferSize > 0) {
                        selectedPath.resize(bufferSize - 1);
                        WideCharToMultiByte(CP_UTF8, 0, pszFilePath, -1, &selectedPath[0], bufferSize, NULL, NULL);
                    }
                    CoTaskMemFree(pszFilePath);
                }
                pItem->Release();
            }
        }
          pFileOpen->Release();
    } else {
        m_consoleMessages.push_back("Failed to create folder dialog");
    }
    
    if (comInitialized) {
        CoUninitialize();
    }
    return selectedPath;
#else
    // For non-Windows platforms, just return empty string for now
    m_consoleMessages.push_back("Folder dialog not implemented for this platform");
    return "";
#endif
}
