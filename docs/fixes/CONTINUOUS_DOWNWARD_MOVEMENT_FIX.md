# Continuous Downward Movement Fix

## Issue Description

**Problem**: When using the Game Logic Window with a PlayerController entity, the entity would continuously travel downward even when no keys were pressed.

**Root Cause**: The PlayerSystem was applying gravity (981 pixels/second²) to ALL entities with PlayerPhysics components, regardless of the movement type. Since the default PlayerPhysics component has `isGrounded = false`, gravity was constantly pulling entities downward.

**Impact**: This made top-down games unplayable as entities would fall continuously instead of staying stationary.

## Solution

### Code Changes

#### 1. Modified PlayerSystem::updatePhysics() Method

**File**: `src/systems/PlayerSystem.cpp`

**Before**:
```cpp
void PlayerSystem::updatePhysics(PlayerPhysics* physics, const Vector2& inputDirection, float deltaTime) {
    // ... movement code ...
    
    // Apply gravity for platformer movement
    if (!physics->isGrounded) {
        applyGravity(physics, deltaTime);
    }
}
```

**After**:
```cpp
void PlayerSystem::updatePhysics(PlayerPhysics* physics, const PlayerController* controller, const Vector2& inputDirection, float deltaTime) {
    // ... movement code ...
    
    // Apply gravity only for platformer/physics movement types
    if (!physics->isGrounded && shouldApplyGravity(controller)) {
        applyGravity(physics, deltaTime);
    }
}
```

#### 2. Added Movement Type Check

**New Method**:
```cpp
bool PlayerSystem::shouldApplyGravity(const PlayerController* controller) const {
    if (!controller) return false;
    
    // Only apply gravity for platformer and physics-based movement
    return controller->movementType == PlayerController::MovementType::Platformer ||
           controller->movementType == PlayerController::MovementType::Physics;
}
```

#### 3. Updated Method Signatures

**Header File**: `src/systems/PlayerSystem.h`

```cpp
// Updated method signatures
void updatePhysics(PlayerPhysics* physics, const PlayerController* controller, const Vector2& inputDirection, float deltaTime);
bool shouldApplyGravity(const PlayerController* controller) const;
```

## Movement Type Gravity Behavior

| Movement Type | Applies Gravity | Use Case |
|---------------|-----------------|----------|
| **TopDown** (default) | ❌ No | Zelda-style games, top-down RPGs |
| **GridBased** | ❌ No | Tile-based games, roguelikes |
| **FirstPerson** | ❌ No | 3D-style movement |
| **Platformer** | ✅ Yes | Side-scrolling games, Mario-style |
| **Physics** | ✅ Yes | Physics-driven gameplay |

## Default Behavior

- **Default Movement Type**: `MovementType::TopDown`
- **Default Gravity**: Disabled for top-down movement
- **Movement Style**: 8-directional WASD movement
- **Physics**: Friction-based deceleration, no gravity

## Testing

### Expected Behavior After Fix

1. **Stationary When Idle**: Entity remains in place when no keys are pressed
2. **No Downward Drift**: No continuous downward movement
3. **Responsive Movement**: WASD keys provide immediate, controlled movement
4. **Proper Stopping**: Entity stops moving when keys are released (due to friction)

### Test Steps

1. Create entity with PlayerController component
2. Start Game Logic Window runtime (F5)
3. Observe entity behavior without key input
4. Test WASD movement in all directions
5. Verify entity stops when keys are released

## Gravity Constants

For reference, the gravity system uses these values:

```cpp
static constexpr float GRAVITY_FORCE = 981.0f; // Pixels per second squared
static constexpr float TERMINAL_VELOCITY = 500.0f; // Max falling speed
```

These values are only applied when:
- `MovementType` is `Platformer` or `Physics`
- `PlayerPhysics::isGrounded` is `false`

## Related Components

### PlayerController Default Settings
```cpp
ControlScheme controlScheme = ControlScheme::WASD;
MovementType movementType = MovementType::TopDown;  // Key setting!
float moveSpeed = 200.0f;
```

### PlayerPhysics Default Settings
```cpp
bool isGrounded = false;  // Would trigger gravity if not for movement type check
float friction = 0.85f;   // Provides natural stopping
float maxSpeed = 300.0f;
```

## Files Modified

1. `src/systems/PlayerSystem.cpp`
   - Updated `updatePhysics()` method signature and logic
   - Added `shouldApplyGravity()` helper method
   - Fixed incomplete method at end of file

2. `src/systems/PlayerSystem.h`
   - Updated method signatures
   - Added `shouldApplyGravity()` declaration

3. `test_gravity_fix.bat`
   - Comprehensive test script for the fix

## Status

✅ **FIXED** - Entities with PlayerController no longer continuously move downward

- Top-down movement (default) works correctly without gravity
- WASD movement is responsive and controlled
- Entities remain stationary when no input is provided
- Gravity is properly restricted to platformer-style games
- All movement types work as intended for their respective game styles

## Usage Notes

- **For Top-Down Games**: Use default settings (MovementType::TopDown)
- **For Platformer Games**: Set `movementType = MovementType::Platformer` to enable gravity
- **For Physics Games**: Set `movementType = MovementType::Physics` for full physics simulation
- **Custom Gravity**: Modify `GRAVITY_FORCE` constant if needed for platformer games
