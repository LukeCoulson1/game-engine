#pragma once

#include "Components.h"
#include <unordered_map>
#include <memory>
#include <typeindex>
#include <array>
#include <stdexcept>

class ComponentManager {
public:
    template<typename T>
    void registerComponent() {
        const std::type_index typeIndex = std::type_index(typeid(T));
        m_componentTypes.emplace(typeIndex, m_nextComponentType);
        m_componentArrays.emplace(typeIndex, std::make_shared<ComponentArray<T>>());
        ++m_nextComponentType;
    }
    
    template<typename T>
    ComponentType getComponentType() const {
        const std::type_index typeIndex = std::type_index(typeid(T));
        return m_componentTypes.at(typeIndex);
    }
    
    template<typename T>
    void addComponent(EntityID entity, T&& component) {
        getComponentArray<T>()->insertData(entity, std::forward<T>(component));
    }
    
    template<typename T>
    void removeComponent(EntityID entity) {
        getComponentArray<T>()->removeData(entity);
    }
    
    template<typename T>
    T& getComponent(EntityID entity) {
        return getComponentArray<T>()->getData(entity);
    }
    
    template<typename T>
    const T& getComponent(EntityID entity) const {
        return getComponentArray<T>()->getData(entity);
    }
    
    template<typename T>
    bool hasComponent(EntityID entity) const {
        return getComponentArray<T>()->hasData(entity);
    }
    
    void entityDestroyed(EntityID entity) {
        for (const auto& [typeIndex, componentArray] : m_componentArrays) {
            componentArray->entityDestroyed(entity);
        }
    }

private:
    class IComponentArray {
    public:
        virtual ~IComponentArray() = default;
        virtual void entityDestroyed(EntityID entity) = 0;
    };
      template<typename T>
    class ComponentArray : public IComponentArray {
    public:
        void insertData(EntityID entity, T&& component) {
            if (m_size >= MAX_COMPONENTS) {
                throw std::runtime_error("Component array is full");
            }
            m_entityToIndexMap[entity] = m_size;
            m_indexToEntityMap[m_size] = entity;
            m_componentArray[m_size] = std::forward<T>(component);
            ++m_size;
        }
        
        void removeData(EntityID entity) {
            auto it = m_entityToIndexMap.find(entity);
            if (it == m_entityToIndexMap.end()) return;
            
            const size_t indexOfRemovedEntity = it->second;
            const size_t indexOfLastElement = m_size - 1;
            
            // Move last element to removed position (if not the same)
            if (indexOfRemovedEntity != indexOfLastElement) {
                m_componentArray[indexOfRemovedEntity] = std::move(m_componentArray[indexOfLastElement]);
                
                const EntityID entityOfLastElement = m_indexToEntityMap[indexOfLastElement];
                m_entityToIndexMap[entityOfLastElement] = indexOfRemovedEntity;
                m_indexToEntityMap[indexOfRemovedEntity] = entityOfLastElement;
            }
            
            m_entityToIndexMap.erase(entity);
            m_indexToEntityMap.erase(indexOfLastElement);
            --m_size;
        }
        
        T& getData(EntityID entity) {
            auto it = m_entityToIndexMap.find(entity);
            if (it == m_entityToIndexMap.end()) {
                throw std::runtime_error("Entity does not have this component");
            }
            return m_componentArray[it->second];
        }
        
        const T& getData(EntityID entity) const {
            auto it = m_entityToIndexMap.find(entity);
            if (it == m_entityToIndexMap.end()) {
                throw std::runtime_error("Entity does not have this component");
            }
            return m_componentArray[it->second];
        }
        
        bool hasData(EntityID entity) const {
            return m_entityToIndexMap.find(entity) != m_entityToIndexMap.end();
        }
        
        void entityDestroyed(EntityID entity) override {
            removeData(entity);
        }
        
    private:
        static constexpr size_t MAX_COMPONENTS = 5000;
        std::array<T, MAX_COMPONENTS> m_componentArray;
        std::unordered_map<EntityID, size_t> m_entityToIndexMap;
        std::unordered_map<size_t, EntityID> m_indexToEntityMap;
        size_t m_size = 0;
    };private:
    template<typename T>
    std::shared_ptr<ComponentArray<T>> getComponentArray() const {
        const std::type_index typeIndex = std::type_index(typeid(T));
        return std::static_pointer_cast<ComponentArray<T>>(m_componentArrays.at(typeIndex));
    }
    
    std::unordered_map<std::type_index, ComponentType> m_componentTypes;
    std::unordered_map<std::type_index, std::shared_ptr<IComponentArray>> m_componentArrays;
    ComponentType m_nextComponentType = 0;
};
