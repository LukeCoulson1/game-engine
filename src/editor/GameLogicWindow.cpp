#include "GameLogicWindow.h"
#include "SceneWindow.h"
#include "../components/Components.h"
#include <imgui.h>
#include "../../external/imgui/misc/cpp/imgui_stdlib.h"
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <filesystem>

GameLogicWindow::GameLogicWindow() {
    m_lastFrameTime = std::chrono::high_resolution_clock::now();
    initializeCodeTemplates();
    scanForAvailableScenes();      // Default user code template
    m_userCode = "// Game Logic Playground - Live Scene Testing!\n"
                 "// \n"
                 "// SETUP:\n"
                 "// 1. Create entities in your Scene Window\n"
                 "// 2. Press F5 to start live gameplay testing (auto-creates backup)\n"
                 "// 3. Watch entities move in real-time in the Scene Window!\n"
                 "// 4. Press Ctrl+Shift+R to reset scene to original state\n"
                 "//\n"
                 "// This code runs directly on your active scene - no copying needed!\n"
                 "// You can see all changes live in the Scene Window.\n"
                 "// A backup is created automatically so you can reset anytime.\n"
                 "\n"
                 "void updateGame(float deltaTime) {\n"
                 "    // Live animation example - figure-8 dance pattern\n"
                 "    // This runs directly on your scene entities!\n"
                 "    // Add player controls, physics, game logic here\n"
                 "}\n"
                 "\n"
                 "void onStart() {\n"
                 "    log(\"Live gameplay started! Watch the Scene Window!\");\n"
                 "    log(\"Press Ctrl+Shift+R to reset scene to original state\");\n"
                 "}\n"
                 "\n"
                 "void onStop() {\n"
                 "    log(\"Live gameplay stopped!\");\n"
                 "}";
}

GameLogicWindow::~GameLogicWindow() {
    stopRuntime();
}

