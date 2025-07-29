#pragma once

#include <cstdint>
#include <bitset>
#include <memory>
#include <typeindex>
#include <unordered_map>
#include <array>
#include <vector>
#include <string>
#include <algorithm>
#include <cmath>
#include <random>
#include "../graphics/Renderer.h"
#include <functional>
#include "graphics/Renderer.h" // Include for Vector2, Rect, Texture, Color

using EntityID = uint32_t;
using ComponentType = uint8_t;

const ComponentType MAX_COMPONENTS = 32;
using ComponentMask = std::bitset<MAX_COMPONENTS>;

// Base component class
class Component {
public:
    virtual ~Component() = default;
};

// Transform component - every entity should have this
class Transform : public Component {
public:
    Vector2 position{0, 0};
    Vector2 scale{1, 1};
    float rotation = 0.0f;
    
    Transform(float x = 0, float y = 0) : position(x, y), scale(1, 1), rotation(0.0f) {}
    Transform(const Vector2& pos) : position(pos), scale(1, 1), rotation(0.0f) {}
};

// Rotation component - separate from transform for node editor
class Rotation : public Component {
public:
    float angle = 0.0f; // Rotation angle in degrees
    
    Rotation() : angle(0.0f) {} // Default constructor
    Rotation(float rot) : angle(rot) {}
};

// Scale component - separate from transform for node editor  
class Scale : public Component {
public:
    Vector2 scale{1, 1}; // Scale factors for X and Y
    
    Scale() : scale(1, 1) {} // Default constructor
    Scale(float x, float y) : scale(x, y) {}
    Scale(const Vector2& s) : scale(s) {}
    Scale(float uniform) : scale(uniform, uniform) {}
};

// Sprite component for rendering
class Sprite : public Component {
public:
    std::shared_ptr<Texture> texture;
    Rect sourceRect; // Source rectangle in texture (for sprite sheets)
    Color tint{255, 255, 255, 255};
    bool visible = true;
    int layer = 0; // Rendering layer (higher = rendered on top)
    
    Sprite(std::shared_ptr<Texture> tex = nullptr) : texture(tex) {
        if (texture) {
            sourceRect = Rect(0, 0, texture->getWidth(), texture->getHeight());
        }
    }
};

// Collider component for physics/collision
class Collider : public Component {
public:
    Vector2 offset{0, 0};
    Vector2 size{32, 32};
    bool isTrigger = false;
    bool isStatic = false;
    
    Collider(float width = 32, float height = 32) : size(width, height) {}
    Collider(const Vector2& sz) : size(sz) {}
    
    Rect getBounds(const Vector2& position) const {
        return Rect(position.x + offset.x, position.y + offset.y, size.x, size.y);
    }
};

// Movement component
class RigidBody : public Component {
public:
    Vector2 velocity{0, 0};
    Vector2 acceleration{0, 0};
    float drag = 0.98f; // Air resistance
    float mass = 1.0f;
    bool useGravity = false;
    
    void addForce(const Vector2& force) {
        acceleration = acceleration + (force * (1.0f / mass));
    }
};

// Name component for entity identification
class Name : public Component {
public:
    std::string name;
    
    Name(const std::string& n = "Entity") : name(n) {}
};

// Component to mark entities as procedurally generated
class ProceduralGenerated : public Component {
public:
    enum class GenerationType {
        ConvertedTile,      // Tile converted to entity via clicking
        DirectGenerated,    // Entity created directly by procedural generator
        GameplayElement     // Special gameplay entities (entrance, exit, etc.)
    };
    
    GenerationType type;
    int tileX = -1;        // Original tile coordinates (if applicable)
    int tileY = -1;
    
    ProceduralGenerated(GenerationType genType = GenerationType::DirectGenerated) 
        : type(genType) {}
    
    ProceduralGenerated(GenerationType genType, int x, int y) 
        : type(genType), tileX(x), tileY(y) {}
};

// Player control schemes and input handling
class PlayerController : public Component {
public:
    enum class ControlScheme {
        WASD,           // W/A/S/D movement
        ArrowKeys,      // Arrow key movement
        WASD_Mouse,     // WASD + mouse for direction
        Gamepad,        // Xbox/PlayStation controller
        Custom          // User-defined bindings
    };
    
    enum class MovementType {
        TopDown,        // 8-directional movement (Zelda-like)
        Platformer,     // Side-scrolling with jump
        FirstPerson,    // 3D-style movement
        GridBased,      // Tile-based movement
        Physics         // Physics-driven movement
    };
    
    ControlScheme controlScheme = ControlScheme::WASD;
    MovementType movementType = MovementType::TopDown;
    
    // Movement settings
    float moveSpeed = 200.0f;           // Pixels per second
    float runSpeedMultiplier = 1.5f;    // Running speed boost
    float acceleration = 800.0f;        // How quickly player reaches max speed
    float deceleration = 600.0f;        // How quickly player stops
    
    // Jump settings (for platformer)
    float jumpForce = 400.0f;
    bool canDoubleJump = false;
    int maxJumps = 1;
    int jumpsRemaining = 1;
    
    // Input state
    Vector2 inputDirection{0, 0};       // Raw input direction
    Vector2 moveDirection{0, 0};        // Processed movement direction
    bool isRunning = false;
    bool isGrounded = true;
    bool jumpPressed = false;
    bool jumpHeld = false;
    
