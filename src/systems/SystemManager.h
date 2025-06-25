#pragma once

#include "System.h"
#include <memory>
#include <unordered_map>
#include <typeindex>

class SystemManager {
public:
    template<typename T>
    std::shared_ptr<T> registerSystem() {
        const char* typeName = typeid(T).name();
        
        auto system = std::make_shared<T>();
        m_systems.insert({typeName, system});
        return system;
    }
    
    template<typename T>
    void setSignature(ComponentMask signature) {
        const char* typeName = typeid(T).name();
        m_signatures.insert({typeName, signature});
    }
    
    void entityDestroyed(EntityID entity) {
        for (auto const& pair : m_systems) {
            auto const& system = pair.second;
            system->entities.erase(entity);
        }
    }
    
    void entitySignatureChanged(EntityID entity, ComponentMask entitySignature) {
        for (auto const& pair : m_systems) {
            auto const& type = pair.first;
            auto const& system = pair.second;
            auto const& systemSignature = m_signatures[type];
            
            if ((entitySignature & systemSignature) == systemSignature) {
                system->entities.insert(entity);
            } else {
                system->entities.erase(entity);
            }
        }
    }
    
    void update(float deltaTime) {
        for (auto const& pair : m_systems) {
            pair.second->update(deltaTime);
        }
    }
    
    void render(Renderer* renderer) {
        for (auto const& pair : m_systems) {
            pair.second->render(renderer);
        }
    }

private:
    std::unordered_map<const char*, ComponentMask> m_signatures{};
    std::unordered_map<const char*, std::shared_ptr<System>> m_systems{};
};
