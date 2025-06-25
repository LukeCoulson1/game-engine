#pragma once

#include <set>

// Forward declarations
class Renderer;
using EntityID = uint32_t;

class System {
public:
    std::set<EntityID> entities;
    
    virtual ~System() = default;
    virtual void update(float deltaTime) {}
    virtual void render(Renderer* renderer) {}
};
