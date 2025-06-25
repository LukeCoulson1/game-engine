#pragma once

#include <string>
#include <unordered_map>
#include <memory>

// Configuration manager for storing and loading editor settings
class ConfigManager {
public:
    static ConfigManager& getInstance();
    
    // Configuration management
    bool loadConfig(const std::string& configFile = "editor_config.json");
    bool saveConfig(const std::string& configFile = "editor_config.json") const;
    
    // Window settings
    void setWindowSize(int width, int height);
    void getWindowSize(int& width, int& height) const;
    void setWindowPosition(int x, int y);
    void getWindowPosition(int& x, int& y) const;
    void setWindowMaximized(bool maximized);
    bool getWindowMaximized() const;
      // UI panel settings
    void setPanelVisible(const std::string& panelName, bool visible);
    bool getPanelVisible(const std::string& panelName) const;
    
    // Asset settings
    void setAssetFolder(const std::string& folder);
    std::string getAssetFolder() const;
    
    // Scene window settings
    void setSceneWindowSize(int width, int height);
    void getSceneWindowSize(int& width, int& height) const;
    
    // Per-scene window settings
    void setSceneWindowSize(const std::string& sceneName, int width, int height);
    void getSceneWindowSize(const std::string& sceneName, int& width, int& height) const;
    bool hasSceneWindowSize(const std::string& sceneName) const;
    
    // Generic settings
    void setInt(const std::string& key, int value);
    int getInt(const std::string& key, int defaultValue = 0) const;
    void setFloat(const std::string& key, float value);
    float getFloat(const std::string& key, float defaultValue = 0.0f) const;
    void setString(const std::string& key, const std::string& value);
    std::string getString(const std::string& key, const std::string& defaultValue = "") const;
    void setBool(const std::string& key, bool value);
    bool getBool(const std::string& key, bool defaultValue = false) const;
    
    // Reset to defaults
    void resetToDefaults();
    
private:
    ConfigManager() = default;
    ~ConfigManager() = default;
    ConfigManager(const ConfigManager&) = delete;
    ConfigManager& operator=(const ConfigManager&) = delete;
    
    // Storage for configuration values
    std::unordered_map<std::string, int> m_intValues;
    std::unordered_map<std::string, float> m_floatValues;
    std::unordered_map<std::string, std::string> m_stringValues;
    std::unordered_map<std::string, bool> m_boolValues;
    
    // Default values
    void setDefaults();
};
