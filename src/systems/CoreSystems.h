#pragma once

#include "System.h"
#include "PlayerSystem.h"
#include "graphics/Renderer.h"
#include <algorithm>

// Forward declare Scene class
class Scene;

class RenderSystem : public System {
public:
    RenderSystem() = default;
    
    void render(Renderer* renderer) override;
    void setScene(Scene* scene) { m_scene = scene; }

private:
    Scene* m_scene = nullptr;
};

class PhysicsSystem : public System {
public:
    PhysicsSystem() = default;
    
    void update(float deltaTime) override;
    void setScene(Scene* scene) { m_scene = scene; }

private:
    Scene* m_scene = nullptr;
    const float GRAVITY = 980.0f; // pixels per second squared
};

class CollisionSystem : public System {
public:
    CollisionSystem() = default;
    
    void update(float deltaTime) override;
    void setScene(Scene* scene) { m_scene = scene; }
    
    // Utility functions for collision detection
    static bool checkCollision(const Rect& a, const Rect& b);
    static Vector2 getCollisionNormal(const Rect& a, const Rect& b);

private:
    Scene* m_scene = nullptr;
};

// Input system for handling player input
class InputSystem : public System {
public:
    InputSystem();
    
    void update(float deltaTime) override;
    void setScene(Scene* scene) { m_scene = scene; }
    void setPlayerSystem(PlayerSystem* playerSystem) { m_playerSystem = playerSystem; }

private:
    Scene* m_scene = nullptr;
    PlayerSystem* m_playerSystem = nullptr;
    const Uint8* m_keyboardState = nullptr;
};