    // Custom key bindings
    std::unordered_map<std::string, int> keyBindings;
    
    PlayerController() {
        setDefaultKeyBindings();
    }
    
private:
    void setDefaultKeyBindings() {
        // Default WASD bindings (SDL scancodes)
        keyBindings["MoveUp"] = 26;     // W
        keyBindings["MoveDown"] = 22;   // S
        keyBindings["MoveLeft"] = 4;    // A
        keyBindings["MoveRight"] = 7;   // D
        keyBindings["Run"] = 225;       // Left Shift
        keyBindings["Jump"] = 44;       // Space
        keyBindings["Interact"] = 8;    // E
        keyBindings["Attack"] = 27;     // X
        keyBindings["Defend"] = 29;     // Z
    }
};

// Player statistics and RPG-like attributes
class PlayerStats : public Component {
public:
    // Core attributes
    struct CoreStats {
        int level = 1;
        int experience = 0;
        int experienceToNext = 100;
        
        // Primary stats
        int strength = 10;      // Affects damage, carry capacity
        int dexterity = 10;     // Affects speed, accuracy
        int intelligence = 10;  // Affects mana, magic damage
        int vitality = 10;      // Affects health, stamina
        int luck = 10;          // Affects critical chance, drops
    } core;
    
    // Derived stats (calculated from core stats)
    struct DerivedStats {
        int maxHealth = 100;
        int currentHealth = 100;
        int maxMana = 50;
        int currentMana = 50;
        int maxStamina = 100;
        int currentStamina = 100;
        
        int physicalDamage = 10;
        int magicalDamage = 5;
        int defense = 5;
        int magicResistance = 5;
        
        float criticalChance = 0.05f;   // 5%
        float criticalMultiplier = 2.0f;
        
        float moveSpeedModifier = 1.0f;
        float attackSpeedModifier = 1.0f;
    } derived;
    
    // Status effects
    struct StatusEffects {
        bool poisoned = false;
        bool burning = false;
        bool frozen = false;
        bool stunned = false;
        bool invulnerable = false;
        
        float poisonDuration = 0.0f;
        float burnDuration = 0.0f;
        float freezeDuration = 0.0f;
        float stunDuration = 0.0f;
        float invulnerabilityDuration = 0.0f;
    } status;
    
    // Methods for stat manipulation
    void gainExperience(int amount) {
        core.experience += amount;
        checkLevelUp();
    }
    
    void takeDamage(int damage) {
        if (status.invulnerable) return;
        
        derived.currentHealth = std::max(0, derived.currentHealth - damage);
    }
    
    void heal(int amount) {
        derived.currentHealth = std::min(derived.maxHealth, derived.currentHealth + amount);
    }
    
    void restoreMana(int amount) {
        derived.currentMana = std::min(derived.maxMana, derived.currentMana + amount);
    }
    
    void restoreStamina(int amount) {
        derived.currentStamina = std::min(derived.maxStamina, derived.currentStamina + amount);
    }
    
    bool isDead() const {
        return derived.currentHealth <= 0;
    }
    
    bool canCast(int manaCost) const {
        return derived.currentMana >= manaCost;
    }
      bool canPerformAction(int staminaCost) const {
        return derived.currentStamina >= staminaCost;
    }
      void recalculateStats() {
        derived.maxHealth = 50 + (core.vitality * 5);
        derived.maxMana = 25 + (core.intelligence * 3);
        derived.maxStamina = 50 + (core.vitality * 3);
        
        derived.physicalDamage = 5 + (core.strength * 2);
        derived.magicalDamage = 2 + (core.intelligence * 2);
        derived.defense = 2 + (core.vitality * 1);
        derived.magicResistance = 1 + (core.intelligence * 1);
        
        derived.criticalChance = 0.05f + (core.luck * 0.01f);
        derived.moveSpeedModifier = 1.0f + (core.dexterity * 0.02f);
        derived.attackSpeedModifier = 1.0f + (core.dexterity * 0.03f);
    }

private:
    void checkLevelUp() {
        while (core.experience >= core.experienceToNext) {
            core.experience -= core.experienceToNext;
            core.level++;
            core.experienceToNext = static_cast<int>(core.experienceToNext * 1.2f);
            onLevelUp();
        }
    }
    
    void onLevelUp() {
        // Increase stats on level up
        core.strength += 2;
        core.dexterity += 2;
        core.intelligence += 2;
        core.vitality += 2;
        core.luck += 1;
        
        // Recalculate derived stats
        recalculateStats();
        
        // Restore health/mana/stamina on level up
        derived.currentHealth = derived.maxHealth;        derived.currentMana = derived.maxMana;
        derived.currentStamina = derived.maxStamina;
    }
};

// Enhanced physics component for player movement
class PlayerPhysics : public Component {
public:
    // Movement physics
    Vector2 velocity{0, 0};
    Vector2 acceleration{0, 0};
    Vector2 externalForces{0, 0};  // Forces from environment, spells, etc.
    
    float maxSpeed = 300.0f;
    float friction = 0.85f;
    float airResistance = 0.98f;
    float mass = 1.0f;
    
    // Ground/collision physics
    bool isGrounded = false;
    bool isOnSlope = false;
    float slopeAngle = 0.0f;
    Vector2 groundNormal{0, -1};   // Normal of the ground surface
    
