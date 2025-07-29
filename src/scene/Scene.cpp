#include "Scene.h"
#include "../systems/SystemManager.h"
#include "../components/EntityManager.h"
#include "../generation/ProceduralGeneration.h"
#include "../components/Components.h"

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
    registerComponent<ProceduralGenerated>();
    registerComponent<Transform>();
    registerComponent<Rotation>();
    registerComponent<Scale>();
    registerComponent<Sprite>();
    registerComponent<Collider>();
    registerComponent<RigidBody>();
    registerComponent<EntitySpawner>();
    registerComponent<ParticleEffect>();
    
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
    // Check for entities with non-empty signatures (living entities)
    for (EntityID entity = 1; entity < EntityManager::getMaxEntities(); ++entity) {
        auto signature = m_entityManager->getSignature(entity);
        if (signature.any()) { // Entity has at least one component
            entities.push_back(entity);
        }
    }
    return entities;
}

void Scene::setProceduralMap(std::shared_ptr<ProceduralMap> map) {
    m_proceduralMap = map;
}

// Explicit template instantiations for all component types used in the game
// This ensures the template methods are compiled into the library
template void Scene::registerComponent<Name>();
template void Scene::registerComponent<ProceduralGenerated>();
template void Scene::registerComponent<Transform>();
template void Scene::registerComponent<Rotation>();
template void Scene::registerComponent<Scale>();
template void Scene::registerComponent<Sprite>();
template void Scene::registerComponent<Collider>();
template void Scene::registerComponent<RigidBody>();
template void Scene::registerComponent<EntitySpawner>();
template void Scene::registerComponent<ParticleEffect>();
template void Scene::registerComponent<PlayerController>();
template void Scene::registerComponent<PlayerStats>();
template void Scene::registerComponent<PlayerPhysics>();
template void Scene::registerComponent<PlayerInventory>();
template void Scene::registerComponent<PlayerAbilities>();
template void Scene::registerComponent<PlayerState>();

template void Scene::addComponent<Name>(EntityID, const Name&);
template void Scene::addComponent<ProceduralGenerated>(EntityID, const ProceduralGenerated&);
template void Scene::addComponent<Transform>(EntityID, const Transform&);
template void Scene::addComponent<Rotation>(EntityID, const Rotation&);
template void Scene::addComponent<Scale>(EntityID, const Scale&);
template void Scene::addComponent<Sprite>(EntityID, const Sprite&);
template void Scene::addComponent<Collider>(EntityID, const Collider&);
template void Scene::addComponent<RigidBody>(EntityID, const RigidBody&);
template void Scene::addComponent<EntitySpawner>(EntityID, const EntitySpawner&);
template void Scene::addComponent<ParticleEffect>(EntityID, const ParticleEffect&);
template void Scene::addComponent<PlayerController>(EntityID, const PlayerController&);
template void Scene::addComponent<PlayerStats>(EntityID, const PlayerStats&);
template void Scene::addComponent<PlayerPhysics>(EntityID, const PlayerPhysics&);
template void Scene::addComponent<PlayerInventory>(EntityID, const PlayerInventory&);
template void Scene::addComponent<PlayerAbilities>(EntityID, const PlayerAbilities&);
template void Scene::addComponent<PlayerState>(EntityID, const PlayerState&);

template void Scene::removeComponent<Name>(EntityID);
template void Scene::removeComponent<ProceduralGenerated>(EntityID);
template void Scene::removeComponent<Transform>(EntityID);
template void Scene::removeComponent<Rotation>(EntityID);
template void Scene::removeComponent<Scale>(EntityID);
template void Scene::removeComponent<Sprite>(EntityID);
template void Scene::removeComponent<Collider>(EntityID);
template void Scene::removeComponent<RigidBody>(EntityID);
template void Scene::removeComponent<EntitySpawner>(EntityID);
template void Scene::removeComponent<ParticleEffect>(EntityID);
template void Scene::removeComponent<PlayerController>(EntityID);
template void Scene::removeComponent<PlayerStats>(EntityID);
template void Scene::removeComponent<PlayerPhysics>(EntityID);
template void Scene::removeComponent<PlayerInventory>(EntityID);
template void Scene::removeComponent<PlayerAbilities>(EntityID);
template void Scene::removeComponent<PlayerState>(EntityID);

template Name& Scene::getComponent<Name>(EntityID);
template ProceduralGenerated& Scene::getComponent<ProceduralGenerated>(EntityID);
template Transform& Scene::getComponent<Transform>(EntityID);
template Rotation& Scene::getComponent<Rotation>(EntityID);
template Scale& Scene::getComponent<Scale>(EntityID);
template Sprite& Scene::getComponent<Sprite>(EntityID);
template Collider& Scene::getComponent<Collider>(EntityID);
template RigidBody& Scene::getComponent<RigidBody>(EntityID);
template EntitySpawner& Scene::getComponent<EntitySpawner>(EntityID);
template ParticleEffect& Scene::getComponent<ParticleEffect>(EntityID);
template PlayerController& Scene::getComponent<PlayerController>(EntityID);
template PlayerStats& Scene::getComponent<PlayerStats>(EntityID);
template PlayerPhysics& Scene::getComponent<PlayerPhysics>(EntityID);
template PlayerInventory& Scene::getComponent<PlayerInventory>(EntityID);
template PlayerAbilities& Scene::getComponent<PlayerAbilities>(EntityID);
template PlayerState& Scene::getComponent<PlayerState>(EntityID);

