# PowerShell script to create sample images for testing
Add-Type -AssemblyName System.Drawing

# Create assets directory if it doesn't exist
$assetsDir = "assets"
if (!(Test-Path $assetsDir)) {
    New-Item -ItemType Directory -Path $assetsDir
}

# Function to create a colored square image
function Create-ColoredSquare {
    param(
        [string]$filename,
        [System.Drawing.Color]$color,
        [int]$size = 64
    )
    
    $bitmap = New-Object System.Drawing.Bitmap($size, $size)
    $graphics = [System.Drawing.Graphics]::FromImage($bitmap)
    $brush = New-Object System.Drawing.SolidBrush($color)
    
    # Fill background
    $graphics.FillRectangle($brush, 0, 0, $size, $size)
    
    # Add border
    $pen = New-Object System.Drawing.Pen([System.Drawing.Color]::Black, 2)
    $graphics.DrawRectangle($pen, 1, 1, $size-2, $size-2)
    
    # Save image
    $path = Join-Path $assetsDir $filename
    $bitmap.Save($path, [System.Drawing.Imaging.ImageFormat]::Png)
    
    $graphics.Dispose()
    $bitmap.Dispose()
    $brush.Dispose()
    $pen.Dispose()
    
    Write-Host "Created: $path"
}

# Function to create a sprite sheet
function Create-SpriteSheet {
    param(
        [string]$filename,
        [int]$tileSize = 32,
        [int]$tilesX = 4,
        [int]$tilesY = 4
    )
    
    $width = $tileSize * $tilesX
    $height = $tileSize * $tilesY
    $bitmap = New-Object System.Drawing.Bitmap($width, $height)
    $graphics = [System.Drawing.Graphics]::FromImage($bitmap)
    
    # Create a checkerboard pattern
    for ($x = 0; $x -lt $tilesX; $x++) {
        for ($y = 0; $y -lt $tilesY; $y++) {
            $color = if (($x + $y) % 2 -eq 0) { [System.Drawing.Color]::LightBlue } else { [System.Drawing.Color]::DarkBlue }
            $brush = New-Object System.Drawing.SolidBrush($color)
            $graphics.FillRectangle($brush, $x * $tileSize, $y * $tileSize, $tileSize, $tileSize)
            $brush.Dispose()
        }
    }
    
    # Add grid lines
    $pen = New-Object System.Drawing.Pen([System.Drawing.Color]::Black, 1)
    for ($x = 0; $x -le $tilesX; $x++) {
        $graphics.DrawLine($pen, $x * $tileSize, 0, $x * $tileSize, $height)
    }
    for ($y = 0; $y -le $tilesY; $y++) {
        $graphics.DrawLine($pen, 0, $y * $tileSize, $width, $y * $tileSize)
    }
    
    # Save image
    $path = Join-Path $assetsDir $filename
    $bitmap.Save($path, [System.Drawing.Imaging.ImageFormat]::Png)
    
    $graphics.Dispose()
    $bitmap.Dispose()
    $pen.Dispose()
    
    Write-Host "Created: $path"
}

# Create sample images
Write-Host "Creating sample images for game engine testing..."

Create-ColoredSquare "player.png" ([System.Drawing.Color]::Green) 64
Create-ColoredSquare "enemy.png" ([System.Drawing.Color]::Red) 64
Create-ColoredSquare "coin.png" ([System.Drawing.Color]::Gold) 32
Create-ColoredSquare "wall.png" ([System.Drawing.Color]::Gray) 32
Create-ColoredSquare "ground.png" ([System.Drawing.Color]::Brown) 32

Create-SpriteSheet "tileset.png" 32 8 8

Write-Host "Sample images created successfully!"
Write-Host "You can now use these images in the Game Editor's Asset Browser."
