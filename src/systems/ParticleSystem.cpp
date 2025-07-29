#include "CoreSystems.h"
#include "../components/Components.h"
#include "../graphics/Renderer.h"
#include "../scene/Scene.h"
#include <SDL.h>

void ParticleSystem::update(float deltaTime) {
    if (!m_scene) return;
    
    // Iterate through all entities that match this system's signature
    for (const auto& entity : entities) {
        auto& particleEffect = m_scene->getComponent<ParticleEffect>(entity);
        auto& transform = m_scene->getComponent<Transform>(entity);
        
        // Update the particle effect
        particleEffect.update(deltaTime, transform.position);
    }
}

void ParticleSystem::render(Renderer* renderer) {
    if (!m_scene || !renderer) return;
    
    // Iterate through all entities that match this system's signature
    for (const auto& entity : entities) {
        auto& particleEffect = m_scene->getComponent<ParticleEffect>(entity);
        auto& transform = m_scene->getComponent<Transform>(entity);
        
        // Render all particles
        for (const auto& particle : particleEffect.particles) {
            renderParticle(renderer, particle, transform.position, particleEffect.texture);
        }
    }
}

void ParticleSystem::renderParticle(Renderer* renderer, const ParticleEffect::Particle& particle, 
                                   const Vector2& position, std::shared_ptr<Texture> texture) {
    if (!renderer) return;
    
    // Calculate world position
    Vector2 worldPos = {
        position.x + particle.position.x,
        position.y + particle.position.y
    };
    
    if (texture) {
        // Set texture color modulation
        SDL_SetTextureColorMod(texture->getSDLTexture(), 
                              particle.color.r,
                              particle.color.g,
                              particle.color.b);
        
        SDL_SetTextureAlphaMod(texture->getSDLTexture(), 
                              particle.color.a);
        
        // Create destination rectangle based on particle size
        SDL_Rect destRect = {
            static_cast<int>(worldPos.x - particle.size * 0.5f),
            static_cast<int>(worldPos.y - particle.size * 0.5f),
            static_cast<int>(particle.size),
            static_cast<int>(particle.size)
        };
        
        // Render with rotation if needed
        if (particle.rotation != 0.0f) {
            SDL_RenderCopyEx(renderer->getSDLRenderer(), texture->getSDLTexture(), 
                           nullptr, &destRect, particle.rotation, nullptr, SDL_FLIP_NONE);
        } else {
            SDL_RenderCopy(renderer->getSDLRenderer(), texture->getSDLTexture(), 
                          nullptr, &destRect);
        }
        
        // Reset texture modulation
        SDL_SetTextureColorMod(texture->getSDLTexture(), 255, 255, 255);
        SDL_SetTextureAlphaMod(texture->getSDLTexture(), 255);
    } else {
        // Fallback: render as colored rectangle if no texture
        SDL_SetRenderDrawColor(renderer->getSDLRenderer(),
                              particle.color.r,
                              particle.color.g,
                              particle.color.b,
                              particle.color.a);
        
        SDL_Rect rect = {
            static_cast<int>(worldPos.x - particle.size * 0.5f),
            static_cast<int>(worldPos.y - particle.size * 0.5f),
            static_cast<int>(particle.size),
            static_cast<int>(particle.size)
        };
        
        SDL_RenderFillRect(renderer->getSDLRenderer(), &rect);
    }
}
