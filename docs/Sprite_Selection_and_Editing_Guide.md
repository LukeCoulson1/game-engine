# 🎨 Sprite Selection and Editing Guide

## ✅ Features Available

Your game engine editor already has **full sprite selection and editing capabilities**! Here's how to use them:

## 🖱️ **Sprite Selection Methods**

### **Method 1: Scene Window Selection (Visual)**
1. **Open a Scene Window** - Click on any scene window or create a new one
2. **Click on Sprites** - Left-click directly on any sprite/entity in the scene viewport
3. **Visual Feedback** - Selected entities show an **orange selection border**
4. **Entity Names** - Entity names are displayed next to selected sprites

### **Method 2: Scene Hierarchy Selection (List)**
1. **Open Scene Hierarchy** - View → Scene Hierarchy
2. **Click Entity** - Click any entity in the list (shows as "🎭 EntityName (ID)")
3. **Auto-sync** - Selection automatically syncs between Scene Hierarchy and Scene Windows

## 🔧 **Sprite Property Editing**

### **Inspector Panel Features**
Once a sprite is selected, the **Inspector panel** shows:

```
Inspector Panel:
├─ 🎭 Entity Name: [Editable Field]     ← Rename entities inline
├─ Entity ID: X
├─ Scene: [Scene Window Name]           ← Shows which scene it belongs to
├─ 
├─ ▼ Transform Component
│   ├─ 📍 World Coordinates
│   ├─ Position: [X, Y] (drag to edit) ← Support for negative coordinates
│   ├─ Scale: [X, Y] (drag to edit)
│   ├─ Rotation: [degrees] (drag to edit)
│   └─ 🖥️ Screen Position: (debug info)
├─
├─ ▼ Sprite Component                   ← MAIN EDITING AREA
│   ├─ 🎨 Sprite Properties:
│   ├─ ☑ Visible (toggle)              ← Show/hide sprite
│   ├─ Layer: [number] (drag to edit)  ← Rendering layer (higher = on top)
│   ├─ Tint Color: [Color Picker]      ← RGBA color tinting
│   ├─ 
│   ├─ 🖼️ Texture Assignment:
│   ├─ ✅ Texture: 64x64 pixels        ← Current texture info
│   ├─ [Remove] button                 ← Remove texture
│   ├─ 
│   ├─ 📁 Available Images:            ← TEXTURE BROWSER
│   ├─ (Click to assign texture)
│   ├─ [Scrollable list of all images]
│   ├─ - 🖼️ player.png                ← Click to assign
│   ├─ - 🖼️ enemy.png                 ← Click to assign
│   ├─ - 🖼️ coin.png                  ← Click to assign
│   ├─ 
│   └─ Source Rectangle (for sprite sheets):
│       ├─ X: [drag to crop]           ← Sprite sheet cropping
│       ├─ Y: [drag to crop]
│       ├─ Width: [drag to resize]
│       ├─ Height: [drag to resize]
│       └─ [Reset to Full Texture]
```

## 🎮 **Step-by-Step Usage**

### **Creating and Editing a Sprite:**

1. **Create Entity**:
   - Scene Hierarchy → "Create Entity" OR
   - Scene Window → Menu → Edit → "Create Entity"

2. **Add Sprite Component**:
   - Select entity → Inspector → "Add Sprite Component"

3. **Assign Texture**:
   - Inspector → Sprite Component → Available Images → Click any image
   - **OR** Asset Browser → Images → Click any image

4. **Edit Properties**:
   - **Visibility**: Toggle "Visible" checkbox
   - **Layering**: Drag "Layer" number (higher = renders on top)
   - **Color**: Use color picker for tinting effects
   - **Position**: Drag Transform → Position values
   - **Size**: Drag Transform → Scale values
   - **Sprite Sheets**: Adjust Source Rectangle to crop texture

### **Advanced Sprite Sheet Editing:**
1. **Assign sprite sheet texture** (e.g., tileset.png)
2. **Adjust Source Rectangle**:
   - X: 0, Y: 0, W: 32, H: 32 (first tile)
   - X: 32, Y: 0, W: 32, H: 32 (second tile)
   - X: 0, Y: 32, W: 32, H: 32 (tile below first)
3. **Reset button** restores full texture size

## 🖼️ **Visual Features**

### **Scene Window Rendering**:
- ✅ **Real textures displayed** (not colored rectangles)
- ✅ **Proper scaling** with zoom level
- ✅ **Selection borders** (orange highlight)
- ✅ **Entity names** shown next to sprites
- ✅ **Grid overlay** for alignment
- ✅ **Zoom and pan** controls

### **Inspector Integration**:
- ✅ **Live preview** of texture assignments
- ✅ **Real-time property updates**
- ✅ **Color picker** with alpha support
- ✅ **Asset browser** embedded in Inspector
- ✅ **Sprite sheet cropping** tools

## 🛠️ **Supported Image Formats**
- PNG (recommended)
- JPG/JPEG  
- BMP
- TGA

## 📁 **Asset Organization**
Images are automatically found in:
- `assets/` folder (main images)
- `assets/tiles/` folder (tile images)

## ⌨️ **Keyboard Shortcuts**

### **Scene Window**:
- **Left Click**: Select entity
- **Mouse Wheel**: Zoom in/out
- **Middle Mouse Drag**: Pan camera
- **Right Mouse Drag**: Pan camera
- **WASD/Arrow Keys**: Move camera
- **+/-**: Zoom in/out
- **1**: Reset zoom to 1:1
- **Delete**: Delete selected entity

### **Inspector**:
- **Enter**: Confirm entity name change
- **Drag**: Adjust numeric values (position, scale, etc.)

## 💡 **Pro Tips**

### **Multi-Window Editing**:
- Open multiple scene windows (File → New Scene)
- Each window has independent selection
- Inspector shows which scene the selected entity belongs to

### **Performance with Large Maps**:
- Procedural generation creates efficient tile rendering
- Only special entities (entrance/exit) become selectable entities
- Regular tiles are rendered as optimized batches

### **Sprite Layering**:
- Use Layer property to control rendering order
- Layer 0 = default, negative = background, positive = foreground
- Useful for creating depth effects

### **Color Tinting**:
- White tint = original colors
- Other colors = multiplicative tinting
- Alpha < 1.0 = transparency effects
- Use for damage effects, power-ups, etc.

## 🔧 **Console Feedback**
The console shows real-time feedback:
- "✅ Assigned texture: player.png to entity 1 in Scene1"
- "Renamed entity to: Player_Character in Scene1"
- "❌ Failed to load texture: missing.png"

## 🎯 **Ready to Use!**

All sprite selection and editing features are **fully implemented and working**! The editor provides:

1. **Visual sprite selection** in scene windows
2. **Comprehensive property editing** in Inspector
3. **Real-time texture assignment** with asset browser
4. **Advanced sprite sheet support** with cropping
5. **Multi-window scene editing** with independent selections
6. **Professional-grade feedback** and error handling

Your game engine editor is now ready for professional 2D game development with full sprite editing capabilities!
