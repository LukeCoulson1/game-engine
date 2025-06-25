#!/bin/bash
# Script to help users replace procedural generation tiles with their own images

echo "🎨 Custom Tile Asset Replacer"
echo "=============================="
echo ""

# Function to show current mappings
show_mappings() {
    echo "📋 Current Tile Mappings:"
    echo "Default Theme (uses your existing assets):"
    echo "  - Wall tiles → wall.png (your asset)"
    echo "  - Floor tiles → ground.png (your asset)" 
    echo "  - Buildings → wall.png (your asset)"
    echo "  - Roads → ground.png (your asset)"
    echo ""
    echo "Dungeon Theme:"
    echo "  - Walls → wall.png (your asset)"
    echo "  - Floors → ground.png (your asset)"
    echo "  - Doors → dungeon_door.png (generated)"
    echo ""
    echo "City Theme:"
    echo "  - Roads → city_road.png (generated)"
    echo "  - Houses → city_house.png (generated)"
    echo "  - Shops → city_shop.png (generated)"
    echo ""
    echo "Terrain Theme:"
    echo "  - Grass → terrain_grass.png (generated)"
    echo "  - Water → terrain_water.png (generated)"
    echo "  - Stone → terrain_stone.png (generated)"
    echo "  - Trees → terrain_tree.png (generated)"
}

# Function to replace a tile
replace_tile() {
    echo "📂 Available asset files:"
    ls -la assets/*.png 2>/dev/null || echo "No PNG files found in assets/"
    echo ""
    ls -la assets/tiles/*.png 2>/dev/null || echo "No PNG files found in assets/tiles/"
    echo ""
    
    echo "To replace a procedural tile with your own image:"
    echo "1. Copy your 32x32 PNG image to assets/tiles/"
    echo "2. Name it according to the tile type you want to replace"
    echo "3. Or update the sprite mappings in TileSpriteManager"
    echo ""
    echo "Example commands:"
    echo "  cp my_wall.png assets/tiles/dungeon_wall.png"
    echo "  cp my_grass.png assets/tiles/terrain_grass.png"
    echo "  cp my_house.png assets/tiles/city_house.png"
}

# Function to validate image format
validate_image() {
    local file="$1"
    if [ -f "$file" ]; then
        echo "✅ Found: $file"
        # Check if it's roughly 32x32 (basic validation)
        echo "💡 Tip: Ensure image is 32x32 pixels for best results"
    else
        echo "❌ Not found: $file"
    fi
}

# Main menu
case "$1" in
    "mappings"|"show")
        show_mappings
        ;;
    "replace")
        replace_tile
        ;;
    "validate")
        echo "🔍 Validating tile assets..."
        validate_image "assets/wall.png"
        validate_image "assets/ground.png"
        validate_image "assets/tiles/dungeon_door.png"
        validate_image "assets/tiles/city_house.png"
        validate_image "assets/tiles/terrain_grass.png"
        ;;
    *)
        echo "Usage: $0 [mappings|replace|validate]"
        echo ""
        echo "Commands:"
        echo "  mappings - Show current tile-to-sprite mappings"
        echo "  replace  - Instructions for replacing tiles with your images"
        echo "  validate - Check if tile assets exist"
        echo ""
        show_mappings
        ;;
esac
