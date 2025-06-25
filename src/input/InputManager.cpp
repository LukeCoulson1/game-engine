#include "InputManager.h"
#include "core/Engine.h"

InputManager::InputManager() : m_mousePosition(0, 0), m_mouseDelta(0, 0), m_lastMousePosition(0, 0) {
    // Initialize controller subsystem
    SDL_InitSubSystem(SDL_INIT_GAMECONTROLLER);
}

InputManager::~InputManager() {
    // Close all controllers
    for (auto& pair : m_controllers) {
        SDL_GameControllerClose(pair.second);
    }
}

void InputManager::handleEvent(const SDL_Event& event) {
    switch (event.type) {
        case SDL_KEYDOWN:
            if (!event.key.repeat) {
                m_keysJustPressed.insert(event.key.keysym.scancode);
                m_keysPressed.insert(event.key.keysym.scancode);
            }
            break;
            
        case SDL_KEYUP:
            m_keysJustReleased.insert(event.key.keysym.scancode);
            m_keysPressed.erase(event.key.keysym.scancode);
            break;
            
        case SDL_MOUSEBUTTONDOWN: {
            MouseButton button = static_cast<MouseButton>(event.button.button);
            m_mouseButtonsJustPressed.insert(button);
            m_mouseButtonsPressed.insert(button);
            break;
        }
        
        case SDL_MOUSEBUTTONUP: {
            MouseButton button = static_cast<MouseButton>(event.button.button);
            m_mouseButtonsJustReleased.insert(button);
            m_mouseButtonsPressed.erase(button);
            break;
        }
        
        case SDL_MOUSEMOTION:
            m_mousePosition = Vector2(static_cast<float>(event.motion.x), static_cast<float>(event.motion.y));
            break;
            
        case SDL_CONTROLLERDEVICEADDED: {
            int deviceIndex = event.cdevice.which;
            SDL_GameController* controller = SDL_GameControllerOpen(deviceIndex);
            if (controller) {
                SDL_JoystickID id = SDL_JoystickInstanceID(SDL_GameControllerGetJoystick(controller));
                m_controllers[id] = controller;
            }
            break;
        }
        
        case SDL_CONTROLLERDEVICEREMOVED: {
            int instanceId = event.cdevice.which;
            auto it = m_controllers.find(instanceId);
            if (it != m_controllers.end()) {
                SDL_GameControllerClose(it->second);
                m_controllers.erase(it);
            }
            break;
        }
    }
}

void InputManager::update() {
    // Calculate mouse delta
    m_mouseDelta = m_mousePosition - m_lastMousePosition;
    m_lastMousePosition = m_mousePosition;
    
    // Clear "just pressed/released" states
    m_keysJustPressed.clear();
    m_keysJustReleased.clear();
    m_mouseButtonsJustPressed.clear();
    m_mouseButtonsJustReleased.clear();
}

bool InputManager::isKeyPressed(SDL_Scancode key) const {
    return m_keysPressed.find(key) != m_keysPressed.end();
}

bool InputManager::isKeyJustPressed(SDL_Scancode key) const {
    return m_keysJustPressed.find(key) != m_keysJustPressed.end();
}

bool InputManager::isKeyJustReleased(SDL_Scancode key) const {
    return m_keysJustReleased.find(key) != m_keysJustReleased.end();
}

bool InputManager::isMouseButtonPressed(MouseButton button) const {
    return m_mouseButtonsPressed.find(button) != m_mouseButtonsPressed.end();
}

bool InputManager::isMouseButtonJustPressed(MouseButton button) const {
    return m_mouseButtonsJustPressed.find(button) != m_mouseButtonsJustPressed.end();
}

bool InputManager::isMouseButtonJustReleased(MouseButton button) const {
    return m_mouseButtonsJustReleased.find(button) != m_mouseButtonsJustReleased.end();
}

Vector2 InputManager::getMouseWorldPosition() const {
    auto& engine = Engine::getInstance();
    if (auto renderer = engine.getRenderer()) {
        return renderer->screenToWorld(m_mousePosition);
    }
    return m_mousePosition;
}

bool InputManager::isControllerConnected(int controllerId) const {
    return m_controllers.find(controllerId) != m_controllers.end();
}

float InputManager::getControllerAxis(int controllerId, SDL_GameControllerAxis axis) const {
    auto it = m_controllers.find(controllerId);
    if (it != m_controllers.end()) {
        Sint16 value = SDL_GameControllerGetAxis(it->second, axis);
        return value / 32767.0f; // Normalize to -1.0 to 1.0
    }
    return 0.0f;
}

bool InputManager::isControllerButtonPressed(int controllerId, SDL_GameControllerButton button) const {
    auto it = m_controllers.find(controllerId);
    if (it != m_controllers.end()) {
        return SDL_GameControllerGetButton(it->second, button) == 1;
    }
    return false;
}
