#include "AudioManager.h"
#include <iostream>

// Sound Implementation
Sound::Sound(Mix_Chunk* chunk) : m_chunk(chunk) {
}

Sound::~Sound() {
    if (m_chunk) {
        Mix_FreeChunk(m_chunk);
    }
}

void Sound::play(int loops, int channel) {
    if (m_chunk) {
        Mix_PlayChannel(channel, m_chunk, loops);
    }
}

void Sound::stop(int channel) {
    Mix_HaltChannel(channel);
}

// Music Implementation
Music::Music(Mix_Music* music) : m_music(music) {
}

Music::~Music() {
    if (m_music) {
        Mix_FreeMusic(m_music);
    }
}

void Music::play(int loops) {
    if (m_music) {
        Mix_PlayMusic(m_music, loops);
    }
}

void Music::stop() {
    Mix_HaltMusic();
}

void Music::pause() {
    Mix_PauseMusic();
}

void Music::resume() {
    Mix_ResumeMusic();
}

bool Music::isPlaying() const {
    return Mix_PlayingMusic() == 1;
}

// AudioManager Implementation
AudioManager::AudioManager() {
}

AudioManager::~AudioManager() {
    shutdown();
}

bool AudioManager::initialize() {
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        std::cerr << "SDL_mixer could not initialize! SDL_mixer Error: " << Mix_GetError() << std::endl;
        return false;
    }
    
    // Set default number of mixing channels
    Mix_AllocateChannels(16);
    
    m_initialized = true;
    return true;
}

void AudioManager::shutdown() {
    if (m_initialized) {
        // Stop all audio
        stopAllSounds();
        stopMusic();
        
        // Clear cached audio
        m_sounds.clear();
        m_music.clear();
        
        Mix_CloseAudio();
        m_initialized = false;
    }
}

std::shared_ptr<Sound> AudioManager::loadSound(const std::string& filepath) {
    // Check if already loaded
    auto it = m_sounds.find(filepath);
    if (it != m_sounds.end()) {
        return it->second;
    }
    
    Mix_Chunk* chunk = Mix_LoadWAV(filepath.c_str());
    if (!chunk) {
        std::cerr << "Failed to load sound effect: " << filepath << " SDL_mixer Error: " << Mix_GetError() << std::endl;
        return nullptr;
    }
    
    auto sound = std::make_shared<Sound>(chunk);
    m_sounds[filepath] = sound;
    return sound;
}

void AudioManager::playSound(const std::string& name, int loops, int channel) {
    auto it = m_sounds.find(name);
    if (it != m_sounds.end()) {
        it->second->play(loops, channel);
    }
}

void AudioManager::stopSound(int channel) {
    Mix_HaltChannel(channel);
}

void AudioManager::stopAllSounds() {
    Mix_HaltChannel(-1);
}

std::shared_ptr<Music> AudioManager::loadMusic(const std::string& filepath) {
    // Check if already loaded
    auto it = m_music.find(filepath);
    if (it != m_music.end()) {
        return it->second;
    }
    
    Mix_Music* music = Mix_LoadMUS(filepath.c_str());
    if (!music) {
        std::cerr << "Failed to load music: " << filepath << " SDL_mixer Error: " << Mix_GetError() << std::endl;
        return nullptr;
    }
    
    auto musicObj = std::make_shared<Music>(music);
    m_music[filepath] = musicObj;
    return musicObj;
}

void AudioManager::playMusic(const std::string& name, int loops) {
    auto it = m_music.find(name);
    if (it != m_music.end()) {
        it->second->play(loops);
    }
}

void AudioManager::stopMusic() {
    Mix_HaltMusic();
}

void AudioManager::pauseMusic() {
    Mix_PauseMusic();
}

void AudioManager::resumeMusic() {
    Mix_ResumeMusic();
}

void AudioManager::setSoundVolume(int volume) {
    m_soundVolume = std::max(0, std::min(MIX_MAX_VOLUME, volume));
    Mix_Volume(-1, m_soundVolume);
}

void AudioManager::setMusicVolume(int volume) {
    m_musicVolume = std::max(0, std::min(MIX_MAX_VOLUME, volume));
    Mix_VolumeMusic(m_musicVolume);
}

int AudioManager::getSoundVolume() const {
    return m_soundVolume;
}

int AudioManager::getMusicVolume() const {
    return m_musicVolume;
}

void AudioManager::setChannels(int numChannels) {
    Mix_AllocateChannels(numChannels);
}

int AudioManager::getAvailableChannel() const {
    // Find first available channel
    for (int i = 0; i < Mix_AllocateChannels(-1); ++i) {
        if (!Mix_Playing(i)) {
            return i;
        }
    }
    return -1; // No available channels
}
