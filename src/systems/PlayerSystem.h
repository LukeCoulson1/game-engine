#pragma once

#include "../components/Components.h"
#include "../input/InputManager.h"
#include "../scene/Scene.h"
#include "../audio/AudioManager.h"
#include <SDL2/SDL.h>
#include <memory>
#include <functional>
#include <vector>

class PlayerSystem {
public:
    PlayerSystem();
    ~PlayerSystem();
    
    // Player entity management
    EntityID createPlayerEntity(Scene* scene, const Vector2& startPosition);
    void setupPlayerComponents(Scene* scene, EntityID playerEntity);
    
    // Main update loop
    void update(Scene* scene, float deltaTime);
    void handleInput(Scene* scene, const Uint8* keyboardState, float deltaTime);
    
    // Player configuration
    void setControlScheme(Scene* scene, EntityID playerEntity, PlayerController::ControlScheme scheme);
    void setMovementType(Scene* scene, EntityID playerEntity, PlayerController::MovementType type);
    void configurePhysics(Scene* scene, EntityID playerEntity, float maxSpeed, float acceleration);
    
    // Player actions
    void movePlayer(Scene* scene, EntityID playerEntity, const Vector2& direction, float deltaTime);
    void jumpPlayer(Scene* scene, EntityID playerEntity);
    void dashPlayer(Scene* scene, EntityID playerEntity, const Vector2& direction);
    void useAbility(Scene* scene, EntityID playerEntity, int abilityIndex);
    void useItem(Scene* scene, EntityID playerEntity, int itemIndex);
    
    // Player queries
    bool isPlayerEntity(Scene* scene, EntityID entity) const;
    EntityID findPlayerEntity(Scene* scene) const;
    Vector2 getPlayerPosition(Scene* scene, EntityID playerEntity) const;
    PlayerStats::DerivedStats getPlayerStats(Scene* scene, EntityID playerEntity) const;
    
    // Player events (for game systems to respond to player actions)
    struct PlayerEvent {
        enum Type {
            LevelUp,
            Death,
            ItemPickup,
            AbilityUsed,
            StateChanged
        };
        
        Type type;
        EntityID playerEntity;
        void* data; // Event-specific data
    };
    
    using PlayerEventCallback = std::function<void(const PlayerEvent&)>;
    void registerEventCallback(PlayerEventCallback callback);

private:
    std::vector<PlayerEventCallback> m_eventCallbacks;
    
    // Input handling helpers
    Vector2 getInputDirection(const PlayerController* controller, const Uint8* keyboardState);
    bool isKeyPressed(const PlayerController* controller, const std::string& action, const Uint8* keyboardState);
    
    // Physics helpers
    void updatePhysics(PlayerPhysics* physics, const Vector2& inputDirection, float deltaTime);
    void applyGravity(PlayerPhysics* physics, float deltaTime);
    void handleCollisions(Scene* scene, EntityID playerEntity, PlayerPhysics* physics, float deltaTime);
    
    // State management helpers
    void updatePlayerState(PlayerState* state, const PlayerController* controller, 
                          const PlayerPhysics* physics, float deltaTime);
    
    // Status effect helpers
    void updateStatusEffects(PlayerStats* stats, float deltaTime);
    
    // Animation helpers
    void updateAnimations(Scene* scene, EntityID playerEntity, const PlayerState* state, float deltaTime);
    
    // Event helpers
    void triggerEvent(PlayerEvent::Type type, EntityID playerEntity, void* data = nullptr);
    
    // Default player configuration
    void setupDefaultAbilities(PlayerAbilities* abilities);
    void setupDefaultItems(PlayerInventory* inventory);
    
    // Constants
    static constexpr float GRAVITY_FORCE = 981.0f; // Pixels per second squared
    static constexpr float TERMINAL_VELOCITY = 600.0f;
    static constexpr float GROUND_CHECK_DISTANCE = 5.0f;
    static constexpr float WALL_CHECK_DISTANCE = 5.0f;
};
