#include "CollisionEditorWindow.h"
#include "GameEditor.h"
#include "../core/Engine.h"
#include "../utils/ResourceManager.h"
#include <algorithm>
#include <cmath>

CollisionEditorWindow::CollisionEditorWindow(GameEditor* editor) 
    : m_editor(editor) {
}

CollisionEditorWindow::~CollisionEditorWindow() {
}

void CollisionEditorWindow::render() {
    if (!m_isOpen) return;
    
    // Create resizable window
    ImGui::SetNextWindowSize(m_windowSize, ImGuiCond_FirstUseEver);
    
    if (ImGui::Begin("Collision Editor", &m_isOpen, ImGuiWindowFlags_None)) {
        
        // Store window size for next frame
        m_windowSize = ImGui::GetWindowSize();
        
        if (m_selectedEntity == 0 || !m_currentScene) {
            ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "No entity selected");
            ImGui::Text("Select an entity in the scene to edit its collision data");
            ImGui::End();
            return;
        }
        
        // Check if entity still exists
        if (!m_currentScene->hasComponent<Transform>(m_selectedEntity)) {
            ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f), "Selected entity no longer exists");
            m_selectedEntity = 0;
            m_currentScene = nullptr;
            ImGui::End();
            return;
        }
        
        // Initialize collision mask if entity changed
        if (m_lastEntity != m_selectedEntity) {
            initializeCollisionMask();
            m_lastEntity = m_selectedEntity;
        }
        
        // Show entity info
        std::string entityName = m_currentScene->getEntityName(m_selectedEntity);
        ImGui::Text("Entity: %s (ID: %u)", entityName.c_str(), m_selectedEntity);
        
        ImGui::Separator();
        
        // Collision component controls
        bool hasCollider = m_currentScene->hasComponent<Collider>(m_selectedEntity);
        
        if (ImGui::Checkbox("Has Collider", &hasCollider)) {
            if (hasCollider && !m_currentScene->hasComponent<Collider>(m_selectedEntity)) {
                // Add collider component with default values
                Collider defaultCollider;
                
                // Try to fit to sprite if available
                if (m_currentScene->hasComponent<Sprite>(m_selectedEntity)) {
                    auto& sprite = m_currentScene->getComponent<Sprite>(m_selectedEntity);
                    if (sprite.texture) {
                        defaultCollider.size = Vector2(sprite.texture->getWidth(), sprite.texture->getHeight());
                    }
                }
                
                m_currentScene->addComponent<Collider>(m_selectedEntity, defaultCollider);
                initializeCollisionMask(); // Reinitialize mask
            } else if (!hasCollider && m_currentScene->hasComponent<Collider>(m_selectedEntity)) {
                // Remove collider component
                m_currentScene->removeComponent<Collider>(m_selectedEntity);
            }
        }
        
        if (hasCollider) {
            renderCollisionEditor();
        }
    }
    ImGui::End();
}

