#include "CoreSystems.h"
#include "../components/Components.h"
#include "../scene/Scene.h"
#include <SDL_mixer.h>
#include <iostream>
#include <unordered_map>

// Static audio resource management
static std::unordered_map<std::string, Mix_Chunk*> s_audioChunks;
static std::unordered_map<std::string, Mix_Music*> s_musicTracks;
static bool s_audioInitialized = false;

void AudioSystem::initialize() {
    if (s_audioInitialized) return;
    
    // Initialize SDL_mixer
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        printf("ERROR: SDL_mixer could not initialize! Mix_Error: %s\n", Mix_GetError());
        return;
    }
    
    // Allocate mixing channels
    Mix_AllocateChannels(32);
    
    s_audioInitialized = true;
    printf("INFO: Audio system initialized successfully\n");
}

void AudioSystem::shutdown() {
    if (!s_audioInitialized) return;
    
    // Free all loaded audio chunks
    for (auto& pair : s_audioChunks) {
        if (pair.second) {
            Mix_FreeChunk(pair.second);
        }
    }
    s_audioChunks.clear();
    
    // Free all loaded music tracks
    for (auto& pair : s_musicTracks) {
        if (pair.second) {
            Mix_FreeMusic(pair.second);
        }
    }
    s_musicTracks.clear();
    
    Mix_CloseAudio();
    s_audioInitialized = false;
    printf("INFO: Audio system shut down\n");
}

void AudioSystem::update(float deltaTime) {
    if (!m_scene || !s_audioInitialized) return;
    
    // Update audio sources
    for (const auto& entity : entities) {
        auto& audioSource = m_scene->getComponent<AudioSource>(entity);
        auto& transform = m_scene->getComponent<Transform>(entity);
        
        // Handle play on start
        if (audioSource.playOnStart && !audioSource.isPlaying && !audioSource.audioFile.empty()) {
            playSound(audioSource, entity);
        }
        
        // Update 3D audio positioning if enabled
        if (audioSource.is3D && audioSource.isPlaying) {
            update3DAudio(audioSource, transform, entity);
        }
    }
}

void AudioSystem::playSound(AudioSource& audioSource, EntityID entity) {
    if (!s_audioInitialized || audioSource.audioFile.empty()) return;
    
    // Load audio file if not already loaded
    Mix_Chunk* chunk = loadAudioChunk(audioSource.audioFile);
    if (!chunk) {
        printf("ERROR: Failed to load audio file: %s\n", audioSource.audioFile.c_str());
        return;
    }
    
    // Set volume (0-128)
    Mix_VolumeChunk(chunk, static_cast<int>(audioSource.volume * 128));
    
    // Play the sound
    int channel = Mix_PlayChannel(-1, chunk, audioSource.loop ? -1 : 0);
    if (channel == -1) {
        printf("ERROR: Failed to play sound: %s\n", Mix_GetError());
        return;
    }
    
    // Store channel for this entity
    m_entityChannels[entity] = channel;
    audioSource.isPlaying = true;
    
    printf("INFO: Playing audio '%s' on channel %d for entity %u\n", 
           audioSource.audioFile.c_str(), channel, entity);
}

void AudioSystem::stopSound(AudioSource& audioSource, EntityID entity) {
    if (!s_audioInitialized) return;
    
    auto it = m_entityChannels.find(entity);
    if (it != m_entityChannels.end()) {
        Mix_HaltChannel(it->second);
        m_entityChannels.erase(it);
    }
    
    audioSource.isPlaying = false;
    printf("INFO: Stopped audio for entity %u\n", entity);
}

void AudioSystem::pauseSound(AudioSource& audioSource, EntityID entity) {
    if (!s_audioInitialized) return;
    
    auto it = m_entityChannels.find(entity);
    if (it != m_entityChannels.end()) {
        Mix_Pause(it->second);
    }
    
    printf("INFO: Paused audio for entity %u\n", entity);
}

void AudioSystem::resumeSound(AudioSource& audioSource, EntityID entity) {
    if (!s_audioInitialized) return;
    
    auto it = m_entityChannels.find(entity);
    if (it != m_entityChannels.end()) {
        Mix_Resume(it->second);
    }
    
    printf("INFO: Resumed audio for entity %u\n", entity);
}

void AudioSystem::setMasterVolume(float volume) {
    if (!s_audioInitialized) return;
    
    m_masterVolume = std::clamp(volume, 0.0f, 1.0f);
    Mix_MasterVolume(static_cast<int>(m_masterVolume * 128));
    
    printf("INFO: Set master volume to %.2f\n", m_masterVolume);
}