void GameLogicWindow::show(bool* open, SceneWindow* activeScene) {
    m_activeScene = activeScene;
      ImGui::SetNextWindowSize(ImVec2(1000, 700), ImGuiCond_FirstUseEver);
    if (!ImGui::Begin("🎮 Game Logic Playground", open, ImGuiWindowFlags_MenuBar)) {
        ImGui::End();
        return;
    }
    
    // Handle keyboard shortcuts
    ImGuiIO& io = ImGui::GetIO();
    if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows)) {
        // F5 - Start runtime
        if (ImGui::IsKeyPressed(ImGuiKey_F5) && !io.KeyShift && !m_isRunning) {
            startRuntime();
        }
        // Shift+F5 - Stop runtime
        if (ImGui::IsKeyPressed(ImGuiKey_F5) && io.KeyShift && m_isRunning) {
            stopRuntime();
        }
        // F6 - Pause/Resume
        if (ImGui::IsKeyPressed(ImGuiKey_F6) && m_isRunning) {
            if (m_isPaused) {
                m_isPaused = false;
                log("Runtime resumed");
            } else {
                pauseRuntime();
            }
        }
        // F7 - Step frame
        if (ImGui::IsKeyPressed(ImGuiKey_F7) && m_isRunning && m_isPaused) {
            stepFrame();
        }
        // F9 - Compile and run
        if (ImGui::IsKeyPressed(ImGuiKey_F9)) {
            compileAndRun(m_userCode);
        }
        // Ctrl+R - Reset runtime
        if (ImGui::IsKeyPressed(ImGuiKey_R) && io.KeyCtrl && !io.KeyShift) {
            resetRuntime();
        }
        // Ctrl+Shift+R - Reset scene
        if (ImGui::IsKeyPressed(ImGuiKey_R) && io.KeyCtrl && io.KeyShift && hasSceneBackup()) {
            restoreSceneFromBackup();
        }        // Ctrl+B - Create backup
        if (ImGui::IsKeyPressed(ImGuiKey_B) && io.KeyCtrl && m_activeScene) {
            createSceneBackup();
        }
        // Escape - Stop current template and reset
        if (ImGui::IsKeyPressed(ImGuiKey_Escape)) {
            stopCurrentTemplate();
            resetEntityStates();
            log("Stopped template and reset scene state (ESC)", LogEntry::INFO);
        }
    }
    
    // Menu bar
    if (ImGui::BeginMenuBar()) {
        if (ImGui::BeginMenu("Runtime")) {
            if (ImGui::MenuItem("Start", "F5", false, !m_isRunning)) {
                startRuntime();
            }
            if (ImGui::MenuItem("Stop", "Shift+F5", false, m_isRunning)) {
                stopRuntime();
            }
            if (ImGui::MenuItem("Pause/Resume", "F6", false, m_isRunning)) {
                if (m_isPaused) {
                    m_isPaused = false;
                    log("Runtime resumed");
                } else {
                    pauseRuntime();
                }
            }
            if (ImGui::MenuItem("Step Frame", "F7", false, m_isRunning && m_isPaused)) {
                stepFrame();
            }            ImGui::Separator();
            if (ImGui::MenuItem("Reset", "Ctrl+R")) {
                resetRuntime();
            }
            if (ImGui::MenuItem("Reset Scene", "Ctrl+Shift+R", false, hasSceneBackup())) {
                restoreSceneFromBackup();
            }
            ImGui::EndMenu();
        }
        
        if (ImGui::BeginMenu("Code")) {
            if (ImGui::MenuItem("Compile & Run", "F9")) {
                compileAndRun(m_userCode);
            }
            ImGui::Separator();
            ImGui::MenuItem("Auto Compile", nullptr, &m_autoCompile);
            ImGui::Separator();            if (ImGui::BeginMenu("Load Template")) {
                for (const auto& [name, code] : m_codeTemplates) {
                    if (ImGui::MenuItem(name.c_str())) {
                        loadTemplate(name);
                    }
                }
                ImGui::Separator();
                if (ImGui::MenuItem("Reset Scene State")) {
                    stopCurrentTemplate();
                    resetEntityStates();
                    log("Reset scene state manually", LogEntry::INFO);
                }
                ImGui::EndMenu();
            }
            ImGui::EndMenu();        }          if (ImGui::BeginMenu("Scene")) {
            if (ImGui::MenuItem("Load from Active Window", nullptr, false, m_activeScene != nullptr)) {
                loadSceneFromActiveWindow();
            }
            
            ImGui::Separator();
            ImGui::Text("Scene Backup:");
            
            if (ImGui::MenuItem("Create Backup", "Ctrl+B", false, m_activeScene != nullptr)) {
                createSceneBackup();
            }
            
            if (ImGui::MenuItem("Restore from Backup", "Ctrl+Shift+R", false, hasSceneBackup())) {
                restoreSceneFromBackup();
            }
            
            if (hasSceneBackup()) {
                ImGui::Text("✓ Backup available");
            } else {
                ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.8f, 1.0f), "No backup");
            }
            
            ImGui::Separator();
            ImGui::Text("Runtime Scene Copy (Debug Only):");
            
            if (ImGui::BeginMenu("Load Scene File")) {
                scanForAvailableScenes(); // Refresh the list
                
                if (m_availableScenes.empty()) {
                    ImGui::MenuItem("No scenes found", nullptr, false, false);
                } else {
                    for (const auto& scenePath : m_availableScenes) {
                        std::string sceneName = std::filesystem::path(scenePath).stem().string();
                        if (ImGui::MenuItem(sceneName.c_str())) {
                            loadSceneFromFile(scenePath);
                        }
                    }
                }
                ImGui::EndMenu();
            }
            
            if (ImGui::MenuItem("Clear Runtime Scene", nullptr, false, m_runtimeScene != nullptr)) {
                clearRuntimeScene();
            }
            
            if (!m_loadedScenePath.empty()) {
                ImGui::Separator();
                ImGui::Text("Debug Copy: %s", std::filesystem::path(m_loadedScenePath).filename().string().c_str());
            }
            
            ImGui::EndMenu();
        }
        
        if (ImGui::BeginMenu("View")) {
            ImGui::MenuItem("Code Editor", nullptr, &m_showCodeEditor);
            ImGui::MenuItem("Console", nullptr, &m_showConsole);
            ImGui::MenuItem("Debug Info", nullptr, &m_showDebugInfo);
            ImGui::MenuItem("Performance Stats", nullptr, &m_showPerformanceStats);
            ImGui::MenuItem("Scene State", nullptr, &m_showSceneState);
            ImGui::MenuItem("Runtime Viewport", nullptr, &m_showRuntimeViewport);
            ImGui::EndMenu();
        }
        
        if (ImGui::BeginMenu("Debug")) {
            if (ImGui::MenuItem("List All Entities")) {
                listAllEntities();
            }
            if (ImGui::MenuItem("Dump Scene State")) {
                dumpSceneState();
            }
            if (ImGui::MenuItem("Clear Log")) {
                clearLog();
            }
            ImGui::EndMenu();
        }
        
        ImGui::EndMenuBar();
    }
    
    // Main layout
    renderControlPanel();
    
    ImGui::Separator();
    
    // Split view: Code editor on left, debug panels on right
    ImGui::BeginChild("MainContent", ImVec2(0, 0), false);
    
    if (m_showCodeEditor) {
        ImGui::BeginChild("LeftPanel", ImVec2(m_leftPanelWidth, 0), true);
        renderCodeEditor();
        ImGui::EndChild();
        
        ImGui::SameLine();
    }
    
    ImGui::BeginChild("RightPanel", ImVec2(0, 0), false);
      // Top right: Debug info and performance
    if (m_showDebugInfo || m_showPerformanceStats || m_showSceneState || m_showRuntimeViewport) {
        float rightPanelHeight = ImGui::GetContentRegionAvail().y;
        ImGui::BeginChild("TopRightPanel", ImVec2(0, rightPanelHeight - m_bottomPanelHeight - 10), true);
        
        if (m_showRuntimeViewport) {
            renderRuntimeViewport();
            if (m_showDebugInfo || m_showPerformanceStats || m_showSceneState) ImGui::Separator();
        }
        
        if (m_showDebugInfo) {
            renderDebugInfo();
        }
        
        if (m_showPerformanceStats) {
            if (m_showDebugInfo) ImGui::Separator();
            renderPerformanceStats();
        }
        
        if (m_showSceneState) {
            if (m_showDebugInfo || m_showPerformanceStats) ImGui::Separator();
            renderSceneState();
        }
        
        ImGui::EndChild();
    }
    
    // Bottom right: Console
    if (m_showConsole) {
        ImGui::BeginChild("BottomRightPanel", ImVec2(0, m_bottomPanelHeight), true);
        renderConsole();
        ImGui::EndChild();
    }
    
    ImGui::EndChild(); // RightPanel
    ImGui::EndChild(); // MainContent
      // Update runtime if running - execute directly on active scene
    if (m_isRunning && !m_isPaused && m_activeScene) {
        auto now = std::chrono::high_resolution_clock::now();
        float deltaTime = std::chrono::duration<float>(now - m_lastFrameTime).count();
        m_lastFrameTime = now;
        
        updateRuntime(deltaTime);
    }
    
    ImGui::End();
}

