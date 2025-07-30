#include "CoreSystems.h"
#include "../components/Components.h"
#include "../graphics/Renderer.h"
#include "../scene/Scene.h"
#include <SDL.h>
#include <cmath>

void LightSystem::update(float deltaTime) {
    if (!m_scene) return;
    
    // Update dynamic light properties
    for (const auto& entity : entities) {
        auto& lightSource = m_scene->getComponent<LightSource>(entity);
        
        if (!lightSource.enabled) continue;
        
        // Handle flickering lights
        if (lightSource.flicker) {
            lightSource.flickerTimer += deltaTime * lightSource.flickerSpeed;
            float flickerAmount = std::sin(lightSource.flickerTimer) * lightSource.flickerIntensity;
            // Intensity will be modified during rendering
        }
    }
}

void LightSystem::render(Renderer* renderer) {
    if (!m_scene || !renderer) return;
    
    // Get current blend mode
    SDL_BlendMode originalBlendMode;
    SDL_GetRenderDrawBlendMode(renderer->getSDLRenderer(), &originalBlendMode);
    
    // Set additive blending for lights
    SDL_SetRenderDrawBlendMode(renderer->getSDLRenderer(), SDL_BLENDMODE_ADD);
    
    // Render all light sources
    for (const auto& entity : entities) {
        auto& lightSource = m_scene->getComponent<LightSource>(entity);
        auto& transform = m_scene->getComponent<Transform>(entity);
        
        if (!lightSource.enabled) continue;
        
        renderLight(renderer, lightSource, transform);
    }
    
    // Restore original blend mode
    SDL_SetRenderDrawBlendMode(renderer->getSDLRenderer(), originalBlendMode);
}

void LightSystem::renderLight(Renderer* renderer, const LightSource& light, const Transform& transform) {
    float currentIntensity = light.intensity;
    
    // Apply flicker effect
    if (light.flicker) {
        float flickerAmount = std::sin(light.flickerTimer) * light.flickerIntensity;
        currentIntensity = std::max(0.0f, light.intensity + flickerAmount);
    }
    
    // Calculate light color with intensity
    Uint8 r = static_cast<Uint8>(light.color.r * currentIntensity);
    Uint8 g = static_cast<Uint8>(light.color.g * currentIntensity);
    Uint8 b = static_cast<Uint8>(light.color.b * currentIntensity);
    
    switch (light.type) {
        case LightSource::LightType::Point:
            renderPointLight(renderer, transform.position, light.range, r, g, b);
            break;
            
        case LightSource::LightType::Directional:
            renderDirectionalLight(renderer, transform.position, light.direction, light.range, r, g, b);
            break;
            
        case LightSource::LightType::Spot:
            renderSpotLight(renderer, transform.position, light.direction, light.range, light.spotAngle, r, g, b);
            break;
    }
}

void LightSystem::renderPointLight(Renderer* renderer, const Vector2& position, float range, Uint8 r, Uint8 g, Uint8 b) {
    SDL_Renderer* sdlRenderer = renderer->getSDLRenderer();
    
    // Render light as a series of concentric circles with decreasing alpha
    int steps = 20;
    for (int i = 0; i < steps; ++i) {
        float ratio = static_cast<float>(i) / steps;
        float currentRadius = range * ratio;
        
        // Calculate alpha falloff (more transparent at edges)
        Uint8 alpha = static_cast<Uint8>(255 * (1.0f - ratio));
        
        SDL_SetRenderDrawColor(sdlRenderer, r, g, b, alpha);
        
        // Draw filled circle approximation
        drawFilledCircle(renderer, static_cast<int>(position.x), static_cast<int>(position.y), 
                        static_cast<int>(currentRadius), r, g, b, alpha);
    }
}

void LightSystem::renderDirectionalLight(Renderer* renderer, const Vector2& position, const Vector2& direction, 
                                       float range, Uint8 r, Uint8 g, Uint8 b) {
    SDL_Renderer* sdlRenderer = renderer->getSDLRenderer();
    
    // Normalize direction
    Vector2 normalizedDir = direction;
    float length = std::sqrt(direction.x * direction.x + direction.y * direction.y);
    if (length > 0) {
        normalizedDir.x /= length;
        normalizedDir.y /= length;
    }
    
    // Create directional light rectangle
    Vector2 perpendicular = {-normalizedDir.y, normalizedDir.x};
    float width = range * 0.5f; // Width of the directional light
    
    // Calculate corners of the light rectangle
    Vector2 corners[4] = {
        {position.x - perpendicular.x * width, position.y - perpendicular.y * width},
        {position.x + perpendicular.x * width, position.y + perpendicular.y * width},
        {position.x + perpendicular.x * width + normalizedDir.x * range, 
         position.y + perpendicular.y * width + normalizedDir.y * range},
        {position.x - perpendicular.x * width + normalizedDir.x * range, 
         position.y - perpendicular.y * width + normalizedDir.y * range}
    };
    
    // Render as a gradient-filled polygon (simplified as rectangle)
    SDL_SetRenderDrawColor(sdlRenderer, r, g, b, 128);
    SDL_Rect lightRect = {
        static_cast<int>(position.x - width/2),
        static_cast<int>(position.y),
        static_cast<int>(width),
        static_cast<int>(range)
    };
    SDL_RenderFillRect(sdlRenderer, &lightRect);
}

