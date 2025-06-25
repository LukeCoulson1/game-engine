@echo off
echo ==================================================
echo Testing Comprehensive Scene Save/Load Functionality
echo ==================================================
echo.
echo This test demonstrates that ALL entity properties are now saved:
echo.
echo 1. Transform (position, scale, rotation)
echo 2. Sprite (visibility, layer, tint, source rect)
echo 3. Collider (offset, size, trigger/static flags)
echo 4. RigidBody (velocity, acceleration, drag, mass, gravity)
echo 5. PlayerController (control scheme, movement settings, input state)
echo 6. PlayerStats (level, experience, attributes, health/mana/stamina, status effects)
echo 7. PlayerPhysics (enhanced physics with jump/dash/wall mechanics)
echo 8. PlayerInventory (items, hotbar, currency, equipment)
echo 9. PlayerAbilities (skills, cooldowns, hotbar abilities)
echo 10. PlayerState (animation state, frame tracking, facing direction)
echo.
echo Instructions:
echo 1. Run GameEditor.exe
echo 2. Create a new scene window (Scene ^> New Scene Window)
echo 3. Create entities and add various components in Inspector
echo 4. Make a player entity (adds all player components)
echo 5. Modify component properties (health, position, inventory items, etc.)
echo 6. Save the scene through Scene Manager
echo 7. Check console output - it shows how many entities/components are saved
echo 8. Close and reopen the scene to verify all properties are restored
echo.
echo Key Features:
echo • Complete component serialization - all properties preserved
echo • Detailed console feedback showing save/load progress
echo • Player inventory items with full item data
echo • Player abilities with cooldowns and skill points
echo • Complex nested data structures (stats, inventory) properly saved
echo • Procedural map data also preserved
echo.

cd build\bin\Release
echo Starting GameEditor...
GameEditor.exe

pause