    // Jump physics
    bool canJump = true;
    float jumpCooldown = 0.0f;
    float coyoteTime = 0.1f;       // Grace period for jumping after leaving ground
    float jumpBufferTime = 0.1f;   // Input buffer for jump
    float coyoteTimer = 0.0f;
    float jumpBufferTimer = 0.0f;
    
    // Wall interaction
    bool canWallJump = false;
    bool isTouchingWall = false;
    Vector2 wallNormal{0, 0};
    
    // Special movement abilities
    bool canDash = false;
    bool isDashing = false;
    float dashForce = 500.0f;
    float dashDuration = 0.2f;
    float dashCooldown = 1.0f;
    float dashTimer = 0.0f;
    float dashCooldownTimer = 0.0f;
    
    void applyForce(const Vector2& force) {
        externalForces = externalForces + force;
    }
    
    void applyImpulse(const Vector2& impulse) {
        velocity = velocity + (impulse * (1.0f / mass));
    }
    
    void startDash(const Vector2& direction) {
        if (canDash && dashCooldownTimer <= 0.0f) {
            isDashing = true;
            dashTimer = dashDuration;
            dashCooldownTimer = dashCooldown;
            velocity = direction * dashForce;
        }
    }
};

// Inventory system for items and equipment
class PlayerInventory : public Component {
public:
    struct Item {
        int id = 0;
        std::string name;
        std::string description;
        int quantity = 1;
        int maxStack = 99;
        bool consumable = false;
        
        // Item stats/effects
        int healthRestore = 0;
        int manaRestore = 0;
        int staminaRestore = 0;
        int damageBonus = 0;
        int defenseBonus = 0;
    };
    
    struct Equipment {
        std::shared_ptr<Item> weapon;
        std::shared_ptr<Item> armor;
        std::shared_ptr<Item> accessory1;
        std::shared_ptr<Item> accessory2;
    };
    
    static const int MAX_INVENTORY_SLOTS = 20;
    static const int HOTBAR_SLOTS = 8;
    
    std::array<std::shared_ptr<Item>, MAX_INVENTORY_SLOTS> items;
    std::array<std::shared_ptr<Item>, HOTBAR_SLOTS> hotbar;
    Equipment equipped;
    
    int selectedHotbarSlot = 0;
    int currency = 0;  // Gold, coins, etc.
    
    bool addItem(std::shared_ptr<Item> item) {
        // Try to stack with existing items first
        for (auto& slot : items) {
            if (slot && slot->id == item->id && slot->quantity < slot->maxStack) {
                int spaceAvailable = slot->maxStack - slot->quantity;
                int amountToAdd = std::min(spaceAvailable, item->quantity);
                slot->quantity += amountToAdd;
                item->quantity -= amountToAdd;
                
                if (item->quantity <= 0) return true;
            }
        }
        
        // Find empty slot
        for (auto& slot : items) {
            if (!slot) {
                slot = item;
                return true;
            }
        }
        
        return false; // Inventory full
    }
    
    void removeItem(int slotIndex, int quantity = 1) {
        if (slotIndex >= 0 && slotIndex < MAX_INVENTORY_SLOTS && items[slotIndex]) {
            items[slotIndex]->quantity -= quantity;
            if (items[slotIndex]->quantity <= 0) {
                items[slotIndex].reset();
            }
        }
    }
    
    void useItem(int slotIndex, PlayerStats* stats = nullptr) {
        if (slotIndex >= 0 && slotIndex < MAX_INVENTORY_SLOTS && items[slotIndex]) {
            auto item = items[slotIndex];
            if (item->consumable && stats) {
                // Apply item effects
                if (item->healthRestore > 0) stats->heal(item->healthRestore);
                if (item->manaRestore > 0) stats->restoreMana(item->manaRestore);
                if (item->staminaRestore > 0) stats->restoreStamina(item->staminaRestore);
                
                removeItem(slotIndex, 1);
            }
        }
    }
    
    void equipItem(int slotIndex) {
        // Implementation depends on item type
        // Would check item type and move to appropriate equipment slot
    }
};

// Player abilities and skills
class PlayerAbilities : public Component {
public:
    struct Ability {
        std::string name;
        std::string description;
        int level = 1;
        int maxLevel = 10;
        
        float cooldown = 0.0f;
        float currentCooldown = 0.0f;
        int manaCost = 0;
        int staminaCost = 0;
        
        bool unlocked = false;
        bool passive = false;  // Passive vs active ability
        
        // Ability effects (to be extended based on game needs)
        float damage = 0.0f;
        float range = 0.0f;
        float duration = 0.0f;
    };
    
    std::vector<Ability> abilities;
    std::array<int, 4> hotbarAbilities{-1, -1, -1, -1}; // Indices into abilities vector
    
    int skillPoints = 0;
    
    void addAbility(const Ability& ability) {
        abilities.push_back(ability);
    }
    
    bool canUseAbility(int abilityIndex, const PlayerStats* stats = nullptr) const {
        if (abilityIndex < 0 || abilityIndex >= abilities.size()) return false;
        
        const auto& ability = abilities[abilityIndex];
        if (!ability.unlocked || ability.currentCooldown > 0.0f) return false;
        
        if (stats) {
            if (ability.manaCost > 0 && !stats->canCast(ability.manaCost)) return false;
            if (ability.staminaCost > 0 && !stats->canPerformAction(ability.staminaCost)) return false;
        }
        
        return true;
    }
    
