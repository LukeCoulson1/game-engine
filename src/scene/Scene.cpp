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
    
    // Register audio and effects components
    registerComponent<AudioSource>();
    registerComponent<AudioListener>();
    registerComponent<VisualEffect>();
    registerComponent<LightSource>();
    
    // Register NPC and AI components
    registerComponent<NPCController>();
    registerComponent<AIBehavior>();
    registerComponent<AIStateMachine>();
    registerComponent<AIPathfinding>();
    
    // Register UI components
    registerComponent<UIElement>();
    registerComponent<UIButton>();
    registerComponent<UIText>();
    registerComponent<UIImage>();
    registerComponent<UIHealthBar>();
    registerComponent<UIInventorySlot>();
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

// Add audio and effects components
template void Scene::addComponent<AudioSource>(EntityID, const AudioSource&);
template void Scene::addComponent<AudioListener>(EntityID, const AudioListener&);
template void Scene::addComponent<VisualEffect>(EntityID, const VisualEffect&);
template void Scene::addComponent<LightSource>(EntityID, const LightSource&);

// Add NPC and AI components
template void Scene::addComponent<NPCController>(EntityID, const NPCController&);
template void Scene::addComponent<AIBehavior>(EntityID, const AIBehavior&);
template void Scene::addComponent<AIStateMachine>(EntityID, const AIStateMachine&);
template void Scene::addComponent<AIPathfinding>(EntityID, const AIPathfinding&);

// Add UI components
template void Scene::addComponent<UIElement>(EntityID, const UIElement&);
template void Scene::addComponent<UIButton>(EntityID, const UIButton&);
template void Scene::addComponent<UIText>(EntityID, const UIText&);
template void Scene::addComponent<UIImage>(EntityID, const UIImage&);
template void Scene::addComponent<UIHealthBar>(EntityID, const UIHealthBar&);
template void Scene::addComponent<UIInventorySlot>(EntityID, const UIInventorySlot&);

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

// Remove audio and effects components
template void Scene::removeComponent<AudioSource>(EntityID);
template void Scene::removeComponent<AudioListener>(EntityID);
template void Scene::removeComponent<VisualEffect>(EntityID);
template void Scene::removeComponent<LightSource>(EntityID);

// Remove NPC and AI components
template void Scene::removeComponent<NPCController>(EntityID);
template void Scene::removeComponent<AIBehavior>(EntityID);
template void Scene::removeComponent<AIStateMachine>(EntityID);
template void Scene::removeComponent<AIPathfinding>(EntityID);

// Remove UI components
template void Scene::removeComponent<UIElement>(EntityID);
template void Scene::removeComponent<UIButton>(EntityID);
template void Scene::removeComponent<UIText>(EntityID);
template void Scene::removeComponent<UIImage>(EntityID);
template void Scene::removeComponent<UIHealthBar>(EntityID);
template void Scene::removeComponent<UIInventorySlot>(EntityID);

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

// Get audio and effects components
template AudioSource& Scene::getComponent<AudioSource>(EntityID);
template AudioListener& Scene::getComponent<AudioListener>(EntityID);
template VisualEffect& Scene::getComponent<VisualEffect>(EntityID);
template LightSource& Scene::getComponent<LightSource>(EntityID);

// Get NPC and AI components
template NPCController& Scene::getComponent<NPCController>(EntityID);
template AIBehavior& Scene::getComponent<AIBehavior>(EntityID);
template AIStateMachine& Scene::getComponent<AIStateMachine>(EntityID);
template AIPathfinding& Scene::getComponent<AIPathfinding>(EntityID);

// Get UI components
template UIElement& Scene::getComponent<UIElement>(EntityID);
template UIButton& Scene::getComponent<UIButton>(EntityID);
template UIText& Scene::getComponent<UIText>(EntityID);
template UIImage& Scene::getComponent<UIImage>(EntityID);
template UIHealthBar& Scene::getComponent<UIHealthBar>(EntityID);
template UIInventorySlot& Scene::getComponent<UIInventorySlot>(EntityID);

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