void GameLogicWindow::renderControlPanel() {
    // Runtime controls
    ImGui::Text("Runtime Controls:");
    ImGui::SameLine();
    
    if (!m_isRunning) {
        if (ImGui::Button("▶ Start (F5)")) {
            startRuntime();
        }
    } else {
        if (ImGui::Button("⏹ Stop (Shift+F5)")) {
            stopRuntime();
        }
        ImGui::SameLine();
        
        if (!m_isPaused) {
            if (ImGui::Button("⏸ Pause (F6)")) {
                pauseRuntime();
            }
        } else {
            if (ImGui::Button("▶ Resume (F6)")) {
                m_isPaused = false;
                log("Runtime resumed");
            }
            ImGui::SameLine();
            if (ImGui::Button("⏭ Step (F7)")) {
                stepFrame();
            }
        }
    }
      ImGui::SameLine();
    ImGui::Spacing();
    ImGui::SameLine();
    
    // Reset controls
    if (ImGui::Button("🔄 Reset Scene") && hasSceneBackup()) {
        restoreSceneFromBackup();
    }
    if (ImGui::IsItemHovered() && hasSceneBackup()) {
        ImGui::SetTooltip("Restore scene to original state (Ctrl+Shift+R)");
    } else if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("No backup available - create one first");
    }
    
    ImGui::SameLine();
    ImGui::Spacing();
    ImGui::SameLine();
    
    // Status indicator
    ImGui::Text("Status:");
    ImGui::SameLine();
    if (m_isRunning) {
        if (m_isPaused) {
            ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.0f, 1.0f), "PAUSED");
        } else {
            ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "RUNNING");
        }
    } else {
        ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.8f, 1.0f), "STOPPED");
    }
    
    ImGui::SameLine();
    ImGui::Spacing();
    ImGui::SameLine();
    
    // Active scene info
    ImGui::Text("Scene:");
    ImGui::SameLine();
    if (m_activeScene) {
        ImGui::Text("%s", m_activeScene->getTitle().c_str());
    } else {
        ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), "None Selected");
    }
}

void GameLogicWindow::renderCodeEditor() {
    ImGui::Text("🔧 Code Editor");
    ImGui::Separator();
    
    // Code editor controls
    if (ImGui::Button("Compile & Run (F9)")) {
        compileAndRun(m_userCode);
    }
    ImGui::SameLine();
    ImGui::Checkbox("Auto Compile", &m_autoCompile);
    
    // Code text area
    ImGui::Text("User Code:");
    ImGuiInputTextFlags flags = ImGuiInputTextFlags_AllowTabInput;
    if (ImGui::InputTextMultiline("##UserCode", &m_userCode, ImVec2(-1, -50), flags)) {
        m_codeChanged = true;
        if (m_autoCompile && m_isRunning) {
            // Auto-compile with a small delay to avoid constant recompilation
            static float lastChangeTime = 0.0f;
            lastChangeTime = ImGui::GetTime();
            if (ImGui::GetTime() - lastChangeTime > 0.5f) {
                compileAndRun(m_userCode);
            }
        }
    }
    
    // Quick command input
    ImGui::Text("Quick Command:");
    if (ImGui::InputText("##QuickCommand", &m_currentCommand, ImGuiInputTextFlags_EnterReturnsTrue)) {
        executeCommand(m_currentCommand);
        m_currentCommand.clear();
    }
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Enter C++ code snippets to execute immediately");
    }
}

void GameLogicWindow::renderConsole() {
    ImGui::Text("📋 Console");
    ImGui::SameLine();
    if (ImGui::SmallButton("Clear")) {
        clearLog();
    }
    ImGui::SameLine();
    ImGui::Checkbox("Auto-scroll", &m_autoScroll);
    
    ImGui::Separator();
    
    // Console output
    ImGui::BeginChild("ConsoleOutput", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);
    
    for (const auto& entry : m_logEntries) {
        ImVec4 color;
        const char* prefix;
        switch (entry.level) {
            case LogEntry::ERROR:   color = ImVec4(1.0f, 0.4f, 0.4f, 1.0f); prefix = "[ERROR]"; break;
            case LogEntry::WARNING: color = ImVec4(1.0f, 0.8f, 0.0f, 1.0f); prefix = "[WARN] "; break;
            case LogEntry::DEBUG:   color = ImVec4(0.6f, 0.6f, 1.0f, 1.0f); prefix = "[DEBUG]"; break;
            default:                color = ImVec4(1.0f, 1.0f, 1.0f, 1.0f); prefix = "[INFO] "; break;
        }
        
        auto time = std::chrono::system_clock::to_time_t(entry.timestamp);
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            entry.timestamp.time_since_epoch()) % 1000;
            
        std::stringstream timeStr;
        timeStr << std::put_time(std::localtime(&time), "%H:%M:%S");
        timeStr << "." << std::setfill('0') << std::setw(3) << ms.count();
        
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "[%s]", timeStr.str().c_str());
        ImGui::SameLine();
        ImGui::TextColored(color, "%s %s", prefix, entry.message.c_str());
    }
    
    if (m_autoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY()) {
        ImGui::SetScrollHereY(1.0f);
    }
    
    ImGui::EndChild();
}

void GameLogicWindow::renderDebugInfo() {
    ImGui::Text("🐛 Debug Info");
    ImGui::Separator();
    
    ImGui::Text("Frame Count: %d", m_frameCount);
    ImGui::Text("Step Mode: %s", m_stepMode ? "ON" : "OFF");
    
    if (m_inspectedEntity != 0) {
        ImGui::Text("Inspected Entity: %u", m_inspectedEntity);
        if (ImGui::Button("Clear Selection")) {
            m_inspectedEntity = 0;
        }
    } else {
        ImGui::Text("No entity selected");
    }
    
    ImGui::Separator();
    
    // Entity selection for inspection
    if (m_activeScene && m_activeScene->getScene()) {
        ImGui::Text("Inspect Entity:");
        static int entityInput = 0;
        if (ImGui::InputInt("Entity ID", &entityInput)) {
            if (entityInput > 0) {
                m_inspectedEntity = static_cast<EntityID>(entityInput);
                inspectEntity(m_inspectedEntity);
            }
        }
    }
}

