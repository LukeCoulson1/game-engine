#pragma once

#include "../graphics/Renderer.h"
#include "../generation/ProceduralGeneration.h"
#include <memory>
#include <vector>
#include <unordered_map>

// Forward declaration for ImGui
struct ImDrawList;

// Efficient tile rendering system for large procedural maps
class TileRenderer {
public:
    struct TileBatch {
        std::shared_ptr<Texture> texture;
        std::vector<Vector2> positions;
        std::vector<Rect> sourceRects;
    };
    
    TileRenderer();
    ~TileRenderer();
    
    // Set the map to render
    void setMap(std::shared_ptr<ProceduralMap> map);
    
    // Render visible tiles only (frustum culling)
    void render(ImDrawList* drawList, const Vector2& cameraPos, const Vector2& viewportSize, 
                float zoomLevel, const Vector2& canvasPos);
    
    // Update tile batches when map changes
    void rebuildBatches();
    
    // Get number of tiles being rendered (for performance metrics)
    int getVisibleTileCount() const { return m_visibleTileCount; }
    int getTotalTileCount() const { return m_totalTileCount; }
    
private:
    void buildBatchesFromMap();
    bool isTileVisible(const Vector2& tilePos, const Vector2& cameraPos, 
                      const Vector2& viewportSize, float zoomLevel, float tileSize) const;
    
    std::shared_ptr<ProceduralMap> m_map;
    std::vector<TileBatch> m_tileBatches;
    std::unordered_map<std::string, std::shared_ptr<Texture>> m_textureCache;
    
    int m_visibleTileCount = 0;
    int m_totalTileCount = 0;
    float m_tileSize = 32.0f;
};
