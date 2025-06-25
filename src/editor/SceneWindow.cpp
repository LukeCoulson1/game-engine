#include "SceneWindow.h"
#include "GameEditor.h"
#include "../core/Engine.h"
#include "../systems/CoreSystems.h"
#include "../rendering/TileRenderer.h"
#include "../generation/ProceduralGeneration.h"
#include "../utils/ResourceManager.h"
#include "../utils/ConfigManager.h"
#include <imgui.h>
#include <iostream>
#include <sstream>
#include <cmath>
#include <algorithm>

int SceneWindow::s_windowCounter = 0;

SceneWindow::SceneWindow(const std::string& title, std::shared_ptr<Scene> scene, GameEditor* editor)
    : m_title(title), m_scene(scene), m_editor(editor), m_windowId(++s_windowCounter), m_lastWindowSize(800, 600) {
    // Initialize tile renderer for efficient procedural map rendering
    m_tileRenderer = std::make_unique<TileRenderer>();
    
    // Don't set any initial window size - let ImGui handle it naturally per scene
    // ImGui will remember window state per window ID automatically
    
    // If the scene has a procedural map, restore it to this window
    if (scene && scene->hasProceduralMap()) {
        setProceduralMap(scene->getProceduralMap());
        std::cout << "Restored procedural map to SceneWindow: " << title << std::endl;
    }
}

SceneWindow::~SceneWindow() {
}

void SceneWindow::setZoomLevel(float zoom) {
    m_zoomLevel = std::max(m_minZoom, std::min(m_maxZoom, zoom));
}

void SceneWindow::setProceduralMap(std::shared_ptr<ProceduralMap> map) {
    m_proceduralMap = map;
    if (m_tileRenderer) {
        m_tileRenderer->setMap(map);
    }
    
    // Also store the procedural map in the scene for persistence
    if (m_scene) {
        m_scene->setProceduralMap(map);
    }
}

std::string SceneWindow::getWindowID() const {
    std::stringstream ss;
    ss << m_title << "##SceneWindow" << m_windowId;
    return ss.str();
}

void SceneWindow::setSelectedEntity(EntityID entity) {
    m_selectedEntity = entity;
}

void SceneWindow::saveWindowSize() {
    ImVec2 windowSize = ImGui::GetWindowSize();
    auto& config = ConfigManager::getInstance();
    
    // Save scene-specific window size
    config.setSceneWindowSize(m_title, static_cast<int>(windowSize.x), static_cast<int>(windowSize.y));
    
    // Also save as global scene window size for new scenes
    config.setSceneWindowSize(static_cast<int>(windowSize.x), static_cast<int>(windowSize.y));
    
    config.saveConfig(); // Save immediately to persist the setting
    std::cout << "DEBUG: SceneWindow '" << m_title << "' saved scene-specific window size: " << windowSize.x << "x" << windowSize.y << std::endl;
}

void SceneWindow::restoreWindowSize() {
    auto& config = ConfigManager::getInstance();
    int width, height;
    
    // Try to get scene-specific window size first
    if (config.hasSceneWindowSize(m_title)) {
        config.getSceneWindowSize(m_title, width, height);
        ImGui::SetNextWindowSize(ImVec2(static_cast<float>(width), static_cast<float>(height)), ImGuiCond_Always);
        m_lastWindowSize = ImVec2(static_cast<float>(width), static_cast<float>(height));
        std::cout << "DEBUG: SceneWindow '" << m_title << "' restored scene-specific size: " << width << "x" << height << std::endl;
    } else {
        // Fall back to global scene window size, then default
        config.getSceneWindowSize(width, height);
        if (width > 0 && height > 0) {
            ImGui::SetNextWindowSize(ImVec2(static_cast<float>(width), static_cast<float>(height)), ImGuiCond_FirstUseEver);
            m_lastWindowSize = ImVec2(static_cast<float>(width), static_cast<float>(height));
            std::cout << "DEBUG: SceneWindow '" << m_title << "' using global size: " << width << "x" << height << std::endl;
        } else {
            // Use default size
            ImGui::SetNextWindowSize(ImVec2(800, 600), ImGuiCond_FirstUseEver);
            m_lastWindowSize = ImVec2(800, 600);
            std::cout << "DEBUG: SceneWindow '" << m_title << "' using default size: 800x600" << std::endl;
        }
    }
}

