#include "Engine.h"
#include "graphics/Renderer.h"
#include "input/InputManager.h"
#include "audio/AudioManager.h"
#include "utils/ResourceManager.h"
#include "scene/Scene.h"

#include <SDL2/SDL.h>
#include <chrono>
#include <iostream>

Engine& Engine::getInstance() {
    static Engine instance;
    return instance;
}

bool Engine::initialize(const std::string& title, int width, int height) {
    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
        std::cerr << "SDL could not initialize! SDL Error: " << SDL_GetError() << std::endl;
        return false;
    }
    
    m_windowWidth = width;
    m_windowHeight = height;
    
    // Initialize core systems
    m_renderer = std::make_unique<Renderer>();
    if (!m_renderer->initialize(title, width, height)) {
        std::cerr << "Failed to initialize renderer!" << std::endl;
        return false;
    }
    
    m_inputManager = std::make_unique<InputManager>();
    m_audioManager = std::make_unique<AudioManager>();
    m_resourceManager = std::make_unique<ResourceManager>();
    
    if (!m_audioManager->initialize()) {
        std::cerr << "Failed to initialize audio manager!" << std::endl;
        return false;
    }
    
    m_running = true;
    return true;
}

void Engine::run() {
    auto lastTime = std::chrono::high_resolution_clock::now();
    
    while (m_running) {
        auto currentTime = std::chrono::high_resolution_clock::now();
        m_deltaTime = std::chrono::duration<float>(currentTime - lastTime).count();
        lastTime = currentTime;
        
        // Handle events
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                m_running = false;
            }
            
            if (m_inputManager) {
                m_inputManager->handleEvent(event);
            }
        }
        
        // Update
        if (m_inputManager) {
            m_inputManager->update();
        }
        
        update(m_deltaTime);
        
        // Render
        render();
        
        // Cap framerate (roughly 60 FPS)
        SDL_Delay(16);
    }
}

void Engine::update(float deltaTime) {
    if (m_activeScene) {
        m_activeScene->update(deltaTime);
    }
}

void Engine::render() {
    if (m_renderer) {
        m_renderer->clear();
        
        if (m_activeScene) {
            m_activeScene->render(m_renderer.get());
        }
        
        m_renderer->present();
    }
}

void Engine::setActiveScene(std::shared_ptr<Scene> scene) {
    m_activeScene = scene;
}

void Engine::shutdown() {
    m_activeScene.reset();
    m_renderer.reset();
    m_inputManager.reset();
    m_audioManager.reset();
    m_resourceManager.reset();
    
    SDL_Quit();
}