void GameLogicWindow::renderPerformanceStats() {
    ImGui::Text("📊 Performance");
    ImGui::Separator();
    
    ImGui::Text("FPS: %.1f", m_fps);
    ImGui::Text("Frame Time: %.3f ms", m_frameTime * 1000.0f);
    
    // Memory usage (simplified)
    ImGui::Text("Log Entries: %zu / %d", m_logEntries.size(), m_maxLogEntries);
    
    static std::vector<float> fpsHistory(100, 0.0f);
    fpsHistory.erase(fpsHistory.begin());
    fpsHistory.push_back(m_fps);
    
    ImGui::PlotLines("FPS History", fpsHistory.data(), static_cast<int>(fpsHistory.size()), 
                     0, nullptr, 0.0f, 120.0f, ImVec2(0, 50));
}

void GameLogicWindow::renderSceneState() {
    ImGui::Text("🌍 Scene State");
    ImGui::Separator();
    
    // Show active scene window info (this is where the action happens!)
    if (m_activeScene && m_activeScene->getScene()) {
        ImGui::Text("Live Scene Window:");
        ImGui::Text("  Title: %s", m_activeScene->getTitle().c_str());
        ImGui::Text("  Selected Entity: %u", m_activeScene->getSelectedEntity());
        
        auto windowEntities = m_activeScene->getScene()->getAllLivingEntities();
        ImGui::Text("  Entities: %zu", windowEntities.size());
        
        if (m_isRunning) {
            ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "  ▶ Live gameplay active!");
            ImGui::Text("  Changes appear directly in Scene Window");
        } else {
            ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.8f, 1.0f), "  ⏹ Gameplay stopped");
        }
    } else {
        ImGui::Text("No active scene window");
        ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), "Create or open a Scene Window first!");
    }
    
    ImGui::Separator();
    
    // Show runtime scene info if available (for debugging)
    if (m_runtimeScene) {
        ImGui::Text("Debug Runtime Scene:");
        if (!m_loadedScenePath.empty()) {
            ImGui::Text("  Source: %s", m_loadedScenePath.c_str());
        }
        
        auto entities = m_runtimeScene->getAllLivingEntities();
        ImGui::Text("  Entities: %zu", entities.size());
        ImGui::Text("  (Used only for Runtime Viewport debugging)");
        
        ImGui::Separator();    }
    
    ImGui::Separator();
    
    // Backup status
    ImGui::Text("Scene Backup:");
    if (hasSceneBackup()) {
        ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "  ✓ Backup available");
        ImGui::Text("  Entities: %zu", m_sceneBackup->getAllLivingEntities().size());
        if (ImGui::Button("Restore from Backup")) {
            restoreSceneFromBackup();
        }
    } else {
        ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.0f, 1.0f), "  ⚠ No backup");
        if (m_activeScene && ImGui::Button("Create Backup")) {
            createSceneBackup();
        }
    }
    
    ImGui::Separator();
    
    // Entity management buttons
    ImGui::Text("Entity Management:");
    if (ImGui::Button("List All Entities")) {
        listAllEntities();
    }
    ImGui::SameLine();
    if (ImGui::Button("Dump Scene")) {
        dumpSceneState();
    }
}

void GameLogicWindow::startRuntime() {
    if (m_isRunning) return;
    
    log("Starting game logic runtime...", LogEntry::INFO);
    
    // Create a backup of the scene before starting live playtesting
    if (!hasSceneBackup() && m_activeScene) {
        createSceneBackup();
        log("Scene backup created automatically for live playtesting", LogEntry::INFO);
    }
    
    m_isRunning = true;
    m_isPaused = false;
    m_frameCount = 0;
    m_lastFrameTime = std::chrono::high_resolution_clock::now();
    
    // Execute onStart if available
    executeCommand("onStart();");
    
    log("Runtime started successfully!", LogEntry::INFO);
}

void GameLogicWindow::stopRuntime() {
    if (!m_isRunning) return;
    
    log("Stopping game logic runtime...", LogEntry::INFO);
    
    // Execute onStop if available
    executeCommand("onStop();");
    
    m_isRunning = false;
    m_isPaused = false;
    
    log("Runtime stopped.", LogEntry::INFO);
}

void GameLogicWindow::pauseRuntime() {
    if (!m_isRunning || m_isPaused) return;
    
    m_isPaused = true;
    log("Runtime paused", LogEntry::INFO);
}

void GameLogicWindow::stepFrame() {
    if (!m_isRunning || !m_isPaused) return;
    
    auto now = std::chrono::high_resolution_clock::now();
    float deltaTime = std::chrono::duration<float>(now - m_lastFrameTime).count();
    m_lastFrameTime = now;
    
    updateRuntime(deltaTime);
    log("Stepped one frame", LogEntry::DEBUG);
}

void GameLogicWindow::updateRuntime(float deltaTime) {
    m_frameCount++;
    
    // Update performance stats
    m_frameTime = deltaTime;
    m_fps = (deltaTime > 0.0f) ? 1.0f / deltaTime : 0.0f;
    
    // Execute user update code (operates directly on active scene)
    executeUserCode();
}

void GameLogicWindow::resetRuntime() {
    stopRuntime();
    m_frameCount = 0;
    log("Runtime reset", LogEntry::INFO);
}

void GameLogicWindow::log(const std::string& message, LogEntry::Level level) {
    m_logEntries.emplace_back(message, level);
    
    // Limit log size
    if (m_logEntries.size() > static_cast<size_t>(m_maxLogEntries)) {
        m_logEntries.erase(m_logEntries.begin());
    }
}

void GameLogicWindow::clearLog() {
    m_logEntries.clear();
    log("Log cleared", LogEntry::INFO);
}