    void useAbility(int abilityIndex, PlayerStats* stats = nullptr) {
        if (!canUseAbility(abilityIndex, stats)) return;
        
        auto& ability = abilities[abilityIndex];
        ability.currentCooldown = ability.cooldown;
        
        if (stats) {
            if (ability.manaCost > 0) {
                stats->derived.currentMana -= ability.manaCost;
            }
            if (ability.staminaCost > 0) {
                stats->derived.currentStamina -= ability.staminaCost;
            }
        }
        
        // Trigger ability effect (to be implemented in game logic)
    }
    
    void updateCooldowns(float deltaTime) {
        for (auto& ability : abilities) {
            if (ability.currentCooldown > 0.0f) {
                ability.currentCooldown = std::max(0.0f, ability.currentCooldown - deltaTime);
            }
        }
    }
    
    void levelUpAbility(int abilityIndex) {
        if (abilityIndex >= 0 && abilityIndex < abilities.size() && skillPoints > 0) {
            auto& ability = abilities[abilityIndex];
            if (ability.level < ability.maxLevel) {
                ability.level++;
                skillPoints--;
                
                // Improve ability stats based on level
                ability.damage *= 1.1f;
                ability.range *= 1.05f;
                ability.cooldown *= 0.95f;
            }
        }
    }
};

// Player state machine for animations and behaviors
class PlayerState : public Component {
public:
    enum class State {
        Idle,
        Walking,
        Running,
        Jumping,
        Falling,
        Attacking,
        Defending,
        Dashing,
        Interacting,
        Dead,
        Stunned
    };
    
    State currentState = State::Idle;
    State previousState = State::Idle;
    float stateTimer = 0.0f;
    
    // Animation frame tracking
    int currentFrame = 0;
    float frameTimer = 0.0f;
    float frameRate = 10.0f; // Frames per second
    
    // State-specific data
    Vector2 facing{1, 0}; // Direction player is facing
    bool actionInProgress = false;
    float actionDuration = 0.0f;
    
    void setState(State newState) {
        if (currentState != newState) {
            previousState = currentState;
            currentState = newState;
            stateTimer = 0.0f;
            currentFrame = 0;
            frameTimer = 0.0f;
            onStateChanged();
        }
    }
    
    void updateState(float deltaTime) {
        stateTimer += deltaTime;
        frameTimer += deltaTime;
        
        // Update animation frame
        if (frameTimer >= (1.0f / frameRate)) {
            currentFrame++;
            frameTimer = 0.0f;
        }
        
        // Update action duration
        if (actionInProgress) {
            actionDuration -= deltaTime;
            if (actionDuration <= 0.0f) {
                actionInProgress = false;
            }
        }
    }
    
    void startAction(float duration) {
        actionInProgress = true;
        actionDuration = duration;
    }
    
private:
    void onStateChanged() {
        // Reset any state-specific flags or timers
        switch (currentState) {
            case State::Attacking:
                startAction(0.5f); // Attack duration
                break;
            case State::Defending:
                startAction(1.0f); // Block duration
                break;
            case State::Dashing:
                startAction(0.2f); // Dash duration
                break;
            default:
                break;
        }
    }
};

// Entity Spawner component - allows entities to spawn other entities
class EntitySpawner : public Component {
public:
    struct SpawnTemplate {
        std::string name;           // Name for the spawned entity
        std::string spriteFile;     // Optional sprite file path
        Vector2 spawnOffset{0, 0};  // Offset from spawner position
        Vector2 velocity{0, 0};     // Initial velocity for spawned entity
        float lifeTime = 0.0f;      // Auto-destroy after this time (0 = permanent)
        bool hasCollider = false;   // Should spawned entity have collider
        bool hasRigidBody = false;  // Should spawned entity have physics
        float scale = 1.0f;         // Scale multiplier for spawned entity
        
        SpawnTemplate() = default;
        SpawnTemplate(const std::string& entityName, const std::string& sprite = "", 
                      Vector2 offset = Vector2(0, 0), Vector2 vel = Vector2(0, 0))
            : name(entityName), spriteFile(sprite), spawnOffset(offset), velocity(vel) {}
    };
    
    std::vector<SpawnTemplate> templates;  // Available spawn templates
    int selectedTemplate = 0;              // Currently selected template index
    float cooldownTime = 0.5f;             // Time between spawns
    float lastSpawnTime = 0.0f;            // Last time entity was spawned
    bool canSpawn = true;                  // Enable/disable spawning
    int maxSpawns = -1;                    // Max entities to spawn (-1 = unlimited)
    int spawnCount = 0;                    // Current spawn count
    Vector2 spawnDirection{1, 0};          // Direction to spawn entities
    bool inheritVelocity = false;          // Should spawned entities inherit spawner's velocity
    
    EntitySpawner() {
        // Add default arrow template for common use case
        templates.emplace_back("Arrow", "", Vector2(20, 0), Vector2(200, 0));
        templates.back().lifeTime = 3.0f;  // Arrows disappear after 3 seconds
        templates.back().hasCollider = true;
    }
    
    // Helper methods
    bool isReady(float currentTime) const {
        return canSpawn && 
               (maxSpawns == -1 || spawnCount < maxSpawns) &&
               (currentTime - lastSpawnTime >= cooldownTime);
    }
    
