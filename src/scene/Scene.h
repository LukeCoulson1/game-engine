#pragma once

#include "components/EntityManager.h"
#include "components/ComponentManager.h"
#include "systems/System.h"
#include "systems/SystemManager.h"
#include <memory>
#include <unordered_map>
#include <utility>

class Renderer;
class ProceduralMap;

class Scene {
public:
    Scene();
    ~Scene();
    
    // Entity management
    EntityID createEntity();
    void destroyEntity(EntityID entity);
    
    // Component management
    template<typename T>
    void registerComponent() {
        m_componentManager->registerComponent<T>();
    }    template<typename T>
    void addComponent(EntityID entity, const T& component) {
        m_componentManager->addComponent<T>(entity, T(component));
        
        auto signature = m_entityManager->getSignature(entity);
        signature.set(m_componentManager->getComponentType<T>(), true);
        m_entityManager->setSignature(entity, signature);
        
        systemManager->entitySignatureChanged(entity, signature);
    }
    
    template<typename T>
    void removeComponent(EntityID entity) {
        m_componentManager->removeComponent<T>(entity);
        
        auto signature = m_entityManager->getSignature(entity);
        signature.set(m_componentManager->getComponentType<T>(), false);
        m_entityManager->setSignature(entity, signature);
        
        systemManager->entitySignatureChanged(entity, signature);
    }
    
    template<typename T>
    T& getComponent(EntityID entity) {
        return m_componentManager->getComponent<T>(entity);
    }
    
    template<typename T>
    const T& getComponent(EntityID entity) const {
        return m_componentManager->getComponent<T>(entity);
    }
    
    template<typename T>
    bool hasComponent(EntityID entity) {
        return m_componentManager->hasComponent<T>(entity);
    }
    
    template<typename T>
    bool hasComponent(EntityID entity) const {
        return m_componentManager->hasComponent<T>(entity);
    }
    
    template<typename T>
    ComponentType getComponentType() {
        return m_componentManager->getComponentType<T>();
    }
    
    // System management
    template<typename T>
    std::shared_ptr<T> registerSystem() {
        return systemManager->registerSystem<T>();
    }
    
    template<typename T>
    void setSystemSignature(ComponentMask signature) {
        systemManager->setSignature<T>(signature);
    }
      // Scene lifecycle
    virtual void initialize();
    virtual void update(float deltaTime);
    virtual void render(Renderer* renderer);
    virtual void cleanup();
    
    // Entity queries
    std::vector<EntityID> getEntitiesWithComponents(ComponentMask signature);
    
    // Entity naming utilities
    void setEntityName(EntityID entity, const std::string& name);
    std::string getEntityName(EntityID entity) const;
    std::vector<EntityID> getAllLivingEntities() const;
    
    // Procedural map support
    void setProceduralMap(std::shared_ptr<ProceduralMap> map);
    std::shared_ptr<ProceduralMap> getProceduralMap() const { return m_proceduralMap; }
    bool hasProceduralMap() const { return m_proceduralMap != nullptr; }
    
protected:
    std::unique_ptr<ComponentManager> m_componentManager;
    std::unique_ptr<EntityManager> m_entityManager;
    std::unique_ptr<SystemManager> systemManager;
    
private:
    std::shared_ptr<ProceduralMap> m_proceduralMap;
};