// Get const audio and effects components
template const AudioSource& Scene::getComponent<AudioSource>(EntityID) const;
template const AudioListener& Scene::getComponent<AudioListener>(EntityID) const;
template const VisualEffect& Scene::getComponent<VisualEffect>(EntityID) const;
template const LightSource& Scene::getComponent<LightSource>(EntityID) const;

// Get const NPC and AI components
template const NPCController& Scene::getComponent<NPCController>(EntityID) const;
template const AIBehavior& Scene::getComponent<AIBehavior>(EntityID) const;
template const AIStateMachine& Scene::getComponent<AIStateMachine>(EntityID) const;
template const AIPathfinding& Scene::getComponent<AIPathfinding>(EntityID) const;

// Get const UI components
template const UIElement& Scene::getComponent<UIElement>(EntityID) const;
template const UIButton& Scene::getComponent<UIButton>(EntityID) const;
template const UIText& Scene::getComponent<UIText>(EntityID) const;
template const UIImage& Scene::getComponent<UIImage>(EntityID) const;
template const UIHealthBar& Scene::getComponent<UIHealthBar>(EntityID) const;
template const UIInventorySlot& Scene::getComponent<UIInventorySlot>(EntityID) const;

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

// Has audio and effects components
template bool Scene::hasComponent<AudioSource>(EntityID);
template bool Scene::hasComponent<AudioListener>(EntityID);
template bool Scene::hasComponent<VisualEffect>(EntityID);
template bool Scene::hasComponent<LightSource>(EntityID);

// Has NPC and AI components
template bool Scene::hasComponent<NPCController>(EntityID);
template bool Scene::hasComponent<AIBehavior>(EntityID);
template bool Scene::hasComponent<AIStateMachine>(EntityID);
template bool Scene::hasComponent<AIPathfinding>(EntityID);

// Has UI components
template bool Scene::hasComponent<UIElement>(EntityID);
template bool Scene::hasComponent<UIButton>(EntityID);
template bool Scene::hasComponent<UIText>(EntityID);
template bool Scene::hasComponent<UIImage>(EntityID);
template bool Scene::hasComponent<UIHealthBar>(EntityID);
template bool Scene::hasComponent<UIInventorySlot>(EntityID);

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

// Has const audio and effects components
template bool Scene::hasComponent<AudioSource>(EntityID) const;
template bool Scene::hasComponent<AudioListener>(EntityID) const;
template bool Scene::hasComponent<VisualEffect>(EntityID) const;
template bool Scene::hasComponent<LightSource>(EntityID) const;

// Has const NPC and AI components
template bool Scene::hasComponent<NPCController>(EntityID) const;
template bool Scene::hasComponent<AIBehavior>(EntityID) const;
template bool Scene::hasComponent<AIStateMachine>(EntityID) const;
template bool Scene::hasComponent<AIPathfinding>(EntityID) const;

// Has const UI components
template bool Scene::hasComponent<UIElement>(EntityID) const;
template bool Scene::hasComponent<UIButton>(EntityID) const;
template bool Scene::hasComponent<UIText>(EntityID) const;
template bool Scene::hasComponent<UIImage>(EntityID) const;
template bool Scene::hasComponent<UIHealthBar>(EntityID) const;
template bool Scene::hasComponent<UIInventorySlot>(EntityID) const;

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

// Get component type for audio and effects components
template ComponentType Scene::getComponentType<AudioSource>();
template ComponentType Scene::getComponentType<AudioListener>();
template ComponentType Scene::getComponentType<VisualEffect>();
template ComponentType Scene::getComponentType<LightSource>();

// Get component type for NPC and AI components
template ComponentType Scene::getComponentType<NPCController>();
template ComponentType Scene::getComponentType<AIBehavior>();
template ComponentType Scene::getComponentType<AIStateMachine>();
template ComponentType Scene::getComponentType<AIPathfinding>();

// Get component type for UI components
template ComponentType Scene::getComponentType<UIElement>();
template ComponentType Scene::getComponentType<UIButton>();
template ComponentType Scene::getComponentType<UIText>();
template ComponentType Scene::getComponentType<UIImage>();
template ComponentType Scene::getComponentType<UIHealthBar>();
template ComponentType Scene::getComponentType<UIInventorySlot>();