void SceneWindow::render() {
    if (!m_isOpen) return;
    
    ImGuiWindowFlags windowFlags = ImGuiWindowFlags_MenuBar;
    
    // Create display title with dirty state
    std::string displayTitle = m_title;
    if (m_isDirty) {
        displayTitle += "*";
    }    // Use only the scene name as the window ID for maximum stability
    // Handle dirty indicator through window content instead of title
    std::string windowID = m_title + "##Scene";
    
    // Debug: Log window ID to track any changes
    static std::string lastWindowID;
    if (windowID != lastWindowID) {
        std::cout << "DEBUG: SceneWindow ID changed from '" << lastWindowID << "' to '" << windowID << "'" << std::endl;
        lastWindowID = windowID;
    }
    
    if (ImGui::Begin(windowID.c_str(), &m_isOpen, windowFlags)) {
        
        // Show dirty indicator in the content area since we can't change the title
        if (m_isDirty) {
            ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "UNSAVED CHANGES");
            ImGui::Separator();
        }
        
        // Update active scene window when this window is focused
        if (ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows)) {
            m_editor->setActiveSceneWindow(this);
        }
        // Menu bar for scene window
        if (ImGui::BeginMenuBar()) {
            if (ImGui::BeginMenu("Scene")) {                if (ImGui::MenuItem("Save", "Ctrl+S")) {
                    if (!m_filePath.empty() && m_editor) {
                        // Use SceneManager to save the scene
                        auto sceneManager = m_editor->getSceneManager();
                        if (sceneManager) {
                            sceneManager->markSceneDirty(m_filePath);
                            // Find the scene info and save it
                            auto* sceneInfo = sceneManager->findSceneByPath(m_filePath);
                            if (sceneInfo && sceneInfo->isLoaded) {
                                sceneManager->saveScene(*sceneInfo);
                                m_isDirty = false;
                            }
                        }
                    } else {
                        // No file path - need Save As
                        ImGui::OpenPopup("SaveAsDialog");
                    }
                }                if (ImGui::MenuItem("Save As...")) {
                    if (m_editor) {
                        auto sceneManager = m_editor->getSceneManager();
                        if (sceneManager) {
                            // Open the Save As dialog through Scene Manager
                            // For now, just mark as dirty to show it needs saving
                            m_isDirty = true;
                            // TODO: Implement Save As dialog in SceneWindow or use SceneManager's
                        }
                    }
                }
                ImGui::Separator();
                if (ImGui::MenuItem("Close", "Ctrl+W")) {
                    m_isOpen = false;
                }
                ImGui::EndMenu();
            }
            
            if (ImGui::BeginMenu("Edit")) {
                if (ImGui::MenuItem("Create Entity", "Ctrl+N")) {
                    if (m_scene) {
                        EntityID newEntity = m_scene->createEntity();
                        m_scene->setEntityName(newEntity, "New Entity");
                        setSelectedEntity(newEntity);
                        setDirty(true);
                    }
                }
                if (ImGui::MenuItem("Delete Entity", "Delete", false, hasSelectedEntity())) {
                    if (m_scene && hasSelectedEntity()) {
                        m_scene->destroyEntity(m_selectedEntity);
                        m_selectedEntity = 0;
                        setDirty(true);
                    }
                }
                ImGui::EndMenu();
            }
              if (ImGui::BeginMenu("View")) {
                if (ImGui::MenuItem("Center Camera", "Home")) {
                    setCameraPosition(Vector2(0, 0));
                }
                ImGui::Separator();
                if (ImGui::MenuItem("Zoom In", "Scroll Up")) {
                    zoomIn();
                }
                if (ImGui::MenuItem("Zoom Out", "Scroll Down")) {
                    zoomOut();
                }
                if (ImGui::MenuItem("Reset Zoom", "1")) {
                    resetZoom();
                }
                ImGui::EndMenu();
            }
            
            ImGui::EndMenuBar();
        }        // Camera and zoom controls in the scene window
        ImGui::Text("Camera: (%.1f, %.1f) | Zoom: %.1fx", m_cameraPosition.x, m_cameraPosition.y, m_zoomLevel);
        
        // Performance info for procedural maps
        if (m_proceduralMap && m_tileRenderer) {
            ImGui::SameLine();
            ImGui::Text("| Tiles: %d/%d", m_tileRenderer->getVisibleTileCount(), m_tileRenderer->getTotalTileCount());
        }
        
        ImGui::SameLine();
        if (ImGui::SmallButton("Center")) {
            setCameraPosition(Vector2(0, 0));
        }
        ImGui::SameLine();
        if (ImGui::SmallButton("1:1")) {
            resetZoom();
        }
        
        ImGui::Text("Zoom: %.1f", m_zoomLevel);
        ImGui::SameLine();
        if (ImGui::SmallButton("+")) {
            setZoomLevel(m_zoomLevel + 0.1f);
        }
        ImGui::SameLine();
        if (ImGui::SmallButton("-")) {
            setZoomLevel(m_zoomLevel - 0.1f);
        }        // Viewport rendering area
        renderViewport();
        
        // Save window size if it changed
        ImVec2 currentWindowSize = ImGui::GetWindowSize();
        if (currentWindowSize.x != m_lastWindowSize.x || currentWindowSize.y != m_lastWindowSize.y) {
            saveWindowSize();
            m_lastWindowSize = currentWindowSize;
        }
    }
    ImGui::End();
}

