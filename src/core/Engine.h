#pragma once

#include <memory>
#include <string>

// Forward declarations
class Scene;
class Renderer;
class InputManager;
class AudioManager;
class ResourceManager;

class Engine {
public:
    static Engine& getInstance();
    
    bool initialize(const std::string& title, int width, int height);
    void run();
    void shutdown();
    
    // Core systems
    Renderer* getRenderer() const { return m_renderer.get(); }
    InputManager* getInputManager() const { return m_inputManager.get(); }
    AudioManager* getAudioManager() const { return m_audioManager.get(); }
    ResourceManager* getResourceManager() const { return m_resourceManager.get(); }
    
    // Scene management
    void setActiveScene(std::shared_ptr<Scene> scene);
    std::shared_ptr<Scene> getActiveScene() const { return m_activeScene; }
    
    // Engine state
    bool isRunning() const { return m_running; }
    void quit() { m_running = false; }
    
    float getDeltaTime() const { return m_deltaTime; }
    int getWindowWidth() const { return m_windowWidth; }
    int getWindowHeight() const { return m_windowHeight; }

private:
    Engine() = default;
    ~Engine() = default;
    Engine(const Engine&) = delete;
    Engine& operator=(const Engine&) = delete;
    
    void update(float deltaTime);
    void render();
    
    bool m_running = false;
    int m_windowWidth = 0;
    int m_windowHeight = 0;
    float m_deltaTime = 0.0f;
    
    std::shared_ptr<Scene> m_activeScene;
    
    std::unique_ptr<Renderer> m_renderer;
    std::unique_ptr<InputManager> m_inputManager;
    std::unique_ptr<AudioManager> m_audioManager;
    std::unique_ptr<ResourceManager> m_resourceManager;
};
