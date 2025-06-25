# 🎨 Using Your Own Images in Procedural Generation

## ✅ **YES! The procedural generator DOES use user-created assets!**

## 📋 **Current Asset Usage**

### **Your Existing Assets (Already Used):**
- ✅ `wall.png` → Used for walls, buildings, stones in dungeons
- ✅ `ground.png` → Used for floors, roads, grass in basic themes  
- ✅ `player.png` → Used by player entities
- ✅ `enemy.png` → Used by enemy entities
- ✅ `coin.png` → Used for collectibles

### **Generated Assets (Can Be Replaced):**
- 🔄 `assets/tiles/dungeon_door.png` → Dungeon doors
- 🔄 `assets/tiles/city_house.png` → City houses
- 🔄 `assets/tiles/city_road.png` → City roads  
- 🔄 `assets/tiles/terrain_grass.png` → Natural grass
- 🔄 `assets/tiles/terrain_water.png` → Water bodies
- 🔄 `assets/tiles/terrain_tree.png` → Forest trees

## 🚀 **How to Use Your Own Images**

### **Method 1: Replace Generated Tiles (Easiest)**
```bash
# Copy your custom images over the generated ones
copy your_custom_wall.png assets\tiles\dungeon_wall.png
copy your_custom_grass.png assets\tiles\terrain_grass.png
copy your_custom_house.png assets\tiles\city_house.png
```

### **Method 2: Add New Custom Images**
1. **Create your 32x32 PNG images**
2. **Place them in `assets/tiles/`**
3. **Update the sprite mappings** (see code section below)

### **Method 3: Use Existing Assets More**
The system already uses your `wall.png` and `ground.png`! You can expand this by:

```cpp
// In TileSpriteManager::loadCitySprites() - add this code:
cityMap[TileType::Building] = "wall.png";     // Use your wall.png
cityMap[TileType::Road] = "ground.png";       // Use your ground.png  
cityMap[TileType::House] = "player.png";      // Creative reuse!
```

## 🔧 **Custom Asset Integration Examples**

### **Example 1: Medieval Theme**
Create these 32x32 images and place in `assets/tiles/`:
- `medieval_castle.png` → For buildings
- `medieval_path.png` → For roads
- `medieval_grass.png` → For terrain
- `medieval_water.png` → For water

Then update the code:
```cpp
// In loadCitySprites():
cityMap[TileType::Building] = "medieval_castle.png";
cityMap[TileType::Road] = "medieval_path.png";
```

### **Example 2: Sci-Fi Theme**
- `scifi_metal.png` → For walls/buildings
- `scifi_floor.png` → For floors/roads  
- `scifi_energy.png` → For special tiles
- `scifi_tech.png` → For equipment

### **Example 3: Using Your Current Assets Creatively**
```cpp
// Reuse existing assets in new ways:
cityMap[TileType::House] = "player.png";      // Houses look like player sprites
cityMap[TileType::Shop] = "coin.png";         // Shops marked with coin
cityMap[TileType::Building] = "enemy.png";    // Buildings use enemy sprite
```

## 📁 **Asset File Structure**

```
assets/
├── wall.png           ← YOUR ASSET (already used!)
├── ground.png         ← YOUR ASSET (already used!)  
├── player.png         ← YOUR ASSET (already used!)
├── enemy.png          ← YOUR ASSET (already used!)
├── coin.png           ← YOUR ASSET (already used!)
└── tiles/
    ├── dungeon_door.png    ← Can replace with your image
    ├── city_house.png      ← Can replace with your image
    ├── city_road.png       ← Can replace with your image
    ├── terrain_grass.png   ← Can replace with your image
    ├── terrain_water.png   ← Can replace with your image
    └── [your_custom].png   ← Add your own!
```

## 🎯 **Quick Start: Replace Tiles Now**

### **Option A: Quick Replace (No Code Changes)**
```powershell
# Replace any generated tile with your image:
Copy-Item "my_awesome_wall.png" "assets\tiles\dungeon_wall.png"
Copy-Item "my_cool_grass.png" "assets\tiles\terrain_grass.png"  
Copy-Item "my_house.png" "assets\tiles\city_house.png"

# Rebuild and test:
.\build.bat
.\build\bin\Release\GameEditor.exe
```

### **Option B: Add New Theme (More Custom)**
1. Create your themed image set (all 32x32 PNG)
2. Place in `assets/tiles/`
3. Update `TileSpriteManager` to use them
4. Rebuild and enjoy!

## 🔍 **Testing Your Custom Assets**

### **Validation Script:**
```powershell
# Check which assets exist:
.\scripts\tile_asset_helper.bat validate

# Show current mappings:
.\scripts\tile_asset_helper.bat mappings
```

### **In the Editor:**
1. **Launch**: `.\build\bin\Release\GameEditor.exe`
2. **Generate**: Go to View → Procedural Generation
3. **Check Sprites**: Click "ℹ️ Sprite Info" to see what images are used
4. **Generate Maps**: Create dungeons/cities/terrain with your assets!

## ✨ **Benefits of Custom Assets**

- 🎨 **Your Art Style**: Procedural worlds match your game's visual style
- 🔄 **Easy Updates**: Just replace PNG files, no code changes needed
- 🎯 **Consistency**: All generated content uses your asset library
- 🚀 **Professional Look**: Custom sprites make generated worlds look polished

## 💡 **Pro Tips**

1. **32x32 Pixels**: Keep images at this size for consistency
2. **Transparent Backgrounds**: Use PNG with transparency for best results
3. **Batch Replace**: Update multiple tiles at once for themed environments
4. **Test Frequently**: Generate maps after each asset change to see results
5. **Backup Originals**: Keep copies of working assets before experimenting

Your procedural generation system is **designed to use your custom art** and will make your generated worlds look exactly how you want them! 🎮✨