void SceneWindow::renderViewport() {
    // Get available space for the viewport
    ImVec2 contentRegion = ImGui::GetContentRegionAvail();
    m_viewportSize = Vector2(contentRegion.x, contentRegion.y);
    
    // Check if viewport is focused
    m_viewportFocused = ImGui::IsWindowFocused();
    
    // Create a child window for the viewport
    if (ImGui::BeginChild("SceneViewport", contentRegion, false, ImGuiWindowFlags_NoMove)) {
        // Render the scene content
        renderSceneContent();
        
        // Handle input when viewport is focused
        if (ImGui::IsWindowHovered()) {
            handleInput();
        }
    }
    ImGui::EndChild();
}

void SceneWindow::renderSceneContent() {
    if (!m_scene) return;
    
    auto& engine = Engine::getInstance();
    auto renderer = engine.getRenderer();
    if (!renderer) return;
    
    // Set camera position for this scene window
    Vector2 originalCamera = renderer->getCamera();
    renderer->setCamera(m_cameraPosition);
    
    // Get ImGui draw list for custom rendering
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    ImVec2 canvasPos = ImGui::GetCursorScreenPos();
    ImVec2 canvasSize = ImGui::GetContentRegionAvail();
    
    // Background
    drawList->AddRectFilled(
        canvasPos,
        ImVec2(canvasPos.x + canvasSize.x, canvasPos.y + canvasSize.y),
        IM_COL32(50, 50, 60, 255)    );
      // Render procedural map tiles efficiently (if present)
    if (m_proceduralMap && m_tileRenderer) {
        Vector2 canvasPosVec(canvasPos.x, canvasPos.y);
        m_tileRenderer->render(drawList, m_cameraPosition, m_viewportSize, m_zoomLevel, canvasPosVec);
    }
      // Render entities in the scene
    if (m_scene) {
        // Get all entities and filter those with transform components
        auto allEntities = m_scene->getAllLivingEntities();
        
        for (EntityID entity : allEntities) {
            if (!m_scene->hasComponent<Transform>(entity)) continue;
            
            auto& transform = m_scene->getComponent<Transform>(entity);            // Convert world position to screen position (matching TileRenderer system)
            Vector2 worldPos = transform.position;
            Vector2 screenPos;
            screenPos.x = (worldPos.x - m_cameraPosition.x) * m_zoomLevel;
            screenPos.y = (worldPos.y - m_cameraPosition.y) * m_zoomLevel;
            
            // Apply canvas offset (same as TileRenderer)
            screenPos.x += canvasPos.x + canvasSize.x / 2;
            screenPos.y += canvasPos.y + canvasSize.y / 2;
              // Check if entity has a sprite component
            if (m_scene->hasComponent<Sprite>(entity)) {
                auto& sprite = m_scene->getComponent<Sprite>(entity);
                if (sprite.texture && sprite.visible) {
                    // Draw actual texture image
                    SDL_Texture* sdlTexture = sprite.texture->getSDLTexture();
                    ImTextureID textureID = (ImTextureID)(intptr_t)sdlTexture;                    // Use texture dimensions, applying transform scale and zoom
                    float baseSize = 32.0f; // Base tile size
                    float texWidth = sprite.texture->getWidth();
                    float texHeight = sprite.texture->getHeight();
                    
                    // Apply transform scale to the base dimensions
                    float scaledWidth = texWidth * transform.scale.x * m_zoomLevel;
                    float scaledHeight = texHeight * transform.scale.y * m_zoomLevel;
                    
                    // For rotation, we need to calculate rotated corners
                    if (transform.rotation != 0.0f) {
                        // Convert rotation to radians
                        float angleRad = transform.rotation * (3.14159f / 180.0f);
                        float cosA = cos(angleRad);
                        float sinA = sin(angleRad);
                        
                        // Calculate the four corners of the rotated rectangle
                        float halfW = scaledWidth / 2.0f;
                        float halfH = scaledHeight / 2.0f;
                        
                        ImVec2 corners[4] = {
                            {screenPos.x + (-halfW * cosA - -halfH * sinA), screenPos.y + (-halfW * sinA + -halfH * cosA)},
                            {screenPos.x + (halfW * cosA - -halfH * sinA), screenPos.y + (halfW * sinA + -halfH * cosA)},
                            {screenPos.x + (halfW * cosA - halfH * sinA), screenPos.y + (halfW * sinA + halfH * cosA)},
                            {screenPos.x + (-halfW * cosA - halfH * sinA), screenPos.y + (-halfW * sinA + halfH * cosA)}
                        };
                        
                        // Draw rotated image using AddImageQuad
                        SDL_Texture* sdlTexture = sprite.texture->getSDLTexture();
                        ImTextureID textureID = (ImTextureID)(intptr_t)sdlTexture;
                        
                        drawList->AddImageQuad(textureID, 
                                             corners[0], corners[1], corners[2], corners[3],
                                             ImVec2(0, 0), ImVec2(1, 0), ImVec2(1, 1), ImVec2(0, 1));
                        
                        // Debug output for transformed sprites
                        printf("SCENEWINDOW RENDER DEBUG: Entity %u - Scale: %.2f,%.2f Rotation: %.1f\n", 
                               entity, transform.scale.x, transform.scale.y, transform.rotation);
                        printf("  Scaled size: %.1f x %.1f\n", scaledWidth, scaledHeight);
                        fflush(stdout);
                        
                        // Draw selection border around rotated sprite (approximate)
                        if (entity == m_selectedEntity) {
                            drawList->AddPolyline(corners, 4, IM_COL32(255, 200, 100, 255), ImDrawFlags_Closed, 2.0f);
                        }
                    } else {
                        // No rotation - use simple AddImage
                        ImVec2 imageMin(screenPos.x - scaledWidth/2, screenPos.y - scaledHeight/2);
                        ImVec2 imageMax(screenPos.x + scaledWidth/2, screenPos.y + scaledHeight/2);
                        
                        // Draw the texture
                        SDL_Texture* sdlTexture = sprite.texture->getSDLTexture();
                        ImTextureID textureID = (ImTextureID)(intptr_t)sdlTexture;
                        drawList->AddImage(textureID, imageMin, imageMax);
                        
                        // Debug output for scaled sprites
                        if (transform.scale.x != 1.0f || transform.scale.y != 1.0f) {
                            printf("SCENEWINDOW RENDER DEBUG: Entity %u - Scale: %.2f,%.2f (no rotation)\n", 
                                   entity, transform.scale.x, transform.scale.y);
                            printf("  Scaled size: %.1f x %.1f\n", scaledWidth, scaledHeight);
                            fflush(stdout);
                        }
                        
                        // Draw selection border
                        if (entity == m_selectedEntity) {
                            drawList->AddRect(imageMin, imageMax, IM_COL32(255, 200, 100, 255), 0.0f, 0, 2.0f);
                        }
                    }                    
                    // Draw entity name
                    std::string entityName = m_scene->getEntityName(entity);
                    if (!entityName.empty()) {
                        drawList->AddText(
                            ImVec2(screenPos.x + scaledWidth/2 + 2, screenPos.y - 8),
                            IM_COL32(255, 255, 255, 200),
                            entityName.c_str()
                        );
                    }} else {
                    // Draw a simple dot for entities with sprites but no texture/not visible
                    ImU32 color = IM_COL32(100, 100, 100, 255);
                    if (entity == m_selectedEntity) {
                        color = IM_COL32(255, 200, 100, 255);
                    }
                    drawList->AddCircleFilled(ImVec2(screenPos.x, screenPos.y), 3.0f * m_zoomLevel, color);
                }
            } else {
                // Draw a simple dot for entities without sprites
                ImU32 color = IM_COL32(150, 150, 150, 255);
                if (entity == m_selectedEntity) {
                    color = IM_COL32(255, 200, 100, 255);
                }
                drawList->AddCircleFilled(ImVec2(screenPos.x, screenPos.y), 3.0f * m_zoomLevel, color);
            }
        }
    }
      // Grid overlay
    if (canvasSize.x > 0 && canvasSize.y > 0 && m_zoomLevel > 0.5f) {
        float gridSize = 32.0f * m_zoomLevel;
        ImU32 gridColor = IM_COL32(70, 70, 80, 100);
        
        // Vertical lines
        for (float x = fmod(-m_cameraPosition.x * m_zoomLevel, gridSize); x < canvasSize.x; x += gridSize) {
            drawList->AddLine(
                ImVec2(canvasPos.x + x, canvasPos.y),
                ImVec2(canvasPos.x + x, canvasPos.y + canvasSize.y),
                gridColor
            );
        }
        
        // Horizontal lines
        for (float y = fmod(-m_cameraPosition.y * m_zoomLevel, gridSize); y < canvasSize.y; y += gridSize) {
            drawList->AddLine(
                ImVec2(canvasPos.x, canvasPos.y + y),
                ImVec2(canvasPos.x + canvasSize.x, canvasPos.y + y),
                gridColor
            );        }
    }
    
    // Restore original camera position
    renderer->setCamera(originalCamera);
}