void CollisionEditorWindow::renderCollisionEditor() {
    auto& collider = m_currentScene->getComponent<Collider>(m_selectedEntity);
    
    // Tool selection
    ImGui::Text("Tools:");
    int toolMode = m_paintbrushMode ? 1 : 0;
    if (ImGui::RadioButton("Handle Mode", &toolMode, 0)) {
        m_paintbrushMode = false;
    }
    ImGui::SameLine();
    if (ImGui::RadioButton("Paintbrush Mode", &toolMode, 1)) {
        m_paintbrushMode = true;
    }
    
    if (m_paintbrushMode) {
        ImGui::Checkbox("Erase Mode", &m_eraseMode);
        ImGui::SameLine();
        ImGui::SliderFloat("Brush Size", &m_brushSize, 1.0f, 20.0f, "%.1f");
        
        if (ImGui::Button("Clear All")) {
            // Clear collision mask
            for (auto& row : m_collisionMask) {
                std::fill(row.begin(), row.end(), false);
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("Fill All")) {
            // Fill collision mask
            for (auto& row : m_collisionMask) {
                std::fill(row.begin(), row.end(), true);
            }
        }
    }
    
    ImGui::Separator();
    
    // Collider properties
    ImGui::Text("Collision Properties:");
    
    // Offset controls
    float offset[2] = { collider.offset.x, collider.offset.y };
    if (ImGui::DragFloat2("Offset", offset, 0.5f)) {
        collider.offset = Vector2(offset[0], offset[1]);
    }
    
    // Size controls
    float size[2] = { collider.size.x, collider.size.y };
    if (ImGui::DragFloat2("Size", size, 0.5f, 1.0f, 1000.0f)) {
        collider.size = Vector2(std::max(1.0f, size[0]), std::max(1.0f, size[1]));
        initializeCollisionMask(); // Reinitialize mask when size changes
    }
    
    // Collider type controls
    ImGui::Checkbox("Is Trigger", &collider.isTrigger);
    ImGui::SameLine();
    ImGui::Checkbox("Is Static", &collider.isStatic);
    
    // Quick action buttons
    if (ImGui::Button("Reset to Default")) {
        resetCollisionToDefaults();
    }
    ImGui::SameLine();
    if (ImGui::Button("Fit to Sprite")) {
        fitCollisionToSprite();
    }
    ImGui::SameLine();
    if (ImGui::Button("Apply Mask")) {
        updateCollisionFromMask();
    }
    
    ImGui::Separator();
    
    // Visual editor settings
    ImGui::Text("Visual Editor:");
    ImGui::SliderFloat("Scale", &m_visualizationScale, 1.0f, 8.0f, "%.1fx");
    ImGui::Checkbox("Show Grid", &m_showGrid);
    
    // Calculate available space for the editor
    ImVec2 availableSize = ImGui::GetContentRegionAvail();
    availableSize.y = std::max(200.0f, availableSize.y - 20.0f); // Leave some padding
    
    // Visual collision editor
    renderSpriteWithCollision();
}

void CollisionEditorWindow::renderSpriteWithCollision() {
    if (!m_currentScene->hasComponent<Collider>(m_selectedEntity)) return;
    
    auto& collider = m_currentScene->getComponent<Collider>(m_selectedEntity);
    auto& transform = m_currentScene->getComponent<Transform>(m_selectedEntity);
    
    // Get sprite info
    bool hasSprite = m_currentScene->hasComponent<Sprite>(m_selectedEntity);
    float spriteWidth = 32.0f;
    float spriteHeight = 32.0f;
    SDL_Texture* spriteTexture = nullptr;
    
    if (hasSprite) {
        auto& sprite = m_currentScene->getComponent<Sprite>(m_selectedEntity);
        if (sprite.texture) {
            spriteWidth = sprite.texture->getWidth();
            spriteHeight = sprite.texture->getHeight();
            spriteTexture = sprite.texture->getSDLTexture();
        }
    }
    
    // Calculate editor dimensions based on available space
    ImVec2 availableSize = ImGui::GetContentRegionAvail();
    float maxDimension = std::max(spriteWidth, spriteHeight);
    float scaleToFit = std::min(availableSize.x / (maxDimension * 1.2f), availableSize.y / (maxDimension * 1.2f));
    m_visualizationScale = std::max(1.0f, std::min(8.0f, scaleToFit));
    
    float editorWidth = spriteWidth * m_visualizationScale;
    float editorHeight = spriteHeight * m_visualizationScale;
    m_editorSize = ImVec2(editorWidth, editorHeight);
    
    // Create child window for the visual editor
    if (ImGui::BeginChild("CollisionVisualizer", ImVec2(0, 0), true, ImGuiWindowFlags_NoScrollbar)) {
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        ImVec2 windowSize = ImGui::GetWindowSize();
        ImVec2 canvasPos = ImGui::GetCursorScreenPos();
        
        // Center the sprite in the available space
        ImVec2 canvasCenter = ImVec2(canvasPos.x + windowSize.x / 2, canvasPos.y + windowSize.y / 2);
        
        // Calculate sprite rectangle
        ImVec2 spriteMin = ImVec2(canvasCenter.x - editorWidth / 2, canvasCenter.y - editorHeight / 2);
        ImVec2 spriteMax = ImVec2(canvasCenter.x + editorWidth / 2, canvasCenter.y + editorHeight / 2);
        
        // Draw sprite if available
        if (spriteTexture) {
            ImTextureID textureID = (ImTextureID)(intptr_t)spriteTexture;
            drawList->AddImage(textureID, spriteMin, spriteMax);
        } else {
            // Draw placeholder rectangle
            drawList->AddRectFilled(spriteMin, spriteMax, IM_COL32(100, 100, 100, 100));
            drawList->AddRect(spriteMin, spriteMax, m_spriteColor, 0.0f, 0, 2.0f);
        }
        
        // Draw grid if enabled
        if (m_showGrid && m_visualizationScale >= 2.0f) {
            float gridSize = m_visualizationScale;
            for (float x = spriteMin.x; x <= spriteMax.x; x += gridSize) {
                drawList->AddLine(ImVec2(x, spriteMin.y), ImVec2(x, spriteMax.y), m_gridColor, 0.5f);
            }
            for (float y = spriteMin.y; y <= spriteMax.y; y += gridSize) {
                drawList->AddLine(ImVec2(spriteMin.x, y), ImVec2(spriteMax.x, y), m_gridColor, 0.5f);
            }
        }
        
        // Draw collision mask or handle-based collision
        if (m_paintbrushMode) {
            // Draw collision mask
            if (m_maskWidth > 0 && m_maskHeight > 0) {
                float pixelWidth = editorWidth / m_maskWidth;
                float pixelHeight = editorHeight / m_maskHeight;
                
                for (int y = 0; y < m_maskHeight; y++) {
                    for (int x = 0; x < m_maskWidth; x++) {
                        if (m_collisionMask[y][x]) {
                            ImVec2 pixelMin = ImVec2(spriteMin.x + x * pixelWidth, spriteMin.y + y * pixelHeight);
                            ImVec2 pixelMax = ImVec2(pixelMin.x + pixelWidth, pixelMin.y + pixelHeight);
                            drawList->AddRectFilled(pixelMin, pixelMax, m_collisionColor);
                        }
                    }
                }
            }
            
            // Handle paintbrush tool
            handlePaintbrushTool();
            
            // Draw brush preview
            if (ImGui::IsWindowHovered()) {
                ImVec2 mousePos = ImGui::GetMousePos();
                float brushRadius = m_brushSize * m_visualizationScale / 2.0f;
                drawList->AddCircle(mousePos, brushRadius, 
                                  m_eraseMode ? IM_COL32(255, 0, 0, 150) : IM_COL32(0, 255, 0, 150), 
                                  0, 2.0f);
            }
        } else {
            // Draw traditional collision rectangle
            renderCollisionVisualization();
            handleCollisionEditing();
        }
    }
    ImGui::EndChild();
}

void CollisionEditorWindow::renderCollisionVisualization() {
    if (!m_currentScene->hasComponent<Collider>(m_selectedEntity)) return;
    
    auto& collider = m_currentScene->getComponent<Collider>(m_selectedEntity);
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    ImVec2 windowSize = ImGui::GetWindowSize();
    ImVec2 canvasPos = ImGui::GetCursorScreenPos();
    ImVec2 canvasCenter = ImVec2(canvasPos.x + windowSize.x / 2, canvasPos.y + windowSize.y / 2);
    
    // Calculate collision rectangle
    float scaledOffsetX = collider.offset.x * m_visualizationScale;
    float scaledOffsetY = collider.offset.y * m_visualizationScale;
    float scaledSizeX = collider.size.x * m_visualizationScale;
    float scaledSizeY = collider.size.y * m_visualizationScale;
    
    ImVec2 collisionMin = ImVec2(canvasCenter.x + scaledOffsetX - scaledSizeX / 2,
                                 canvasCenter.y + scaledOffsetY - scaledSizeY / 2);
    ImVec2 collisionMax = ImVec2(canvasCenter.x + scaledOffsetX + scaledSizeX / 2,
                                 canvasCenter.y + scaledOffsetY + scaledSizeY / 2);
    
    // Draw collision rectangle
    drawList->AddRectFilled(collisionMin, collisionMax, m_collisionColor);
    drawList->AddRect(collisionMin, collisionMax, m_collisionBorderColor, 0.0f, 0, 2.0f);
    
    // Draw resize handles
    const float handleSize = 6.0f;
    ImVec2 handles[9] = {
        // Corners
        ImVec2(collisionMin.x - handleSize/2, collisionMin.y - handleSize/2), // Top-left
        ImVec2(collisionMax.x - handleSize/2, collisionMin.y - handleSize/2), // Top-right
        ImVec2(collisionMin.x - handleSize/2, collisionMax.y - handleSize/2), // Bottom-left
        ImVec2(collisionMax.x - handleSize/2, collisionMax.y - handleSize/2), // Bottom-right
        // Edges
        ImVec2((collisionMin.x + collisionMax.x)/2 - handleSize/2, collisionMin.y - handleSize/2), // Top
        ImVec2((collisionMin.x + collisionMax.x)/2 - handleSize/2, collisionMax.y - handleSize/2), // Bottom
        ImVec2(collisionMin.x - handleSize/2, (collisionMin.y + collisionMax.y)/2 - handleSize/2), // Left
        ImVec2(collisionMax.x - handleSize/2, (collisionMin.y + collisionMax.y)/2 - handleSize/2), // Right
        // Center
        ImVec2((collisionMin.x + collisionMax.x)/2 - handleSize/2, (collisionMin.y + collisionMax.y)/2 - handleSize/2) // Center
    };
    
    for (int i = 0; i < 9; i++) {
        ImVec2 handleMin = handles[i];
        ImVec2 handleMax = ImVec2(handleMin.x + handleSize, handleMin.y + handleSize);
        drawList->AddRectFilled(handleMin, handleMax, m_handleColor);
        drawList->AddRect(handleMin, handleMax, IM_COL32(0, 0, 0, 255));
    }
}

void CollisionEditorWindow::handlePaintbrushTool() {
    if (!ImGui::IsWindowHovered() || m_maskWidth == 0 || m_maskHeight == 0) return;
    
    ImVec2 mousePos = ImGui::GetMousePos();
    ImVec2 windowSize = ImGui::GetWindowSize();
    ImVec2 canvasPos = ImGui::GetCursorScreenPos();
    ImVec2 canvasCenter = ImVec2(canvasPos.x + windowSize.x / 2, canvasPos.y + windowSize.y / 2);
    
    float editorWidth = m_maskWidth * m_visualizationScale;
    float editorHeight = m_maskHeight * m_visualizationScale;
    ImVec2 spriteMin = ImVec2(canvasCenter.x - editorWidth / 2, canvasCenter.y - editorHeight / 2);
    
    if (ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
        // Convert mouse position to mask coordinates
        float relativeX = (mousePos.x - spriteMin.x) / m_visualizationScale;
        float relativeY = (mousePos.y - spriteMin.y) / m_visualizationScale;
        
        int centerX = static_cast<int>(relativeX);
        int centerY = static_cast<int>(relativeY);
        
        // Paint with brush
        int brushRadius = static_cast<int>(m_brushSize / 2.0f);
        for (int dy = -brushRadius; dy <= brushRadius; dy++) {
            for (int dx = -brushRadius; dx <= brushRadius; dx++) {
                int px = centerX + dx;
                int py = centerY + dy;
                
                if (px >= 0 && px < m_maskWidth && py >= 0 && py < m_maskHeight) {
                    float distance = sqrt(dx * dx + dy * dy);
                    if (distance <= brushRadius) {
                        m_collisionMask[py][px] = !m_eraseMode;
                    }
                }
            }
        }
    }
}

void CollisionEditorWindow::handleCollisionEditing() {
    if (!m_currentScene->hasComponent<Collider>(m_selectedEntity)) return;
    
    auto& collider = m_currentScene->getComponent<Collider>(m_selectedEntity);
    ImVec2 mousePos = ImGui::GetMousePos();
    ImVec2 windowSize = ImGui::GetWindowSize();
    ImVec2 canvasPos = ImGui::GetCursorScreenPos();
    ImVec2 canvasCenter = ImVec2(canvasPos.x + windowSize.x / 2, canvasPos.y + windowSize.y / 2);
    
    // Calculate collision rectangle
    float scaledOffsetX = collider.offset.x * m_visualizationScale;
    float scaledOffsetY = collider.offset.y * m_visualizationScale;
    float scaledSizeX = collider.size.x * m_visualizationScale;
    float scaledSizeY = collider.size.y * m_visualizationScale;
    
    ImVec2 collisionMin = ImVec2(canvasCenter.x + scaledOffsetX - scaledSizeX / 2,
                                 canvasCenter.y + scaledOffsetY - scaledSizeY / 2);
    ImVec2 collisionMax = ImVec2(canvasCenter.x + scaledOffsetX + scaledSizeX / 2,
                                 canvasCenter.y + scaledOffsetY + scaledSizeY / 2);
    
    if (ImGui::IsWindowHovered()) {
        if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
            m_isDragging = false;
            m_isResizing = false;
            m_activeHandle = ResizeHandle::None;
            
            const float handleSize = 6.0f;
            
            // Check for handle clicks
            ImVec2 handles[9] = {
                ImVec2(collisionMin.x - handleSize/2, collisionMin.y - handleSize/2), // Top-left
                ImVec2(collisionMax.x - handleSize/2, collisionMin.y - handleSize/2), // Top-right
                ImVec2(collisionMin.x - handleSize/2, collisionMax.y - handleSize/2), // Bottom-left
                ImVec2(collisionMax.x - handleSize/2, collisionMax.y - handleSize/2), // Bottom-right
                ImVec2((collisionMin.x + collisionMax.x)/2 - handleSize/2, collisionMin.y - handleSize/2), // Top
                ImVec2((collisionMin.x + collisionMax.x)/2 - handleSize/2, collisionMax.y - handleSize/2), // Bottom
                ImVec2(collisionMin.x - handleSize/2, (collisionMin.y + collisionMax.y)/2 - handleSize/2), // Left
                ImVec2(collisionMax.x - handleSize/2, (collisionMin.y + collisionMax.y)/2 - handleSize/2), // Right
                ImVec2((collisionMin.x + collisionMax.x)/2 - handleSize/2, (collisionMin.y + collisionMax.y)/2 - handleSize/2) // Center
            };
            
            ResizeHandle handleTypes[9] = {
                ResizeHandle::TopLeft, ResizeHandle::TopRight, ResizeHandle::BottomLeft, ResizeHandle::BottomRight,
                ResizeHandle::Top, ResizeHandle::Bottom, ResizeHandle::Left, ResizeHandle::Right, ResizeHandle::Center
            };
            
            for (int i = 0; i < 9; i++) {
                ImVec2 handleMin = handles[i];
                ImVec2 handleMax = ImVec2(handleMin.x + handleSize, handleMin.y + handleSize);
                
                if (mousePos.x >= handleMin.x && mousePos.x <= handleMax.x &&
                    mousePos.y >= handleMin.y && mousePos.y <= handleMax.y) {
                    m_activeHandle = handleTypes[i];
                    m_dragStartPos = mousePos;
                    m_originalOffset = collider.offset;
                    m_originalSize = collider.size;
                    m_isResizing = (i != 8); // Not center handle
                    m_isDragging = (i == 8); // Center handle
                    break;
                }
            }
        }
        
        // Handle dragging/resizing
        if ((m_isDragging || m_isResizing) && ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
            ImVec2 mouseDelta = ImVec2(mousePos.x - m_dragStartPos.x, mousePos.y - m_dragStartPos.y);
            
            if (m_isDragging && m_activeHandle == ResizeHandle::Center) {
                // Move the collision box
                collider.offset.x = m_originalOffset.x + mouseDelta.x / m_visualizationScale;
                collider.offset.y = m_originalOffset.y + mouseDelta.y / m_visualizationScale;
            } else if (m_isResizing) {
                // Resize the collision box based on handle
                float deltaX = mouseDelta.x / m_visualizationScale;
                float deltaY = mouseDelta.y / m_visualizationScale;
                
                switch (m_activeHandle) {
                    case ResizeHandle::TopLeft:
                        collider.size.x = std::max(1.0f, m_originalSize.x - deltaX);
                        collider.size.y = std::max(1.0f, m_originalSize.y - deltaY);
                        collider.offset.x = m_originalOffset.x + deltaX / 2;
                        collider.offset.y = m_originalOffset.y + deltaY / 2;
                        break;
                    case ResizeHandle::TopRight:
                        collider.size.x = std::max(1.0f, m_originalSize.x + deltaX);
                        collider.size.y = std::max(1.0f, m_originalSize.y - deltaY);
                        collider.offset.x = m_originalOffset.x + deltaX / 2;
                        collider.offset.y = m_originalOffset.y + deltaY / 2;
                        break;
                    case ResizeHandle::BottomLeft:
                        collider.size.x = std::max(1.0f, m_originalSize.x - deltaX);
                        collider.size.y = std::max(1.0f, m_originalSize.y + deltaY);
                        collider.offset.x = m_originalOffset.x + deltaX / 2;
                        collider.offset.y = m_originalOffset.y + deltaY / 2;
                        break;
                    case ResizeHandle::BottomRight:
                        collider.size.x = std::max(1.0f, m_originalSize.x + deltaX);
                        collider.size.y = std::max(1.0f, m_originalSize.y + deltaY);
                        collider.offset.x = m_originalOffset.x + deltaX / 2;
                        collider.offset.y = m_originalOffset.y + deltaY / 2;
                        break;
                    case ResizeHandle::Top:
                        collider.size.y = std::max(1.0f, m_originalSize.y - deltaY);
                        collider.offset.y = m_originalOffset.y + deltaY / 2;
                        break;
                    case ResizeHandle::Bottom:
                        collider.size.y = std::max(1.0f, m_originalSize.y + deltaY);
                        collider.offset.y = m_originalOffset.y + deltaY / 2;
                        break;
                    case ResizeHandle::Left:
                        collider.size.x = std::max(1.0f, m_originalSize.x - deltaX);
                        collider.offset.x = m_originalOffset.x + deltaX / 2;
                        break;
                    case ResizeHandle::Right:
                        collider.size.x = std::max(1.0f, m_originalSize.x + deltaX);
                        collider.offset.x = m_originalOffset.x + deltaX / 2;
                        break;
                }
            }
        }
        
        if (ImGui::IsMouseReleased(ImGuiMouseButton_Left)) {
            m_isDragging = false;
            m_isResizing = false;
            m_activeHandle = ResizeHandle::None;
        }
    }
}

