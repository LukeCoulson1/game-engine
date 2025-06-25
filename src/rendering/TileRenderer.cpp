#include "TileRenderer.h"
#include "../core/Engine.h"
#include "../utils/ResourceManager.h"
#include <imgui.h>
#include <algorithm>

TileRenderer::TileRenderer() {
}

TileRenderer::~TileRenderer() {
}

void TileRenderer::setMap(std::shared_ptr<ProceduralMap> map) {
    m_map = map;
    rebuildBatches();
}

void TileRenderer::rebuildBatches() {
    if (!m_map) return;
    
    m_tileBatches.clear();
    m_textureCache.clear();
    buildBatchesFromMap();
}

void TileRenderer::buildBatchesFromMap() {
    auto& engine = Engine::getInstance();
    auto resourceManager = engine.getResourceManager();
    if (!resourceManager) return;
    
    std::unordered_map<std::string, TileBatch> batchMap;
    m_totalTileCount = 0;
    
    // Group tiles by texture for efficient batch rendering
    for (int y = 0; y < m_map->getHeight(); ++y) {
        for (int x = 0; x < m_map->getWidth(); ++x) {
            const Tile& tile = m_map->getTile(x, y);
            
            if (tile.type != TileType::Empty && !tile.spriteName.empty()) {
                m_totalTileCount++;
                
                // Get or load texture
                std::shared_ptr<Texture> texture;
                auto texIt = m_textureCache.find(tile.spriteName);
                if (texIt != m_textureCache.end()) {
                    texture = texIt->second;
                } else {
                    texture = resourceManager->loadTexture(tile.spriteName);
                    if (texture) {
                        m_textureCache[tile.spriteName] = texture;
                    }
                }
                
                if (texture) {
                    // Add to batch for this texture
                    TileBatch& batch = batchMap[tile.spriteName];
                    if (!batch.texture) {
                        batch.texture = texture;
                    }
                    
                    Vector2 worldPos = m_map->getWorldPosition(x, y);
                    batch.positions.push_back(worldPos);
                    batch.sourceRects.push_back(Rect(0, 0, texture->getWidth(), texture->getHeight()));
                }
            }
        }
    }
    
    // Convert map to vector for faster iteration
    m_tileBatches.reserve(batchMap.size());
    for (auto& pair : batchMap) {
        m_tileBatches.push_back(std::move(pair.second));
    }
}

void TileRenderer::render(ImDrawList* drawList, const Vector2& cameraPos, const Vector2& viewportSize, 
                         float zoomLevel, const Vector2& canvasPos) {
    if (!m_map || m_tileBatches.empty()) return;
    
    m_visibleTileCount = 0;
    float scaledTileSize = m_tileSize * zoomLevel;
    
    // Render each batch
    for (const auto& batch : m_tileBatches) {
        if (!batch.texture) continue;
        
        SDL_Texture* sdlTexture = batch.texture->getSDLTexture();
        ImTextureID textureID = (ImTextureID)(intptr_t)sdlTexture;
        
        // Render all tiles in this batch
        for (size_t i = 0; i < batch.positions.size(); ++i) {
            const Vector2& worldPos = batch.positions[i];
            
            // Frustum culling - only render visible tiles
            if (!isTileVisible(worldPos, cameraPos, viewportSize, zoomLevel, m_tileSize)) {
                continue;
            }
            
            m_visibleTileCount++;
            
            // Convert world position to screen position with zoom
            Vector2 screenPos;
            screenPos.x = (worldPos.x - cameraPos.x) * zoomLevel;
            screenPos.y = (worldPos.y - cameraPos.y) * zoomLevel;
            
            // Apply canvas offset
            screenPos.x += canvasPos.x + viewportSize.x / 2;
            screenPos.y += canvasPos.y + viewportSize.y / 2;
            
            // Calculate draw rectangle
            ImVec2 imageMin(screenPos.x - scaledTileSize/2, screenPos.y - scaledTileSize/2);
            ImVec2 imageMax(screenPos.x + scaledTileSize/2, screenPos.y + scaledTileSize/2);
            
            // Draw the tile
            drawList->AddImage(textureID, imageMin, imageMax);
        }
    }
}

bool TileRenderer::isTileVisible(const Vector2& tilePos, const Vector2& cameraPos, 
                                const Vector2& viewportSize, float zoomLevel, float tileSize) const {
    // Calculate tile's screen position
    float screenX = (tilePos.x - cameraPos.x) * zoomLevel + viewportSize.x / 2;
    float screenY = (tilePos.y - cameraPos.y) * zoomLevel + viewportSize.y / 2;
    
    float scaledTileSize = tileSize * zoomLevel;
    
    // Check if tile is within viewport bounds (with some margin)
    float margin = scaledTileSize;
    return screenX + scaledTileSize >= -margin && screenX - scaledTileSize <= viewportSize.x + margin &&
           screenY + scaledTileSize >= -margin && screenY - scaledTileSize <= viewportSize.y + margin;
}
