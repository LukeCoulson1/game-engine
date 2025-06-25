#pragma once

#include "components/Components.h"
#include <queue>
#include <array>
#include <stdexcept>

class EntityManager {
public:
    EntityManager() {
        // Initialize the queue with all possible entity IDs
        // Start from 1 since 0 is reserved as "null/invalid" entity
        for (EntityID entity = 1; entity < MAX_ENTITIES; ++entity) {
            m_availableEntities.push(entity);
        }
    }
    
    EntityID createEntity() {
        if (m_livingEntityCount >= MAX_ENTITIES) {
            throw std::runtime_error("Too many entities in existence");
        }
        
        const EntityID id = m_availableEntities.front();
        m_availableEntities.pop();
        ++m_livingEntityCount;
        
        return id;
    }
    
    void destroyEntity(EntityID entity) {
        if (entity == 0 || entity >= MAX_ENTITIES) {
            throw std::runtime_error("Invalid entity ID");
        }
        
        m_signatures[entity].reset();
        m_availableEntities.push(entity);
        --m_livingEntityCount;
    }
    
    void setSignature(EntityID entity, const ComponentMask& signature) {
        if (entity == 0 || entity >= MAX_ENTITIES) {
            throw std::runtime_error("Invalid entity ID");
        }
        
        m_signatures[entity] = signature;
    }
    
    const ComponentMask& getSignature(EntityID entity) const {
        if (entity == 0 || entity >= MAX_ENTITIES) {
            throw std::runtime_error("Invalid entity ID");
        }
        
        return m_signatures[entity];
    }
    
    uint32_t getLivingEntityCount() const noexcept {
        return m_livingEntityCount;
    }
    
    static constexpr uint32_t getMaxEntities() noexcept {
        return MAX_ENTITIES;
    }

private:
    static constexpr uint32_t MAX_ENTITIES = 5000;
    
    std::queue<EntityID> m_availableEntities;
    std::array<ComponentMask, MAX_ENTITIES> m_signatures;
    uint32_t m_livingEntityCount = 0;
};
