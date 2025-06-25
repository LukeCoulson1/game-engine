#include "PlayerSystem.h"
#include "../core/Engine.h"
#include "../utils/ResourceManager.h"
#include <algorithm>
#include <cmath>

PlayerSystem::PlayerSystem() {
}

PlayerSystem::~PlayerSystem() {
}

EntityID PlayerSystem::createPlayerEntity(Scene* scene, const Vector2& startPosition) {
    if (!scene) return 0;
    
    EntityID playerEntity = scene->createEntity();
    
    // Add basic transform
    scene->addComponent<Transform>(playerEntity, Transform(startPosition));
    
    // Set up all player-specific components
    setupPlayerComponents(scene, playerEntity);
    
    // Set entity name
    scene->addComponent<Name>(playerEntity, Name("Player"));
    
    // Trigger creation event
    triggerEvent(PlayerEvent::StateChanged, playerEntity);
    
    return playerEntity;
}

void PlayerSystem::setupPlayerComponents(Scene* scene, EntityID playerEntity) {
    if (!scene || !scene->hasComponent<Transform>(playerEntity)) return;
    
    // Add player controller
    PlayerController controller;
    controller.controlScheme = PlayerController::ControlScheme::WASD;
    controller.movementType = PlayerController::MovementType::TopDown;
    scene->addComponent<PlayerController>(playerEntity, controller);
    
    // Add player stats
    PlayerStats stats;
    scene->addComponent<PlayerStats>(playerEntity, stats);
    
    // Add player physics
    PlayerPhysics physics;
    scene->addComponent<PlayerPhysics>(playerEntity, physics);
    
    // Add collider
    Collider collider(24, 32); // Slightly smaller than a tile for smooth movement
    scene->addComponent<Collider>(playerEntity, collider);
    
    // Add sprite component
    auto& engine = Engine::getInstance();
    auto resourceManager = engine.getResourceManager();
    if (resourceManager) {
        Sprite sprite;
        // Try to load a default player sprite
        auto texture = resourceManager->loadTexture("assets/sprites/player.png");
        if (texture) {
            sprite.texture = texture;
            sprite.sourceRect = Rect(0, 0, texture->getWidth(), texture->getHeight());
            sprite.layer = 10; // Higher layer for player
        }
        scene->addComponent<Sprite>(playerEntity, sprite);
    }
    
    // Add inventory
    PlayerInventory inventory;
    setupDefaultItems(&inventory);
    scene->addComponent<PlayerInventory>(playerEntity, inventory);
    
    // Add abilities
    PlayerAbilities abilities;
    setupDefaultAbilities(&abilities);
    scene->addComponent<PlayerAbilities>(playerEntity, abilities);
    
    // Add state machine
    PlayerState state;
    scene->addComponent<PlayerState>(playerEntity, state);
}

void PlayerSystem::update(Scene* scene, float deltaTime) {
    if (!scene) return;
    
    // Get all entities with player components
    auto allEntities = scene->getAllLivingEntities();
    
    for (EntityID entity : allEntities) {
        if (!isPlayerEntity(scene, entity)) continue;
        
        // Get components
        auto* controller = &scene->getComponent<PlayerController>(entity);
        auto* stats = &scene->getComponent<PlayerStats>(entity);
        auto* physics = &scene->getComponent<PlayerPhysics>(entity);
        auto* state = &scene->getComponent<PlayerState>(entity);
        auto* abilities = &scene->getComponent<PlayerAbilities>(entity);
        
        // Update timers and cooldowns
        abilities->updateCooldowns(deltaTime);
        updateStatusEffects(stats, deltaTime);
        
        // Update physics
        updatePhysics(physics, controller->moveDirection, deltaTime);
        handleCollisions(scene, entity, physics, deltaTime);
        
        // Update state machine
        updatePlayerState(state, controller, physics, deltaTime);
        
        // Update animations
        updateAnimations(scene, entity, state, deltaTime);
        
        // Apply physics to transform
        auto& transform = scene->getComponent<Transform>(entity);
        transform.position = transform.position + (physics->velocity * deltaTime);
        
        // Reset input direction for next frame
        controller->moveDirection = Vector2(0, 0);
        controller->jumpPressed = false;
    }
}

