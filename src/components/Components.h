#pragma once

#include <cstdint>
#include <bitset>
#include <memory>
#include <typeindex>
#include <unordered_map>
#include <array>
#include <vector>
#include <string>
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