void SceneWindow::handleInput() {
    ImGuiIO& io = ImGui::GetIO();
      // Mouse wheel zoom towards cursor
    if (io.MouseWheel != 0.0f) {
        ImVec2 mousePos = ImGui::GetMousePos();
        ImVec2 canvasPos = ImGui::GetCursorScreenPos();
        ImVec2 canvasSize = ImGui::GetContentRegionAvail();
        
        // Convert mouse position to world coordinates before zoom
        ImVec2 canvasCenter = ImVec2(canvasPos.x + canvasSize.x / 2, canvasPos.y + canvasSize.y / 2);
        Vector2 worldMousePos;
        worldMousePos.x = ((mousePos.x - canvasCenter.x) / m_zoomLevel) + m_cameraPosition.x;
        worldMousePos.y = ((mousePos.y - canvasCenter.y) / m_zoomLevel) + m_cameraPosition.y;
        
        // Apply zoom
        float zoomFactor = 1.0f + (io.MouseWheel * 0.1f);
        float newZoomLevel = std::max(m_minZoom, std::min(m_maxZoom, m_zoomLevel * zoomFactor));
        
        // Convert mouse position to world coordinates after zoom
        Vector2 newWorldMousePos;
        newWorldMousePos.x = ((mousePos.x - canvasCenter.x) / newZoomLevel) + m_cameraPosition.x;
        newWorldMousePos.y = ((mousePos.y - canvasCenter.y) / newZoomLevel) + m_cameraPosition.y;
        
        // Adjust camera position to keep the mouse world position the same
        m_cameraPosition.x += (worldMousePos.x - newWorldMousePos.x);
        m_cameraPosition.y += (worldMousePos.y - newWorldMousePos.y);
        
        // Set the new zoom level
        m_zoomLevel = newZoomLevel;
    }
      // Camera panning with middle mouse button or right mouse button
    if (ImGui::IsMouseDragging(ImGuiMouseButton_Middle) || 
        (ImGui::IsMouseDragging(ImGuiMouseButton_Right) && !io.KeyCtrl)) {
        ImVec2 delta = ImGui::GetMouseDragDelta(ImGuiMouseButton_Middle);
        if (delta.x != 0 || delta.y != 0) {
            m_cameraPosition.x -= delta.x / m_zoomLevel;
            m_cameraPosition.y -= delta.y / m_zoomLevel;
            ImGui::ResetMouseDragDelta(ImGuiMouseButton_Middle);
        }
        
        delta = ImGui::GetMouseDragDelta(ImGuiMouseButton_Right);
        if (delta.x != 0 || delta.y != 0) {
            m_cameraPosition.x -= delta.x / m_zoomLevel;
            m_cameraPosition.y -= delta.y / m_zoomLevel;
            ImGui::ResetMouseDragDelta(ImGuiMouseButton_Right);
        }
    }

    // Left mouse button handling: drag to pan, click to select
    if (ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
        ImVec2 mousePos = ImGui::GetMousePos();
        
        if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
            // Start tracking for potential drag
            m_isDragging = false;
            m_dragStartPos = mousePos;
        } else if (!m_isDragging) {
            // Check if we've moved far enough to start dragging
            ImVec2 dragDelta = ImVec2(mousePos.x - m_dragStartPos.x, mousePos.y - m_dragStartPos.y);
            float dragDistance = sqrt(dragDelta.x * dragDelta.x + dragDelta.y * dragDelta.y);
            
            if (dragDistance > m_dragThreshold) {
                m_isDragging = true;
            }
        }
        
        if (m_isDragging) {
            // Pan the camera based on mouse movement
            ImVec2 delta = ImGui::GetMouseDragDelta(ImGuiMouseButton_Left);
            if (delta.x != 0 || delta.y != 0) {
                m_cameraPosition.x -= delta.x / m_zoomLevel;
                m_cameraPosition.y -= delta.y / m_zoomLevel;
                ImGui::ResetMouseDragDelta(ImGuiMouseButton_Left);
            }
        }
    }
    
    // Entity selection with left mouse button (only if not dragging)
    if (ImGui::IsMouseReleased(ImGuiMouseButton_Left) && !m_isDragging) {
        ImVec2 mousePos = ImGui::GetMousePos();
        ImVec2 canvasPos = ImGui::GetCursorScreenPos();
        ImVec2 canvasSize = ImGui::GetContentRegionAvail();
        
        // Convert mouse position to world coordinates (matching TileRenderer coordinate system)
        ImVec2 canvasCenter = ImVec2(canvasPos.x + canvasSize.x / 2, canvasPos.y + canvasSize.y / 2);
        Vector2 worldMousePos;
        // Use the same coordinate transformation as TileRenderer
        worldMousePos.x = ((mousePos.x - canvasCenter.x) / m_zoomLevel) + m_cameraPosition.x;
        worldMousePos.y = ((mousePos.y - canvasCenter.y) / m_zoomLevel) + m_cameraPosition.y;
          // Find entity at mouse position (using consistent coordinate system)
        EntityID clickedEntity = 0;
        float closestDistance = 16.0f / m_zoomLevel; // Click tolerance adjusted for zoom
        
        if (m_scene) {
            auto allEntities = m_scene->getAllLivingEntities();
            for (EntityID entity : allEntities) {
                if (!m_scene->hasComponent<Transform>(entity)) continue;
                
                auto& transform = m_scene->getComponent<Transform>(entity);
                Vector2 entityPos = transform.position;
                float distance = sqrt(pow(worldMousePos.x - entityPos.x, 2) + 
                                    pow(worldMousePos.y - entityPos.y, 2));
                
                if (distance < closestDistance) {
                    closestDistance = distance;
                    clickedEntity = entity;
                }
            }
        }
        
        // If no entity was found, check for procedural tiles and convert to entity
        if (clickedEntity == 0 && m_proceduralMap) {
            // Convert world position to tile coordinates
            Vector2 gridPos = m_proceduralMap->getGridPosition(worldMousePos);
            int tileX = static_cast<int>(gridPos.x);
            int tileY = static_cast<int>(gridPos.y);
            
            if (m_proceduralMap->isValidPosition(tileX, tileY)) {
                const Tile& tile = m_proceduralMap->getTile(tileX, tileY);
                
                // Convert non-empty tiles to entities for selection and editing
                if (tile.type != TileType::Empty) {
                    clickedEntity = convertTileToEntity(tileX, tileY, tile);
                }
            }
        }
          setSelectedEntity(clickedEntity);
        
        // Reset drag state after selection
        m_isDragging = false;
    }
    
    // Reset drag state when mouse is released
    if (ImGui::IsMouseReleased(ImGuiMouseButton_Left)) {
        m_isDragging = false;
    }
      // Keyboard shortcuts
    if (m_viewportFocused) {
        // Zoom shortcuts
        if (ImGui::IsKeyPressed(ImGuiKey_Equal) || ImGui::IsKeyPressed(ImGuiKey_KeypadAdd)) {
            zoomIn();
        }
        if (ImGui::IsKeyPressed(ImGuiKey_Minus) || ImGui::IsKeyPressed(ImGuiKey_KeypadSubtract)) {
            zoomOut();
        }
        if (ImGui::IsKeyPressed(ImGuiKey_1)) {
            resetZoom();
        }
        
        // WASD camera movement
        float moveSpeed = 5.0f;
        if (ImGui::IsKeyDown(ImGuiKey_W) || ImGui::IsKeyDown(ImGuiKey_UpArrow)) {
            m_cameraPosition.y -= moveSpeed;
        }
        if (ImGui::IsKeyDown(ImGuiKey_S) || ImGui::IsKeyDown(ImGuiKey_DownArrow)) {
            m_cameraPosition.y += moveSpeed;
        }
        if (ImGui::IsKeyDown(ImGuiKey_A) || ImGui::IsKeyDown(ImGuiKey_LeftArrow)) {
            m_cameraPosition.x -= moveSpeed;
        }
        if (ImGui::IsKeyDown(ImGuiKey_D) || ImGui::IsKeyDown(ImGuiKey_RightArrow)) {
            m_cameraPosition.x += moveSpeed;
        }
    }
    
    // Handle keyboard shortcuts
    if (ImGui::IsWindowFocused() && ImGui::IsKeyPressed(ImGuiKey_S) && ImGui::GetIO().KeyCtrl) {
        // Trigger Ctrl+S save
        if (!m_filePath.empty() && m_editor) {
            auto sceneManager = m_editor->getSceneManager();
            if (sceneManager) {
                sceneManager->markSceneDirty(m_filePath);
                auto* sceneInfo = sceneManager->findSceneByPath(m_filePath);
                if (sceneInfo && sceneInfo->isLoaded) {
                    sceneManager->saveScene(*sceneInfo);
                    m_isDirty = false;
                }
            }
        }
    }
}