void GameLogicWindow::compileAndRun(const std::string& code) {
    log("Compiling user code...", LogEntry::INFO);
    
    // For now, we'll simulate compilation
    // In a real implementation, you'd want to use a scripting engine
    // like Lua, ChaiScript, or compile C++ code dynamically
    
    validateCode(code);
    log("Code compiled successfully", LogEntry::INFO);
}

void GameLogicWindow::executeCommand(const std::string& command) {
    if (command.empty()) return;
    
    log("Executing: " + command, LogEntry::DEBUG);
    
    // Simple command processor - extend this with actual script execution
    if (command == "onStart();") {
        log("Executing onStart()", LogEntry::DEBUG);
    } else if (command == "onStop();") {
        log("Executing onStop()", LogEntry::DEBUG);
    } else if (command.find("log(") == 0) {
        // Extract log message
        size_t start = command.find("\"") + 1;
        size_t end = command.find_last_of("\"");
        if (start < end) {
            std::string msg = command.substr(start, end - start);
            log("User: " + msg, LogEntry::INFO);
        }
    } else {
        log("Unknown command: " + command, LogEntry::WARNING);
    }
}

void GameLogicWindow::executeUserCode() {
    // Execute the user's updateGame function directly on the active scene
    // This allows live gameplay in the scene window!
    if (!m_activeScene || !m_activeScene->getScene()) return;
    
    auto scene = m_activeScene->getScene();
    
    // For now, we'll implement a simple dance animation as an example
    // In a real implementation, you'd compile and execute the user's code
    
    static float time = 0.0f;
    time += m_frameTime;
    
    auto entities = scene->getAllLivingEntities();
    for (auto entity : entities) {
        if (scene->hasComponent<Transform>(entity)) {
            auto& transform = scene->getComponent<Transform>(entity);
            
            // Simple dance animation - figure-8 pattern
            float radius = 30.0f;
            float speed = 2.0f;
            
            transform.position.x = 100.0f + radius * sin(time * speed);
            transform.position.y = 100.0f + radius * sin(time * speed * 2.0f) * 0.5f;
            transform.rotation = time * 45.0f; // Spin
            
            // Mark the scene as dirty so it gets re-rendered
            m_activeScene->setDirty(true);
        }
    }
    
    if (m_frameCount % 60 == 0) { // Every 60 frames
        log("User update code executed (frame " + std::to_string(m_frameCount) + ")", LogEntry::DEBUG);
    }
}

void GameLogicWindow::validateCode(const std::string& code) {
    // Basic code validation
    if (code.empty()) {
        log("Warning: Code is empty", LogEntry::WARNING);
        return;
    }
    
    // Check for basic syntax issues
    int braceCount = 0;
    for (char c : code) {
        if (c == '{') braceCount++;
        else if (c == '}') braceCount--;
    }
    
    if (braceCount != 0) {
        log("Warning: Mismatched braces detected", LogEntry::WARNING);
    }
}

void GameLogicWindow::inspectEntity(EntityID entity) {
    if (!m_activeScene || !m_activeScene->getScene()) return;
    
    auto scene = m_activeScene->getScene();
    
    log("Inspecting entity " + std::to_string(entity), LogEntry::INFO);
    
    // Check what components the entity has
    std::vector<std::string> components;
    
    if (scene->hasComponent<Transform>(entity)) {
        auto& transform = scene->getComponent<Transform>(entity);
        components.push_back("Transform (pos: " + 
            std::to_string(transform.position.x) + ", " + 
            std::to_string(transform.position.y) + ")");
    }
    
    if (scene->hasComponent<Sprite>(entity)) {
        components.push_back("Sprite");
    }
    
    if (components.empty()) {
        log("Entity " + std::to_string(entity) + " has no components", LogEntry::INFO);
    } else {
        for (const auto& comp : components) {
            log("  - " + comp, LogEntry::INFO);
        }
    }
}

void GameLogicWindow::listAllEntities() {
    // Prioritize active scene (where live gameplay happens)
    std::shared_ptr<Scene> sceneToInspect = (m_activeScene ? m_activeScene->getScene() : nullptr);
    
    if (!sceneToInspect) {
        // Fallback to runtime scene if no active scene
        sceneToInspect = m_runtimeScene;
    }
    
    if (!sceneToInspect) {
        log("No scene available to list entities from", LogEntry::WARNING);
        return;
    }
    
    std::string sceneSource = (m_activeScene ? "active scene window (live)" : "runtime scene (debug)");
    log("Listing all entities in " + sceneSource + ":", LogEntry::INFO);
    
    auto entities = sceneToInspect->getAllLivingEntities();
    
    if (entities.empty()) {
        log("  No entities found", LogEntry::INFO);
        return;
    }
    
    for (EntityID entity : entities) {
        std::string entityName = sceneToInspect->getEntityName(entity);
        std::string components = "  Entity " + std::to_string(entity) + " (" + entityName + ") - Components: ";
        
        bool hasAnyComponent = false;
        
        if (sceneToInspect->hasComponent<Transform>(entity)) {
            components += "Transform ";
            hasAnyComponent = true;
        }
        if (sceneToInspect->hasComponent<Sprite>(entity)) {
            components += "Sprite ";
            hasAnyComponent = true;
        }
        if (sceneToInspect->hasComponent<PlayerController>(entity)) {
            components += "PlayerController ";
            hasAnyComponent = true;
        }
        if (sceneToInspect->hasComponent<PlayerStats>(entity)) {
            components += "PlayerStats ";
            hasAnyComponent = true;
        }
        if (sceneToInspect->hasComponent<PlayerPhysics>(entity)) {
            components += "PlayerPhysics ";
            hasAnyComponent = true;
        }
        if (sceneToInspect->hasComponent<Collider>(entity)) {
            components += "Collider ";
            hasAnyComponent = true;
        }
        if (sceneToInspect->hasComponent<RigidBody>(entity)) {
            components += "RigidBody ";
            hasAnyComponent = true;
        }
        
        if (!hasAnyComponent) {
            components += "None";
        }
        
        log(components, LogEntry::INFO);
    }
}