void PlayerSystem::handleInput(Scene* scene, const Uint8* keyboardState, float deltaTime) {
    if (!scene || !keyboardState) return;
    
    EntityID playerEntity = findPlayerEntity(scene);
    if (playerEntity == 0) return;
    
    auto& controller = scene->getComponent<PlayerController>(playerEntity);
    auto& physics = scene->getComponent<PlayerPhysics>(playerEntity);
    
    // Get input direction
    Vector2 inputDir = getInputDirection(&controller, keyboardState);
    controller.inputDirection = inputDir;
    
    // Process movement based on control scheme and movement type
    if (inputDir.x != 0 || inputDir.y != 0) {
        if (controller.movementType == PlayerController::MovementType::TopDown) {
            // Normalize diagonal movement
            float length = sqrt(inputDir.x * inputDir.x + inputDir.y * inputDir.y);
            if (length > 1.0f) {
                inputDir.x /= length;
                inputDir.y /= length;
            }
        }
        
        controller.moveDirection = inputDir;
    }
    
    // Check for running
    controller.isRunning = isKeyPressed(&controller, "Run", keyboardState);
    
    // Check for jump
    if (isKeyPressed(&controller, "Jump", keyboardState)) {
        controller.jumpPressed = true;
        jumpPlayer(scene, playerEntity);
    }
    
    // Check for dash
    if (isKeyPressed(&controller, "Dash", keyboardState) && physics.canDash) {
        Vector2 dashDir = inputDir;
        if (dashDir.x == 0 && dashDir.y == 0) {
            // Dash in facing direction if no input
            auto& state = scene->getComponent<PlayerState>(playerEntity);
            dashDir = state.facing;
        }
        dashPlayer(scene, playerEntity, dashDir);
    }
    
    // Check for ability usage (1-4 keys for hotbar abilities)
    if (scene->hasComponent<PlayerAbilities>(playerEntity)) {
        for (int i = 0; i < 4; ++i) {
            int key = SDL_SCANCODE_1 + i;
            if (keyboardState[key]) {
                useAbility(scene, playerEntity, i);
            }
        }
    }
    
    // Check for item usage (Q key for selected hotbar item)
    if (keyboardState[SDL_SCANCODE_Q] && scene->hasComponent<PlayerInventory>(playerEntity)) {
        auto& inventory = scene->getComponent<PlayerInventory>(playerEntity);
        useItem(scene, playerEntity, inventory.selectedHotbarSlot);
    }
}

Vector2 PlayerSystem::getInputDirection(const PlayerController* controller, const Uint8* keyboardState) {
    Vector2 direction(0, 0);
    
    if (!controller || !keyboardState) return direction;
    
    // Get movement keys from bindings
    bool upPressed = keyboardState[controller->keyBindings.at("MoveUp")];
    bool downPressed = keyboardState[controller->keyBindings.at("MoveDown")];
    bool leftPressed = keyboardState[controller->keyBindings.at("MoveLeft")];
    bool rightPressed = keyboardState[controller->keyBindings.at("MoveRight")];
    
    // Calculate direction
    if (upPressed) direction.y -= 1.0f;
    if (downPressed) direction.y += 1.0f;
    if (leftPressed) direction.x -= 1.0f;
    if (rightPressed) direction.x += 1.0f;
    
    return direction;
}

bool PlayerSystem::isKeyPressed(const PlayerController* controller, const std::string& action, const Uint8* keyboardState) {
    if (!controller || !keyboardState) return false;
    
    auto it = controller->keyBindings.find(action);
    if (it != controller->keyBindings.end()) {
        return keyboardState[it->second];
    }
    
    return false;
}

