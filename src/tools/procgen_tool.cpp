// Create a command-line procedural generation tool
// Add this to src/tools/procgen_tool.cpp

#include <iostream>
#include <string>
#include "generation/ProceduralGeneration.h"
#include "scene/Scene.h"

int main(int argc, char* argv[]) {
    if (argc < 4) {
        std::cout << "Usage: procgen_tool <type> <width> <height> [seed]\n";
        std::cout << "Types: dungeon, city, terrain\n";
        std::cout << "Example: procgen_tool dungeon 50 50 12345\n";
        return 1;
    }
    
    std::string type = argv[1];
    int width = std::stoi(argv[2]);
    int height = std::stoi(argv[3]);
    unsigned int seed = (argc > 4) ? std::stoi(argv[4]) : std::random_device{}();
    
    ProceduralGenerationManager manager;
    std::shared_ptr<ProceduralMap> map;
    
    if (type == "dungeon") {
        map = manager.generateDungeon(width, height, seed);
    } else if (type == "city") {
        map = manager.generateCity(width, height, seed);
    } else if (type == "terrain") {
        map = manager.generateTerrain(width, height, seed);
    } else {
        std::cerr << "Unknown type: " << type << std::endl;
        return 1;
    }
    
    // Print ASCII representation
    std::cout << "Generated " << type << " (" << width << "x" << height 
              << ", seed: " << seed << "):\n\n";
    
    for (int y = 0; y < map->getHeight(); ++y) {
        for (int x = 0; x < map->getWidth(); ++x) {
            const Tile& tile = map->getTile(x, y);
            char symbol = ' ';
            
            switch (tile.type) {
                case TileType::Wall: case TileType::Stone: case TileType::Building:
                    symbol = '#'; break;
                case TileType::Floor: case TileType::Road: case TileType::Grass:
                    symbol = '.'; break;
                case TileType::Water:
                    symbol = '~'; break;
                case TileType::Tree:
                    symbol = 'T'; break;
                case TileType::Door:
                    symbol = 'D'; break;
                default:
                    symbol = ' '; break;
            }
            std::cout << symbol;
        }
        std::cout << '\n';
    }
    
    return 0;
}