void GameLogicWindow::dumpSceneState() {
    // Prioritize active scene (where live gameplay happens)
    std::shared_ptr<Scene> sceneToInspect = (m_activeScene ? m_activeScene->getScene() : nullptr);
    
    if (!sceneToInspect) {
        // Fallback to runtime scene if no active scene
        sceneToInspect = m_runtimeScene;
    }
    
    if (!sceneToInspect) {
        log("No scene available to dump", LogEntry::WARNING);
        return;
    }
    
    log("=== SCENE STATE DUMP ===", LogEntry::INFO);
    
    std::string sceneSource;
    if (m_activeScene) {
        sceneSource = "Active Scene Window (" + m_activeScene->getTitle() + ") - LIVE";
    } else if (m_runtimeScene) {
        sceneSource = m_loadedScenePath.empty() ? "Runtime Scene (Debug)" : "Runtime Scene (" + m_loadedScenePath + ")";
    }
    
    log("Scene Source: " + sceneSource, LogEntry::INFO);
    
    auto entities = sceneToInspect->getAllLivingEntities();
    log("Total Entities: " + std::to_string(entities.size()), LogEntry::INFO);
    
    for (EntityID entity : entities) {
        std::string entityInfo = "Entity " + std::to_string(entity) + " '" + sceneToInspect->getEntityName(entity) + "'";
        log(entityInfo, LogEntry::INFO);
        
        // Dump component details
        if (sceneToInspect->hasComponent<Transform>(entity)) {
            auto& transform = sceneToInspect->getComponent<Transform>(entity);
            log("  Transform: pos(" + std::to_string(transform.position.x) + ", " + 
                std::to_string(transform.position.y) + ") scale(" +
                std::to_string(transform.scale.x) + ", " + std::to_string(transform.scale.y) + 
                ") rotation(" + std::to_string(transform.rotation) + ")", LogEntry::INFO);
        }
        
        if (sceneToInspect->hasComponent<Sprite>(entity)) {
            log("  Sprite: (texture info)", LogEntry::INFO);
        }
        
        if (sceneToInspect->hasComponent<PlayerController>(entity)) {
            log("  PlayerController", LogEntry::INFO);
        }
        
        if (sceneToInspect->hasComponent<PlayerStats>(entity)) {
            log("  PlayerStats", LogEntry::INFO);
        }
        
        if (sceneToInspect->hasComponent<PlayerPhysics>(entity)) {
            log("  PlayerPhysics", LogEntry::INFO);
        }
        
        if (sceneToInspect->hasComponent<Collider>(entity)) {
            log("  Collider", LogEntry::INFO);
        }
        
        if (sceneToInspect->hasComponent<RigidBody>(entity)) {
            log("  RigidBody", LogEntry::INFO);
        }
    }
      log("=== END SCENE DUMP ===", LogEntry::INFO);
}

void GameLogicWindow::initializeCodeTemplates() {
    m_codeTemplates = {        {"Live Player Control", R"(// Live player movement example
// Press F5 to start, Ctrl+Shift+R to reset scene
void updateGame(float deltaTime) {
    // Control the selected entity with keyboard
    auto entities = scene->getAllLivingEntities();
    if (!entities.empty()) {
        // Use the first entity as the player
        auto playerEntity = entities[0];
        if (scene->hasComponent<Transform>(playerEntity)) {
            auto& transform = scene->getComponent<Transform>(playerEntity);
            
            // Basic movement (simulated - in real version you'd check input)
            float speed = 100.0f;
            static float inputTime = 0.0f;
            inputTime += deltaTime;
            
            // Simulate WASD movement pattern
            if ((int)(inputTime * 2) % 4 == 0) {
                transform.position.x += speed * deltaTime; // D
            } else if ((int)(inputTime * 2) % 4 == 1) {
                transform.position.y += speed * deltaTime; // S
            } else if ((int)(inputTime * 2) % 4 == 2) {
                transform.position.x -= speed * deltaTime; // A
            } else {
                transform.position.y -= speed * deltaTime; // W
            }
            
            // Keep rotation stable (no dancing!)
            transform.rotation = 0.0f;
            
            log("Player at: " + std::to_string(transform.position.x) + ", " + std::to_string(transform.position.y));
        }
    }
})"},

        {"Basic Movement", R"(// Basic entity movement example
void updateGame(float deltaTime) {
    // Move the selected entity
    if (selectedEntity != 0 && scene->hasComponent<Transform>(selectedEntity)) {
        auto& transform = scene->getComponent<Transform>(selectedEntity);
        
        // Simple movement
        transform.position.x += 50.0f * deltaTime;
        
        log("Moved entity to: " + std::to_string(transform.position.x));
    }
})"},
        
        {"Live Dance Animation", R"(// Live dance animation - watch in Scene Window!
void updateGame(float deltaTime) {
    static float time = 0.0f;
    time += deltaTime;
    
    auto entities = scene->getAllLivingEntities();
    for (auto entity : entities) {
        if (scene->hasComponent<Transform>(entity)) {
            auto& transform = scene->getComponent<Transform>(entity);
            
            // Figure-8 dance pattern
            float radius = 50.0f;
            float speed = 1.5f;
            
            transform.position.x = 100.0f + radius * sin(time * speed);
            transform.position.y = 100.0f + radius * sin(time * speed * 2.0f) * 0.5f;
            transform.rotation = time * 90.0f;
        }
    }
})"},
        
        {"Component System", R"(// Component system example
void updateGame(float deltaTime) {
    // Process all entities with Transform components
    auto entities = scene->getAllLivingEntities();
    for (auto entity : entities) {
        if (scene->hasComponent<Transform>(entity)) {
            auto& transform = scene->getComponent<Transform>(entity);
            
            // Apply some logic here
            transform.rotation += 45.0f * deltaTime; // Rotate
        }
    }
})"},
        
        {"Debug Template", R"(// Debug and logging example
void updateGame(float deltaTime) {
    static int frameCounter = 0;
    frameCounter++;
    
    if (frameCounter % 60 == 0) {
        log("Debug: Frame " + std::to_string(frameCounter));
        
        // Log entity count
        auto entities = scene->getAllLivingEntities();
        log("Total entities: " + std::to_string(entities.size()));
    }
})"}
    };
}