void PlayerSystem::updatePhysics(PlayerPhysics* physics, const Vector2& inputDirection, float deltaTime) {
    if (!physics) return;
    
    // Apply external forces
    physics->acceleration = physics->externalForces * (1.0f / physics->mass);
    physics->externalForces = Vector2(0, 0); // Reset external forces
    
    // Handle dash physics
    if (physics->isDashing) {
        physics->dashTimer -= deltaTime;
        if (physics->dashTimer <= 0.0f) {
            physics->isDashing = false;
            physics->velocity = physics->velocity * 0.3f; // Reduce velocity after dash
        }
        return; // Skip normal movement during dash
    }
    
    // Update dash cooldown
    if (physics->dashCooldownTimer > 0.0f) {
        physics->dashCooldownTimer -= deltaTime;
    }
    
    // Apply movement acceleration
    if (inputDirection.x != 0 || inputDirection.y != 0) {
        Vector2 targetVelocity = inputDirection * physics->maxSpeed;
        Vector2 velocityDiff = targetVelocity - physics->velocity;
        
        float accelerationRate = 800.0f; // Pixels per second squared
        Vector2 accelerationThisFrame = velocityDiff * accelerationRate * deltaTime;
        
        // Limit acceleration to prevent overshooting
        float maxAcceleration = accelerationRate * deltaTime;
        if (accelerationThisFrame.x * accelerationThisFrame.x + 
            accelerationThisFrame.y * accelerationThisFrame.y > maxAcceleration * maxAcceleration) {
            float length = sqrt(accelerationThisFrame.x * accelerationThisFrame.x + 
                              accelerationThisFrame.y * accelerationThisFrame.y);
            accelerationThisFrame = accelerationThisFrame * (maxAcceleration / length);
        }
        
        physics->velocity = physics->velocity + accelerationThisFrame;
    } else {
        // Apply friction when no input
        physics->velocity = physics->velocity * physics->friction;
        
        // Stop very small velocities
        if (abs(physics->velocity.x) < 1.0f) physics->velocity.x = 0.0f;
        if (abs(physics->velocity.y) < 1.0f) physics->velocity.y = 0.0f;
    }
    
    // Apply gravity for platformer movement
    if (!physics->isGrounded) {
        applyGravity(physics, deltaTime);
    }
    
    // Limit velocity to max speed
    float currentSpeed = sqrt(physics->velocity.x * physics->velocity.x + 
                            physics->velocity.y * physics->velocity.y);
    if (currentSpeed > physics->maxSpeed) {
        physics->velocity = physics->velocity * (physics->maxSpeed / currentSpeed);
    }
    
    // Update timers
    if (physics->jumpCooldown > 0.0f) {
        physics->jumpCooldown -= deltaTime;
    }
    
    if (physics->coyoteTimer > 0.0f) {
        physics->coyoteTimer -= deltaTime;
    }
    
    if (physics->jumpBufferTimer > 0.0f) {
        physics->jumpBufferTimer -= deltaTime;
    }
}

void PlayerSystem::applyGravity(PlayerPhysics* physics, float deltaTime) {
    if (!physics) return;
    
    physics->velocity.y += GRAVITY_FORCE * deltaTime;
    
    // Limit to terminal velocity
    if (physics->velocity.y > TERMINAL_VELOCITY) {
        physics->velocity.y = TERMINAL_VELOCITY;
    }
}

