#pragma once

#include <imgui.h>
#include <string>
#include <vector>
#include <memory>
#include <chrono>
#include "../scene/Scene.h"
#include "../core/Engine.h"
#include "../graphics/Renderer.h"

// Forward declarations
class SceneWindow;

struct LogEntry {
    std::string message;
    std::chrono::system_clock::time_point timestamp;
    enum Level { INFO, WARNING, ERROR, DEBUG } level;
    
    LogEntry(const std::string& msg, Level lvl = INFO) 
        : message(msg), level(lvl), timestamp(std::chrono::system_clock::now()) {}
};

class GameLogicWindow {
public:
    GameLogicWindow();
    ~GameLogicWindow();
    
    void show(bool* open, SceneWindow* activeScene = nullptr);
    
    // Runtime control
    void startRuntime();
    void stopRuntime();
    void pauseRuntime();
    void stepFrame();
    bool isRunning() const { return m_isRunning; }
    bool isPaused() const { return m_isPaused; }
    
    // Logging
    void log(const std::string& message, LogEntry::Level level = LogEntry::INFO);
    void clearLog();
    
    // Script compilation and execution
    void compileAndRun(const std::string& code);
    void executeCommand(const std::string& command);
    
private:
    // UI rendering methods
    void renderControlPanel();
    void renderCodeEditor();
    void renderConsole();
    void renderDebugInfo();
    void renderPerformanceStats();
    void renderSceneState();
    void renderRuntimeViewport();
    
    // Runtime management
    void updateRuntime(float deltaTime);
    void resetRuntime();
    
    // Code execution
    void executeUserCode();
    void validateCode(const std::string& code);
    
    // Template and state management
    void resetEntityStates();
    void stopCurrentTemplate();
    
    // Debug helpers
    void inspectEntity(EntityID entity);
    void listAllEntities();
    void dumpSceneState();
      // Scene management
    void loadSceneFromFile(const std::string& scenePath);
    void loadSceneFromActiveWindow();
    void createRuntimeSceneCopy();
    void clearRuntimeScene();
    
    // Scene backup/restore for live playtesting
    void createSceneBackup();
    void restoreSceneFromBackup();
    bool hasSceneBackup() const { return m_sceneBackup != nullptr; }
    
private:    // Window state
    bool m_isRunning = false;
    bool m_isPaused = false;
    bool m_showCodeEditor = true;
    bool m_showConsole = true;
    bool m_showDebugInfo = true;
    bool m_showPerformanceStats = true;
    bool m_showSceneState = true;
    bool m_showRuntimeViewport = false;    // Runtime state
    SceneWindow* m_activeScene = nullptr;
    std::shared_ptr<Scene> m_runtimeScene = nullptr;
    std::unique_ptr<Renderer> m_runtimeRenderer = nullptr;
    std::string m_loadedScenePath;
    std::vector<std::string> m_availableScenes;
    
    // Scene backup for live playtesting
    std::shared_ptr<Scene> m_sceneBackup = nullptr;
    
    // Code editor
    std::string m_userCode;
    std::string m_currentCommand;
    bool m_codeChanged = false;
    bool m_autoCompile = true;
    
    // Console and logging
    std::vector<LogEntry> m_logEntries;
    bool m_autoScroll = true;
    int m_maxLogEntries = 1000;
    
    // Performance tracking
    float m_fps = 0.0f;
    float m_frameTime = 0.0f;
    std::chrono::high_resolution_clock::time_point m_lastFrameTime;
    
    // Debug state
    EntityID m_inspectedEntity = 0;
    bool m_stepMode = false;
    int m_frameCount = 0;
    
    // UI layout
    float m_leftPanelWidth = 300.0f;
    float m_bottomPanelHeight = 200.0f;
    
    // Predefined code templates
    std::vector<std::pair<std::string, std::string>> m_codeTemplates;
      void initializeCodeTemplates();
    void loadTemplate(const std::string& templateName);
    void scanForAvailableScenes();
};
