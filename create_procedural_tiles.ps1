# Create Procedural Generation Tiles
# This script creates basic placeholder tiles for procedural generation

Add-Type -AssemblyName System.Drawing

function Create-ColoredTile {
    param(
        [string]$Path,
        [System.Drawing.Color]$Color,
        [int]$Size = 32
    )
    
    $bitmap = New-Object System.Drawing.Bitmap($Size, $Size)
    $graphics = [System.Drawing.Graphics]::FromImage($bitmap)
    
    # Fill with solid color
    $brush = New-Object System.Drawing.SolidBrush($Color)
    $graphics.FillRectangle($brush, 0, 0, $Size, $Size)
    
    # Add border
    $pen = New-Object System.Drawing.Pen([System.Drawing.Color]::Black, 1)
    $graphics.DrawRectangle($pen, 0, 0, $Size-1, $Size-1)
    
    $bitmap.Save($Path, [System.Drawing.Imaging.ImageFormat]::Png)
    
    $graphics.Dispose()
    $bitmap.Dispose()
    $brush.Dispose()
    $pen.Dispose()
}

# Ensure tiles directory exists
if (-not (Test-Path "assets\tiles")) {
    New-Item -ItemType Directory -Path "assets\tiles" -Force
}

Write-Host "Creating procedural generation tiles..."

# Dungeon theme tiles
Create-ColoredTile -Path "assets\tiles\dungeon_wall.png" -Color ([System.Drawing.Color]::FromArgb(101, 67, 33))  # Brown
Create-ColoredTile -Path "assets\tiles\dungeon_floor.png" -Color ([System.Drawing.Color]::FromArgb(139, 139, 139))  # Dark gray
Create-ColoredTile -Path "assets\tiles\dungeon_door.png" -Color ([System.Drawing.Color]::FromArgb(160, 82, 45))  # Saddle brown

# City theme tiles  
Create-ColoredTile -Path "assets\tiles\city_road.png" -Color ([System.Drawing.Color]::FromArgb(64, 64, 64))  # Dark gray
Create-ColoredTile -Path "assets\tiles\city_house.png" -Color ([System.Drawing.Color]::FromArgb(0, 100, 200))  # Blue
Create-ColoredTile -Path "assets\tiles\city_shop.png" -Color ([System.Drawing.Color]::FromArgb(200, 0, 0))  # Red  
Create-ColoredTile -Path "assets\tiles\city_building.png" -Color ([System.Drawing.Color]::FromArgb(128, 128, 128))  # Gray

# Terrain theme tiles
Create-ColoredTile -Path "assets\tiles\terrain_grass.png" -Color ([System.Drawing.Color]::FromArgb(34, 139, 34))  # Forest green
Create-ColoredTile -Path "assets\tiles\terrain_water.png" -Color ([System.Drawing.Color]::FromArgb(0, 191, 255))  # Deep sky blue
Create-ColoredTile -Path "assets\tiles\terrain_stone.png" -Color ([System.Drawing.Color]::FromArgb(105, 105, 105))  # Dim gray
Create-ColoredTile -Path "assets\tiles\terrain_tree.png" -Color ([System.Drawing.Color]::FromArgb(0, 100, 0))  # Dark green

# Special tiles
Create-ColoredTile -Path "assets\tiles\tile_entrance.png" -Color ([System.Drawing.Color]::FromArgb(255, 215, 0))  # Gold
Create-ColoredTile -Path "assets\tiles\tile_exit.png" -Color ([System.Drawing.Color]::FromArgb(255, 0, 0))  # Red
Create-ColoredTile -Path "assets\tiles\tile_empty.png" -Color ([System.Drawing.Color]::FromArgb(50, 50, 50))  # Very dark gray

Write-Host "Created procedural generation tiles:"
Get-ChildItem "assets\tiles\*.png" | ForEach-Object { Write-Host "  $($_.Name)" }

Write-Host ""
Write-Host "✅ Procedural generation tiles created successfully!"
Write-Host "You can now generate dungeons, cities, and terrain with visual tiles."