void PlayerSystem::jumpPlayer(Scene* scene, EntityID playerEntity) {
    if (!scene || !scene->hasComponent<PlayerController>(playerEntity) || 
        !scene->hasComponent<PlayerPhysics>(playerEntity)) return;
    
    auto& controller = scene->getComponent<PlayerController>(playerEntity);
    auto& physics = scene->getComponent<PlayerPhysics>(playerEntity);
    
    // Check if can jump (grounded or coyote time or has remaining jumps)
    bool canJump = physics.isGrounded || 
                   physics.coyoteTimer > 0.0f || 
                   controller.jumpsRemaining > 0;
    
    if (canJump && physics.jumpCooldown <= 0.0f) {
        physics.velocity.y = -controller.jumpForce;
        physics.isGrounded = false;
        physics.coyoteTimer = 0.0f;
        physics.jumpCooldown = 0.1f; // Prevent multiple jumps in one frame
        
        if (!physics.isGrounded) {
            controller.jumpsRemaining--;
        }
        
        // Play jump sound effect
        auto& engine = Engine::getInstance();
        auto audioManager = engine.getAudioManager();
        if (audioManager) {
            audioManager->playSound("assets/audio/jump.wav");
        }
    }
}

void PlayerSystem::dashPlayer(Scene* scene, EntityID playerEntity, const Vector2& direction) {
    if (!scene || !scene->hasComponent<PlayerPhysics>(playerEntity)) return;
    
    auto& physics = scene->getComponent<PlayerPhysics>(playerEntity);
    physics.startDash(direction);
    
    if (physics.isDashing) {
        // Play dash sound effect
        auto& engine = Engine::getInstance();
        auto audioManager = engine.getAudioManager();
        if (audioManager) {
            audioManager->playSound("assets/audio/dash.wav");
        }
    }
}

void PlayerSystem::handleCollisions(Scene* scene, EntityID playerEntity, PlayerPhysics* physics, float deltaTime) {
    // Basic collision detection and response
    // This is a simplified implementation - a full collision system would be more complex
    
    if (!scene || !physics || !scene->hasComponent<Transform>(playerEntity) || 
        !scene->hasComponent<Collider>(playerEntity)) return;
    
    auto& transform = scene->getComponent<Transform>(playerEntity);
    auto& collider = scene->getComponent<Collider>(playerEntity);
    
    // Get player bounds
    Rect playerBounds = collider.getBounds(transform.position);
    
    // Check collision with other entities
    auto allEntities = scene->getAllLivingEntities();
    for (EntityID otherEntity : allEntities) {
        if (otherEntity == playerEntity) continue;
        
        if (scene->hasComponent<Transform>(otherEntity) && 
            scene->hasComponent<Collider>(otherEntity)) {
            
            auto& otherTransform = scene->getComponent<Transform>(otherEntity);
            auto& otherCollider = scene->getComponent<Collider>(otherEntity);
            
            if (otherCollider.isStatic) {
                Rect otherBounds = otherCollider.getBounds(otherTransform.position);
                  // Simple AABB collision check
                if (playerBounds.x < otherBounds.x + otherBounds.width &&
                    playerBounds.x + playerBounds.width > otherBounds.x &&
                    playerBounds.y < otherBounds.y + otherBounds.height &&
                    playerBounds.y + playerBounds.height > otherBounds.y) {
                    
                    // Simple collision response - stop movement
                    physics->velocity = Vector2(0, 0);
                    
                    // Set grounded if colliding from above
                    if (playerBounds.y + playerBounds.height <= otherBounds.y + 10) {
                        physics->isGrounded = true;
                        physics->coyoteTimer = physics->coyoteTime;
                        
                        // Reset jumps when landing
                        auto& controller = scene->getComponent<PlayerController>(playerEntity);
                        controller.jumpsRemaining = controller.maxJumps;
                    }
                }
            }
        }
    }
}

