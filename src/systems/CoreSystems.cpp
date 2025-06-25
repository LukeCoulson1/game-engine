#include "CoreSystems.h"
#include "scene/Scene.h"
#include <vector>
#include <algorithm>
#include <iostream>

// RenderSystem Implementation
void RenderSystem::render(Renderer* renderer) {
    // Create a vector of entities with sprites for sorting
    std::vector<EntityID> renderableEntities;
    
    for (auto entity : entities) {
        if (m_scene->hasComponent<Sprite>(entity)) {
            auto& sprite = m_scene->getComponent<Sprite>(entity);
            if (sprite.visible) {
                renderableEntities.push_back(entity);
            }
        }
    }
    
    // Sort by layer (lower layers rendered first)
    std::sort(renderableEntities.begin(), renderableEntities.end(), 
        [this](EntityID a, EntityID b) {
            auto& spriteA = m_scene->getComponent<Sprite>(a);
            auto& spriteB = m_scene->getComponent<Sprite>(b);
            return spriteA.layer < spriteB.layer;
        });
    
    // Render all sprites
    for (auto entity : renderableEntities) {
        auto& transform = m_scene->getComponent<Transform>(entity);
        auto& sprite = m_scene->getComponent<Sprite>(entity);
          if (sprite.texture) {
            Rect dstRect(
                transform.position.x, 
                transform.position.y,
                sprite.sourceRect.width * transform.scale.x,
                sprite.sourceRect.height * transform.scale.y
            );
            
            Vector2 center(dstRect.width / 2, dstRect.height / 2);            // Debug output for all entities with non-default scale/rotation
            if (transform.scale.x != 1.0f || transform.scale.y != 1.0f || transform.rotation != 0.0f) {
                printf("RENDER DEBUG: Entity %u - Scale: %.2f,%.2f Rotation: %.1f Pos: %.1f,%.1f\n", 
                       entity, transform.scale.x, transform.scale.y, transform.rotation, 
                       transform.position.x, transform.position.y);
                printf("  SrcRect: %.1f,%.1f,%.1f,%.1f\n", 
                       sprite.sourceRect.x, sprite.sourceRect.y, sprite.sourceRect.width, sprite.sourceRect.height);
                printf("  DstRect: %.1f,%.1f,%.1f,%.1f\n", 
                       dstRect.x, dstRect.y, dstRect.width, dstRect.height);
                printf("  Center: %.1f,%.1f\n", center.x, center.y);
                fflush(stdout);
            }
            
            renderer->drawTexture(sprite.texture, sprite.sourceRect, dstRect, 
                                transform.rotation, center);
        }
    }
}

// PhysicsSystem Implementation
void PhysicsSystem::update(float deltaTime) {
    for (auto entity : entities) {
        auto& transform = m_scene->getComponent<Transform>(entity);
        auto& rigidBody = m_scene->getComponent<RigidBody>(entity);
        
        // Apply gravity
        if (rigidBody.useGravity) {
            rigidBody.acceleration.y += GRAVITY * deltaTime;
        }
        
        // Update velocity with acceleration
        rigidBody.velocity = rigidBody.velocity + (rigidBody.acceleration * deltaTime);
        
        // Apply drag
        rigidBody.velocity = rigidBody.velocity * rigidBody.drag;
        
        // Update position with velocity
        transform.position = transform.position + (rigidBody.velocity * deltaTime);
        
        // Reset acceleration for next frame
        rigidBody.acceleration = Vector2(0, 0);
    }
}

