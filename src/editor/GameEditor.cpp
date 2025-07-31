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
#include <fstream>
#include <sstream>

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
    m_nodeEditor = std::make_unique<NodeEditor::NodeEditorWindow>();
    
    // Set up callback for code refresh
    m_nodeEditor->setCodeRefreshCallback([this]() {
        this->loadCodeFiles();
    });
    
    m_sceneManager = std::make_unique<SceneManager>(this);
    
    // Initialize game logic window
    m_gameLogicWindow = std::make_unique<GameLogicWindow>();
    
    // Initialize collision editor window
    m_collisionEditor = std::make_unique<CollisionEditorWindow>(this);
    
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
    
    // Create a default scene window with an empty scene on startup
    auto defaultScene = std::make_shared<Scene>();
    openSceneInNewWindow(defaultScene, "Initial Scene");
    
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
    if (m_showProceduralGeneration) showProceduralGeneration();
    if (m_showNodeEditor) showNodeEditor();
    if (m_showSceneManager) showSceneManager();
    if (m_showGameLogicWindow) showGameLogicWindow();
    if (m_showCollisionEditor) showCollisionEditor();
    if (m_showCodeViewer) showCodeViewer();
    if (m_showDemo) ImGui::ShowDemoWindow(&m_showDemo);
    
    // Render all scene windows
    for (auto& sceneWindow : m_sceneWindows) {
        if (sceneWindow->isOpen()) {
            sceneWindow->render();
        }
    }
    
    // Clean up closed windows
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
            ImGui::MenuItem("Collision Editor", nullptr, &m_showCollisionEditor);
            ImGui::MenuItem("Code Viewer", nullptr, &m_showCodeViewer);
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
    
    bool hasActiveScene = (m_activeSceneWindow && m_activeSceneWindow->isOpen() && m_activeSceneWindow->getScene());
    
    if (ImGui::Button("Create Entity") && hasActiveScene) {
        auto scene = m_activeSceneWindow->getScene();
        m_consoleMessages.push_back("DEBUG: About to create entity...");
        std::cout << "DEBUG: About to create entity..." << std::endl;
        
        EntityID entity = scene->createEntity();
        m_consoleMessages.push_back("DEBUG: Created entity ID: " + std::to_string(entity));
        std::cout << "DEBUG: Created entity ID: " << entity << std::endl;
        
        scene->addComponent<Transform>(entity, Transform(0.0f, 0.0f));
        m_consoleMessages.push_back("DEBUG: Added Transform component");
        std::cout << "DEBUG: Added Transform component" << std::endl;
        
        std::string defaultName = "Entity_" + std::to_string(entity);
        scene->setEntityName(entity, defaultName);
        m_consoleMessages.push_back("DEBUG: Set entity name: " + defaultName);
        std::cout << "DEBUG: Set entity name: " << defaultName << std::endl;
        
        m_activeSceneWindow->setSelectedEntity(entity);
        m_activeSceneWindow->setDirty(true);
        
        // Debug logging to track entity creation
        std::string debugMsg = "Created entity: " + defaultName + " in " + m_activeSceneWindow->getTitle() + 
                              " (Entity ID: " + std::to_string(entity) + ", Window ID: " + std::to_string(reinterpret_cast<uintptr_t>(m_activeSceneWindow)) + ")";
        m_consoleMessages.push_back(debugMsg);
        std::cout << debugMsg << std::endl;
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
                }                if (ImGui::DragFloat2("Scale", &transform.scale.x, 0.01f, 0.01f, 50.0f)) {
                    m_activeSceneWindow->setDirty(true);
                    printf("DEBUG: Scale changed to %.2f, %.2f\n", transform.scale.x, transform.scale.y);
                    
                    // Verify the change was persisted by re-reading from the scene
                    auto& verifyTransform = scene->getComponent<Transform>(selectedEntity);
                    printf("DEBUG: Verified persisted scale: %.2f, %.2f\n", verifyTransform.scale.x, verifyTransform.scale.y);
                    fflush(stdout);
                }
                
                // Scale preset buttons
                ImGui::Text("Quick Scale:");
                if (ImGui::SmallButton("0.1x")) {
                    transform.scale = Vector2(0.1f, 0.1f);
                    m_activeSceneWindow->setDirty(true);
                }
                ImGui::SameLine();
                if (ImGui::SmallButton("0.5x")) {
                    transform.scale = Vector2(0.5f, 0.5f);
                    m_activeSceneWindow->setDirty(true);
                }
                ImGui::SameLine();
                if (ImGui::SmallButton("1x")) {
                    transform.scale = Vector2(1.0f, 1.0f);
                    m_activeSceneWindow->setDirty(true);
                }
                ImGui::SameLine();
                if (ImGui::SmallButton("2x")) {
                    transform.scale = Vector2(2.0f, 2.0f);
                    m_activeSceneWindow->setDirty(true);
                }
                ImGui::SameLine();
                if (ImGui::SmallButton("5x")) {
                    transform.scale = Vector2(5.0f, 5.0f);
                    m_activeSceneWindow->setDirty(true);
                }
                ImGui::SameLine();
                if (ImGui::SmallButton("10x")) {
                    transform.scale = Vector2(10.0f, 10.0f);
                    m_activeSceneWindow->setDirty(true);
                }
                
                // Fine-tuning controls
                ImGui::Separator();
                ImGui::Text("Fine Scale Control:");
                float scaleStep = 0.1f;
                if (ImGui::SmallButton("-0.1")) {
                    float newScaleX = transform.scale.x - scaleStep;
                    float newScaleY = transform.scale.y - scaleStep;
                    transform.scale.x = (newScaleX < 0.01f) ? 0.01f : newScaleX;
                    transform.scale.y = (newScaleY < 0.01f) ? 0.01f : newScaleY;
                    m_activeSceneWindow->setDirty(true);
                }
                ImGui::SameLine();
                if (ImGui::SmallButton("+0.1")) {
                    float newScaleX = transform.scale.x + scaleStep;
                    float newScaleY = transform.scale.y + scaleStep;
                    transform.scale.x = (newScaleX > 50.0f) ? 50.0f : newScaleX;
                    transform.scale.y = (newScaleY > 50.0f) ? 50.0f : newScaleY;
                    m_activeSceneWindow->setDirty(true);
                }
                ImGui::SameLine();
                ImGui::TextDisabled("(?)");
                if (ImGui::IsItemHovered()) {
                    ImGui::SetTooltip("Current scale: %.3fx, %.3fx\\nGrid-fitted sprites are automatically scaled to fit 48x48 grid\\nAdditional transform scale multiplies on top of grid-fitting");
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
        
        if (scene->hasComponent<EntitySpawner>(selectedEntity)) {
            if (ImGui::CollapsingHeader("🏭 Entity Spawner", ImGuiTreeNodeFlags_DefaultOpen)) {
                auto& spawner = scene->getComponent<EntitySpawner>(selectedEntity);
                
                // Spawner status
                ImGui::Text("Spawner Status:");
                ImGui::Checkbox("Can Spawn", &spawner.canSpawn);
                ImGui::SameLine();
                ImGui::Text("Spawned: %d", spawner.spawnCount);
                
                // Spawn settings
                ImGui::DragFloat("Cooldown Time", &spawner.cooldownTime, 0.01f, 0.0f, 10.0f, "%.2fs");
                ImGui::DragInt("Max Spawns", &spawner.maxSpawns, 1, -1, 1000);
                if (ImGui::IsItemHovered()) {
                    ImGui::SetTooltip("-1 = unlimited spawns");
                }
                
                ImGui::DragFloat2("Spawn Direction", &spawner.spawnDirection.x, 0.1f, -1.0f, 1.0f);
                ImGui::Checkbox("Inherit Velocity", &spawner.inheritVelocity);
                
                ImGui::Separator();
                
                // Template management (read-only display)
                ImGui::Text("🎯 Spawn Templates:");
                ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "Templates are managed through the Node Editor");
                
                if (spawner.templates.empty()) {
                    ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "No templates configured");
                    ImGui::TextWrapped("💡 To add templates: Open Node Editor → Connect Entity nodes to EntitySpawner's 'Template' input pin");
                } else {
                    // Template selector (read-only)
                    std::vector<const char*> templateNames;
                    for (const auto& tmpl : spawner.templates) {
                        templateNames.push_back(tmpl.name.c_str());
                    }
                    
                    ImGui::Combo("Selected Template", &spawner.selectedTemplate, 
                                templateNames.data(), static_cast<int>(templateNames.size()));
                    
                    // Show current template info (read-only)
                    if (spawner.selectedTemplate >= 0 && spawner.selectedTemplate < static_cast<int>(spawner.templates.size())) {
                        const auto& tmpl = spawner.templates[spawner.selectedTemplate];
                        
                        ImGui::Separator();
                        ImGui::Text("📝 Template Info (Read-Only):");
                        
                        ImGui::Text("Name: %s", tmpl.name.c_str());
                        
                        if (tmpl.spriteFile.find("TEMPLATE_ENTITY_") == 0) {
                            // Extract entity ID from template reference
                            std::string idStr = tmpl.spriteFile.substr(16);
                            ImGui::Text("Template Entity: Entity %s", idStr.c_str());
                        } else if (!tmpl.spriteFile.empty()) {
                            ImGui::Text("Sprite File: %s", tmpl.spriteFile.c_str());
                        } else {
                            ImGui::Text("Sprite File: (none)");
                        }
                        
                        ImGui::Text("Spawn Offset: (%.1f, %.1f)", tmpl.spawnOffset.x, tmpl.spawnOffset.y);
                        ImGui::Text("Initial Velocity: (%.1f, %.1f)", tmpl.velocity.x, tmpl.velocity.y);
                        ImGui::Text("Life Time: %.1fs %s", tmpl.lifeTime, tmpl.lifeTime == 0.0f ? "(permanent)" : "");
                        ImGui::Text("Scale: %.2f", tmpl.scale);
                        ImGui::Text("Has Collider: %s", tmpl.hasCollider ? "Yes" : "No");
                        ImGui::Text("Has RigidBody: %s", tmpl.hasRigidBody ? "Yes" : "No");
                    }
                }
                
                // Manual spawn button for testing
                ImGui::Separator();
                if (ImGui::Button("Spawn Now") && spawner.canSpawn && !spawner.templates.empty()) {
                    float currentTime = static_cast<float>(SDL_GetTicks()) / 1000.0f;
                    if (spawner.isReady(currentTime)) {
                        spawnEntityFromTemplate(selectedEntity, spawner, currentTime);
                        spawner.updateLastSpawnTime(currentTime);
                        m_activeSceneWindow->setDirty(true);
                    }
                }
                
                // Reset utility function
                ImGui::Separator();
                if (ImGui::Button("Reset Spawn Count")) {
                    spawner.reset();
                    m_activeSceneWindow->setDirty(true);
                }
            }
        }
        
        // ParticleEffect Component
        if (scene->hasComponent<ParticleEffect>(selectedEntity)) {
            if (ImGui::CollapsingHeader("✨ Particle Effect", ImGuiTreeNodeFlags_DefaultOpen)) {
                auto& particleEffect = scene->getComponent<ParticleEffect>(selectedEntity);
                
                // Emission settings
                ImGui::Text("🚀 Emission Settings:");
                ImGui::DragFloat("Emission Rate", &particleEffect.emissionRate, 1.0f, 0.0f, 1000.0f, "%.0f particles/sec");
                ImGui::DragFloat("Burst Count", &particleEffect.burstCount, 1.0f, 0.0f, 1000.0f, "%.0f particles");
                
                // Emission shape
                const char* shapeNames[] = { "Point", "Circle", "Box", "Cone" };
                int currentShape = static_cast<int>(particleEffect.shape);
                if (ImGui::Combo("Emission Shape", &currentShape, shapeNames, 4)) {
                    particleEffect.shape = static_cast<ParticleEffect::EmissionShape>(currentShape);
                    m_activeSceneWindow->setDirty(true);
                }
                
                if (particleEffect.shape == ParticleEffect::EmissionShape::Circle || 
                    particleEffect.shape == ParticleEffect::EmissionShape::Box) {
                    ImGui::DragFloat2("Emission Size", &particleEffect.emissionSize.x, 1.0f, 0.0f, 1000.0f, "%.1f");
                } else if (particleEffect.shape == ParticleEffect::EmissionShape::Cone) {
                    ImGui::DragFloat("Cone Angle", &particleEffect.coneAngle, 1.0f, 0.0f, 360.0f, "%.1f°");
                }
                
                ImGui::Separator();
                
                // Particle properties
                ImGui::Text("⚡ Particle Properties:");
                ImGui::DragFloat("Min Lifetime", &particleEffect.minLifetime, 0.1f, 0.1f, 60.0f, "%.1fs");
                ImGui::DragFloat("Max Lifetime", &particleEffect.maxLifetime, 0.1f, 0.1f, 60.0f, "%.1fs");
                ImGui::DragFloat2("Min Velocity", &particleEffect.minVelocity.x, 1.0f, -1000.0f, 1000.0f, "%.1f");
                ImGui::DragFloat2("Max Velocity", &particleEffect.maxVelocity.x, 1.0f, -1000.0f, 1000.0f, "%.1f");
                
                ImGui::DragFloat("Min Size", &particleEffect.minSize, 0.1f, 0.1f, 100.0f, "%.1f");
                ImGui::DragFloat("Max Size", &particleEffect.maxSize, 0.1f, 0.1f, 100.0f, "%.1f");
                
                ImGui::DragFloat("Min Rotation Speed", &particleEffect.minRotationSpeed, 1.0f, -360.0f, 360.0f, "%.1f°/s");
                ImGui::DragFloat("Max Rotation Speed", &particleEffect.maxRotationSpeed, 1.0f, -360.0f, 360.0f, "%.1f°/s");
                
                ImGui::Separator();
                
                // Visual properties
                ImGui::Text("🎨 Visual Properties:");
                
                // Convert Color to float array for ImGui
                float startColor[4] = {
                    particleEffect.startColor.r / 255.0f,
                    particleEffect.startColor.g / 255.0f,
                    particleEffect.startColor.b / 255.0f,
                    particleEffect.startColor.a / 255.0f
                };
                if (ImGui::ColorEdit4("Start Color", startColor)) {
                    particleEffect.startColor.r = (uint8_t)(startColor[0] * 255);
                    particleEffect.startColor.g = (uint8_t)(startColor[1] * 255);
                    particleEffect.startColor.b = (uint8_t)(startColor[2] * 255);
                    particleEffect.startColor.a = (uint8_t)(startColor[3] * 255);
                    m_activeSceneWindow->setDirty(true);
                }
                
                float endColor[4] = {
                    particleEffect.endColor.r / 255.0f,
                    particleEffect.endColor.g / 255.0f,
                    particleEffect.endColor.b / 255.0f,
                    particleEffect.endColor.a / 255.0f
                };
                if (ImGui::ColorEdit4("End Color", endColor)) {
                    particleEffect.endColor.r = (uint8_t)(endColor[0] * 255);
                    particleEffect.endColor.g = (uint8_t)(endColor[1] * 255);
                    particleEffect.endColor.b = (uint8_t)(endColor[2] * 255);
                    particleEffect.endColor.a = (uint8_t)(endColor[3] * 255);
                    m_activeSceneWindow->setDirty(true);
                }
                
                // Blend mode
                const char* blendModes[] = { "Normal", "Additive", "Multiply" };
                int currentBlend = static_cast<int>(particleEffect.blendMode);
                if (ImGui::Combo("Blend Mode", &currentBlend, blendModes, 3)) {
                    particleEffect.blendMode = static_cast<ParticleEffect::BlendMode>(currentBlend);
                    m_activeSceneWindow->setDirty(true);
                }
                
                ImGui::Separator();
                
                // Physics
                ImGui::Text("🌍 Physics:");
                ImGui::DragFloat2("Gravity", &particleEffect.gravity.x, 1.0f, -1000.0f, 1000.0f, "%.1f");
                
                ImGui::Separator();
                
                // Preset effects
                ImGui::Text("🎯 Quick Presets:");
                if (ImGui::Button("Fire Effect")) {
                    particleEffect.setupFireEffect();
                    m_activeSceneWindow->setDirty(true);
                }
                ImGui::SameLine();
                if (ImGui::Button("Smoke Effect")) {
                    particleEffect.setupSmokeEffect();
                    m_activeSceneWindow->setDirty(true);
                }
                
                if (ImGui::Button("Spark Effect")) {
                    particleEffect.setupSparkEffect();
                    m_activeSceneWindow->setDirty(true);
                }
                ImGui::SameLine();
                if (ImGui::Button("Magic Effect")) {
                    particleEffect.setupMagicEffect();
                    m_activeSceneWindow->setDirty(true);
                }
                
                ImGui::Separator();
                
                // Control buttons
                ImGui::Text("🎮 Controls:");
                if (ImGui::Button("Start/Resume")) {
                    particleEffect.isEmitting = true;
                    m_activeSceneWindow->setDirty(true);
                }
                ImGui::SameLine();
                if (ImGui::Button("Pause")) {
                    particleEffect.isEmitting = false;
                    m_activeSceneWindow->setDirty(true);
                }
                ImGui::SameLine();
                if (ImGui::Button("Clear Particles")) {
                    particleEffect.particles.clear();
                    m_activeSceneWindow->setDirty(true);
                }
                
                // Status info
                ImGui::Text("Status: %s | Active Particles: %d", 
                           particleEffect.isEmitting ? "Running" : "Paused",
                           particleEffect.getActiveParticleCount());
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
        ImGui::SameLine();
        if (ImGui::Button("Add Entity Spawner") && !scene->hasComponent<EntitySpawner>(selectedEntity)) {
            scene->addComponent<EntitySpawner>(selectedEntity, EntitySpawner());
            m_activeSceneWindow->setDirty(true);
        }
        
        if (ImGui::Button("Add Particle Effect") && !scene->hasComponent<ParticleEffect>(selectedEntity)) {
            scene->addComponent<ParticleEffect>(selectedEntity, ParticleEffect());
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
        
        ImGui::Separator();
        
        // Entity Creation Section
        if (ImGui::CollapsingHeader("Entity Creation", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::Text("🏭 Create New Entities:");
            
            static char entityNameBuffer[256] = "New Entity";
            static float spawnPosX = 0.0f;
            static float spawnPosY = 0.0f;
            static bool spawnAtCamera = true;
            
            ImGui::InputText("Entity Name", entityNameBuffer, sizeof(entityNameBuffer));
            
            ImGui::Checkbox("Spawn at Camera Position", &spawnAtCamera);
            
            if (!spawnAtCamera) {
                ImGui::DragFloat2("Spawn Position", &spawnPosX, 1.0f, -1000.0f, 1000.0f);
            }
            
            ImGui::Separator();
            
            // Basic entity creation
            if (ImGui::Button("Create Empty Entity", ImVec2(-1, 0))) {
                if (scene) {
                    EntityID newEntity = scene->createEntity();
                    scene->setEntityName(newEntity, std::string(entityNameBuffer));
                    
                    Vector2 spawnPos = spawnAtCamera ? m_activeSceneWindow->getCameraPosition() : Vector2(spawnPosX, spawnPosY);
                    scene->addComponent<Transform>(newEntity, Transform(spawnPos));
                    
                    m_activeSceneWindow->applyGridFittingToEntity(newEntity);
                    m_activeSceneWindow->setSelectedEntity(newEntity);
                    m_activeSceneWindow->setDirty(true);
                    
                    m_consoleMessages.push_back("✅ Created empty entity: " + std::string(entityNameBuffer));
                }
            }
            
            // Pre-configured entity types
            ImGui::Text("📦 Pre-configured Templates:");
            
            if (ImGui::Button("Create Sprite Entity", ImVec2(-1, 0))) {
                if (scene) {
                    m_consoleMessages.push_back("DEBUG: Creating sprite entity...");
                    
                    EntityID newEntity = scene->createEntity();
                    m_consoleMessages.push_back("DEBUG: Created entity ID: " + std::to_string(newEntity));
                    
                    scene->setEntityName(newEntity, std::string(entityNameBuffer) + "_Sprite");
                    m_consoleMessages.push_back("DEBUG: Set entity name");
                    
                    Vector2 spawnPos = spawnAtCamera ? m_activeSceneWindow->getCameraPosition() : Vector2(spawnPosX, spawnPosY);
                    scene->addComponent<Transform>(newEntity, Transform(spawnPos));
                    m_consoleMessages.push_back("DEBUG: Added Transform component");
                    
                    // Add sprite component
                    Sprite sprite;
                    sprite.visible = true;
                    sprite.layer = 0;
                    scene->addComponent<Sprite>(newEntity, sprite);
                    m_consoleMessages.push_back("DEBUG: Added Sprite component");
                    
                    m_activeSceneWindow->applyGridFittingToEntity(newEntity);
                    m_activeSceneWindow->setSelectedEntity(newEntity);
                    m_activeSceneWindow->setDirty(true);
                    
                    m_consoleMessages.push_back("✅ Created sprite entity: " + std::string(entityNameBuffer) + "_Sprite");
                }
            }
            
            if (ImGui::Button("Create Player Entity", ImVec2(-1, 0))) {
                if (scene) {
                    m_consoleMessages.push_back("DEBUG: Creating player entity...");
                    
                    EntityID newEntity = scene->createEntity();
                    m_consoleMessages.push_back("DEBUG: Created entity ID: " + std::to_string(newEntity));
                    
                    scene->setEntityName(newEntity, std::string(entityNameBuffer) + "_Player");
                    m_consoleMessages.push_back("DEBUG: Set entity name");
                    
                    Vector2 spawnPos = spawnAtCamera ? m_activeSceneWindow->getCameraPosition() : Vector2(spawnPosX, spawnPosY);
                    scene->addComponent<Transform>(newEntity, Transform(spawnPos));
                    m_consoleMessages.push_back("DEBUG: Added Transform component");
                    
                    // Add all player components
                    Sprite sprite;
                    sprite.visible = true;
                    sprite.layer = 1;
                    scene->addComponent<Sprite>(newEntity, sprite);
                    m_consoleMessages.push_back("DEBUG: Added Sprite component");
                    
                    scene->addComponent<Collider>(newEntity, Collider());
                    m_consoleMessages.push_back("DEBUG: Added Collider component");
                    
                    scene->addComponent<RigidBody>(newEntity, RigidBody());
                    m_consoleMessages.push_back("DEBUG: Added RigidBody component");
                    
                    scene->addComponent<PlayerController>(newEntity, PlayerController());
                    m_consoleMessages.push_back("DEBUG: Added PlayerController component");
                    
                    scene->addComponent<PlayerStats>(newEntity, PlayerStats());
                    m_consoleMessages.push_back("DEBUG: Added PlayerStats component");
                    
                    m_activeSceneWindow->applyGridFittingToEntity(newEntity);
                    m_activeSceneWindow->setSelectedEntity(newEntity);
                    m_activeSceneWindow->setDirty(true);
                    
                    m_consoleMessages.push_back("✅ Created player entity: " + std::string(entityNameBuffer) + "_Player");
                }
            }
            
            if (ImGui::Button("Create Enemy Entity", ImVec2(-1, 0))) {
                if (scene) {
                    EntityID newEntity = scene->createEntity();
                    scene->setEntityName(newEntity, std::string(entityNameBuffer) + "_Enemy");
                    
                    Vector2 spawnPos = spawnAtCamera ? m_activeSceneWindow->getCameraPosition() : Vector2(spawnPosX, spawnPosY);
                    scene->addComponent<Transform>(newEntity, Transform(spawnPos));
                    
                    // Add enemy components
                    Sprite sprite;
                    sprite.visible = true;
                    sprite.layer = 1;
                    scene->addComponent<Sprite>(newEntity, sprite);
                    scene->addComponent<Collider>(newEntity, Collider());
                    scene->addComponent<RigidBody>(newEntity, RigidBody());
                    scene->addComponent<PlayerStats>(newEntity, PlayerStats()); // Can be used for enemy stats too
                    
                    m_activeSceneWindow->applyGridFittingToEntity(newEntity);
                    m_activeSceneWindow->setSelectedEntity(newEntity);
                    m_activeSceneWindow->setDirty(true);
                    
                    m_consoleMessages.push_back("✅ Created enemy entity: " + std::string(entityNameBuffer) + "_Enemy");
                }
            }
            
            ImGui::Separator();
            
            // Entity duplication from selected
            if (m_activeSceneWindow->hasSelectedEntity()) {
                EntityID selectedEntity = m_activeSceneWindow->getSelectedEntity();
                std::string selectedName = scene->getEntityName(selectedEntity);
                
                if (ImGui::Button(("Duplicate Selected Entity (" + selectedName + ")").c_str(), ImVec2(-1, 0))) {
                    EntityID newEntity = scene->createEntity();
                    scene->setEntityName(newEntity, selectedName + "_Copy");
                    
                    // Copy transform component
                    if (scene->hasComponent<Transform>(selectedEntity)) {
                        auto& originalTransform = scene->getComponent<Transform>(selectedEntity);
                        Transform newTransform = originalTransform;
                        newTransform.position.x += 50.0f; // Offset to avoid overlapping
                        scene->addComponent<Transform>(newEntity, newTransform);
                    }
                    
                    // Copy sprite component
                    if (scene->hasComponent<Sprite>(selectedEntity)) {
                        auto& originalSprite = scene->getComponent<Sprite>(selectedEntity);
                        scene->addComponent<Sprite>(newEntity, originalSprite);
                    }
                    
                    // Copy collider component
                    if (scene->hasComponent<Collider>(selectedEntity)) {
                        auto& originalCollider = scene->getComponent<Collider>(selectedEntity);
                        scene->addComponent<Collider>(newEntity, originalCollider);
                    }
                    
                    // Copy rigidbody component
                    if (scene->hasComponent<RigidBody>(selectedEntity)) {
                        auto& originalRigidBody = scene->getComponent<RigidBody>(selectedEntity);
                        scene->addComponent<RigidBody>(newEntity, originalRigidBody);
                    }
                    
                    // Copy player components if they exist
                    if (scene->hasComponent<PlayerController>(selectedEntity)) {
                        auto& original = scene->getComponent<PlayerController>(selectedEntity);
                        scene->addComponent<PlayerController>(newEntity, original);
                    }
                    if (scene->hasComponent<PlayerStats>(selectedEntity)) {
                        auto& original = scene->getComponent<PlayerStats>(selectedEntity);
                        scene->addComponent<PlayerStats>(newEntity, original);
                    }
                    if (scene->hasComponent<PlayerPhysics>(selectedEntity)) {
                        auto& original = scene->getComponent<PlayerPhysics>(selectedEntity);
                        scene->addComponent<PlayerPhysics>(newEntity, original);
                    }
                    if (scene->hasComponent<PlayerInventory>(selectedEntity)) {
                        auto& original = scene->getComponent<PlayerInventory>(selectedEntity);
                        scene->addComponent<PlayerInventory>(newEntity, original);
                    }
                    if (scene->hasComponent<PlayerAbilities>(selectedEntity)) {
                        auto& original = scene->getComponent<PlayerAbilities>(selectedEntity);
                        scene->addComponent<PlayerAbilities>(newEntity, original);
                    }
                    if (scene->hasComponent<PlayerState>(selectedEntity)) {
                        auto& original = scene->getComponent<PlayerState>(selectedEntity);
                        scene->addComponent<PlayerState>(newEntity, original);
                    }
                    if (scene->hasComponent<EntitySpawner>(selectedEntity)) {
                        auto& original = scene->getComponent<EntitySpawner>(selectedEntity);
                        scene->addComponent<EntitySpawner>(newEntity, original);
                    }
                    if (scene->hasComponent<ParticleEffect>(selectedEntity)) {
                        auto& original = scene->getComponent<ParticleEffect>(selectedEntity);
                        scene->addComponent<ParticleEffect>(newEntity, original);
                    }
                    
                    m_activeSceneWindow->setSelectedEntity(newEntity);
                    m_activeSceneWindow->setDirty(true);
                    
                    m_consoleMessages.push_back("✅ Duplicated entity: " + selectedName + " -> " + selectedName + "_Copy");
                }
            }
        }
        
        // Node Editor Entity Spawning Section
        if (ImGui::CollapsingHeader("Node Editor Entity Spawning", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::Text("🔗 Create Entity Spawner Nodes:");
            ImGui::TextWrapped("Add entity spawning capabilities to your visual scripting.");
            
            if (ImGui::Button("Add Entity Spawner Node", ImVec2(-1, 0))) {
                if (m_nodeEditor) {
                    // Create EntitySpawner node at center of node editor
                    ImVec2 nodePos(200, 200);
                    m_nodeEditor->createNode(NodeEditor::NodeType::EntitySpawner, nodePos);
                    m_consoleMessages.push_back("✅ Added Entity Spawner node to Node Editor");
                }
            }
            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("Creates an EntitySpawner node in the Node Editor.\nInputs: Spawn event, Template input (connect Entity nodes), Position\nOutputs: Spawned event, New entity");
            }
            
            if (ImGui::Button("Add Entity Factory Node", ImVec2(-1, 0))) {
                if (m_nodeEditor) {
                    // Create EntityFactory node at center of node editor
                    ImVec2 nodePos(200, 300);
                    m_nodeEditor->createNode(NodeEditor::NodeType::EntityFactory, nodePos);
                    m_consoleMessages.push_back("✅ Added Entity Factory node to Node Editor");
                }
            }
            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("Creates a node that can generate new entities dynamically.\nInputs: Create event, Entity name, Position\nOutputs: Created event, Entity reference");
            }
            
            ImGui::Separator();
            
            ImGui::Text("💡 Quick Setup:");
            if (ImGui::Button("Create Complete Spawning System", ImVec2(-1, 0))) {
                if (m_nodeEditor && scene) {
                    // Create a complete entity spawning setup with multiple nodes
                    
                    // Entity node (template)
                    ImVec2 entityNodePos(50, 150);
                    int entityNodeId = m_nodeEditor->createNode(NodeEditor::NodeType::Entity, entityNodePos);
                    
                    // Entity Spawner node
                    ImVec2 spawnerNodePos(300, 150);
                    int spawnerNodeId = m_nodeEditor->createNode(NodeEditor::NodeType::EntitySpawner, spawnerNodePos);
                    
                    // Entity Factory node
                    ImVec2 factoryNodePos(300, 300);
                    int factoryNodeId = m_nodeEditor->createNode(NodeEditor::NodeType::EntityFactory, factoryNodePos);
                    
                    m_consoleMessages.push_back("✅ Created complete entity spawning system in Node Editor");
                    m_consoleMessages.push_back("💡 Connect Entity nodes to the EntitySpawner's Template input to configure spawning");
                }
            }
            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("Creates Entity, EntitySpawner, and EntityFactory nodes.\nConnect Entity node output → EntitySpawner Template input to set up spawning.");
            }
        }
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
    auto particleSystem = m_currentScene->registerSystem<ParticleSystem>();
    auto lightSystem = m_currentScene->registerSystem<LightSystem>();
    auto audioSystem = m_currentScene->registerSystem<AudioSystem>();
    
    // Set scene pointer for each system
    renderSystem->setScene(m_currentScene.get());
    physicsSystem->setScene(m_currentScene.get());
    collisionSystem->setScene(m_currentScene.get());
    particleSystem->setScene(m_currentScene.get());
    lightSystem->setScene(m_currentScene.get());
    audioSystem->setScene(m_currentScene.get());
    
    // Initialize audio system
    audioSystem->initialize();
    
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
    
    ComponentMask particleSignature;
    particleSignature.set(m_currentScene->getComponentType<Transform>());
    particleSignature.set(m_currentScene->getComponentType<ParticleEffect>());
    m_currentScene->setSystemSignature<ParticleSystem>(particleSignature);
    
    // Temporarily disable new system signatures to test for crashes
    /*
    ComponentMask lightSignature;
    lightSignature.set(m_currentScene->getComponentType<Transform>());
    lightSignature.set(m_currentScene->getComponentType<LightSource>());
    m_currentScene->setSystemSignature<LightSystem>(lightSignature);
    
    ComponentMask audioSignature;
    audioSignature.set(m_currentScene->getComponentType<Transform>());
    audioSignature.set(m_currentScene->getComponentType<AudioSource>());
    m_currentScene->setSystemSignature<AudioSystem>(audioSignature);
    */
    
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
    static bool clearExistingEntities = true;  // Safe to clear by default (only procedural entities)
    
    ImGui::Text("📐 Map Settings:");
    ImGui::DragInt("Width", &mapWidth, 1.0f, 10, 200);
    ImGui::DragInt("Height", &mapHeight, 1.0f, 10, 200);
    ImGui::InputInt("Seed", reinterpret_cast<int*>(&seed));
    ImGui::SameLine();
    if (ImGui::Button("Random Seed")) {
        seed = std::random_device{}();
    }
    
    ImGui::Checkbox("Clear Existing Entities", &clearExistingEntities);
    ImGui::SameLine();
    ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "(Only clears procedural entities, preserves user entities)");
    
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
            
            // Clear existing procedural map
            m_activeSceneWindow->setProceduralMap(nullptr);
            
            // Optionally clear existing entities based on user choice
            if (clearExistingEntities) {
                auto activeScene = m_activeSceneWindow->getScene();
                auto entities = activeScene->getAllLivingEntities();
                
                // Only destroy procedurally generated entities, preserve user-created ones
                int destroyedCount = 0;
                for (EntityID entity : entities) {
                    if (activeScene->hasComponent<ProceduralGenerated>(entity)) {
                        activeScene->destroyEntity(entity);
                        destroyedCount++;
                    }
                }
                
                if (destroyedCount > 0) {
                    m_consoleMessages.push_back("Cleared " + std::to_string(destroyedCount) + " procedurally generated entities (preserved user entities)");
                } else {
                    m_consoleMessages.push_back("No procedurally generated entities to clear");
                }
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
            
            // Clear existing procedural map
            m_activeSceneWindow->setProceduralMap(nullptr);
            
            // Optionally clear existing entities based on user choice
            if (clearExistingEntities) {
                auto activeScene = m_activeSceneWindow->getScene();
                auto entities = activeScene->getAllLivingEntities();
                
                // Only destroy procedurally generated entities, preserve user-created ones
                int destroyedCount = 0;
                for (EntityID entity : entities) {
                    if (activeScene->hasComponent<ProceduralGenerated>(entity)) {
                        activeScene->destroyEntity(entity);
                        destroyedCount++;
                    }
                }
                
                if (destroyedCount > 0) {
                    m_consoleMessages.push_back("Cleared " + std::to_string(destroyedCount) + " procedurally generated entities (preserved user entities)");
                } else {
                    m_consoleMessages.push_back("No procedurally generated entities to clear");
                }
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
            
            // Clear existing procedural map
            m_activeSceneWindow->setProceduralMap(nullptr);
            
            // Optionally clear existing entities based on user choice
            if (clearExistingEntities) {
                auto activeScene = m_activeSceneWindow->getScene();
                auto entities = activeScene->getAllLivingEntities();
                
                // Only destroy procedurally generated entities, preserve user-created ones
                int destroyedCount = 0;
                for (EntityID entity : entities) {
                    if (activeScene->hasComponent<ProceduralGenerated>(entity)) {
                        activeScene->destroyEntity(entity);
                        destroyedCount++;
                    }
                }
                
                if (destroyedCount > 0) {
                    m_consoleMessages.push_back("Cleared " + std::to_string(destroyedCount) + " procedurally generated entities (preserved user entities)");
                } else {
                    m_consoleMessages.push_back("No procedurally generated entities to clear");
                }
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

void GameEditor::showCollisionEditor() {
    if (m_collisionEditor) {
        m_collisionEditor->setOpen(m_showCollisionEditor);
        
        // Update selected entity if we have an active scene window
        if (m_activeSceneWindow && m_activeSceneWindow->getScene()) {
            EntityID selectedEntity = m_activeSceneWindow->getSelectedEntity();
            m_collisionEditor->setSelectedEntity(selectedEntity, m_activeSceneWindow->getScene());
        } else {
            m_collisionEditor->setSelectedEntity(0, nullptr);
        }
        
        m_collisionEditor->render();
        m_showCollisionEditor = m_collisionEditor->isOpen();
    }
}

void GameEditor::showCodeViewer() {
    if (!m_showCodeViewer) return;
    
    // Load code files if not already loaded
    if (!m_codeFilesLoaded) {
        loadCodeFiles();
        m_codeFilesLoaded = true;
    }
    
    ImGui::SetNextWindowSize(ImVec2(1000, 700), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Game Code Viewer", &m_showCodeViewer)) {
        ImGui::Text("This window shows the code for your game project created with this editor.");
        ImGui::Text("Game logic, scripts, components, and scene files are displayed here.");
        ImGui::Separator();
        
        // Refresh button
        if (ImGui::Button("Refresh Code Files")) {
            m_codeFilesLoaded = false;
            loadCodeFiles();
            m_codeFilesLoaded = true;
        }
        
        ImGui::SameLine();
        if (ImGui::Button("Create Game Directory")) {
            createGameDirectoryStructure();
        }
        
        ImGui::SameLine();
        ImGui::Text("Files: %d", (int)m_codeFiles.size());
        
        // Create two-pane layout
        ImGui::BeginChild("CodeFileList", ImVec2(250, 0), true);
        
        // File list
        ImGui::Text("Game Code Files:");
        ImGui::Separator();
        
        for (int i = 0; i < m_codeFiles.size(); i++) {
            const auto& file = m_codeFiles[i];
            
            // Color code different file types
            ImVec4 color = ImVec4(1.0f, 1.0f, 1.0f, 1.0f); // Default white
            std::string ext = "";
            size_t dotPos = file.filename.find_last_of('.');
            if (dotPos != std::string::npos) {
                ext = file.filename.substr(dotPos);
            }
            
            if (ext == ".h" || ext == ".hpp") {
                color = ImVec4(0.7f, 0.9f, 1.0f, 1.0f); // Light blue for headers
            } else if (ext == ".cpp" || ext == ".c") {
                color = ImVec4(0.9f, 1.0f, 0.7f, 1.0f); // Light green for C++ source
            } else if (ext == ".lua") {
                color = ImVec4(1.0f, 0.8f, 0.6f, 1.0f); // Orange for Lua scripts
            } else if (ext == ".js") {
                color = ImVec4(1.0f, 1.0f, 0.6f, 1.0f); // Yellow for JavaScript
            } else if (ext == ".py") {
                color = ImVec4(0.6f, 1.0f, 0.8f, 1.0f); // Light cyan for Python
            } else if (ext == ".json" || ext == ".scene") {
                color = ImVec4(0.9f, 0.7f, 1.0f, 1.0f); // Light purple for data files
            } else if (ext == ".cs") {
                color = ImVec4(0.8f, 1.0f, 0.9f, 1.0f); // Light mint for C#
            }
            
            ImGui::PushStyleColor(ImGuiCol_Text, color);
            
            bool isSelected = (i == m_selectedCodeFile);
            if (ImGui::Selectable(file.filename.c_str(), isSelected)) {
                m_selectedCodeFile = i;
            }
            
            ImGui::PopStyleColor();
            
            // Tooltip with full path
            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("%s", file.path.c_str());
            }
        }
        
        ImGui::EndChild();
        
        ImGui::SameLine();
        
        // Code content
        ImGui::BeginChild("CodeContent", ImVec2(0, 0), true);
        
        if (m_selectedCodeFile >= 0 && m_selectedCodeFile < m_codeFiles.size()) {
            const auto& selectedFile = m_codeFiles[m_selectedCodeFile];
            
            ImGui::Text("File: %s", selectedFile.path.c_str());
            ImGui::Text("Lines: %d", (int)std::count(selectedFile.content.begin(), selectedFile.content.end(), '\n') + 1);
            ImGui::Separator();
            
            // Code content with syntax highlighting (basic)
            ImGui::PushFont(ImGui::GetIO().FontDefault);
            
            // Use InputTextMultiline for better code display
            ImGui::InputTextMultiline("##code", const_cast<char*>(selectedFile.content.c_str()), 
                                    selectedFile.content.size(), ImVec2(-1, -1), 
                                    ImGuiInputTextFlags_ReadOnly | ImGuiInputTextFlags_AllowTabInput);
            
            ImGui::PopFont();
        } else {
            ImGui::Text("Select a file to view its content.");
        }
        
        ImGui::EndChild();
    }
    ImGui::End();
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
    auto particleSystem = scene->registerSystem<ParticleSystem>();
    auto lightSystem = scene->registerSystem<LightSystem>();
    auto audioSystem = scene->registerSystem<AudioSystem>();
    
    // Set scene pointer for each system
    renderSystem->setScene(scene.get());
    physicsSystem->setScene(scene.get());
    collisionSystem->setScene(scene.get());
    particleSystem->setScene(scene.get());
    lightSystem->setScene(scene.get());
    audioSystem->setScene(scene.get());
    
    // Initialize audio system
    audioSystem->initialize();
    
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
    
    ComponentMask particleSignature;
    particleSignature.set(scene->getComponentType<Transform>());
    particleSignature.set(scene->getComponentType<ParticleEffect>());
    scene->setSystemSignature<ParticleSystem>(particleSignature);
    
    // Temporarily disable new system signatures to test for crashes
    /*
    ComponentMask lightSignature;
    lightSignature.set(scene->getComponentType<Transform>());
    lightSignature.set(scene->getComponentType<LightSource>());
    scene->setSystemSignature<LightSystem>(lightSignature);
    
    ComponentMask audioSignature;
    audioSignature.set(scene->getComponentType<Transform>());
    audioSignature.set(scene->getComponentType<AudioSource>());
    scene->setSystemSignature<AudioSystem>(audioSignature);
    */
    
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

void GameEditor::loadCodeFiles() {
    m_codeFiles.clear();
    
    // Define directories to scan for user-created game code
    std::vector<std::string> gameCodeDirs = {
        "game",           // Main game code directory
        "game/scripts",   // Game scripts and logic
        "game/entities",  // Custom entity definitions
        "game/components", // Custom game components
        "game/systems",   // Custom game systems
        "game/scenes",    // Scene-specific code
        "assets/scripts", // Asset-based scripts
        "scripts"         // Alternative scripts directory
    };
    
    // Look for game-specific files in the project root
    std::vector<std::string> gameCodeFiles = {
        "game.cpp",
        "game.h",
        "GameLogic.cpp",
        "GameLogic.h",
        "PlayerController.cpp",
        "PlayerController.h"
    };
    
    auto loadFile = [this](const std::string& filepath) {
        std::ifstream file(filepath);
        if (!file.is_open()) return;
        
        CodeFile codeFile;
        codeFile.path = filepath;
        codeFile.filename = std::filesystem::path(filepath).filename().string();
        codeFile.isGameCode = true;
        
        // Read file content
        std::stringstream buffer;
        buffer << file.rdbuf();
        codeFile.content = buffer.str();
        
        m_codeFiles.push_back(std::move(codeFile));
    };
    
    // Load individual game files from project root
    for (const auto& filepath : gameCodeFiles) {
        if (std::filesystem::exists(filepath)) {
            loadFile(filepath);
        }
    }
    
    // Load files from game directories
    for (const auto& dir : gameCodeDirs) {
        if (!std::filesystem::exists(dir)) continue;
        
        try {
            for (const auto& entry : std::filesystem::recursive_directory_iterator(dir)) {
                if (entry.is_regular_file()) {
                    std::string filepath = entry.path().string();
                    std::string extension = entry.path().extension().string();
                    
                    // Include various script and code file types
                    if (extension == ".cpp" || extension == ".h" || extension == ".hpp" ||
                        extension == ".lua" || extension == ".js" || extension == ".py" ||
                        extension == ".cs" || extension == ".txt") {
                        loadFile(filepath);
                    }
                }
            }
        } catch (const std::filesystem::filesystem_error& ex) {
            std::string errorMsg = "Error scanning directory " + dir + ": " + ex.what();
            m_consoleMessages.push_back(errorMsg);
        }
    }
    
    // Also scan scene files for embedded logic
    if (std::filesystem::exists("assets/scenes")) {
        try {
            for (const auto& entry : std::filesystem::recursive_directory_iterator("assets/scenes")) {
                if (entry.is_regular_file()) {
                    std::string filepath = entry.path().string();
                    std::string extension = entry.path().extension().string();
                    
                    if (extension == ".json" || extension == ".scene") {
                        loadFile(filepath);
                    }
                }
            }
        } catch (const std::filesystem::filesystem_error& ex) {
            std::string errorMsg = "Error scanning scenes directory: " + std::string(ex.what());
            m_consoleMessages.push_back(errorMsg);
        }
    }
    
    // If no game code found, create example templates
    if (m_codeFiles.empty()) {
        createExampleGameCode();
    }
    
    // Sort files by name for better organization
    std::sort(m_codeFiles.begin(), m_codeFiles.end(), 
              [](const CodeFile& a, const CodeFile& b) {
                  return a.filename < b.filename;
              });
    
    std::string msg = "Loaded " + std::to_string(m_codeFiles.size()) + " game code files";
    m_consoleMessages.push_back(msg);
}

void GameEditor::createExampleGameCode() {
    // Create example game code templates to show what the viewer is for
    
    CodeFile gameHeader;
    gameHeader.path = "game/GameLogic.h (Template)";
    gameHeader.filename = "GameLogic.h";
    gameHeader.isGameCode = true;
    gameHeader.content = R"(#pragma once

#include "core/Engine.h" 
#include "components/Components.h"

// Main game logic class
// This is where you implement your game's core mechanics
class GameLogic {
public:
    GameLogic();
    ~GameLogic();
    
    void initialize();
    void update(float deltaTime);
    void render();
    void shutdown();
    
    // Game-specific methods
    void setupPlayer();
    void setupEnemies();
    void handleCollisions();
    void updateGameState();
    
private:
    EntityID m_player;
    std::vector<EntityID> m_enemies;
    int m_score;
    bool m_gameRunning;
};

// Custom game components
struct PlayerComponent {
    float health = 100.0f;
    float speed = 200.0f;
    int lives = 3;
};

struct EnemyComponent {
    float damage = 10.0f;
    float speed = 100.0f;
    bool isActive = true;
};
)";
    
    CodeFile gameSource;
    gameSource.path = "game/GameLogic.cpp (Template)";
    gameSource.filename = "GameLogic.cpp";
    gameSource.isGameCode = true;
    gameSource.content = R"(#include "GameLogic.h"
#include "systems/CoreSystems.h"

GameLogic::GameLogic() 
    : m_player(0), m_score(0), m_gameRunning(false) {
}

GameLogic::~GameLogic() {
}

void GameLogic::initialize() {
    m_gameRunning = true;
    m_score = 0;
    
    setupPlayer();
    setupEnemies();
    
    std::cout << "Game initialized!" << std::endl;
}

void GameLogic::update(float deltaTime) {
    if (!m_gameRunning) return;
    
    updateGameState();
    handleCollisions();
    
    // Update score display, etc.
}

void GameLogic::render() {
    // Custom rendering logic here
    // UI, effects, etc.
}

void GameLogic::setupPlayer() {
    auto& engine = Engine::getInstance();
    auto scene = engine.getCurrentScene();
    
    if (scene) {
        m_player = scene->createEntity();
        scene->setEntityName(m_player, "Player");
        
        // Add components
        scene->addComponent<Transform>(m_player, Transform(0, 0));
        scene->addComponent<PlayerComponent>(m_player, PlayerComponent());
        
        // Add sprite if you have player texture
        // scene->addComponent<Sprite>(m_player, playerSprite);
    }
}

void GameLogic::setupEnemies() {
    // Create enemies, populate game world
    // This is where your game-specific setup goes
}

void GameLogic::handleCollisions() {
    // Game-specific collision handling
    // Player vs enemies, bullets vs targets, etc.
}

void GameLogic::updateGameState() {
    // Update game state, check win conditions, etc.
}
)";

    CodeFile playerController;
    playerController.path = "game/PlayerController.cpp (Template)";
    playerController.filename = "PlayerController.cpp"; 
    playerController.isGameCode = true;
    playerController.content = R"(#include "PlayerController.h"
#include "input/InputManager.h"

void PlayerController::update(EntityID player, float deltaTime) {
    auto& inputManager = InputManager::getInstance();
    auto scene = Engine::getInstance().getCurrentScene();
    
    if (!scene || !scene->hasComponent<Transform>(player)) return;
    
    auto& transform = scene->getComponent<Transform>(player);
    auto& playerComp = scene->getComponent<PlayerComponent>(player);
    
    // Movement input
    Vector2 movement(0, 0);
    
    if (inputManager.isKeyPressed(SDLK_w) || inputManager.isKeyPressed(SDLK_UP)) {
        movement.y -= 1.0f;
    }
    if (inputManager.isKeyPressed(SDLK_s) || inputManager.isKeyPressed(SDLK_DOWN)) {
        movement.y += 1.0f;
    }
    if (inputManager.isKeyPressed(SDLK_a) || inputManager.isKeyPressed(SDLK_LEFT)) {
        movement.x -= 1.0f;
    }
    if (inputManager.isKeyPressed(SDLK_d) || inputManager.isKeyPressed(SDLK_RIGHT)) {
        movement.x += 1.0f;
    }
    
    // Apply movement
    if (movement.x != 0 || movement.y != 0) {
        // Normalize diagonal movement
        float length = sqrt(movement.x * movement.x + movement.y * movement.y);
        movement.x /= length;
        movement.y /= length;
        
        transform.position.x += movement.x * playerComp.speed * deltaTime;
        transform.position.y += movement.y * playerComp.speed * deltaTime;
    }
    
    // Action input
    if (inputManager.isKeyPressed(SDLK_SPACE)) {
        // Shoot, jump, interact, etc.
        performAction(player);
    }
}

void PlayerController::performAction(EntityID player) {
    // Implement player actions here
    // Shooting, jumping, interacting with objects, etc.
}
)";

    CodeFile sceneScript;
    sceneScript.path = "assets/scenes/level1.scene (Example)";
    sceneScript.filename = "level1.scene";
    sceneScript.isGameCode = true;
    sceneScript.content = R"({
  "name": "Level 1",
  "entities": [
    {
      "id": 1,
      "name": "Player",
      "components": {
        "Transform": {
          "position": { "x": 100, "y": 100 },
          "scale": { "x": 1, "y": 1 },
          "rotation": 0
        },
        "Sprite": {
          "texture": "assets/textures/player.png",
          "visible": true
        },
        "PlayerComponent": {
          "health": 100,
          "speed": 200,
          "lives": 3
        }
      }
    },
    {
      "id": 2,
      "name": "Enemy1",
      "components": {
        "Transform": {
          "position": { "x": 300, "y": 150 },
          "scale": { "x": 1, "y": 1 },
          "rotation": 0
        },
        "Sprite": {
          "texture": "assets/textures/enemy.png",
          "visible": true
        },
        "EnemyComponent": {
          "damage": 10,
          "speed": 100,
          "isActive": true
        }
      }
    }
  ],
  "systems": [
    "RenderSystem",
    "PhysicsSystem", 
    "PlayerControllerSystem",
    "EnemyAISystem"
  ]
}
)";

    // Add all template files
    m_codeFiles.push_back(std::move(gameHeader));
    m_codeFiles.push_back(std::move(gameSource));
    m_codeFiles.push_back(std::move(playerController));
    m_codeFiles.push_back(std::move(sceneScript));
    
    m_consoleMessages.push_back("No game code found - showing example templates");
    m_consoleMessages.push_back("Create files in 'game/' directory to see your actual game code");
}