void PlayerSystem::updatePlayerState(PlayerState* state, const PlayerController* controller, 
                                   const PlayerPhysics* physics, float deltaTime) {
    if (!state || !controller || !physics) return;
    
    state->updateState(deltaTime);
    
    // Determine new state based on player condition
    PlayerState::State newState = state->currentState;
    
    if (physics->isDashing) {
        newState = PlayerState::State::Dashing;
    } else if (!physics->isGrounded && physics->velocity.y > 0) {
        newState = PlayerState::State::Falling;
    } else if (!physics->isGrounded && physics->velocity.y < 0) {
        newState = PlayerState::State::Jumping;
    } else if (controller->moveDirection.x != 0 || controller->moveDirection.y != 0) {
        newState = controller->isRunning ? PlayerState::State::Running : PlayerState::State::Walking;
    } else {
        newState = PlayerState::State::Idle;
    }
    
    state->setState(newState);
    
    // Update facing direction
    if (controller->moveDirection.x != 0) {
        state->facing.x = controller->moveDirection.x > 0 ? 1.0f : -1.0f;
        state->facing.y = 0.0f;
    }
}

void PlayerSystem::updateStatusEffects(PlayerStats* stats, float deltaTime) {
    if (!stats) return;
    
    auto& status = stats->status;
    
    // Update poison
    if (status.poisoned) {
        status.poisonDuration -= deltaTime;
        if (status.poisonDuration <= 0.0f) {
            status.poisoned = false;
        } else {
            // Apply poison damage every second
            static float poisonTimer = 0.0f;
            poisonTimer += deltaTime;
            if (poisonTimer >= 1.0f) {
                stats->takeDamage(5);
                poisonTimer = 0.0f;
            }
        }
    }
    
    // Update other status effects similarly...
    if (status.invulnerable) {
        status.invulnerabilityDuration -= deltaTime;
        if (status.invulnerabilityDuration <= 0.0f) {
            status.invulnerable = false;
        }
    }
}

void PlayerSystem::updateAnimations(Scene* scene, EntityID playerEntity, const PlayerState* state, float deltaTime) {
    if (!scene || !state || !scene->hasComponent<Sprite>(playerEntity)) return;
    
    auto& sprite = scene->getComponent<Sprite>(playerEntity);
    
    // Simple animation system - cycle through frames based on state
    // This would be more sophisticated in a full implementation
    if (sprite.texture) {
        int frameWidth = 32; // Assume 32x32 sprite frames
        int frameHeight = 32;
        int framesPerRow = sprite.texture->getWidth() / frameWidth;
        
        // Calculate frame based on state and animation timer
        int animFrame = 0;
        switch (state->currentState) {
            case PlayerState::State::Idle:
                animFrame = 0;
                break;
            case PlayerState::State::Walking:
                animFrame = (state->currentFrame % 4) + 1; // Frames 1-4 for walking
                break;
            case PlayerState::State::Running:
                animFrame = (state->currentFrame % 4) + 5; // Frames 5-8 for running
                break;
            default:
                animFrame = 0;
                break;
        }
        
        // Update sprite source rectangle
        int frameX = (animFrame % framesPerRow) * frameWidth;
        int frameY = (animFrame / framesPerRow) * frameHeight;
        sprite.sourceRect = Rect(frameX, frameY, frameWidth, frameHeight);
    }
}

void PlayerSystem::setupDefaultAbilities(PlayerAbilities* abilities) {
    if (!abilities) return;
    
    // Add some default abilities
    PlayerAbilities::Ability fireball;
    fireball.name = "Fireball";
    fireball.description = "Launch a fireball projectile";
    fireball.cooldown = 2.0f;
    fireball.manaCost = 10;
    fireball.damage = 25.0f;
    fireball.range = 200.0f;
    fireball.unlocked = true;
    abilities->addAbility(fireball);
    
    PlayerAbilities::Ability heal;
    heal.name = "Heal";
    heal.description = "Restore health over time";
    heal.cooldown = 5.0f;
    heal.manaCost = 15;
    heal.duration = 3.0f;
    heal.unlocked = true;
    abilities->addAbility(heal);
    
    // Set default hotbar
    abilities->hotbarAbilities[0] = 0; // Fireball
    abilities->hotbarAbilities[1] = 1; // Heal
}