void CollisionEditorWindow::initializeCollisionMask() {
    if (!m_currentScene || m_selectedEntity == 0) return;
    
    // Get sprite dimensions for mask size
    bool hasSprite = m_currentScene->hasComponent<Sprite>(m_selectedEntity);
    int spriteWidth = 32;
    int spriteHeight = 32;
    
    if (hasSprite) {
        auto& sprite = m_currentScene->getComponent<Sprite>(m_selectedEntity);
        if (sprite.texture) {
            spriteWidth = sprite.texture->getWidth();
            spriteHeight = sprite.texture->getHeight();
        }
    }
    
    // Initialize mask with appropriate resolution
    m_maskWidth = spriteWidth;
    m_maskHeight = spriteHeight;
    
    m_collisionMask.clear();
    m_collisionMask.resize(m_maskHeight, std::vector<bool>(m_maskWidth, false));
    
    // If entity has a collider, initialize mask based on current collision box
    if (m_currentScene->hasComponent<Collider>(m_selectedEntity)) {
        auto& collider = m_currentScene->getComponent<Collider>(m_selectedEntity);
        
        // Convert collision box to mask
        int startX = std::max(0, static_cast<int>(collider.offset.x - collider.size.x / 2 + spriteWidth / 2));
        int endX = std::min(m_maskWidth, static_cast<int>(collider.offset.x + collider.size.x / 2 + spriteWidth / 2));
        int startY = std::max(0, static_cast<int>(collider.offset.y - collider.size.y / 2 + spriteHeight / 2));
        int endY = std::min(m_maskHeight, static_cast<int>(collider.offset.y + collider.size.y / 2 + spriteHeight / 2));
        
        for (int y = startY; y < endY; y++) {
            for (int x = startX; x < endX; x++) {
                m_collisionMask[y][x] = true;
            }
        }
    }
}

