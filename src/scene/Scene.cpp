#include "Scene.h"
#include "systems/SystemManager.h"
#include "components/EntityManager.h"
#include "../generation/ProceduralGeneration.h"

Scene::Scene() {
    m_componentManager = std::make_unique<ComponentManager>();
    m_entityManager = std::make_unique<EntityManager>();
    systemManager = std::make_unique<SystemManager>();
}

Scene::~Scene() {
    cleanup();
}

EntityID Scene::createEntity() {
    return m_entityManager->createEntity();
}

void Scene::destroyEntity(EntityID entity) {
    m_entityManager->destroyEntity(entity);
    m_componentManager->entityDestroyed(entity);
    systemManager->entityDestroyed(entity);
}

void Scene::initialize() {
    // Register common components
    registerComponent<Name>();
    registerComponent<Transform>();
    registerComponent<Sprite>();
    registerComponent<Collider>();
    registerComponent<RigidBody>();
    
    // Register player-specific components
    registerComponent<PlayerController>();
    registerComponent<PlayerStats>();
    registerComponent<PlayerPhysics>();
    registerComponent<PlayerInventory>();
    registerComponent<PlayerAbilities>();
    registerComponent<PlayerState>();
}

void Scene::update(float deltaTime) {
    systemManager->update(deltaTime);
}

void Scene::render(Renderer* renderer) {
    systemManager->render(renderer);
}

void Scene::cleanup() {
    // Cleanup will be handled by unique_ptr destructors
}

std::vector<EntityID> Scene::getEntitiesWithComponents(ComponentMask signature) {
    std::vector<EntityID> entities;
    
    // This is a simple implementation - in a real engine you'd want more efficient queries
    for (EntityID entity = 0; entity < m_entityManager->getLivingEntityCount(); ++entity) {
        auto entitySignature = m_entityManager->getSignature(entity);
        if ((entitySignature & signature) == signature) {
            entities.push_back(entity);
        }
    }
    
    return entities;
}

void Scene::setEntityName(EntityID entity, const std::string& name) {
    if (!hasComponent<Name>(entity)) {
        addComponent<Name>(entity, Name(name));
    } else {
        auto& nameComponent = getComponent<Name>(entity);
        nameComponent.name = name;
    }
}

std::string Scene::getEntityName(EntityID entity) const {
    if (hasComponent<Name>(entity)) {
        return getComponent<Name>(entity).name;
    }
    return "Entity " + std::to_string(entity);
}

std::vector<EntityID> Scene::getAllLivingEntities() const {
    std::vector<EntityID> entities;
    // Check for entities with Transform component (basic requirement for most entities)
    for (EntityID entity = 0; entity < EntityManager::getMaxEntities(); ++entity) {
        if (hasComponent<Transform>(entity)) {
            entities.push_back(entity);
        }
    }
    return entities;
}

void Scene::setProceduralMap(std::shared_ptr<ProceduralMap> map) {
    m_proceduralMap = map;
}
