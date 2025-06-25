#pragma once

#include <SDL2/SDL.h>
#include <unordered_map>
#include <unordered_set>
#include "graphics/Renderer.h"

enum class MouseButton {
    Left = SDL_BUTTON_LEFT,
    Middle = SDL_BUTTON_MIDDLE,
    Right = SDL_BUTTON_RIGHT
};

class InputManager {
public:
    InputManager();
    ~InputManager();
    
    void handleEvent(const SDL_Event& event);
    void update();
    
    // Keyboard input
    bool isKeyPressed(SDL_Scancode key) const;
    bool isKeyJustPressed(SDL_Scancode key) const;
    bool isKeyJustReleased(SDL_Scancode key) const;
    
    // Mouse input
    bool isMouseButtonPressed(MouseButton button) const;
    bool isMouseButtonJustPressed(MouseButton button) const;
    bool isMouseButtonJustReleased(MouseButton button) const;
    
    Vector2 getMousePosition() const { return m_mousePosition; }
    Vector2 getMouseWorldPosition() const;
    Vector2 getMouseDelta() const { return m_mouseDelta; }
    
    // Controller input (for future expansion)
    bool isControllerConnected(int controllerId = 0) const;
    float getControllerAxis(int controllerId, SDL_GameControllerAxis axis) const;
    bool isControllerButtonPressed(int controllerId, SDL_GameControllerButton button) const;

private:
    // Keyboard state
    std::unordered_set<SDL_Scancode> m_keysPressed;
    std::unordered_set<SDL_Scancode> m_keysJustPressed;
    std::unordered_set<SDL_Scancode> m_keysJustReleased;
    
    // Mouse state
    std::unordered_set<MouseButton> m_mouseButtonsPressed;
    std::unordered_set<MouseButton> m_mouseButtonsJustPressed;
    std::unordered_set<MouseButton> m_mouseButtonsJustReleased;
    Vector2 m_mousePosition;
    Vector2 m_mouseDelta;
    Vector2 m_lastMousePosition;
    
    // Controller state
    std::unordered_map<int, SDL_GameController*> m_controllers;
};