void PlayerSystem::setupDefaultItems(PlayerInventory* inventory) {
    if (!inventory) return;
    
    // Add some default items
    auto healthPotion = std::make_shared<PlayerInventory::Item>();
    healthPotion->id = 1;
    healthPotion->name = "Health Potion";
    healthPotion->description = "Restores 50 health";
    healthPotion->healthRestore = 50;
    healthPotion->consumable = true;
    healthPotion->quantity = 3;
    
    auto manaPotion = std::make_shared<PlayerInventory::Item>();
    manaPotion->id = 2;
    manaPotion->name = "Mana Potion";
    manaPotion->description = "Restores 30 mana";
    manaPotion->manaRestore = 30;
    manaPotion->consumable = true;
    manaPotion->quantity = 2;
    
    inventory->addItem(healthPotion);
    inventory->addItem(manaPotion);
    
    // Add to hotbar
    inventory->hotbar[0] = healthPotion;
    inventory->hotbar[1] = manaPotion;
}

EntityID PlayerSystem::findPlayerEntity(Scene* scene) const {
    if (!scene) return 0;
    
    auto allEntities = scene->getAllLivingEntities();
    for (EntityID entity : allEntities) {
        if (isPlayerEntity(scene, entity)) {
            return entity;
        }
    }
    
    return 0;
}

bool PlayerSystem::isPlayerEntity(Scene* scene, EntityID entity) const {
    if (!scene) return false;
    
    return scene->hasComponent<PlayerController>(entity) &&
           scene->hasComponent<PlayerStats>(entity) &&
           scene->hasComponent<PlayerPhysics>(entity);
}

void PlayerSystem::triggerEvent(PlayerEvent::Type type, EntityID playerEntity, void* data) {
    PlayerEvent event;
    event.type = type;
    event.playerEntity = playerEntity;
    event.data = data;
    
    for (auto& callback : m_eventCallbacks) {
        callback(event);
    }
}

void PlayerSystem::registerEventCallback(PlayerEventCallback callback) {
    m_eventCallbacks.push_back(callback);
}

void PlayerSystem::useAbility(Scene* scene, EntityID playerEntity, int abilityIndex) {
    if (!scene || !scene->hasComponent<PlayerAbilities>(playerEntity)) return;
    
    auto& abilities = scene->getComponent<PlayerAbilities>(playerEntity);
    auto& stats = scene->getComponent<PlayerStats>(playerEntity);
    
    // Check if ability index is valid
    if (abilityIndex < 0 || abilityIndex >= static_cast<int>(abilities.abilities.size())) {
        return;
    }
    
    auto& ability = abilities.abilities[abilityIndex];
    
    // Check if ability is available and on cooldown
    if (!ability.unlocked || ability.currentCooldown > 0.0f) {
        return;
    }
    
    // Check if player has enough mana and stamina
    if (ability.manaCost > 0 && stats.derived.currentMana < ability.manaCost) {
        return;
    }
    
    if (ability.staminaCost > 0 && stats.derived.currentStamina < ability.staminaCost) {
        return;
    }
    
    // Use the ability
    if (ability.manaCost > 0) {
        stats.derived.currentMana -= ability.manaCost;
    }
    if (ability.staminaCost > 0) {
        stats.derived.currentStamina -= ability.staminaCost;
    }
    ability.currentCooldown = ability.cooldown;
    
    // Apply ability effect based on type
    if (ability.passive) {
        // Passive abilities don't have immediate effects
        printf("Player activated passive ability: %s\n", ability.name.c_str());
    } else {
        // Active ability effects
        if (ability.name == "Heal") {
            float healAmount = ability.damage; // Using damage field as heal amount
            stats.derived.currentHealth = std::min(stats.derived.currentHealth + (int)healAmount, stats.derived.maxHealth);
            printf("Player used Heal: +%.0f HP\n", healAmount);
        }        else if (ability.name == "Speed Boost") {
            auto& physics = scene->getComponent<PlayerPhysics>(playerEntity);
            physics.maxSpeed *= 1.5f; // Temporarily increase max speed
            printf("Player used Speed Boost (Speed: %.1f)\n", physics.maxSpeed);
        }        else if (ability.name == "Shield") {
            stats.status.invulnerable = true;
            stats.status.invulnerabilityDuration = ability.duration;
            printf("Player activated Shield\n");
        }
        else {
            // Generic ability effect
            printf("Player used ability: %s (Damage: %.1f, Range: %.1f)\n", 
                   ability.name.c_str(), ability.damage, ability.range);
        }
    }
    
    // Trigger ability used event
    triggerEvent(PlayerEvent::AbilityUsed, playerEntity, &ability);
}