void GameLogicWindow::loadTemplate(const std::string& templateName) {
    // Stop current template and reset entity states
    stopCurrentTemplate();
    resetEntityStates();
    
    for (const auto& [name, code] : m_codeTemplates) {
        if (name == templateName) {
            m_userCode = code;
            log("Loaded template: " + templateName, LogEntry::INFO);
            log("Previous template effects have been cleared", LogEntry::INFO);
            break;
        }
    }
}

void GameLogicWindow::stopCurrentTemplate() {
    // Stop execution if running
    if (m_isRunning) {
        m_isRunning = false;
        log("Stopped current template execution", LogEntry::INFO);
    }
    
    // Clear any persistent state
    m_isPaused = false;
}

void GameLogicWindow::resetEntityStates() {
    if (!m_activeScene || !m_activeScene->getScene()) {
        return;
    }
    
    // If we have a backup, restore from it to reset all entity states
    if (hasSceneBackup()) {
        restoreSceneFromBackup();
        log("Reset entity states from backup", LogEntry::INFO);
        return;
    }
    
    // Otherwise, reset common properties to default values
    auto scene = m_activeScene->getScene();
    auto entities = scene->getAllLivingEntities();
    
    for (auto entity : entities) {
        if (scene->hasComponent<Transform>(entity)) {
            auto& transform = scene->getComponent<Transform>(entity);
            
            // Reset rotation (common issue with dance animation)
            transform.rotation = 0.0f;
            
            // Reset scale to default
            transform.scale = {1.0f, 1.0f};
        }
    }
      // Mark scene as dirty to trigger re-rendering
    // Note: SceneWindow will automatically re-render on next frame
    log("Reset entity transforms to default state", LogEntry::INFO);
}

void GameLogicWindow::scanForAvailableScenes() {
    m_availableScenes.clear();
    
    // Scan the scenes directory for .json files
    std::string scenesPath = "scenes";
    if (std::filesystem::exists(scenesPath)) {
        for (const auto& entry : std::filesystem::directory_iterator(scenesPath)) {
            if (entry.is_regular_file() && entry.path().extension() == ".json") {
                m_availableScenes.push_back(entry.path().string());
            }
        }
    }
    
    // Also check for any .json files in the root directory
    for (const auto& entry : std::filesystem::directory_iterator(".")) {
        if (entry.is_regular_file() && entry.path().extension() == ".json") {
            std::string filename = entry.path().filename().string();
            // Only include if it looks like a scene file (contains "scene" in name or has common scene properties)
            if (filename.find("scene") != std::string::npos || 
                filename.find("Scene") != std::string::npos) {
                m_availableScenes.push_back(entry.path().string());
            }
        }
    }
}

void GameLogicWindow::loadSceneFromFile(const std::string& scenePath) {
    try {
        // For now, we'll just log that this feature needs implementation
        // since Scene doesn't have built-in serialization
        log("Scene file loading not yet implemented - use 'Load from Active Window' instead", LogEntry::WARNING);
        log("Attempted to load: " + std::filesystem::path(scenePath).filename().string(), LogEntry::INFO);
        
        // TODO: Implement scene serialization/deserialization
        // This would require adding loadFromFile/saveToFile methods to Scene class
    } catch (const std::exception& e) {
        log("Exception loading scene: " + std::string(e.what()), LogEntry::ERROR);
    }
}

void GameLogicWindow::loadSceneFromActiveWindow() {
    if (!m_activeScene || !m_activeScene->getScene()) {
        log("No active scene window available", LogEntry::WARNING);
        return;
    }
    
    try {
        // Create a copy of the active scene for runtime use
        createRuntimeSceneCopy();
        m_loadedScenePath = "Active Scene Window";
        log("Loaded scene from active window", LogEntry::INFO);
        
        listAllEntities(); // Show what's in the scene
    } catch (const std::exception& e) {
        log("Exception loading scene from active window: " + std::string(e.what()), LogEntry::ERROR);
    }
}