void AudioSystem::playMusic(const std::string& filename, bool loop) {
    if (!s_audioInitialized || filename.empty()) return;
    
    // Stop current music
    if (Mix_PlayingMusic()) {
        Mix_HaltMusic();
    }
    
    // Load music file
    Mix_Music* music = loadMusicTrack(filename);
    if (!music) {
        printf("ERROR: Failed to load music file: %s\n", filename.c_str());
        return;
    }
    
    // Play music
    if (Mix_PlayMusic(music, loop ? -1 : 0) == -1) {
        printf("ERROR: Failed to play music: %s\n", Mix_GetError());
        return;
    }
    
    printf("INFO: Playing music '%s' %s\n", filename.c_str(), loop ? "(looping)" : "(once)");
}

void AudioSystem::stopMusic() {
    if (!s_audioInitialized) return;
    
    if (Mix_PlayingMusic()) {
        Mix_HaltMusic();
        printf("INFO: Stopped music\n");
    }
}

void AudioSystem::setMusicVolume(float volume) {
    if (!s_audioInitialized) return;
    
    volume = std::clamp(volume, 0.0f, 1.0f);
    Mix_VolumeMusic(static_cast<int>(volume * 128));
    
    printf("INFO: Set music volume to %.2f\n", volume);
}

Mix_Chunk* AudioSystem::loadAudioChunk(const std::string& filename) {
    // Check if already loaded
    auto it = s_audioChunks.find(filename);
    if (it != s_audioChunks.end()) {
        return it->second;
    }
    
    // Load new audio chunk
    Mix_Chunk* chunk = Mix_LoadWAV(filename.c_str());
    if (!chunk) {
        printf("ERROR: Failed to load audio chunk '%s': %s\n", filename.c_str(), Mix_GetError());
        return nullptr;
    }
    
    // Store in cache
    s_audioChunks[filename] = chunk;
    printf("INFO: Loaded audio chunk: %s\n", filename.c_str());
    
    return chunk;
}

Mix_Music* AudioSystem::loadMusicTrack(const std::string& filename) {
    // Check if already loaded
    auto it = s_musicTracks.find(filename);
    if (it != s_musicTracks.end()) {
        return it->second;
    }
    
    // Load new music track
    Mix_Music* music = Mix_LoadMUS(filename.c_str());
    if (!music) {
        printf("ERROR: Failed to load music track '%s': %s\n", filename.c_str(), Mix_GetError());
        return nullptr;
    }
    
    // Store in cache
    s_musicTracks[filename] = music;
    printf("INFO: Loaded music track: %s\n", filename.c_str());
    
    return music;
}

void AudioSystem::update3DAudio(AudioSource& audioSource, const Transform& transform, EntityID entity) {
    if (!s_audioInitialized) return;
    
    auto it = m_entityChannels.find(entity);
    if (it == m_entityChannels.end()) return;
    
    int channel = it->second;
    
    // Calculate distance from listener (assuming listener is at origin for simplicity)
    Vector2 listenerPos = m_listenerPosition;
    float distance = std::sqrt(std::pow(transform.position.x - listenerPos.x, 2) + 
                              std::pow(transform.position.y - listenerPos.y, 2));
    
    // Calculate volume based on distance
    float volumeAttenuation = 1.0f;
    if (distance > audioSource.minDistance) {
        if (distance >= audioSource.maxDistance) {
            volumeAttenuation = 0.0f;
        } else {
            float distanceRatio = (distance - audioSource.minDistance) / 
                                (audioSource.maxDistance - audioSource.minDistance);
            volumeAttenuation = 1.0f - std::pow(distanceRatio, audioSource.rolloffFactor);
        }
    }
    
    // Apply volume
    int finalVolume = static_cast<int>(audioSource.volume * volumeAttenuation * 128);
    Mix_Volume(channel, finalVolume);
    
    // Calculate panning (simplified stereo panning)
    float panningRatio = (transform.position.x - listenerPos.x) / audioSource.maxDistance;
    panningRatio = std::clamp(panningRatio, -1.0f, 1.0f);
    
    // SDL_mixer doesn't have built-in panning, so this is a placeholder
    // In a full implementation, you would use Mix_SetPanning or custom audio processing
    
    printf("DEBUG: 3D Audio - Entity %u, Distance: %.1f, Volume: %d, Pan: %.2f\n", 
           entity, distance, finalVolume, panningRatio);
}

void AudioSystem::setListenerPosition(const Vector2& position) {
    m_listenerPosition = position;
}

void AudioSystem::setListenerPosition(float x, float y) {
    m_listenerPosition = {x, y};
}
