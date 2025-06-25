#include "core/Engine.h"
#include "scene/Scene.h"
#include "systems/CoreSystems.h"
#include <iostream>

// Example game scene for Zelda-like game
class GameScene : public Scene {
public:
    void initialize() override {        Scene::initialize();
        
        // Set up systems
        auto renderSystem = registerSystem<RenderSystem>();
        auto physicsSystem = registerSystem<PhysicsSystem>();
        auto collisionSystem = registerSystem<CollisionSystem>();
        
        // Set scene pointer for each system
        renderSystem->setScene(this);
        physicsSystem->setScene(this);
        collisionSystem->setScene(this);
        
        ComponentMask renderSignature;
        renderSignature.set(getComponentType<Transform>());
        renderSignature.set(getComponentType<Sprite>());
        setSystemSignature<RenderSystem>(renderSignature);
        
        ComponentMask physicsSignature;
        physicsSignature.set(getComponentType<Transform>());
        physicsSignature.set(getComponentType<RigidBody>());
        setSystemSignature<PhysicsSystem>(physicsSignature);
        
        ComponentMask collisionSignature;
        collisionSignature.set(getComponentType<Transform>());
        collisionSignature.set(getComponentType<Collider>());
        setSystemSignature<CollisionSystem>(collisionSignature);
        
        // Create a player entity
        createPlayer();
        
        // Create some test entities
        createTestEntities();
    }
    
private:
    void createPlayer() {
        EntityID player = createEntity();
        
        addComponent<Transform>(player, Transform(100, 100));
        
        // Create a simple colored rectangle for the player (since we don't have textures loaded)
        addComponent<Sprite>(player, Sprite());
        
        addComponent<Collider>(player, Collider(32, 32));
        addComponent<RigidBody>(player, RigidBody());
    }
    
    void createTestEntities() {
        // Create some static obstacles
        for (int i = 0; i < 5; ++i) {
            EntityID obstacle = createEntity();
            
            addComponent<Transform>(obstacle, Transform(200 + i * 100, 300));
            addComponent<Sprite>(obstacle, Sprite());
            
            Collider collider(64, 64);
            collider.isStatic = true;
            addComponent<Collider>(obstacle, collider);
        }
    }
};

int main(int argc, char* argv[]) {
    auto& engine = Engine::getInstance();
    
    if (!engine.initialize("Game Runtime", 800, 600)) {
        std::cerr << "Failed to initialize engine!" << std::endl;
        return -1;
    }
    
    // Create and set the game scene
    auto gameScene = std::make_shared<GameScene>();
    gameScene->initialize();
    engine.setActiveScene(gameScene);
    
    // Run the game
    engine.run();
    engine.shutdown();
    
    return 0;
}