    void updateLastSpawnTime(float currentTime) {
        lastSpawnTime = currentTime;
        spawnCount++;
    }
    
    void addTemplate(const std::string& name, const std::string& sprite = "", 
                     Vector2 offset = Vector2(0, 0), Vector2 velocity = Vector2(0, 0)) {
        templates.emplace_back(name, sprite, offset, velocity);
    }
    
    void clearTemplates() {
        templates.clear();
        selectedTemplate = 0;
    }
    
    const SpawnTemplate* getCurrentTemplate() const {
        if (selectedTemplate >= 0 && selectedTemplate < static_cast<int>(templates.size())) {
            return &templates[selectedTemplate];
        }
        return nullptr;
    }
    
    void reset() {
        spawnCount = 0;
        lastSpawnTime = 0.0f;
    }
};

// Particle Effect component - allows entities to emit particles
class ParticleEffect : public Component {
public:
    struct Particle {
        Vector2 position{0, 0};
        Vector2 velocity{0, 0};
        Vector2 acceleration{0, 0};
        Color color{255, 255, 255, 255};
        float life = 1.0f;           // Current life remaining
        float maxLife = 1.0f;        // Maximum life time
        float size = 1.0f;           // Size multiplier
        float rotation = 0.0f;       // Rotation in degrees
        float rotationSpeed = 0.0f;  // Rotation speed per second
        bool active = true;
        
        Particle() = default;
        Particle(Vector2 pos, Vector2 vel, float lifetime = 1.0f) 
            : position(pos), velocity(vel), life(lifetime), maxLife(lifetime) {}
    };
    
    enum class EmissionShape {
        Point,      // Emit from a single point
        Circle,     // Emit from circle perimeter
        Box,        // Emit from rectangle area
        Cone        // Emit in a cone direction
    };
    
    enum class BlendMode {
        Normal,     // Standard alpha blending
        Additive,   // Additive blending for fire/glow effects
        Multiply    // Multiply blending for shadow effects
    };
    
    // Emission properties
    bool isEmitting = true;
    bool continuous = true;      // Continuous emission vs burst
    float emissionRate = 10.0f;  // Particles per second
    int maxParticles = 100;      // Maximum particle count
    float burstCount = 50;       // Particles in a burst
    
    // Particle lifetime
    float minLifetime = 1.0f;
    float maxLifetime = 3.0f;
    
    // Emission shape and area
    EmissionShape shape = EmissionShape::Point;
    Vector2 emissionSize{10, 10}; // Size of emission area (for Box/Circle)
    float coneAngle = 45.0f;      // Cone angle in degrees
    Vector2 direction{0, -1};     // Default emission direction (upward)
    
    // Velocity properties
    Vector2 minVelocity{-50, -100};
    Vector2 maxVelocity{50, -200};
    Vector2 gravity{0, 98};       // Gravity acceleration
    
    // Visual properties
    Color startColor{255, 255, 255, 255};
    Color endColor{255, 255, 255, 0};     // Fade to transparent
    float minSize = 1.0f;
    float maxSize = 5.0f;
    float sizeOverLife = 1.0f;    // Size multiplier over lifetime (1.0 = no change)
    
    // Rotation properties
    float minRotation = 0.0f;
    float maxRotation = 360.0f;
    float minRotationSpeed = 0.0f;
    float maxRotationSpeed = 180.0f;
    
    // Rendering
    BlendMode blendMode = BlendMode::Normal;
    std::shared_ptr<Texture> texture; // Optional texture (uses colored rectangles if null)
    int renderLayer = 10;         // Higher than most sprites for overlay effect
    
    // Runtime data
    std::vector<Particle> particles;
    float emissionTimer = 0.0f;
    float systemTime = 0.0f;      // Total time the system has been running
    
    // Control methods
    void startEmission() { isEmitting = true; }
    void stopEmission() { isEmitting = false; }
    void burst() { emitParticles(static_cast<int>(burstCount)); }
    void clear() { particles.clear(); }
    
    // Update method (called by particle system)
    void update(float deltaTime, const Vector2& emitterPosition);
    
    // Utility methods
    int getActiveParticleCount() const {
        return static_cast<int>(std::count_if(particles.begin(), particles.end(), 
            [](const Particle& p) { return p.active && p.life > 0; }));
    }
    
    bool isDead() const {
        return !isEmitting && getActiveParticleCount() == 0;
    }
    
    // Preset configurations
    void setupFireEffect() {
        startColor = Color(255, 100, 0, 255);    // Orange
        endColor = Color(255, 0, 0, 0);          // Red fading to transparent
        minVelocity = Vector2(-30, -100);
        maxVelocity = Vector2(30, -200);
        gravity = Vector2(0, -50);               // Slight upward force
        minLifetime = 0.5f;
        maxLifetime = 2.0f;
        shape = EmissionShape::Circle;
        emissionSize = Vector2(20, 20);
        blendMode = BlendMode::Additive;
        renderLayer = 15;
    }
    