void CollisionEditorWindow::updateCollisionFromMask() {
    if (!m_currentScene || m_selectedEntity == 0 || !m_currentScene->hasComponent<Collider>(m_selectedEntity)) return;
    
    auto& collider = m_currentScene->getComponent<Collider>(m_selectedEntity);
    
    // Find bounding box of all true pixels in mask
    int minX = m_maskWidth, maxX = -1;
    int minY = m_maskHeight, maxY = -1;
    
    for (int y = 0; y < m_maskHeight; y++) {
        for (int x = 0; x < m_maskWidth; x++) {
            if (m_collisionMask[y][x]) {
                minX = std::min(minX, x);
                maxX = std::max(maxX, x);
                minY = std::min(minY, y);
                maxY = std::max(maxY, y);
            }
        }
    }
    
    if (maxX >= minX && maxY >= minY) {
        // Calculate size and offset relative to sprite center
        collider.size.x = maxX - minX + 1;
        collider.size.y = maxY - minY + 1;
        
        // Calculate offset from sprite center
        float centerX = (minX + maxX) / 2.0f;
        float centerY = (minY + maxY) / 2.0f;
        collider.offset.x = centerX - m_maskWidth / 2.0f;
        collider.offset.y = centerY - m_maskHeight / 2.0f;
    }
}