void GameEditor::createGameDirectoryStructure() {
    try {
        // Create game directory structure
        std::vector<std::string> directories = {
            "game",
            "game/scripts",
            "game/entities", 
            "game/components",
            "game/systems",
            "game/scenes"
        };
        
        for (const auto& dir : directories) {
            if (!std::filesystem::exists(dir)) {
                std::filesystem::create_directories(dir);
                m_consoleMessages.push_back("Created directory: " + dir);
            }
        }
        
        // Create starter game files
        createStarterGameFiles();
        
        m_consoleMessages.push_back("Game directory structure created successfully!");
        
        // Refresh the code files to show new structure
        m_codeFilesLoaded = false;
        loadCodeFiles();
        m_codeFilesLoaded = true;
        
    } catch (const std::filesystem::filesystem_error& ex) {
        m_consoleMessages.push_back("Error creating game directories: " + std::string(ex.what()));
    }
}

void GameEditor::createStarterGameFiles() {
    // Create basic GameLogic.h file
    std::string gameLogicH = R"(#pragma once

// Your game's main logic class
// Implement your game mechanics, rules, and state management here

class GameLogic {
public:
    GameLogic();
    ~GameLogic();
    
    void initialize();
    void update(float deltaTime);
    void shutdown();
    
private:
    // Add your game state variables here
    bool m_gameRunning;
    int m_score;
};
)";

    std::string gameLogicCpp = R"(#include "GameLogic.h"