    void setupSmokeEffect() {
        startColor = Color(200, 200, 200, 150);  // Light gray
        endColor = Color(100, 100, 100, 0);      // Dark gray fading out
        minVelocity = Vector2(-20, -50);
        maxVelocity = Vector2(20, -100);
        gravity = Vector2(0, -20);               // Slight upward drift
        minLifetime = 2.0f;
        maxLifetime = 4.0f;
        minSize = 2.0f;
        maxSize = 8.0f;
        sizeOverLife = 2.0f;                     // Particles grow over time
        shape = EmissionShape::Circle;
        emissionSize = Vector2(15, 15);
        blendMode = BlendMode::Normal;
    }
    
    void setupSparkEffect() {
        startColor = Color(255, 255, 100, 255);  // Bright yellow
        endColor = Color(255, 50, 0, 0);         // Orange red fading out
        minVelocity = Vector2(-150, -150);
        maxVelocity = Vector2(150, -50);
        gravity = Vector2(0, 200);               // Strong downward gravity
        minLifetime = 0.2f;
        maxLifetime = 1.0f;
        minSize = 0.5f;
        maxSize = 2.0f;
        emissionRate = 50.0f;
        maxParticles = 50;
        shape = EmissionShape::Point;
        blendMode = BlendMode::Additive;
        minRotationSpeed = -360.0f;
        maxRotationSpeed = 360.0f;
    }
    
    void setupMagicEffect() {
        startColor = Color(150, 100, 255, 200);  // Purple
        endColor = Color(255, 200, 255, 0);      // Light purple fading out
        minVelocity = Vector2(-80, -80);
        maxVelocity = Vector2(80, 80);
        gravity = Vector2(0, 0);                 // No gravity
        minLifetime = 1.0f;
        maxLifetime = 3.0f;
        minSize = 1.0f;
        maxSize = 4.0f;
        shape = EmissionShape::Circle;
        emissionSize = Vector2(30, 30);
        blendMode = BlendMode::Additive;
        minRotationSpeed = -90.0f;
        maxRotationSpeed = 90.0f;
        continuous = true;
        emissionRate = 20.0f;
    }
    
private:
    void emitParticles(int count);
    Vector2 getRandomVelocity() const;
    Vector2 getRandomPosition() const;
    Color interpolateColor(const Color& start, const Color& end, float t) const;
    float randomFloat(float min, float max) const;
};

//================================================================================
// NPC AND AI COMPONENTS
//================================================================================

// NPC Controller for non-player character behavior
class NPCController : public Component {
public:
    enum class NPCType {
        Friendly,
        Neutral,
        Hostile,
        Merchant,
        QuestGiver
    };
    
    enum class NPCState {
        Idle,
        Patrolling,
        Chasing,
        Attacking,
        Fleeing,
        Talking,
        Dead
    };
    
    NPCType type = NPCType::Neutral;
    NPCState currentState = NPCState::Idle;
    NPCState previousState = NPCState::Idle;
    
    float health = 100.0f;
    float maxHealth = 100.0f;
    float moveSpeed = 100.0f;
    float detectionRange = 150.0f;
    float attackRange = 50.0f;
    float fleeHealthThreshold = 20.0f;
    
    EntityID targetEntity = 0;
    Vector2 lastKnownTargetPosition{0, 0};
    float stateTimer = 0.0f;
    bool canInteract = true;
    
    std::string dialogueFile;
    std::string questId;
};

// AI Behavior component for complex AI logic
class AIBehavior : public Component {
public:
    enum class BehaviorType {
        StateMachine,
        BehaviorTree,
        UtilityBased,
        GoalOriented
    };
    
    BehaviorType type = BehaviorType::StateMachine;
    std::string behaviorScript;
    std::unordered_map<std::string, float> parameters;
    std::unordered_map<std::string, bool> flags;
    
    float aggroRadius = 100.0f;
    float hearingRadius = 80.0f;
    float fieldOfViewAngle = 60.0f;
    
    bool isAggressive = false;
    bool canHearPlayer = true;
    bool hasLineOfSight = false;
};

// AI State Machine component
class AIStateMachine : public Component {
public:
    struct State {
        std::string name;
        std::function<void()> onEnter;
        std::function<void(float)> onUpdate;
        std::function<void()> onExit;
        std::vector<std::string> transitions;
    };
    
    std::unordered_map<std::string, State> states;
    std::string currentState = "idle";
    std::string previousState;
    float stateTime = 0.0f;
    
    void addState(const std::string& name, const State& state) {
        states[name] = state;
    }
    
    void transitionTo(const std::string& newState) {
        if (states.find(newState) != states.end()) {
            if (states.find(currentState) != states.end()) {
                states[currentState].onExit();
            }
            previousState = currentState;
            currentState = newState;
            stateTime = 0.0f;
            states[currentState].onEnter();
        }
    }
};

// AI Pathfinding component
class AIPathfinding : public Component {
public:
    struct PathNode {
        Vector2 position;
        float cost = 0.0f;
        bool isBlocked = false;
    };
    
    std::vector<Vector2> currentPath;
    int currentPathIndex = 0;
    Vector2 destination{0, 0};
    float pathfindingRadius = 300.0f;
    float nodeSpacing = 32.0f;
    bool hasPath = false;
    bool reachedDestination = true;
    
    void setDestination(const Vector2& dest) {
        destination = dest;
        hasPath = false;
        reachedDestination = false;
    }
    
    Vector2 getNextPathPoint() const {
        if (currentPathIndex < currentPath.size()) {
            return currentPath[currentPathIndex];
        }
        return destination;
    }
};

