# Nomenclature-Based Tile Assignment System

The game engine now features an intelligent nomenclature system that automatically assigns sprites to tiles based on standardized naming conventions. This makes it much easier to organize and use sprite assets.

## 🎯 How It Works

### **Naming Convention Hierarchy**

The system searches for sprites in this order of priority:

1. **Theme-specific sprites**: `{theme}_{tiletype}.png`
   - Examples: `city_wall.png`, `dungeon_floor.png`, `terrain_grass.png`

2. **Generic sprites**: `{tiletype}.png`
   - Examples: `wall.png`, `floor.png`, `grass.png`

3. **Alternative names**: Common alternative patterns
   - `ground.png` for floor tiles
   - `barrier.png` for wall tiles
   - `path.png` for road tiles

### **Supported Themes**

- **`dungeon`** - For dungeon generation
- **`city`** - For urban environments  
- **`terrain`** - For natural landscapes
- **`fantasy`** - For fantasy settings
- **`modern`** - For contemporary scenes

### **Tile Type Patterns**

Each tile type has multiple naming patterns it will recognize:

| Tile Type | Primary Pattern | Alternative Patterns |
|-----------|----------------|---------------------|
| Wall | `wall` | `barrier`, `block` |
| Floor | `floor` | `ground`, `tile` |
| Door | `door` | `entrance`, `gate` |
| Water | `water` | `sea`, `ocean`, `lake` |
| Grass | `grass` | `lawn`, `field` |
| Stone | `stone` | `rock`, `boulder` |
| Tree | `tree` | `forest`, `wood` |
| Building | `building` | `structure`, `house` |
| Road | `road` | `path`, `street` |
| House | `house` | `home`, `dwelling` |
| Shop | `shop` | `store`, `market` |
| Entrance | `entrance` | `entry`, `start` |
| Exit | `exit` | `end`, `finish` |

## 📁 **Recommended File Organization**

### **Directory Structure**
```
assets/
├── tiles/                    # Theme-specific tiles
│   ├── city_wall.png
│   ├── city_road.png
│   ├── city_house.png
│   ├── city_shop.png
│   ├── city_building.png
│   ├── dungeon_wall.png
│   ├── dungeon_floor.png
│   ├── dungeon_door.png
│   ├── terrain_grass.png
│   ├── terrain_water.png
│   ├── terrain_stone.png
│   └── terrain_tree.png
└── wall.png                 # Generic fallback sprites
    ground.png
    door.png
    etc.
```

### **Example Naming Schemes**

**City Theme:**
- `city_wall.png` - Building walls
- `city_road.png` - Streets and roads
- `city_house.png` - Residential buildings
- `city_shop.png` - Commercial buildings
- `city_building.png` - General structures

**Dungeon Theme:**
- `dungeon_wall.png` - Stone walls
- `dungeon_floor.png` - Walkable floors
- `dungeon_door.png` - Entrance/exit doors

**Terrain Theme:**
- `terrain_grass.png` - Grass fields
- `terrain_water.png` - Rivers, lakes
- `terrain_stone.png` - Rocky areas
- `terrain_tree.png` - Forest areas

## 🎮 **Using the System**

### **In the Editor**

1. **Open Procedural Generation Panel**
   - Windows → Procedural Generation

2. **Access Tile Image Assignment**
   - Expand the "🖼️ Tile Image Assignment" section

3. **Use Smart Auto-Assignment**
   - Click **"🔍 Auto-Assign Current Theme"** - Assigns sprites for the currently selected theme
   - Click **"🔍 Auto-Assign All Themes"** - Assigns sprites for all themes at once

4. **Check Results**
   - The system will automatically find and assign appropriate sprites
   - Check the Console panel for assignment results
   - Generated content will immediately use the new sprites

### **Manual Override**

You can still manually assign specific sprites:
- Use the text input fields for custom paths
- Click quick-assign buttons (Wall, Ground, Clear)
- The nomenclature system works alongside manual assignments

## 🔧 **Advanced Features**

### **File Extensions Supported**
- `.png` (recommended)
- `.jpg`, `.jpeg`
- `.bmp`
- `.tga`

### **Search Locations**
- `assets/tiles/` (primary location for theme-specific sprites)
- `assets/` (fallback location for generic sprites)

### **Automatic Detection**
The system automatically:
- Scans for matching files on startup
- Updates assignments when themes change
- Falls back gracefully if specific sprites aren't found

## 📋 **Best Practices**

### **For Artists/Content Creators**

1. **Use consistent naming**: Stick to the pattern `{theme}_{type}.png`
2. **Create fallbacks**: Provide generic versions (`wall.png`) for themes that don't have specific sprites
3. **Organize by theme**: Keep related sprites in the same directory
4. **Use descriptive names**: `forest_tree.png` is better than `sprite_001.png`

### **For Developers**

1. **Test auto-assignment**: Use the auto-assign buttons to verify nomenclature works
2. **Check console output**: Monitor for missing sprites or assignment issues
3. **Provide fallbacks**: Ensure basic sprites (wall, floor, etc.) are always available

## 🚨 **Troubleshooting**

### **Sprites Not Being Assigned**

1. **Check file names**: Ensure they match the expected patterns
2. **Verify file paths**: Files should be in `assets/tiles/` or `assets/`
3. **Check file extensions**: Use supported formats (.png, .jpg, etc.)
4. **Review console messages**: Look for error messages about missing files

### **Wrong Sprites Being Used**

1. **Check priority order**: Theme-specific sprites override generic ones
2. **Verify theme naming**: Make sure theme prefixes are correct
3. **Manual override**: Use manual assignment if needed

### **Performance Issues**

1. **Organize files**: Keep sprite files organized and avoid too many files in one directory
2. **Use appropriate sizes**: Don't use unnecessarily large image files

## 💡 **Examples**

### **Setting Up a City Theme**

1. Create these files in `assets/tiles/`:
   ```
   city_wall.png     (gray building)
   city_road.png     (asphalt texture)
   city_house.png    (residential building)
   city_shop.png     (commercial building)
   ```

2. Generate a city - sprites will be automatically assigned!

### **Custom Fantasy Theme**

1. Create files with the pattern:
   ```
   fantasy_wall.png     (magic barrier)
   fantasy_floor.png    (enchanted ground)
   fantasy_tree.png     (magical tree)
   ```

2. The system will automatically detect and use them when you select the Fantasy theme

The nomenclature system makes sprite management effortless while maintaining full flexibility for custom setups! 🎨✨