template const Name& Scene::getComponent<Name>(EntityID) const;
template const ProceduralGenerated& Scene::getComponent<ProceduralGenerated>(EntityID) const;
template const Transform& Scene::getComponent<Transform>(EntityID) const;
template const Rotation& Scene::getComponent<Rotation>(EntityID) const;
template const Scale& Scene::getComponent<Scale>(EntityID) const;
template const Sprite& Scene::getComponent<Sprite>(EntityID) const;
template const Collider& Scene::getComponent<Collider>(EntityID) const;
template const RigidBody& Scene::getComponent<RigidBody>(EntityID) const;
template const EntitySpawner& Scene::getComponent<EntitySpawner>(EntityID) const;
template const ParticleEffect& Scene::getComponent<ParticleEffect>(EntityID) const;
template const PlayerController& Scene::getComponent<PlayerController>(EntityID) const;
template const PlayerStats& Scene::getComponent<PlayerStats>(EntityID) const;
template const PlayerPhysics& Scene::getComponent<PlayerPhysics>(EntityID) const;
template const PlayerInventory& Scene::getComponent<PlayerInventory>(EntityID) const;
template const PlayerAbilities& Scene::getComponent<PlayerAbilities>(EntityID) const;
template const PlayerState& Scene::getComponent<PlayerState>(EntityID) const;

template bool Scene::hasComponent<Name>(EntityID);
template bool Scene::hasComponent<ProceduralGenerated>(EntityID);
template bool Scene::hasComponent<Transform>(EntityID);
template bool Scene::hasComponent<Rotation>(EntityID);
template bool Scene::hasComponent<Scale>(EntityID);
template bool Scene::hasComponent<Sprite>(EntityID);
template bool Scene::hasComponent<Collider>(EntityID);
template bool Scene::hasComponent<RigidBody>(EntityID);
template bool Scene::hasComponent<EntitySpawner>(EntityID);
template bool Scene::hasComponent<ParticleEffect>(EntityID);
template bool Scene::hasComponent<PlayerController>(EntityID);
template bool Scene::hasComponent<PlayerStats>(EntityID);
template bool Scene::hasComponent<PlayerPhysics>(EntityID);
template bool Scene::hasComponent<PlayerInventory>(EntityID);
template bool Scene::hasComponent<PlayerAbilities>(EntityID);
template bool Scene::hasComponent<PlayerState>(EntityID);

template bool Scene::hasComponent<Name>(EntityID) const;
template bool Scene::hasComponent<ProceduralGenerated>(EntityID) const;
template bool Scene::hasComponent<Transform>(EntityID) const;
template bool Scene::hasComponent<Rotation>(EntityID) const;
template bool Scene::hasComponent<Scale>(EntityID) const;
template bool Scene::hasComponent<Sprite>(EntityID) const;
template bool Scene::hasComponent<Collider>(EntityID) const;
template bool Scene::hasComponent<RigidBody>(EntityID) const;
template bool Scene::hasComponent<EntitySpawner>(EntityID) const;
template bool Scene::hasComponent<ParticleEffect>(EntityID) const;
template bool Scene::hasComponent<PlayerController>(EntityID) const;
template bool Scene::hasComponent<PlayerStats>(EntityID) const;
template bool Scene::hasComponent<PlayerPhysics>(EntityID) const;
template bool Scene::hasComponent<PlayerInventory>(EntityID) const;
template bool Scene::hasComponent<PlayerAbilities>(EntityID) const;
template bool Scene::hasComponent<PlayerState>(EntityID) const;

template ComponentType Scene::getComponentType<Name>();
template ComponentType Scene::getComponentType<ProceduralGenerated>();
template ComponentType Scene::getComponentType<Transform>();
template ComponentType Scene::getComponentType<Rotation>();
template ComponentType Scene::getComponentType<Scale>();
template ComponentType Scene::getComponentType<Sprite>();
template ComponentType Scene::getComponentType<Collider>();
template ComponentType Scene::getComponentType<RigidBody>();
template ComponentType Scene::getComponentType<PlayerController>();
template ComponentType Scene::getComponentType<PlayerStats>();
template ComponentType Scene::getComponentType<PlayerPhysics>();
template ComponentType Scene::getComponentType<PlayerInventory>();
template ComponentType Scene::getComponentType<PlayerAbilities>();
template ComponentType Scene::getComponentType<PlayerState>();