// CollisionSystem Implementation
void CollisionSystem::update(float deltaTime) {
    std::vector<EntityID> colliders;
    
    // Collect all entities with colliders
    for (auto entity : entities) {
        if (m_scene->hasComponent<Collider>(entity)) {
            colliders.push_back(entity);
        }
    }
    
    // Check collisions between all pairs
    for (size_t i = 0; i < colliders.size(); ++i) {
        for (size_t j = i + 1; j < colliders.size(); ++j) {
            EntityID entityA = colliders[i];
            EntityID entityB = colliders[j];
            
            auto& transformA = m_scene->getComponent<Transform>(entityA);
            auto& transformB = m_scene->getComponent<Transform>(entityB);
            auto& colliderA = m_scene->getComponent<Collider>(entityA);
            auto& colliderB = m_scene->getComponent<Collider>(entityB);
            
            Rect boundsA = colliderA.getBounds(transformA.position);
            Rect boundsB = colliderB.getBounds(transformB.position);
            
            if (checkCollision(boundsA, boundsB)) {
                // Handle collision response
                if (!colliderA.isTrigger && !colliderB.isTrigger) {
                    // Physical collision - separate objects
                    Vector2 normal = getCollisionNormal(boundsA, boundsB);
                    
                    // Only move non-static objects
                    if (!colliderA.isStatic && !colliderB.isStatic) {
                        // Move both objects away from each other
                        float separation = 2.0f; // Minimum separation distance
                        transformA.position = transformA.position - (normal * separation * 0.5f);
                        transformB.position = transformB.position + (normal * separation * 0.5f);
                    } else if (!colliderA.isStatic) {
                        // Only move A
                        float separation = 2.0f;
                        transformA.position = transformA.position - (normal * separation);
                    } else if (!colliderB.isStatic) {
                        // Only move B
                        float separation = 2.0f;
                        transformB.position = transformB.position + (normal * separation);
                    }
                    
                    // Adjust velocities if entities have RigidBody components
                    if (m_scene->hasComponent<RigidBody>(entityA) && !colliderA.isStatic) {
                        auto& rb = m_scene->getComponent<RigidBody>(entityA);
                        // Simple velocity reflection
                        if (normal.x != 0) rb.velocity.x *= -0.5f;
                        if (normal.y != 0) rb.velocity.y *= -0.5f;
                    }
                    
                    if (m_scene->hasComponent<RigidBody>(entityB) && !colliderB.isStatic) {
                        auto& rb = m_scene->getComponent<RigidBody>(entityB);
                        // Simple velocity reflection
                        if (normal.x != 0) rb.velocity.x *= -0.5f;
                        if (normal.y != 0) rb.velocity.y *= -0.5f;
                    }
                }
                
                // Trigger events could be handled here
                // onCollision(entityA, entityB);
            }
        }
    }
}

bool CollisionSystem::checkCollision(const Rect& a, const Rect& b) {
    return (a.x < b.x + b.width &&
            a.x + a.width > b.x &&
            a.y < b.y + b.height &&
            a.y + a.height > b.y);
}

Vector2 CollisionSystem::getCollisionNormal(const Rect& a, const Rect& b) {
    Vector2 centerA(a.x + a.width / 2, a.y + a.height / 2);
    Vector2 centerB(b.x + b.width / 2, b.y + b.height / 2);
    
    Vector2 diff = centerA - centerB;
    
    // Determine which axis has the smallest overlap
    float overlapX = (a.width + b.width) / 2 - std::abs(diff.x);
    float overlapY = (a.height + b.height) / 2 - std::abs(diff.y);
    
    if (overlapX < overlapY) {
        // Horizontal collision
        return Vector2(diff.x > 0 ? 1.0f : -1.0f, 0.0f);
    } else {
        // Vertical collision
        return Vector2(0.0f, diff.y > 0 ? 1.0f : -1.0f);
    }
}

// InputSystem Implementation
InputSystem::InputSystem() {
    m_keyboardState = SDL_GetKeyboardState(nullptr);
}

void InputSystem::update(float deltaTime) {
    if (!m_scene || !m_playerSystem || !m_keyboardState) return;
    
    // Update keyboard state
    m_keyboardState = SDL_GetKeyboardState(nullptr);
    
    // Handle player input through PlayerSystem
    m_playerSystem->handleInput(m_scene, m_keyboardState, deltaTime);
}