#include <iostream>

GameLogic::GameLogic() : m_gameRunning(false), m_score(0) {
}

GameLogic::~GameLogic() {
}

void GameLogic::initialize() {
    m_gameRunning = true;
    m_score = 0;
    std::cout << "Game initialized!" << std::endl;
    
    // TODO: Initialize your game here
    // Create entities, setup scene, etc.
}

void GameLogic::update(float deltaTime) {
    if (!m_gameRunning) return;
    
    // TODO: Update your game logic here
    // Handle input, update entities, check collisions, etc.
}

void GameLogic::shutdown() {
    m_gameRunning = false;
    std::cout << "Game shutdown. Final score: " << m_score << std::endl;
}
)";

    // Write files
    try {
        std::ofstream headerFile("game/GameLogic.h");
        headerFile << gameLogicH;
        headerFile.close();
        
        std::ofstream sourceFile("game/GameLogic.cpp");
        sourceFile << gameLogicCpp;
        sourceFile.close();
        
        m_consoleMessages.push_back("Created starter files: GameLogic.h and GameLogic.cpp");
    } catch (const std::exception& ex) {
        m_consoleMessages.push_back("Error creating starter files: " + std::string(ex.what()));
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

void GameEditor::spawnEntityFromTemplate(EntityID spawnerEntity, EntitySpawner& spawner, float currentTime) {
    if (!m_activeSceneWindow || !m_activeSceneWindow->getScene()) {
        return;
    }
    
    Scene* scene = m_activeSceneWindow->getScene().get();
    
    // Check if spawner is ready
    if (!spawner.isReady(currentTime)) {
        return;
    }
    
    // Get the selected template
    if (spawner.selectedTemplate < 0 || spawner.selectedTemplate >= static_cast<int>(spawner.templates.size())) {
        return;
    }
    
    const auto& spawnTemplate = spawner.templates[spawner.selectedTemplate];
    
    // Check if this is a template entity reference
    if (spawnTemplate.spriteFile.find("TEMPLATE_ENTITY_") == 0) {
        // Extract template entity ID
        std::string idStr = spawnTemplate.spriteFile.substr(16); // Remove "TEMPLATE_ENTITY_" prefix
        try {
            EntityID templateEntityId = static_cast<EntityID>(std::stoul(idStr));
            
            // Clone the template entity
            EntityID newEntity = cloneEntity(scene, templateEntityId);
            if (newEntity != 0) {
                // Get spawner position for spawning offset
                Vector2 spawnerPos = {0, 0};
                if (scene->hasComponent<Transform>(spawnerEntity)) {
                    auto& spawnerTransform = scene->getComponent<Transform>(spawnerEntity);
                    spawnerPos = spawnerTransform.position;
                }
                
                // Apply spawn offset
                if (scene->hasComponent<Transform>(newEntity)) {
                    auto& newTransform = scene->getComponent<Transform>(newEntity);
                    newTransform.position.x = spawnerPos.x + spawnTemplate.spawnOffset.x;
                    newTransform.position.y = spawnerPos.y + spawnTemplate.spawnOffset.y;
                    newTransform.scale = spawnTemplate.scale;
                }
                
                // Apply velocity if RigidBody component exists
                if (scene->hasComponent<RigidBody>(newEntity)) {
                    auto& rigidBody = scene->getComponent<RigidBody>(newEntity);
                    rigidBody.velocity = spawnTemplate.velocity;
                }
                
                // Update spawner state
                spawner.lastSpawnTime = currentTime;
                spawner.spawnCount++;
                
                // Mark scene as dirty
                m_activeSceneWindow->setDirty(true);
                
                printf("DEBUG: Spawned entity %u from template entity %u through spawner entity %u\n", 
                       newEntity, templateEntityId, spawnerEntity);
            }
        } catch (const std::exception& e) {
            printf("ERROR: Failed to parse template entity ID from '%s': %s\n", 
                   spawnTemplate.spriteFile.c_str(), e.what());
        }
    } else {
        // Regular template spawning (existing logic)
        EntityID newEntity = scene->createEntity();
        scene->setEntityName(newEntity, spawnTemplate.name);
        
        // Get spawner position
        Vector2 spawnerPos = {0, 0};
        if (scene->hasComponent<Transform>(spawnerEntity)) {
            auto& spawnerTransform = scene->getComponent<Transform>(spawnerEntity);
            spawnerPos = spawnerTransform.position;
        }
        
        // Add Transform component
        Transform transform;
        transform.position.x = spawnerPos.x + spawnTemplate.spawnOffset.x;
        transform.position.y = spawnerPos.y + spawnTemplate.spawnOffset.y;
        transform.scale = spawnTemplate.scale;
        scene->addComponent<Transform>(newEntity, transform);
        
        // Add Sprite component if sprite file is specified
        if (!spawnTemplate.spriteFile.empty() && spawnTemplate.spriteFile != "TEMPLATE_ENTITY_") {
            Sprite sprite;
            // sprite.texture = m_resourceManager->getTexture(spawnTemplate.spriteFile);
            scene->addComponent<Sprite>(newEntity, sprite);
        }
        
        // Add RigidBody component with velocity if needed
        if (spawnTemplate.hasRigidBody) {
            RigidBody rigidBody;
            rigidBody.velocity = spawnTemplate.velocity;
            scene->addComponent<RigidBody>(newEntity, rigidBody);
        }
        
        // Add Collider if specified
        if (spawnTemplate.hasCollider) {
            Collider collider;
            collider.size = Vector2(32, 32); // Default size
            scene->addComponent<Collider>(newEntity, collider);
        }
        
        // Add LifeTime component if specified (comment out for now since LifeTime may not exist)
        /*
        if (spawnTemplate.lifeTime > 0.0f) {
            LifeTime lifeTime;
            lifeTime.duration = spawnTemplate.lifeTime;
            lifeTime.startTime = currentTime;
            scene->addComponent<LifeTime>(newEntity, lifeTime);
        }
        */
        
        // Update spawner state
        spawner.lastSpawnTime = currentTime;
        spawner.spawnCount++;
        
        // Mark scene as dirty
        m_activeSceneWindow->setDirty(true);
        
        printf("DEBUG: Spawned regular entity %u from template '%s' through spawner entity %u\n", 
               newEntity, spawnTemplate.name.c_str(), spawnerEntity);
    }
}

EntityID GameEditor::cloneEntity(Scene* scene, EntityID sourceEntity) {
    if (!scene || sourceEntity == 0) {
        return 0;
    }
    
    // Create new entity with similar name
    std::string sourceName = scene->getEntityName(sourceEntity);
    std::string newName = sourceName + "_Clone";
    EntityID newEntity = scene->createEntity();
    scene->setEntityName(newEntity, newName);
    
    // Copy Transform component
    if (scene->hasComponent<Transform>(sourceEntity)) {
        auto& sourceTransform = scene->getComponent<Transform>(sourceEntity);
        scene->addComponent<Transform>(newEntity, sourceTransform);
    }
    
    // Copy Sprite component
    if (scene->hasComponent<Sprite>(sourceEntity)) {
        auto& sourceSprite = scene->getComponent<Sprite>(sourceEntity);
        scene->addComponent<Sprite>(newEntity, sourceSprite);
    }
    
    // Copy RigidBody component  
    if (scene->hasComponent<RigidBody>(sourceEntity)) {
        auto& sourceRigidBody = scene->getComponent<RigidBody>(sourceEntity);
        scene->addComponent<RigidBody>(newEntity, sourceRigidBody);
    }
    
    // Copy Collider component
    if (scene->hasComponent<Collider>(sourceEntity)) {
        auto& sourceCollider = scene->getComponent<Collider>(sourceEntity);
        scene->addComponent<Collider>(newEntity, sourceCollider);
    }
    
    // Copy ParticleEffect component
    if (scene->hasComponent<ParticleEffect>(sourceEntity)) {
        auto& sourceParticleEffect = scene->getComponent<ParticleEffect>(sourceEntity);
        scene->addComponent<ParticleEffect>(newEntity, sourceParticleEffect);
    }
    
    // Don't copy EntitySpawner component to avoid recursive spawning
    
    return newEntity;
}
