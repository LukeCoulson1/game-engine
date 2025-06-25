#pragma once

#include <unordered_map>
#include <memory>
#include <string>

// Forward declarations
class Texture;
class Sound;
class Music;

class ResourceManager {
public:
    ResourceManager();
    ~ResourceManager();
    
    // Texture management
    std::shared_ptr<Texture> loadTexture(const std::string& filepath);
    std::shared_ptr<Texture> getTexture(const std::string& name);
    void unloadTexture(const std::string& name);
    
    // Audio management
    std::shared_ptr<Sound> loadSound(const std::string& filepath);
    std::shared_ptr<Sound> getSound(const std::string& name);
    void unloadSound(const std::string& name);
    
    std::shared_ptr<Music> loadMusic(const std::string& filepath);
    std::shared_ptr<Music> getMusic(const std::string& name);
    void unloadMusic(const std::string& name);
    
    // Resource cleanup
    void clearAll();
    void clearTextures();
    void clearAudio();
    
    // Resource info
    size_t getLoadedTextureCount() const { return m_textures.size(); }
    size_t getLoadedSoundCount() const { return m_sounds.size(); }
    size_t getLoadedMusicCount() const { return m_music.size(); }

private:
    std::unordered_map<std::string, std::shared_ptr<Texture>> m_textures;
    std::unordered_map<std::string, std::shared_ptr<Sound>> m_sounds;
    std::unordered_map<std::string, std::shared_ptr<Music>> m_music;
};