EntityID SceneWindow::convertTileToEntity(int tileX, int tileY, const Tile& tile) {
    if (!m_scene) return 0;
    
    // Create a new entity for this tile
    EntityID entity = m_scene->createEntity();
    
    // Add transform component
    Vector2 worldPos = m_proceduralMap->getWorldPosition(tileX, tileY);
    m_scene->addComponent<Transform>(entity, Transform(worldPos));
    
    // Add sprite component with the tile's texture
    auto& engine = Engine::getInstance();
    auto resourceManager = engine.getResourceManager();
    if (resourceManager && !tile.spriteName.empty()) {
        Sprite sprite;
        auto texture = resourceManager->loadTexture(tile.spriteName);
        if (texture) {
            sprite.texture = texture;
            sprite.sourceRect = Rect(0, 0, texture->getWidth(), texture->getHeight());
            sprite.visible = true;
            sprite.layer = 0;
        }
        m_scene->addComponent<Sprite>(entity, sprite);
    }
    
    // Set entity name based on tile type and position
    std::string tileName = "Procedural_";
    if (m_proceduralMap->getSpriteManager()) {
        tileName += m_proceduralMap->getSpriteManager()->getTileTypeName(tile.type);
    } else {
        tileName += "Tile";
    }
    tileName += "_" + std::to_string(tileX) + "_" + std::to_string(tileY);
    m_scene->setEntityName(entity, tileName);
    
    // Mark scene as dirty
    setDirty(true);
    
    // Note: The tile will still be rendered by TileRenderer, but now we also have
    // a selectable entity on top of it. This allows editing while maintaining performance.
    
    return entity;
}
