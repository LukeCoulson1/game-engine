#include "ResourceManager.h"
#include "core/Engine.h"
#include "graphics/Renderer.h"
#include "audio/AudioManager.h"

ResourceManager::ResourceManager() {
}

ResourceManager::~ResourceManager() {
    clearAll();
}

std::shared_ptr<Texture> ResourceManager::loadTexture(const std::string& filepath) {
    // Check if already loaded
    auto it = m_textures.find(filepath);
    if (it != m_textures.end()) {
        return it->second;
    }
    
    // Load new texture
    auto& engine = Engine::getInstance();
    auto renderer = engine.getRenderer();
    if (!renderer) {
        return nullptr;
    }
    
    auto texture = renderer->loadTexture(filepath);
    if (texture) {
        m_textures[filepath] = texture;
    }
    
    return texture;
}

std::shared_ptr<Texture> ResourceManager::getTexture(const std::string& name) {
    auto it = m_textures.find(name);
    return (it != m_textures.end()) ? it->second : nullptr;
}

void ResourceManager::unloadTexture(const std::string& name) {
    m_textures.erase(name);
}

std::shared_ptr<Sound> ResourceManager::loadSound(const std::string& filepath) {
    // Check if already loaded
    auto it = m_sounds.find(filepath);
    if (it != m_sounds.end()) {
        return it->second;
    }
    
    // Load new sound
    auto& engine = Engine::getInstance();
    auto audioManager = engine.getAudioManager();
    if (!audioManager) {
        return nullptr;
    }
    
    auto sound = audioManager->loadSound(filepath);
    if (sound) {
        m_sounds[filepath] = sound;
    }
    
    return sound;
}

std::shared_ptr<Sound> ResourceManager::getSound(const std::string& name) {
    auto it = m_sounds.find(name);
    return (it != m_sounds.end()) ? it->second : nullptr;
}

void ResourceManager::unloadSound(const std::string& name) {
    m_sounds.erase(name);
}

std::shared_ptr<Music> ResourceManager::loadMusic(const std::string& filepath) {
    // Check if already loaded
    auto it = m_music.find(filepath);
    if (it != m_music.end()) {
        return it->second;
    }
    
    // Load new music
    auto& engine = Engine::getInstance();
    auto audioManager = engine.getAudioManager();
    if (!audioManager) {
        return nullptr;
    }
    
    auto music = audioManager->loadMusic(filepath);
    if (music) {
        m_music[filepath] = music;
    }
    
    return music;
}

std::shared_ptr<Music> ResourceManager::getMusic(const std::string& name) {
    auto it = m_music.find(name);
    return (it != m_music.end()) ? it->second : nullptr;
}

void ResourceManager::unloadMusic(const std::string& name) {
    m_music.erase(name);
}

void ResourceManager::clearAll() {
    clearTextures();
    clearAudio();
}

void ResourceManager::clearTextures() {
    m_textures.clear();
}

void ResourceManager::clearAudio() {
    m_sounds.clear();
    m_music.clear();
}
