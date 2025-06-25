#pragma once

#include "../scene/Scene.h"
#include "../graphics/Renderer.h"
#include "../rendering/TileRenderer.h"
#include <imgui.h>
#include <memory>
#include <string>

// Forward declarations
class GameEditor;
struct Tile;

class SceneWindow {
public:
    SceneWindow(const std::string& title, std::shared_ptr<Scene> scene, GameEditor* editor);
    ~SceneWindow();
    
    // Window management
    void render();
    bool isOpen() const { return m_isOpen; }
    void close() { m_isOpen = false; }
    
    // Scene access
    std::shared_ptr<Scene> getScene() const { return m_scene; }
    void setScene(std::shared_ptr<Scene> scene) { m_scene = scene; }
    
    // Window properties
    const std::string& getTitle() const { return m_title; }
    void setTitle(const std::string& title) { m_title = title; }
    
    // Window size management
    void saveWindowSize();
    void restoreWindowSize();
    
    // Selection state
    EntityID getSelectedEntity() const { return m_selectedEntity; }
    void setSelectedEntity(EntityID entity);
    bool hasSelectedEntity() const { return m_selectedEntity != 0; }
    
    // Viewport state
    Vector2 getViewportSize() const { return m_viewportSize; }
    bool isViewportFocused() const { return m_viewportFocused; }
      // Camera controls
    Vector2 getCameraPosition() const { return m_cameraPosition; }
    void setCameraPosition(const Vector2& position) { m_cameraPosition = position; }
      // Zoom controls
    float getZoomLevel() const { return m_zoomLevel; }
    void setZoomLevel(float zoom);
    void zoomIn() { setZoomLevel(m_zoomLevel * 1.2f); }
    void zoomOut() { setZoomLevel(m_zoomLevel / 1.2f); }
    void resetZoom() { setZoomLevel(1.0f); }
    
    // Procedural map rendering
    void setProceduralMap(std::shared_ptr<ProceduralMap> map);
    std::shared_ptr<ProceduralMap> getProceduralMap() const { return m_proceduralMap; }
    
    // File operations
    const std::string& getFilePath() const { return m_filePath; }
    void setFilePath(const std::string& path) { m_filePath = path; }
    bool isDirty() const { return m_isDirty; }
    void setDirty(bool dirty) { m_isDirty = dirty; }
    
    // Scene window ID for ImGui
    std::string getWindowID() const;

private:
    void renderViewport();
    void renderSceneContent();
    void handleInput();
    
    // Tile-to-entity conversion for procedural content selection
    EntityID convertTileToEntity(int tileX, int tileY, const Tile& tile);
    
    std::string m_title;
    std::shared_ptr<Scene> m_scene;
    GameEditor* m_editor;
    
    bool m_isOpen = true;    EntityID m_selectedEntity = 0;
    Vector2 m_viewportSize{800, 600};
    bool m_viewportFocused = false;
    Vector2 m_cameraPosition{0, 0};    float m_zoomLevel = 1.0f;
    const float m_minZoom = 0.1f;
    const float m_maxZoom = 10.0f;
    
    // Mouse interaction state
    bool m_isDragging = false;
    ImVec2 m_dragStartPos{0, 0};
    const float m_dragThreshold = 5.0f; // Pixels to move before starting drag
    
    // Efficient tile rendering
    std::shared_ptr<ProceduralMap> m_proceduralMap;
    std::unique_ptr<TileRenderer> m_tileRenderer;
    
    std::string m_filePath;
    bool m_isDirty = false;
    
    // Window size tracking for persistence
    ImVec2 m_lastWindowSize{800, 600};
    
    // Unique identifier for this window
    static int s_windowCounter;
    int m_windowId;
};
