#include "ConfigManager.h"
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>

// SDL window position constants
#ifndef SDL_WINDOWPOS_CENTERED
#define SDL_WINDOWPOS_CENTERED 0x2FFF0000u
#endif

using json = nlohmann::json;

ConfigManager& ConfigManager::getInstance() {
    static ConfigManager instance;
    return instance;
}

bool ConfigManager::loadConfig(const std::string& configFile) {
    try {
        std::ifstream file(configFile);
        if (!file.is_open()) {
            // File doesn't exist, use defaults
            setDefaults();
            return true;
        }
        
        json config;
        file >> config;
        file.close();
        
        // Load all sections
        if (config.contains("window")) {
            auto window = config["window"];
            if (window.contains("width")) m_intValues["window.width"] = window["width"];
            if (window.contains("height")) m_intValues["window.height"] = window["height"];
            if (window.contains("x")) m_intValues["window.x"] = window["x"];
            if (window.contains("y")) m_intValues["window.y"] = window["y"];
            if (window.contains("maximized")) m_boolValues["window.maximized"] = window["maximized"];
        }
        
        if (config.contains("panels")) {
            auto panels = config["panels"];
            for (auto& panel : panels.items()) {
                m_boolValues["panel." + panel.key()] = panel.value();
            }
        }
        
        if (config.contains("settings")) {
            auto settings = config["settings"];
            for (auto& setting : settings.items()) {
                std::string key = setting.key();
                auto value = setting.value();
                
                if (value.is_number_integer()) {
                    m_intValues[key] = value;
                } else if (value.is_number_float()) {
                    m_floatValues[key] = value;
                } else if (value.is_string()) {
                    m_stringValues[key] = value;
                } else if (value.is_boolean()) {
                    m_boolValues[key] = value;
                }
            }
        }
        
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Failed to load config: " << e.what() << std::endl;
        setDefaults();
        return false;
    }
}

bool ConfigManager::saveConfig(const std::string& configFile) const {
    try {
        json config;
        
        // Save window settings
        config["window"]["width"] = getInt("window.width", 1280);
        config["window"]["height"] = getInt("window.height", 720);
        config["window"]["x"] = getInt("window.x", SDL_WINDOWPOS_CENTERED);
        config["window"]["y"] = getInt("window.y", SDL_WINDOWPOS_CENTERED);
        config["window"]["maximized"] = getBool("window.maximized", false);
          // Save panel visibility
        json panels;
        for (const auto& pair : m_boolValues) {
            if (pair.first.substr(0, 6) == "panel.") { // Check if starts with "panel."
                std::string panelName = pair.first.substr(6); // Remove "panel." prefix
                panels[panelName] = pair.second;
            }
        }
        config["panels"] = panels;
          // Save other settings
        json settings;
        for (const auto& pair : m_intValues) {
            if (pair.first.substr(0, 7) != "window.") { // Check if doesn't start with "window."
                settings[pair.first] = pair.second;
            }
        }
        for (const auto& pair : m_floatValues) {
            settings[pair.first] = pair.second;
        }
        for (const auto& pair : m_stringValues) {
            settings[pair.first] = pair.second;
        }
        for (const auto& pair : m_boolValues) {
            if (pair.first.substr(0, 6) != "panel." && pair.first.substr(0, 7) != "window.") {
                settings[pair.first] = pair.second;
            }
        }
        config["settings"] = settings;        std::ofstream file(configFile);
        if (!file.is_open()) {
            std::cerr << "Failed to open config file for writing: " << configFile << std::endl;
            return false;
        }
        
        file << config.dump(4); // Pretty print with 4-space indentation
        file.close();
        
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Failed to save config: " << e.what() << std::endl;
        return false;
    }
}

void ConfigManager::setWindowSize(int width, int height) {
    m_intValues["window.width"] = width;
    m_intValues["window.height"] = height;
}

void ConfigManager::getWindowSize(int& width, int& height) const {
    width = getInt("window.width", 1280);
    height = getInt("window.height", 720);
}

void ConfigManager::setWindowPosition(int x, int y) {
    m_intValues["window.x"] = x;
    m_intValues["window.y"] = y;
}

void ConfigManager::getWindowPosition(int& x, int& y) const {
    x = getInt("window.x", SDL_WINDOWPOS_CENTERED);
    y = getInt("window.y", SDL_WINDOWPOS_CENTERED);
}

void ConfigManager::setWindowMaximized(bool maximized) {
    m_boolValues["window.maximized"] = maximized;
}

bool ConfigManager::getWindowMaximized() const {
    return getBool("window.maximized", false);
}

void ConfigManager::setPanelVisible(const std::string& panelName, bool visible) {
    m_boolValues["panel." + panelName] = visible;
}

bool ConfigManager::getPanelVisible(const std::string& panelName) const {
    return getBool("panel." + panelName, true); // Default to visible
}

void ConfigManager::setAssetFolder(const std::string& folder) {
    m_stringValues["assets.folder"] = folder;
}

std::string ConfigManager::getAssetFolder() const {
    return getString("assets.folder", "assets");
}

void ConfigManager::setInt(const std::string& key, int value) {
    m_intValues[key] = value;
}

int ConfigManager::getInt(const std::string& key, int defaultValue) const {
    auto it = m_intValues.find(key);
    return (it != m_intValues.end()) ? it->second : defaultValue;
}

void ConfigManager::setFloat(const std::string& key, float value) {
    m_floatValues[key] = value;
}

float ConfigManager::getFloat(const std::string& key, float defaultValue) const {
    auto it = m_floatValues.find(key);
    return (it != m_floatValues.end()) ? it->second : defaultValue;
}

void ConfigManager::setString(const std::string& key, const std::string& value) {
    m_stringValues[key] = value;
}

std::string ConfigManager::getString(const std::string& key, const std::string& defaultValue) const {
    auto it = m_stringValues.find(key);
    return (it != m_stringValues.end()) ? it->second : defaultValue;
}

void ConfigManager::setBool(const std::string& key, bool value) {
    m_boolValues[key] = value;
}

bool ConfigManager::getBool(const std::string& key, bool defaultValue) const {
    auto it = m_boolValues.find(key);
    return (it != m_boolValues.end()) ? it->second : defaultValue;
}

void ConfigManager::resetToDefaults() {
    m_intValues.clear();
    m_floatValues.clear();
    m_stringValues.clear();
    m_boolValues.clear();
    setDefaults();
}

void ConfigManager::setDefaults() {
    // Default window settings
    m_intValues["window.width"] = 1280;
    m_intValues["window.height"] = 720;
    m_intValues["window.x"] = SDL_WINDOWPOS_CENTERED;
    m_intValues["window.y"] = SDL_WINDOWPOS_CENTERED;
    m_boolValues["window.maximized"] = false;
    
    // Default panel visibility
    m_boolValues["panel.SceneHierarchy"] = true;
    m_boolValues["panel.Inspector"] = true;
    m_boolValues["panel.AssetBrowser"] = true;
    m_boolValues["panel.Console"] = true;
    m_boolValues["panel.CameraControls"] = true;
    m_boolValues["panel.ProceduralGeneration"] = false; // Hidden by default
    
    // Default asset settings
    m_stringValues["assets.folder"] = "assets";
}