void PlayerSystem::useItem(Scene* scene, EntityID playerEntity, int itemIndex) {
    if (!scene || !scene->hasComponent<PlayerInventory>(playerEntity)) return;
    
    auto& inventory = scene->getComponent<PlayerInventory>(playerEntity);
    auto& stats = scene->getComponent<PlayerStats>(playerEntity);
    
    // Check if item index is valid for hotbar
    if (itemIndex < 0 || itemIndex >= static_cast<int>(inventory.hotbar.size())) {
        return;
    }
    
    auto& itemPtr = inventory.hotbar[itemIndex];
    
    // Check if item slot has an item and quantity > 0
    if (!itemPtr || itemPtr->quantity <= 0) {
        return;
    }
    
    auto& item = *itemPtr;
    
    // Apply item effect based on properties
    bool itemUsed = false;
    
    if (item.consumable) {
        // Apply consumable effects
        if (item.name == "Health Potion" || item.healthRestore > 0) {
            int healAmount = std::max(item.healthRestore, 50); // Default 50 if not specified
            if (stats.derived.currentHealth < stats.derived.maxHealth) {
                stats.derived.currentHealth = std::min(stats.derived.currentHealth + healAmount, stats.derived.maxHealth);
                itemUsed = true;
                printf("Player used %s: +%d HP\n", item.name.c_str(), healAmount);
            }
        }
        else if (item.name == "Mana Potion" || item.manaRestore > 0) {
            int manaAmount = std::max(item.manaRestore, 30); // Default 30 if not specified
            if (stats.derived.currentMana < stats.derived.maxMana) {
                stats.derived.currentMana = std::min(stats.derived.currentMana + manaAmount, stats.derived.maxMana);
                itemUsed = true;
                printf("Player used %s: +%d MP\n", item.name.c_str(), manaAmount);
            }
        }
        else if (item.name == "Stamina Potion" || item.staminaRestore > 0) {
            int staminaAmount = std::max(item.staminaRestore, 40); // Default 40 if not specified
            if (stats.derived.currentStamina < stats.derived.maxStamina) {
                stats.derived.currentStamina = std::min(stats.derived.currentStamina + staminaAmount, stats.derived.maxStamina);
                itemUsed = true;
                printf("Player used %s: +%d Stamina\n", item.name.c_str(), staminaAmount);
            }
        }
        else {
            // Generic consumable
            printf("Player used consumable item: %s\n", item.name.c_str());
            itemUsed = true;
        }
    }
    else {
        // Handle non-consumable items (equipment, tools, etc.)
        printf("Player used item: %s\n", item.name.c_str());
        // Non-consumables don't get consumed but might have effects
    }
    
    // Consume the item if it was used and is consumable
    if (itemUsed && item.consumable) {
        item.quantity--;
        
        // Remove item from hotbar if quantity reaches 0
        if (item.quantity <= 0) {
            inventory.hotbar[itemIndex].reset(); // Reset shared_ptr to nullptr
        }
    }
    
    // Trigger item pickup event (you might want a separate ItemUsed event)
    if (itemUsed) {
        triggerEvent(PlayerEvent::ItemPickup, playerEntity, &item);
    }
}