void LightSystem::renderSpotLight(Renderer* renderer, const Vector2& position, const Vector2& direction, 
                                float range, float spotAngle, Uint8 r, Uint8 g, Uint8 b) {
    SDL_Renderer* sdlRenderer = renderer->getSDLRenderer();
    
    // Normalize direction
    Vector2 normalizedDir = direction;
    float length = std::sqrt(direction.x * direction.x + direction.y * direction.y);
    if (length > 0) {
        normalizedDir.x /= length;
        normalizedDir.y /= length;
    }
    
    // Calculate spot light cone
    float halfAngle = spotAngle * 0.5f * (M_PI / 180.0f); // Convert to radians
    int segments = 16;
    
    // Draw filled triangle fan for spot light
    SDL_SetRenderDrawColor(sdlRenderer, r, g, b, 200);
    
    std::vector<SDL_Point> points;
    points.push_back({static_cast<int>(position.x), static_cast<int>(position.y)});
    
    for (int i = 0; i <= segments; ++i) {
        float angle = -halfAngle + (2.0f * halfAngle * i / segments);
        float cosAngle = std::cos(angle);
        float sinAngle = std::sin(angle);
        
        // Rotate direction vector by angle
        float rotatedX = normalizedDir.x * cosAngle - normalizedDir.y * sinAngle;
        float rotatedY = normalizedDir.x * sinAngle + normalizedDir.y * cosAngle;
        
        Vector2 endPoint = {
            position.x + rotatedX * range,
            position.y + rotatedY * range
        };
        
        points.push_back({static_cast<int>(endPoint.x), static_cast<int>(endPoint.y)});
    }
    
    // Draw the spot light as connected lines (simplified rendering)
    for (size_t i = 1; i < points.size(); ++i) {
        SDL_RenderDrawLine(sdlRenderer, points[0].x, points[0].y, points[i].x, points[i].y);
    }
}

void LightSystem::drawFilledCircle(Renderer* renderer, int centerX, int centerY, int radius, 
                                 Uint8 r, Uint8 g, Uint8 b, Uint8 alpha) {
    SDL_Renderer* sdlRenderer = renderer->getSDLRenderer();
    SDL_SetRenderDrawColor(sdlRenderer, r, g, b, alpha);
    
    // Use Bresenham's circle algorithm to fill circle
    for (int y = -radius; y <= radius; y++) {
        for (int x = -radius; x <= radius; x++) {
            if (x*x + y*y <= radius*radius) {
                SDL_RenderDrawPoint(sdlRenderer, centerX + x, centerY + y);
            }
        }
    }
}

void LightSystem::setAmbientLight(float r, float g, float b, float intensity) {
    m_ambientLight.r = static_cast<Uint8>(r * 255 * intensity);
    m_ambientLight.g = static_cast<Uint8>(g * 255 * intensity);
    m_ambientLight.b = static_cast<Uint8>(b * 255 * intensity);
    m_ambientLight.a = 255;
}

void LightSystem::renderAmbientLight(Renderer* renderer, int screenWidth, int screenHeight) {
    SDL_Renderer* sdlRenderer = renderer->getSDLRenderer();
    
    // Get current blend mode
    SDL_BlendMode originalBlendMode;
    SDL_GetRenderDrawBlendMode(sdlRenderer, &originalBlendMode);
    
    // Set blend mode for ambient light
    SDL_SetRenderDrawBlendMode(sdlRenderer, SDL_BLENDMODE_BLEND);
    
    // Fill screen with ambient light color
    SDL_SetRenderDrawColor(sdlRenderer, m_ambientLight.r, m_ambientLight.g, m_ambientLight.b, m_ambientLight.a);
    SDL_Rect screenRect = {0, 0, screenWidth, screenHeight};
    SDL_RenderFillRect(sdlRenderer, &screenRect);
    
    // Restore original blend mode
    SDL_SetRenderDrawBlendMode(sdlRenderer, originalBlendMode);
}