// NPC Dialogue component
class NPCDialogue : public Component {
public:
    struct DialogueOption {
        std::string text;
        std::string response;
        std::string conditionScript;
        std::string actionScript;
        bool available = true;
    };
    
    struct DialogueNode {
        std::string text;
        std::vector<DialogueOption> options;
        std::string nextNode;
        bool isEnd = false;
    };
    
    std::unordered_map<std::string, DialogueNode> dialogueTree;
    std::string currentNode = "start";
    std::string npcName = "NPC";
    bool dialogueActive = false;
    bool hasSpokenBefore = false;
    
    void startDialogue() {
        dialogueActive = true;
        currentNode = hasSpokenBefore ? "greeting_repeat" : "greeting_first";
        hasSpokenBefore = true;
    }
};

// NPC Interaction component
class NPCInteraction : public Component {
public:
    enum class InteractionType {
        Talk,
        Trade,
        Quest,
        Heal,
        Custom
    };
    
    InteractionType type = InteractionType::Talk;
    float interactionRange = 64.0f;
    bool canInteract = true;
    std::string interactionPrompt = "Press E to interact";
    std::string interactionScript;
    
    // Trading specific
    std::vector<int> sellItems;
    std::vector<int> buyItems;
    float priceModifier = 1.0f;
    
    // Quest specific
    std::string questId;
    bool questCompleted = false;
};

//================================================================================
// ENVIRONMENT COMPONENTS
//================================================================================

// Environment Collider for world geometry
class EnvironmentCollider : public Component {
public:
    enum class ColliderShape {
        Rectangle,
        Circle,
        Polygon,
        Tilemap
    };
    
    ColliderShape shape = ColliderShape::Rectangle;
    Vector2 size{32, 32};
    float radius = 16.0f;
    std::vector<Vector2> vertices;
    bool isOneWayPlatform = false;
    bool isSlope = false;
    float slopeAngle = 0.0f;
};

// Environment Trigger for area-based events
class EnvironmentTrigger : public Component {
public:
    enum class TriggerType {
        Enter,
        Exit,
        Stay,
        Interact
    };
    
    TriggerType type = TriggerType::Enter;
    Vector2 size{64, 64};
    bool triggerOnce = false;
    bool hasTriggered = false;
    std::string triggerScript;
    std::vector<std::string> triggerTags;
    
    void reset() {
        hasTriggered = false;
    }
};

// Environment Hazard for damaging areas
class EnvironmentHazard : public Component {
public:
    enum class HazardType {
        Spikes,
        Fire,
        Poison,
        Electric,
        Ice,
        Void
    };
    
    HazardType type = HazardType::Spikes;
    float damage = 10.0f;
    float damageInterval = 1.0f;
    float lastDamageTime = 0.0f;
    bool instantKill = false;
    std::string statusEffect;
    float effectDuration = 0.0f;
};

// Environment Door for area transitions
class EnvironmentDoor : public Component {
public:
    enum class DoorType {
        Normal,
        Locked,
        Key,
        Switch,
        Timed
    };
    
    DoorType type = DoorType::Normal;
    bool isOpen = false;
    bool canOpen = true;
    std::string targetScene;
    Vector2 targetPosition{0, 0};
    
    // Lock/key system
    std::string requiredKey;
    int requiredKeyCount = 1;
    
    // Switch system
    std::string requiredSwitch;
    
    // Timed system
    float openDuration = 5.0f;
    float openTimer = 0.0f;
    
    void open() {
        if (canOpen) {
            isOpen = true;
            if (type == DoorType::Timed) {
                openTimer = openDuration;
            }
        }
    }
    
    void close() {
        isOpen = false;
        openTimer = 0.0f;
    }
};

// Environment Switch for activating mechanisms
class EnvironmentSwitch : public Component {
public:
    enum class SwitchType {
        Toggle,
        Pressure,
        Timed,
        Lever
    };
    
    SwitchType type = SwitchType::Toggle;
    bool isActivated = false;
    bool canActivate = true;
    std::string switchId;
    std::vector<std::string> connectedEntities;
    
    // Timed switches
    float activeDuration = 5.0f;
    float activeTimer = 0.0f;
    
    // Pressure switches
    int requiredWeight = 1;
    int currentWeight = 0;
    
    void activate() {
        if (canActivate) {
            isActivated = true;
            if (type == SwitchType::Timed) {
                activeTimer = activeDuration;
            }
        }
    }
    
    void deactivate() {
        isActivated = false;
        activeTimer = 0.0f;
    }
};

// Environment Platform for moving platforms
class EnvironmentPlatform : public Component {
public:
    enum class PlatformType {
        Static,
        Moving,
        Falling,
        Rotating
    };
    
    PlatformType type = PlatformType::Static;
    std::vector<Vector2> waypoints;
    int currentWaypoint = 0;
    float moveSpeed = 50.0f;
    bool looping = true;
    bool pingPong = false;
    bool movingForward = true;
    
    // Falling platform
    float fallDelay = 1.0f;
    float fallTimer = 0.0f;
    bool isFalling = false;
    bool resetAfterFall = true;
    
    // Rotating platform
    float rotationSpeed = 45.0f; // degrees per second
    Vector2 rotationCenter{0, 0};
};

//================================================================================
// AUDIO AND EFFECTS COMPONENTS
//================================================================================