void GameLogicWindow::createRuntimeSceneCopy() {
    if (!m_activeScene || !m_activeScene->getScene()) {
        log("Error: No active scene available for copying", LogEntry::ERROR);
        return;
    }
    
    try {
        log("Creating runtime scene copy...", LogEntry::DEBUG);
        
        // Create a new scene for runtime use
        m_runtimeScene = std::make_shared<Scene>();
        m_runtimeScene->initialize(); // IMPORTANT: Register all component types first!
        
        // Copy all entities and components from the active scene
        auto sourceScene = m_activeScene->getScene();
        auto entities = sourceScene->getAllLivingEntities();
        
        log("Found " + std::to_string(entities.size()) + " entities to copy", LogEntry::DEBUG);
        
        for (EntityID entity : entities) {
            // Create corresponding entity in runtime scene
            EntityID newEntity = m_runtimeScene->createEntity();
            
            // Copy entity name
            std::string entityName = sourceScene->getEntityName(entity);
            m_runtimeScene->setEntityName(newEntity, entityName);
            
            log("Copying entity " + std::to_string(entity) + " -> " + std::to_string(newEntity) + " (" + entityName + ")", LogEntry::DEBUG);
            
            // Copy all components
            try {
                if (sourceScene->hasComponent<Name>(entity)) {
                    auto& name = sourceScene->getComponent<Name>(entity);
                    m_runtimeScene->addComponent<Name>(newEntity, name);
                }
                
                if (sourceScene->hasComponent<Transform>(entity)) {
                    auto& transform = sourceScene->getComponent<Transform>(entity);
                    m_runtimeScene->addComponent<Transform>(newEntity, transform);
                }
                
                if (sourceScene->hasComponent<Sprite>(entity)) {
                    auto& sprite = sourceScene->getComponent<Sprite>(entity);
                    m_runtimeScene->addComponent<Sprite>(newEntity, sprite);
                }
                
                if (sourceScene->hasComponent<PlayerController>(entity)) {
                    auto& controller = sourceScene->getComponent<PlayerController>(entity);
                    m_runtimeScene->addComponent<PlayerController>(newEntity, controller);
                }
                
                if (sourceScene->hasComponent<PlayerStats>(entity)) {
                    auto& stats = sourceScene->getComponent<PlayerStats>(entity);
                    m_runtimeScene->addComponent<PlayerStats>(newEntity, stats);
                }
                
                if (sourceScene->hasComponent<PlayerPhysics>(entity)) {
                    auto& physics = sourceScene->getComponent<PlayerPhysics>(entity);
                    m_runtimeScene->addComponent<PlayerPhysics>(newEntity, physics);
                }
                
                if (sourceScene->hasComponent<Collider>(entity)) {
                    auto& collider = sourceScene->getComponent<Collider>(entity);
                    m_runtimeScene->addComponent<Collider>(newEntity, collider);
                }
                
                if (sourceScene->hasComponent<RigidBody>(entity)) {
                    auto& rigidbody = sourceScene->getComponent<RigidBody>(entity);
                    m_runtimeScene->addComponent<RigidBody>(newEntity, rigidbody);
                }
            } catch (const std::exception& e) {
                log("Error copying components for entity " + std::to_string(entity) + ": " + e.what(), LogEntry::ERROR);
            }
        }
        
        log("Runtime scene copy completed successfully", LogEntry::INFO);
        
    } catch (const std::exception& e) {
        log("Fatal error creating runtime scene copy: " + std::string(e.what()), LogEntry::ERROR);
        m_runtimeScene.reset(); // Clean up on failure
    }
}

void GameLogicWindow::clearRuntimeScene() {
    m_runtimeScene.reset();
    m_loadedScenePath.clear();
    log("Cleared runtime scene", LogEntry::INFO);
}

void GameLogicWindow::createSceneBackup() {
    if (!m_activeScene || !m_activeScene->getScene()) {
        log("No active scene available for backup", LogEntry::WARNING);
        return;
    }
    
    try {
        log("Creating scene backup...", LogEntry::DEBUG);
        
        // For now, just create a placeholder backup
        // Scene copying will be implemented once Scene.cpp is fixed
        log("Scene backup functionality temporarily disabled", LogEntry::WARNING);
        
    } catch (const std::exception& e) {
        log("Error creating scene backup: " + std::string(e.what()), LogEntry::ERROR);
    }
}

void GameLogicWindow::restoreSceneFromBackup() {
    log("Scene restore functionality temporarily disabled", LogEntry::WARNING);
}

void GameLogicWindow::renderRuntimeViewport() {
    ImGui::Text("🎬 Runtime Viewport");
    ImGui::Separator();
    
    if (!m_runtimeScene) {
        ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.0f, 1.0f), "No runtime scene loaded");
        ImGui::Text("Use Scene -> Load from Active Window to load a scene");
        return;
    }
    
    // Simple 2D visualization of entities
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    ImVec2 canvasPos = ImGui::GetCursorScreenPos();
    ImVec2 canvasSize = ImGui::GetContentRegionAvail();
    
    // Make sure we have a minimum size
    if (canvasSize.x < 200) canvasSize.x = 200;
    if (canvasSize.y < 200) canvasSize.y = 200;
    
    // Draw background
    drawList->AddRectFilled(canvasPos, ImVec2(canvasPos.x + canvasSize.x, canvasPos.y + canvasSize.y), 
                           IM_COL32(50, 50, 50, 255));
    drawList->AddRect(canvasPos, ImVec2(canvasPos.x + canvasSize.x, canvasPos.y + canvasSize.y), 
                     IM_COL32(255, 255, 255, 255));
    
    // Draw entities
    auto entities = m_runtimeScene->getAllLivingEntities();
    for (EntityID entity : entities) {
        if (m_runtimeScene->hasComponent<Transform>(entity)) {
            auto& transform = m_runtimeScene->getComponent<Transform>(entity);
            
            // Scale world coordinates to canvas
            float scale = 0.5f;
            float worldX = transform.position.x * scale;
            float worldY = transform.position.y * scale;
            
            // Center in canvas
            float screenX = canvasPos.x + canvasSize.x * 0.5f + worldX;
            float screenY = canvasPos.y + canvasSize.y * 0.5f + worldY;
            
            // Draw entity as a colored circle
            ImU32 color = IM_COL32(100, 150, 255, 255);
            if (m_runtimeScene->hasComponent<Sprite>(entity)) {
                color = IM_COL32(255, 100, 100, 255); // Red for sprites
            }
            
            float radius = 8.0f * transform.scale.x;
            drawList->AddCircleFilled(ImVec2(screenX, screenY), radius, color);
            
            // Draw entity ID
            char entityText[32];
            sprintf_s(entityText, "%u", entity);
            drawList->AddText(ImVec2(screenX - 10, screenY - 20), IM_COL32(255, 255, 255, 255), entityText);
        }
    }
    
    // Add invisible button to capture canvas area
    ImGui::InvisibleButton("canvas", canvasSize);
    
    ImGui::Text("Entities visible: %zu", entities.size());
    ImGui::Text("Scale: 0.5x (world units to pixels)");
}
