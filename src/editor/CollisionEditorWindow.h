#pragma once

#include "../components/Components.h"
#include "../scene/Scene.h"
#include <imgui.h>
#include <memory>
#include <vector>

class GameEditor;

class CollisionEditorWindow {
public:
    CollisionEditorWindow(GameEditor* editor);
    ~CollisionEditorWindow();

    void render();
    void setSelectedEntity(EntityID entity, std::shared_ptr<Scene> scene);
    
    bool isOpen() const { return m_isOpen; }
    void setOpen(bool open) { m_isOpen = open; }

private:
    void renderCollisionEditor();
    void renderCollisionVisualization();
    void renderSpriteWithCollision();
    void handleCollisionEditing();
    void handlePaintbrushTool();
    void resetCollisionToDefaults();
    void fitCollisionToSprite();
    void initializeCollisionMask();
    void updateCollisionFromMask();
    
    GameEditor* m_editor;
    bool m_isOpen = true;
    
    // Current entity being edited
    EntityID m_selectedEntity = 0;
    std::shared_ptr<Scene> m_currentScene = nullptr;
    
    // Editor state
    bool m_isDragging = false;
    bool m_isResizing = false;
    enum class ResizeHandle {
        None,
        TopLeft,
        TopRight,
        BottomLeft,
        BottomRight,
        Top,
        Bottom,
        Left,
        Right,
        Center
    } m_activeHandle = ResizeHandle::None;
    
    ImVec2 m_dragStartPos;
    Vector2 m_originalOffset;
    Vector2 m_originalSize;
    
    // Paintbrush tool state
    bool m_paintbrushMode = false;
    bool m_eraseMode = false;
    float m_brushSize = 8.0f;
    bool m_isPainting = false;
    bool m_showGrid = true;
    
    // Collision pixel mask for precise collision editing
    std::vector<std::vector<bool>> m_collisionMask;
    int m_maskWidth = 0;
    int m_maskHeight = 0;
    EntityID m_lastEntity = 0; // Track when entity changes to reinitialize mask
    
    // Visualization settings
    float m_visualizationScale = 4.0f;  // Scale factor for the collision editor
    ImVec2 m_editorSize = ImVec2(400, 400);
    ImVec2 m_windowSize = ImVec2(600, 700); // Resizable window size
    
    // Colors
    ImU32 m_collisionColor = IM_COL32(255, 100, 100, 100);  // Semi-transparent red
    ImU32 m_collisionBorderColor = IM_COL32(255, 100, 100, 255);  // Solid red
    ImU32 m_handleColor = IM_COL32(255, 255, 255, 255);  // White handles
    ImU32 m_spriteColor = IM_COL32(200, 200, 200, 255);  // Light gray for sprite
    ImU32 m_gridColor = IM_COL32(80, 80, 80, 255);  // Grid color
};