void CollisionEditorWindow::resetCollisionToDefaults() {
    if (!m_currentScene || m_selectedEntity == 0 || !m_currentScene->hasComponent<Collider>(m_selectedEntity)) return;
    
    auto& collider = m_currentScene->getComponent<Collider>(m_selectedEntity);
    collider.offset = Vector2(0, 0);
    collider.size = Vector2(32, 32);
    collider.isTrigger = false;
    collider.isStatic = false;
    
    initializeCollisionMask();
}

void CollisionEditorWindow::fitCollisionToSprite() {
    if (!m_currentScene || m_selectedEntity == 0 || !m_currentScene->hasComponent<Collider>(m_selectedEntity)) return;
    
    auto& collider = m_currentScene->getComponent<Collider>(m_selectedEntity);
    
    if (m_currentScene->hasComponent<Sprite>(m_selectedEntity)) {
        auto& sprite = m_currentScene->getComponent<Sprite>(m_selectedEntity);
        if (sprite.texture) {
            collider.offset = Vector2(0, 0);
            collider.size = Vector2(sprite.texture->getWidth(), sprite.texture->getHeight());
            initializeCollisionMask();
        }
    }
}

void CollisionEditorWindow::setSelectedEntity(EntityID entity, std::shared_ptr<Scene> scene) {
    m_selectedEntity = entity;
    m_currentScene = scene;
    
    if (entity != 0 && scene) {
        initializeCollisionMask();
    }
}