// Audio Source component
class AudioSource : public Component {
public:
    std::string audioFile;
    float volume = 1.0f;
    float pitch = 1.0f;
    bool loop = false;
    bool playOnStart = false;
    bool is3D = false;
    bool isPlaying = false;
    
    // 3D audio properties
    float minDistance = 10.0f;
    float maxDistance = 100.0f;
    float rolloffFactor = 1.0f;
    
    void play() {
        isPlaying = true;
    }
    
    void stop() {
        isPlaying = false;
    }
    
    void pause() {
        // Implementation would pause the audio
    }
};

// Audio Listener component (usually on player/camera)
class AudioListener : public Component {
public:
    Vector2 forward{0, -1}; // Forward direction for 3D audio
    Vector2 up{0, 1};       // Up direction for 3D audio
    float masterVolume = 1.0f;
    bool active = true;
};

// Visual Effect component
class VisualEffect : public Component {
public:
    enum class EffectType {
        Explosion,
        Smoke,
        Fire,
        Lightning,
        Magic,
        Blood,
        Sparkles
    };
    
    EffectType type = EffectType::Explosion;
    float duration = 1.0f;
    float currentTime = 0.0f;
    bool autoDestroy = true;
    bool loop = false;
    
    // Visual properties
    Color startColor{255, 255, 255, 255};
    Color endColor{255, 255, 255, 0};
    float startScale = 1.0f;
    float endScale = 2.0f;
    
    bool isFinished() const {
        return !loop && currentTime >= duration;
    }
};

// Light Source component
class LightSource : public Component {
public:
    enum class LightType {
        Point,
        Directional,
        Spot
    };
    
    LightType type = LightType::Point;
    Color color{255, 255, 255, 255};
    float intensity = 1.0f;
    float range = 100.0f;
    Vector2 direction{0, -1}; // For directional/spot lights
    float spotAngle = 45.0f;  // For spot lights
    
    bool castShadows = false;
    bool enabled = true;
    
    // Dynamic light properties
    bool flicker = false;
    float flickerSpeed = 5.0f;
    float flickerIntensity = 0.2f;
    float flickerTimer = 0.0f;
};

//================================================================================
// UI COMPONENTS
//================================================================================

// Base UI Element component
class UIElement : public Component {
public:
    enum class AnchorType {
        TopLeft,
        TopCenter,
        TopRight,
        CenterLeft,
        Center,
        CenterRight,
        BottomLeft,
        BottomCenter,
        BottomRight
    };
    
    AnchorType anchor = AnchorType::TopLeft;
    Vector2 offset{0, 0};
    Vector2 size{100, 30};
    bool visible = true;
    bool interactive = true;
    int zIndex = 0;
    
    Color backgroundColor{50, 50, 50, 200};
    Color borderColor{100, 100, 100, 255};
    float borderWidth = 1.0f;
};

// UI Button component
class UIButton : public Component {
public:
    std::string text = "Button";
    Color normalColor{70, 70, 70, 200};
    Color hoverColor{90, 90, 90, 200};
    Color pressedColor{50, 50, 50, 200};
    Color textColor{255, 255, 255, 255};
    
    bool isHovered = false;
    bool isPressed = false;
    bool wasClicked = false;
    
    std::function<void()> onClick;
    
    void click() {
        wasClicked = true;        if (onClick) {
            onClick();
        }
    }
};

// UI Text component
class UIText : public Component {
public:
    std::string text = "Text";
    Color color{255, 255, 255, 255};
    int fontSize = 16;
    std::string fontFamily = "default";
    
    enum class Alignment {
        Left,
        Center,
        Right
    };
    
    Alignment alignment = Alignment::Left;
    bool wordWrap = false;
    float lineSpacing = 1.0f;
};

// UI Image component
class UIImage : public Component {
public:
    std::shared_ptr<Texture> texture;
    Color tint{255, 255, 255, 255};
    Rect sourceRect; // For sprite sheets
    
    enum class ScaleMode {
        Stretch,
        KeepAspect,
        Crop
    };
    
    ScaleMode scaleMode = ScaleMode::Stretch;
};

// UI Health Bar component
class UIHealthBar : public Component {
public:
    float currentValue = 100.0f;
    float maxValue = 100.0f;
    Color fillColor{255, 0, 0, 255};
    Color backgroundColor{50, 50, 50, 200};
    Color borderColor{100, 100, 100, 255};
    
    bool showText = true;
    bool showPercentage = false;
    bool animateChanges = true;
    float animationSpeed = 2.0f;
    
    float getPercentage() const {
        return maxValue > 0 ? (currentValue / maxValue) : 0.0f;
    }
};

// UI Inventory Slot component
class UIInventorySlot : public Component {
public:
    int slotIndex = 0;
    int itemId = 0;
    int itemCount = 0;
    bool isEmpty = true;
    bool isSelected = false;
    bool isHighlighted = false;
    
    Color emptyColor{40, 40, 40, 200};
    Color filledColor{60, 60, 60, 200};
    Color selectedColor{100, 150, 255, 200};
    Color highlightColor{255, 255, 100, 100};
    
    std::function<void(int)> onSlotClicked;
    
    void setItem(int id, int count) {
        itemId = id;
        itemCount = count;
        isEmpty = (count <= 0);
    }
    
    void clearItem() {
        itemId = 0;
        itemCount = 0;
        isEmpty = true;
    }
};
