#pragma once

#include <SDL2/SDL_mixer.h>
#include <string>
#include <unordered_map>
#include <memory>

class Sound {
public:
    Sound(Mix_Chunk* chunk);
    ~Sound();
    
    void play(int loops = 0, int channel = -1);
    void stop(int channel = -1);
    
    Mix_Chunk* getChunk() const { return m_chunk; }

private:
    Mix_Chunk* m_chunk;
};

class Music {
public:
    Music(Mix_Music* music);
    ~Music();
    
    void play(int loops = -1);
    void stop();
    void pause();
    void resume();
    
    bool isPlaying() const;
    
    Mix_Music* getMusic() const { return m_music; }

private:
    Mix_Music* m_music;
};

class AudioManager {
public:
    AudioManager();
    ~AudioManager();
    
    bool initialize();
    void shutdown();
    
    // Sound effects
    std::shared_ptr<Sound> loadSound(const std::string& filepath);
    void playSound(const std::string& name, int loops = 0, int channel = -1);
    void stopSound(int channel = -1);
    void stopAllSounds();
    
    // Background music
    std::shared_ptr<Music> loadMusic(const std::string& filepath);
    void playMusic(const std::string& name, int loops = -1);
    void stopMusic();
    void pauseMusic();
    void resumeMusic();
    
    // Volume control (0-128)
    void setSoundVolume(int volume);
    void setMusicVolume(int volume);
    int getSoundVolume() const;
    int getMusicVolume() const;
    
    // Channel management
    void setChannels(int numChannels);
    int getAvailableChannel() const;

private:
    std::unordered_map<std::string, std::shared_ptr<Sound>> m_sounds;
    std::unordered_map<std::string, std::shared_ptr<Music>> m_music;
    
    bool m_initialized = false;
    int m_soundVolume = MIX_MAX_VOLUME;
    int m_musicVolume = MIX_MAX_VOLUME;
};
