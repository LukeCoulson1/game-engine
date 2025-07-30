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

// Particle system for managing particle effects
class ParticleSystem : public System {
public:
    ParticleSystem() = default;
    
    void update(float deltaTime) override;
    void render(Renderer* renderer) override;
    void setScene(Scene* scene) { m_scene = scene; }

private:
    Scene* m_scene = nullptr;
    
    void renderParticle(Renderer* renderer, const ParticleEffect::Particle& particle, 
                       const Vector2& position, std::shared_ptr<Texture> texture);
};

// Light system for rendering dynamic lighting
class LightSystem : public System {
public:
    LightSystem() = default;
    
    void update(float deltaTime) override;
    void render(Renderer* renderer) override;
    void setScene(Scene* scene) { m_scene = scene; }
    
    // Ambient light controls
    void setAmbientLight(float r, float g, float b, float intensity = 0.2f);
    void renderAmbientLight(Renderer* renderer, int screenWidth, int screenHeight);

private:
    Scene* m_scene = nullptr;
    Color m_ambientLight{50, 50, 80, 255}; // Default dim blue ambient light
    
    // Light rendering functions
    void renderLight(Renderer* renderer, const LightSource& light, const Transform& transform);
    void renderPointLight(Renderer* renderer, const Vector2& position, float range, Uint8 r, Uint8 g, Uint8 b);
    void renderDirectionalLight(Renderer* renderer, const Vector2& position, const Vector2& direction, 
                               float range, Uint8 r, Uint8 g, Uint8 b);
    void renderSpotLight(Renderer* renderer, const Vector2& position, const Vector2& direction, 
                        float range, float spotAngle, Uint8 r, Uint8 g, Uint8 b);
    void drawFilledCircle(Renderer* renderer, int centerX, int centerY, int radius, 
                         Uint8 r, Uint8 g, Uint8 b, Uint8 alpha);
};

// Audio system for managing sound effects and music
class AudioSystem : public System {
public:
    AudioSystem() = default;
    ~AudioSystem() = default;
    
    // System lifecycle
    void initialize();
    void shutdown();
    void update(float deltaTime) override;
    void setScene(Scene* scene) { m_scene = scene; }
    
    // Audio playback controls
    void playSound(AudioSource& audioSource, EntityID entity);
    void stopSound(AudioSource& audioSource, EntityID entity);
    void pauseSound(AudioSource& audioSource, EntityID entity);
    void resumeSound(AudioSource& audioSource, EntityID entity);
    
    // Global audio controls
    void setMasterVolume(float volume);
    void playMusic(const std::string& filename, bool loop = true);
    void stopMusic();
    void setMusicVolume(float volume);
    
    // 3D Audio
    void setListenerPosition(const Vector2& position);
    void setListenerPosition(float x, float y);

private:
    Scene* m_scene = nullptr;
    float m_masterVolume = 1.0f;
    Vector2 m_listenerPosition{0, 0};
    
    // Entity to audio channel mapping
    std::unordered_map<EntityID, int> m_entityChannels;
    
    // Audio resource management
    Mix_Chunk* loadAudioChunk(const std::string& filename);
    Mix_Music* loadMusicTrack(const std::string& filename);
    
    // 3D audio processing
    void update3DAudio(AudioSource& audioSource, const Transform& transform, EntityID entity);
};
