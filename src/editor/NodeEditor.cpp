#include "NodeEditor.h"
#include "SceneWindow.h"
#include "../core/Engine.h"
#include "../components/Components.h"
#include "../utils/ConfigManager.h"
#include "../utils/ResourceManager.h"
#include <algorithm>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <iomanip>

namespace NodeEditor {

    static NodeEditorWindow* s_currentNodeEditor = nullptr;

    Node::Node(int nodeId, NodeType nodeType, const std::string& nodeName, ImVec2 pos)
        : id(nodeId), type(nodeType), name(nodeName), position(pos), size(120, 80) {
        
        // Set up pins and properties based on node type
        if (type == NodeType::Entity) {
            // Entity nodes have a single output pin that can connect to any compatible node
            outputPins.push_back({nodeId * PIN_ID_MULTIPLIER + 1, PinType::Output, PinDataType::Entity, "Entity", ImVec2()});
            printf("DEBUG: Created Entity node %d with single Entity output pin %d\n", nodeId, nodeId * PIN_ID_MULTIPLIER + 1);
            headerColor = IM_COL32(70, 130, 180, 255);
            description = "Represents a game entity with components";
        } else {
            // Setup pins and data based on specific node type
            setupBasicPins(nodeType);
            setupNodeAppearance(nodeType);
            createComponentData(nodeType);
        }
    }
    
    void Node::setupBasicPins(NodeType nodeType) {
        switch (nodeType) {
            // Component nodes
            case NodeType::SpriteComponent:
            case NodeType::Transform:
            case NodeType::Collider:
            case NodeType::RigidBody:
            case NodeType::PlayerController:
            case NodeType::PlayerStats:
            case NodeType::PlayerPhysics:
            case NodeType::PlayerInventory:
            case NodeType::PlayerAbilities:
            case NodeType::PlayerState:
                inputPins.push_back({id * PIN_ID_MULTIPLIER + 1, PinType::Input, PinDataType::Entity, "Entity", ImVec2()});
                break;
                
            // NPC and AI Component nodes
            case NodeType::NPCController:
            case NodeType::AIBehavior:
            case NodeType::AIStateMachine:
            case NodeType::AIPathfinding:
            case NodeType::NPCDialogue:
            case NodeType::NPCInteraction:
                inputPins.push_back({id * PIN_ID_MULTIPLIER + 1, PinType::Input, PinDataType::Entity, "Entity", ImVec2()});
                break;
                
            // Environment Component nodes
            case NodeType::EnvironmentCollider:
            case NodeType::EnvironmentTrigger:
            case NodeType::EnvironmentHazard:
            case NodeType::EnvironmentDoor:
            case NodeType::EnvironmentSwitch:
            case NodeType::EnvironmentPlatform:
                inputPins.push_back({id * PIN_ID_MULTIPLIER + 1, PinType::Input, PinDataType::Entity, "Entity", ImVec2()});
                break;
                
            // Audio and Effects Component nodes
            case NodeType::AudioSource:
            case NodeType::AudioListener:
            case NodeType::ParticleSystem:
            case NodeType::ParticleEmitter:
            case NodeType::VisualEffect:
            case NodeType::LightSource:
                inputPins.push_back({id * PIN_ID_MULTIPLIER + 1, PinType::Input, PinDataType::Entity, "Entity", ImVec2()});
                break;
                
            // UI Component nodes
            case NodeType::UIElement:
            case NodeType::UIButton:
            case NodeType::UIText:
            case NodeType::UIImage:
            case NodeType::UIHealthBar:
            case NodeType::UIInventorySlot:
                inputPins.push_back({id * PIN_ID_MULTIPLIER + 1, PinType::Input, PinDataType::Entity, "Entity", ImVec2()});
                break;
                
            // Math nodes
            case NodeType::MathAdd:
            case NodeType::MathSubtract:
            case NodeType::MathMultiply:
            case NodeType::MathDivide:
                inputPins.push_back({id * PIN_ID_MULTIPLIER + 1, PinType::Input, PinDataType::Float, "A", ImVec2()});
                inputPins.push_back({id * PIN_ID_MULTIPLIER + 2, PinType::Input, PinDataType::Float, "B", ImVec2()});
                outputPins.push_back({id * PIN_ID_MULTIPLIER + 3, PinType::Output, PinDataType::Float, "Result", ImVec2()});
                break;
                
            case NodeType::MathSin:
            case NodeType::MathCos:
            case NodeType::MathAbs:
                inputPins.push_back({id * PIN_ID_MULTIPLIER + 1, PinType::Input, PinDataType::Float, "Value", ImVec2()});
                outputPins.push_back({id * PIN_ID_MULTIPLIER + 2, PinType::Output, PinDataType::Float, "Result", ImVec2()});
                break;
                
            case NodeType::MathMin:
            case NodeType::MathMax:
                inputPins.push_back({id * PIN_ID_MULTIPLIER + 1, PinType::Input, PinDataType::Float, "A", ImVec2()});
                inputPins.push_back({id * PIN_ID_MULTIPLIER + 2, PinType::Input, PinDataType::Float, "B", ImVec2()});
                outputPins.push_back({id * PIN_ID_MULTIPLIER + 3, PinType::Output, PinDataType::Float, "Result", ImVec2()});
                break;
                
            case NodeType::MathDistance:
                inputPins.push_back({id * PIN_ID_MULTIPLIER + 1, PinType::Input, PinDataType::Vector2, "Point A", ImVec2()});
                inputPins.push_back({id * PIN_ID_MULTIPLIER + 2, PinType::Input, PinDataType::Vector2, "Point B", ImVec2()});
                outputPins.push_back({id * PIN_ID_MULTIPLIER + 3, PinType::Output, PinDataType::Float, "Distance", ImVec2()});
                break;
                
            case NodeType::MathNormalize:
                inputPins.push_back({id * PIN_ID_MULTIPLIER + 1, PinType::Input, PinDataType::Vector2, "Vector", ImVec2()});
                outputPins.push_back({id * PIN_ID_MULTIPLIER + 2, PinType::Output, PinDataType::Vector2, "Normalized", ImVec2()});
                break;
                
            case NodeType::MathDotProduct:
                inputPins.push_back({id * PIN_ID_MULTIPLIER + 1, PinType::Input, PinDataType::Vector2, "A", ImVec2()});
                inputPins.push_back({id * PIN_ID_MULTIPLIER + 2, PinType::Input, PinDataType::Vector2, "B", ImVec2()});
                outputPins.push_back({id * PIN_ID_MULTIPLIER + 3, PinType::Output, PinDataType::Float, "Dot Product", ImVec2()});
                break;
                
            case NodeType::RandomFloat:
                inputPins.push_back({id * PIN_ID_MULTIPLIER + 1, PinType::Input, PinDataType::Float, "Min", ImVec2()});
                inputPins.push_back({id * PIN_ID_MULTIPLIER + 2, PinType::Input, PinDataType::Float, "Max", ImVec2()});
                outputPins.push_back({id * PIN_ID_MULTIPLIER + 3, PinType::Output, PinDataType::Float, "Random", ImVec2()});
                break;
                
            case NodeType::RandomInt:
                inputPins.push_back({id * PIN_ID_MULTIPLIER + 1, PinType::Input, PinDataType::Int, "Min", ImVec2()});
                inputPins.push_back({id * PIN_ID_MULTIPLIER + 2, PinType::Input, PinDataType::Int, "Max", ImVec2()});
                outputPins.push_back({id * PIN_ID_MULTIPLIER + 3, PinType::Output, PinDataType::Int, "Random", ImVec2()});
                break;
                
            case NodeType::MathClamp:
                inputPins.push_back({id * PIN_ID_MULTIPLIER + 1, PinType::Input, PinDataType::Float, "Value", ImVec2()});
                inputPins.push_back({id * PIN_ID_MULTIPLIER + 2, PinType::Input, PinDataType::Float, "Min", ImVec2()});
                inputPins.push_back({id * PIN_ID_MULTIPLIER + 3, PinType::Input, PinDataType::Float, "Max", ImVec2()});
                outputPins.push_back({id * PIN_ID_MULTIPLIER + 4, PinType::Output, PinDataType::Float, "Result", ImVec2()});
                break;
                
            case NodeType::MathLerp:
                inputPins.push_back({id * PIN_ID_MULTIPLIER + 1, PinType::Input, PinDataType::Float, "A", ImVec2()});
                inputPins.push_back({id * PIN_ID_MULTIPLIER + 2, PinType::Input, PinDataType::Float, "B", ImVec2()});
                inputPins.push_back({id * PIN_ID_MULTIPLIER + 3, PinType::Input, PinDataType::Float, "T", ImVec2()});
                outputPins.push_back({id * PIN_ID_MULTIPLIER + 4, PinType::Output, PinDataType::Float, "Result", ImVec2()});
                break;
                
            // Logic nodes
            case NodeType::LogicAND:
            case NodeType::LogicOR:
            case NodeType::LogicXOR:
                inputPins.push_back({id * PIN_ID_MULTIPLIER + 1, PinType::Input, PinDataType::Bool, "A", ImVec2()});
                inputPins.push_back({id * PIN_ID_MULTIPLIER + 2, PinType::Input, PinDataType::Bool, "B", ImVec2()});
                outputPins.push_back({id * PIN_ID_MULTIPLIER + 3, PinType::Output, PinDataType::Bool, "Result", ImVec2()});
                break;
                
            case NodeType::LogicNOT:
                inputPins.push_back({id * PIN_ID_MULTIPLIER + 1, PinType::Input, PinDataType::Bool, "Input", ImVec2()});
                outputPins.push_back({id * PIN_ID_MULTIPLIER + 2, PinType::Output, PinDataType::Bool, "Result", ImVec2()});
                break;
                
            case NodeType::Compare:
                inputPins.push_back({id * PIN_ID_MULTIPLIER + 1, PinType::Input, PinDataType::Float, "A", ImVec2()});
                inputPins.push_back({id * PIN_ID_MULTIPLIER + 2, PinType::Input, PinDataType::Float, "B", ImVec2()});
                outputPins.push_back({id * PIN_ID_MULTIPLIER + 3, PinType::Output, PinDataType::Bool, "A > B", ImVec2()});
                outputPins.push_back({id * PIN_ID_MULTIPLIER + 4, PinType::Output, PinDataType::Bool, "A < B", ImVec2()});
                outputPins.push_back({id * PIN_ID_MULTIPLIER + 5, PinType::Output, PinDataType::Bool, "A == B", ImVec2()});
                break;
                
            case NodeType::Branch:
                inputPins.push_back({id * PIN_ID_MULTIPLIER + 1, PinType::Input, PinDataType::Event, "Exec", ImVec2()});
                inputPins.push_back({id * PIN_ID_MULTIPLIER + 2, PinType::Input, PinDataType::Bool, "Condition", ImVec2()});
                outputPins.push_back({id * PIN_ID_MULTIPLIER + 3, PinType::Output, PinDataType::Event, "True", ImVec2()});
                outputPins.push_back({id * PIN_ID_MULTIPLIER + 4, PinType::Output, PinDataType::Event, "False", ImVec2()});
                break;
                
            // Flow control nodes
            case NodeType::Sequence:
                inputPins.push_back({id * PIN_ID_MULTIPLIER + 1, PinType::Input, PinDataType::Event, "Start", ImVec2()});
                outputPins.push_back({id * PIN_ID_MULTIPLIER + 2, PinType::Output, PinDataType::Event, "Step 1", ImVec2()});
                outputPins.push_back({id * PIN_ID_MULTIPLIER + 3, PinType::Output, PinDataType::Event, "Step 2", ImVec2()});
                outputPins.push_back({id * PIN_ID_MULTIPLIER + 4, PinType::Output, PinDataType::Event, "Finished", ImVec2()});
                break;
                
            case NodeType::Parallel:
                inputPins.push_back({id * PIN_ID_MULTIPLIER + 1, PinType::Input, PinDataType::Event, "Start", ImVec2()});
                outputPins.push_back({id * PIN_ID_MULTIPLIER + 2, PinType::Output, PinDataType::Event, "Branch 1", ImVec2()});
                outputPins.push_back({id * PIN_ID_MULTIPLIER + 3, PinType::Output, PinDataType::Event, "Branch 2", ImVec2()});
                outputPins.push_back({id * PIN_ID_MULTIPLIER + 4, PinType::Output, PinDataType::Event, "Branch 3", ImVec2()});
                break;
                
            case NodeType::Delay:
                inputPins.push_back({id * PIN_ID_MULTIPLIER + 1, PinType::Input, PinDataType::Event, "Start", ImVec2()});
                inputPins.push_back({id * PIN_ID_MULTIPLIER + 2, PinType::Input, PinDataType::Float, "Duration", ImVec2()});
                outputPins.push_back({id * PIN_ID_MULTIPLIER + 3, PinType::Output, PinDataType::Event, "Finished", ImVec2()});
                break;
                
            case NodeType::ForLoop:
                inputPins.push_back({id * PIN_ID_MULTIPLIER + 1, PinType::Input, PinDataType::Event, "Start", ImVec2()});
                inputPins.push_back({id * PIN_ID_MULTIPLIER + 2, PinType::Input, PinDataType::Int, "Count", ImVec2()});
                outputPins.push_back({id * PIN_ID_MULTIPLIER + 3, PinType::Output, PinDataType::Event, "Loop Body", ImVec2()});
                outputPins.push_back({id * PIN_ID_MULTIPLIER + 4, PinType::Output, PinDataType::Int, "Index", ImVec2()});
                outputPins.push_back({id * PIN_ID_MULTIPLIER + 5, PinType::Output, PinDataType::Event, "Completed", ImVec2()});
                break;
                
            case NodeType::WhileLoop:
                inputPins.push_back({id * PIN_ID_MULTIPLIER + 1, PinType::Input, PinDataType::Event, "Start", ImVec2()});
                inputPins.push_back({id * PIN_ID_MULTIPLIER + 2, PinType::Input, PinDataType::Bool, "Condition", ImVec2()});
                outputPins.push_back({id * PIN_ID_MULTIPLIER + 3, PinType::Output, PinDataType::Event, "Loop Body", ImVec2()});
                outputPins.push_back({id * PIN_ID_MULTIPLIER + 4, PinType::Output, PinDataType::Event, "Completed", ImVec2()});
                break;
                
            // Constant nodes
            case NodeType::ConstantFloat:
                outputPins.push_back({id * PIN_ID_MULTIPLIER + 1, PinType::Output, PinDataType::Float, "Value", ImVec2()});
                outputPins[0].floatValue = 0.0f;
                break;
                
            case NodeType::ConstantInt:
                outputPins.push_back({id * PIN_ID_MULTIPLIER + 1, PinType::Output, PinDataType::Int, "Value", ImVec2()});
                outputPins[0].intValue = 0;
                break;
                
            case NodeType::ConstantBool:
                outputPins.push_back({id * PIN_ID_MULTIPLIER + 1, PinType::Output, PinDataType::Bool, "Value", ImVec2()});
                outputPins[0].boolValue = false;
                break;
                
            case NodeType::ConstantString:
                outputPins.push_back({id * PIN_ID_MULTIPLIER + 1, PinType::Output, PinDataType::String, "Value", ImVec2()});
                outputPins[0].stringValue = "";
                break;
                
            case NodeType::ConstantVector2:
                outputPins.push_back({id * PIN_ID_MULTIPLIER + 1, PinType::Output, PinDataType::Vector2, "Value", ImVec2()});
                outputPins[0].vector2Value = Vector2(0, 0);
                break;
                
            case NodeType::Variable:
                inputPins.push_back({id * PIN_ID_MULTIPLIER + 1, PinType::Input, PinDataType::Any, "Set", ImVec2()});
                outputPins.push_back({id * PIN_ID_MULTIPLIER + 2, PinType::Output, PinDataType::Any, "Get", ImVec2()});
                break;
                
            case NodeType::GlobalVariable:
                inputPins.push_back({id * PIN_ID_MULTIPLIER + 1, PinType::Input, PinDataType::Any, "Set", ImVec2()});
                outputPins.push_back({id * PIN_ID_MULTIPLIER + 2, PinType::Output, PinDataType::Any, "Get", ImVec2()});
                break;
                
            case NodeType::EntityReference:
                outputPins.push_back({id * PIN_ID_MULTIPLIER + 1, PinType::Output, PinDataType::Entity, "Entity", ImVec2()});
                break;
                
            // Event nodes
            case NodeType::OnKeyPress:
                outputPins.push_back({id * PIN_ID_MULTIPLIER + 1, PinType::Output, PinDataType::Event, "Pressed", ImVec2()});
                break;
                
            case NodeType::OnKeyRelease:
                outputPins.push_back({id * PIN_ID_MULTIPLIER + 1, PinType::Output, PinDataType::Event, "Released", ImVec2()});
                break;
                
            case NodeType::OnMouseClick:
                outputPins.push_back({id * PIN_ID_MULTIPLIER + 1, PinType::Output, PinDataType::Event, "Clicked", ImVec2()});
                outputPins.push_back({id * PIN_ID_MULTIPLIER + 2, PinType::Output, PinDataType::Vector2, "Position", ImVec2()});
                break;
                
            case NodeType::OnMouseHover:
                inputPins.push_back({id * PIN_ID_MULTIPLIER + 1, PinType::Input, PinDataType::Entity, "Entity", ImVec2()});
                outputPins.push_back({id * PIN_ID_MULTIPLIER + 2, PinType::Output, PinDataType::Event, "Enter", ImVec2()});
                outputPins.push_back({id * PIN_ID_MULTIPLIER + 3, PinType::Output, PinDataType::Event, "Exit", ImVec2()});
                break;
                
            case NodeType::OnCollision:
                inputPins.push_back({id * PIN_ID_MULTIPLIER + 1, PinType::Input, PinDataType::Entity, "Entity", ImVec2()});
                outputPins.push_back({id * PIN_ID_MULTIPLIER + 2, PinType::Output, PinDataType::Event, "OnEnter", ImVec2()});
                outputPins.push_back({id * PIN_ID_MULTIPLIER + 3, PinType::Output, PinDataType::Event, "OnExit", ImVec2()});
                outputPins.push_back({id * PIN_ID_MULTIPLIER + 4, PinType::Output, PinDataType::Entity, "Other Entity", ImVec2()});
                break;
                
            case NodeType::OnTriggerEnter:
                inputPins.push_back({id * PIN_ID_MULTIPLIER + 1, PinType::Input, PinDataType::Entity, "Trigger", ImVec2()});
                outputPins.push_back({id * PIN_ID_MULTIPLIER + 2, PinType::Output, PinDataType::Event, "Triggered", ImVec2()});
                outputPins.push_back({id * PIN_ID_MULTIPLIER + 3, PinType::Output, PinDataType::Entity, "Other Entity", ImVec2()});
                break;
                
            case NodeType::OnTriggerExit:
                inputPins.push_back({id * PIN_ID_MULTIPLIER + 1, PinType::Input, PinDataType::Entity, "Trigger", ImVec2()});
                outputPins.push_back({id * PIN_ID_MULTIPLIER + 2, PinType::Output, PinDataType::Event, "Exited", ImVec2()});
                outputPins.push_back({id * PIN_ID_MULTIPLIER + 3, PinType::Output, PinDataType::Entity, "Other Entity", ImVec2()});
                break;
                
            case NodeType::OnEntityDestroyed:
                inputPins.push_back({id * PIN_ID_MULTIPLIER + 1, PinType::Input, PinDataType::Entity, "Entity", ImVec2()});
                outputPins.push_back({id * PIN_ID_MULTIPLIER + 2, PinType::Output, PinDataType::Event, "Destroyed", ImVec2()});
                break;
                
            case NodeType::OnEntitySpawned:
                outputPins.push_back({id * PIN_ID_MULTIPLIER + 1, PinType::Output, PinDataType::Event, "Spawned", ImVec2()});
                outputPins.push_back({id * PIN_ID_MULTIPLIER + 2, PinType::Output, PinDataType::Entity, "New Entity", ImVec2()});
                break;
                
            case NodeType::TimerNode:
                inputPins.push_back({id * PIN_ID_MULTIPLIER + 1, PinType::Input, PinDataType::Float, "Duration", ImVec2()});
                inputPins.push_back({id * PIN_ID_MULTIPLIER + 2, PinType::Input, PinDataType::Bool, "Loop", ImVec2()});
                outputPins.push_back({id * PIN_ID_MULTIPLIER + 3, PinType::Output, PinDataType::Event, "Triggered", ImVec2()});
                outputPins.push_back({id * PIN_ID_MULTIPLIER + 4, PinType::Output, PinDataType::Float, "Remaining", ImVec2()});
                break;
                
            // Game action nodes
            case NodeType::MoveTowards:
                inputPins.push_back({id * PIN_ID_MULTIPLIER + 1, PinType::Input, PinDataType::Entity, "Entity", ImVec2()});
                inputPins.push_back({id * PIN_ID_MULTIPLIER + 2, PinType::Input, PinDataType::Vector2, "Target", ImVec2()});
                inputPins.push_back({id * PIN_ID_MULTIPLIER + 3, PinType::Input, PinDataType::Float, "Speed", ImVec2()});
                outputPins.push_back({id * PIN_ID_MULTIPLIER + 4, PinType::Output, PinDataType::Event, "Reached", ImVec2()});
                break;
                
            case NodeType::SpawnEntity:
                inputPins.push_back({id * PIN_ID_MULTIPLIER + 1, PinType::Input, PinDataType::Event, "Spawn", ImVec2()});
                inputPins.push_back({id * PIN_ID_MULTIPLIER + 2, PinType::Input, PinDataType::Vector2, "Position", ImVec2()});
                inputPins.push_back({id * PIN_ID_MULTIPLIER + 3, PinType::Input, PinDataType::String, "Template", ImVec2()});
                outputPins.push_back({id * PIN_ID_MULTIPLIER + 4, PinType::Output, PinDataType::Entity, "Spawned", ImVec2()});
                break;
                
            case NodeType::DestroyEntity:
                inputPins.push_back({id * PIN_ID_MULTIPLIER + 1, PinType::Input, PinDataType::Event, "Destroy", ImVec2()});
                inputPins.push_back({id * PIN_ID_MULTIPLIER + 2, PinType::Input, PinDataType::Entity, "Entity", ImVec2()});
                break;
                
            case NodeType::PlaySound:
                inputPins.push_back({id * PIN_ID_MULTIPLIER + 1, PinType::Input, PinDataType::Event, "Play", ImVec2()});
                inputPins.push_back({id * PIN_ID_MULTIPLIER + 2, PinType::Input, PinDataType::String, "Sound File", ImVec2()});
                inputPins.push_back({id * PIN_ID_MULTIPLIER + 3, PinType::Input, PinDataType::Float, "Volume", ImVec2()});
                break;
                
            case NodeType::StopSound:
                inputPins.push_back({id * PIN_ID_MULTIPLIER + 1, PinType::Input, PinDataType::Event, "Stop", ImVec2()});
                inputPins.push_back({id * PIN_ID_MULTIPLIER + 2, PinType::Input, PinDataType::String, "Sound File", ImVec2()});
                break;
                
            // Physics nodes
            case NodeType::ApplyForce:
                inputPins.push_back({id * PIN_ID_MULTIPLIER + 1, PinType::Input, PinDataType::Event, "Apply", ImVec2()});
                inputPins.push_back({id * PIN_ID_MULTIPLIER + 2, PinType::Input, PinDataType::Entity, "Entity", ImVec2()});
                inputPins.push_back({id * PIN_ID_MULTIPLIER + 3, PinType::Input, PinDataType::Vector2, "Force", ImVec2()});
                break;
                
            case NodeType::SetVelocity:
                inputPins.push_back({id * PIN_ID_MULTIPLIER + 1, PinType::Input, PinDataType::Event, "Set", ImVec2()});
                inputPins.push_back({id * PIN_ID_MULTIPLIER + 2, PinType::Input, PinDataType::Entity, "Entity", ImVec2()});
                inputPins.push_back({id * PIN_ID_MULTIPLIER + 3, PinType::Input, PinDataType::Vector2, "Velocity", ImVec2()});
                break;
                
            case NodeType::Raycast:
                inputPins.push_back({id * PIN_ID_MULTIPLIER + 1, PinType::Input, PinDataType::Vector2, "Start", ImVec2()});
                inputPins.push_back({id * PIN_ID_MULTIPLIER + 2, PinType::Input, PinDataType::Vector2, "Direction", ImVec2()});
                inputPins.push_back({id * PIN_ID_MULTIPLIER + 3, PinType::Input, PinDataType::Float, "Distance", ImVec2()});
                outputPins.push_back({id * PIN_ID_MULTIPLIER + 4, PinType::Output, PinDataType::Bool, "Hit", ImVec2()});
                outputPins.push_back({id * PIN_ID_MULTIPLIER + 5, PinType::Output, PinDataType::Vector2, "Hit Point", ImVec2()});
                outputPins.push_back({id * PIN_ID_MULTIPLIER + 6, PinType::Output, PinDataType::Entity, "Hit Entity", ImVec2()});
                break;
                
            // Game State nodes
            case NodeType::SaveGame:
                inputPins.push_back({id * PIN_ID_MULTIPLIER + 1, PinType::Input, PinDataType::Event, "Save", ImVec2()});
                inputPins.push_back({id * PIN_ID_MULTIPLIER + 2, PinType::Input, PinDataType::String, "Save Slot", ImVec2()});
                outputPins.push_back({id * PIN_ID_MULTIPLIER + 3, PinType::Output, PinDataType::Event, "Saved", ImVec2()});
                break;
                
            case NodeType::LoadGame:
                inputPins.push_back({id * PIN_ID_MULTIPLIER + 1, PinType::Input, PinDataType::Event, "Load", ImVec2()});
                inputPins.push_back({id * PIN_ID_MULTIPLIER + 2, PinType::Input, PinDataType::String, "Save Slot", ImVec2()});
                outputPins.push_back({id * PIN_ID_MULTIPLIER + 3, PinType::Output, PinDataType::Event, "Loaded", ImVec2()});
                break;
                
            case NodeType::SceneLoader:
                inputPins.push_back({id * PIN_ID_MULTIPLIER + 1, PinType::Input, PinDataType::Event, "Load", ImVec2()});
                inputPins.push_back({id * PIN_ID_MULTIPLIER + 2, PinType::Input, PinDataType::String, "Scene Name", ImVec2()});
                outputPins.push_back({id * PIN_ID_MULTIPLIER + 3, PinType::Output, PinDataType::Event, "Loaded", ImVec2()});
                break;
                
            // Health and Combat nodes
            case NodeType::DealDamage:
                inputPins.push_back({id * PIN_ID_MULTIPLIER + 1, PinType::Input, PinDataType::Event, "Deal", ImVec2()});
                inputPins.push_back({id * PIN_ID_MULTIPLIER + 2, PinType::Input, PinDataType::Entity, "Target", ImVec2()});
                inputPins.push_back({id * PIN_ID_MULTIPLIER + 3, PinType::Input, PinDataType::Float, "Damage", ImVec2()});
                outputPins.push_back({id * PIN_ID_MULTIPLIER + 4, PinType::Output, PinDataType::Event, "Dealt", ImVec2()});
                break;
                
            case NodeType::HealEntity:
                inputPins.push_back({id * PIN_ID_MULTIPLIER + 1, PinType::Input, PinDataType::Event, "Heal", ImVec2()});
                inputPins.push_back({id * PIN_ID_MULTIPLIER + 2, PinType::Input, PinDataType::Entity, "Target", ImVec2()});
                inputPins.push_back({id * PIN_ID_MULTIPLIER + 3, PinType::Input, PinDataType::Float, "Amount", ImVec2()});
                outputPins.push_back({id * PIN_ID_MULTIPLIER + 4, PinType::Output, PinDataType::Event, "Healed", ImVec2()});
                break;
                
            case NodeType::CheckHealth:
                inputPins.push_back({id * PIN_ID_MULTIPLIER + 1, PinType::Input, PinDataType::Entity, "Entity", ImVec2()});
                outputPins.push_back({id * PIN_ID_MULTIPLIER + 2, PinType::Output, PinDataType::Float, "Health", ImVec2()});
                outputPins.push_back({id * PIN_ID_MULTIPLIER + 3, PinType::Output, PinDataType::Float, "Max Health", ImVec2()});
                outputPins.push_back({id * PIN_ID_MULTIPLIER + 4, PinType::Output, PinDataType::Bool, "Is Dead", ImVec2()});
                break;
                
            // Dialogue nodes
            case NodeType::DialogueNode:
                inputPins.push_back({id * PIN_ID_MULTIPLIER + 1, PinType::Input, PinDataType::Event, "Start", ImVec2()});
                inputPins.push_back({id * PIN_ID_MULTIPLIER + 2, PinType::Input, PinDataType::String, "Text", ImVec2()});
                outputPins.push_back({id * PIN_ID_MULTIPLIER + 3, PinType::Output, PinDataType::Event, "Choice 1", ImVec2()});
                outputPins.push_back({id * PIN_ID_MULTIPLIER + 4, PinType::Output, PinDataType::Event, "Choice 2", ImVec2()});
                outputPins.push_back({id * PIN_ID_MULTIPLIER + 5, PinType::Output, PinDataType::Event, "End", ImVec2()});
                break;
                
            // Debug and utility nodes
            case NodeType::Print:
                inputPins.push_back({id * PIN_ID_MULTIPLIER + 1, PinType::Input, PinDataType::Event, "Print", ImVec2()});
                inputPins.push_back({id * PIN_ID_MULTIPLIER + 2, PinType::Input, PinDataType::Any, "Value", ImVec2()});
                outputPins.push_back({id * PIN_ID_MULTIPLIER + 3, PinType::Output, PinDataType::Event, "Printed", ImVec2()});
                break;
                
            case NodeType::Debug:
                inputPins.push_back({id * PIN_ID_MULTIPLIER + 1, PinType::Input, PinDataType::Event, "Debug", ImVec2()});
                inputPins.push_back({id * PIN_ID_MULTIPLIER + 2, PinType::Input, PinDataType::Any, "Value", ImVec2()});
                break;
                
            // Entity creation nodes
            case NodeType::EntitySpawner:
                inputPins.push_back({id * PIN_ID_MULTIPLIER + 1, PinType::Input, PinDataType::Entity, "Target", ImVec2()});     // Entity(1) - gets EntitySpawner component
                inputPins.push_back({id * PIN_ID_MULTIPLIER + 2, PinType::Input, PinDataType::Entity, "Template", ImVec2()});   // Entity(2) - serves as spawn template
                inputPins.push_back({id * PIN_ID_MULTIPLIER + 3, PinType::Input, PinDataType::Event, "Spawn", ImVec2()});       // Trigger event
                inputPins.push_back({id * PIN_ID_MULTIPLIER + 4, PinType::Input, PinDataType::Vector2, "Position", ImVec2()});   // Spawn position
                outputPins.push_back({id * PIN_ID_MULTIPLIER + 5, PinType::Output, PinDataType::Event, "Spawned", ImVec2()});
                outputPins.push_back({id * PIN_ID_MULTIPLIER + 6, PinType::Output, PinDataType::Entity, "New Entity", ImVec2()});
                printf("DEBUG: Created EntitySpawner node %d with Target pin %d and Template pin %d\n", id, id * PIN_ID_MULTIPLIER + 1, id * PIN_ID_MULTIPLIER + 2);
                break;
                
            case NodeType::EntityFactory:
                inputPins.push_back({id * PIN_ID_MULTIPLIER + 1, PinType::Input, PinDataType::Event, "Create", ImVec2()});
                inputPins.push_back({id * PIN_ID_MULTIPLIER + 2, PinType::Input, PinDataType::String, "Entity Name", ImVec2()});
                inputPins.push_back({id * PIN_ID_MULTIPLIER + 3, PinType::Input, PinDataType::Vector2, "Position", ImVec2()});
                outputPins.push_back({id * PIN_ID_MULTIPLIER + 4, PinType::Output, PinDataType::Event, "Created", ImVec2()});
                outputPins.push_back({id * PIN_ID_MULTIPLIER + 5, PinType::Output, PinDataType::Entity, "Entity", ImVec2()});
                break;
                
            case NodeType::ParticleEffect:
                inputPins.push_back({id * PIN_ID_MULTIPLIER + 1, PinType::Input, PinDataType::Event, "Trigger", ImVec2()});
                // Note: Target Entity is now selected via dropdown, not input pin
                inputPins.push_back({id * PIN_ID_MULTIPLIER + 3, PinType::Input, PinDataType::String, "Effect Type", ImVec2()});
                outputPins.push_back({id * PIN_ID_MULTIPLIER + 4, PinType::Output, PinDataType::Event, "Effect Added", ImVec2()});
                break;
                
            case NodeType::Comment:
                // Comment nodes have no pins, just text
                minSize = ImVec2(200, 100);
                break;
                
            default:
                // Generic single input/output for unhandled types
                inputPins.push_back({id * PIN_ID_MULTIPLIER + 1, PinType::Input, PinDataType::Any, "Input", ImVec2()});
                outputPins.push_back({id * PIN_ID_MULTIPLIER + 2, PinType::Output, PinDataType::Any, "Output", ImVec2()});
                break;
        }
    }    void Node::draw(ImVec2 displayPos, float zoom) {
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        
        // Use provided display position, or fallback to stored position
        ImVec2 nodePos = (displayPos.x != 0 || displayPos.y != 0) ? displayPos : position;
        ImVec2 nodeSize = ImVec2(getNodeSize().x * zoom, getNodeSize().y * zoom);
        ImVec2 nodeRectMin = nodePos;
        ImVec2 nodeRectMax = ImVec2(nodePos.x + nodeSize.x, nodePos.y + nodeSize.y);
        
        // Node background
        ImU32 nodeColor = selected ? IM_COL32(100, 150, 255, 200) : IM_COL32(60, 60, 60, 200);
        drawList->AddRectFilled(nodeRectMin, nodeRectMax, nodeColor, 5.0f * zoom);
        drawList->AddRect(nodeRectMin, nodeRectMax, IM_COL32(200, 200, 200, 255), 5.0f * zoom, 0, 2.0f * zoom);
        
        // Node title (scale text size would require font scaling, so we'll keep text readable)
        ImVec2 titlePos = ImVec2(nodePos.x + 10 * zoom, nodePos.y + 10 * zoom);
        drawList->AddText(titlePos, IM_COL32(255, 255, 255, 255), name.c_str());
        
        // Special handling for different node types
        if (type == NodeType::Entity) {
            drawEntityNodeContent(nodePos, nodeSize);
        } else if (type == NodeType::SpriteComponent) {
            drawSpriteNodeContent(nodePos, nodeSize);
        } else if (type == NodeType::Rotation) {
            drawRotationNodeContent(nodePos, nodeSize);
        } else if (type == NodeType::Scale) {
            drawScaleNodeContent(nodePos, nodeSize);
        } else if (type == NodeType::Transform) {
            drawTransformNodeContent(nodePos, nodeSize);
        } else if (type == NodeType::ParticleEffect) {
            drawParticleNodeContent(nodePos, nodeSize);
        } else if (type == NodeType::OnKeyPress) {
            drawKeyPressNodeContent(nodePos, nodeSize);
        }
        
        // Draw resize handle for selected nodes
        if (selected) {
            ImVec2 resizeHandlePos = ImVec2(nodeRectMax.x - 10 * zoom, nodeRectMax.y - 10 * zoom);
            ImVec2 resizeHandleMax = nodeRectMax;
            
            // Resize handle background
            drawList->AddTriangleFilled(
                resizeHandlePos,
                ImVec2(resizeHandleMax.x, resizeHandlePos.y),
                ImVec2(resizeHandlePos.x, resizeHandleMax.y),
                IM_COL32(150, 150, 150, 200)
            );
            
            // Resize handle border/grip lines
            drawList->AddLine(
                ImVec2(resizeHandlePos.x + 2 * zoom, resizeHandleMax.y - 2 * zoom),
                ImVec2(resizeHandleMax.x - 2 * zoom, resizeHandlePos.y + 2 * zoom),
                IM_COL32(100, 100, 100, 255), 1.0f * zoom
            );
            drawList->AddLine(
                ImVec2(resizeHandlePos.x + 5 * zoom, resizeHandleMax.y - 2 * zoom),
                ImVec2(resizeHandleMax.x - 2 * zoom, resizeHandlePos.y + 5 * zoom),
                IM_COL32(100, 100, 100, 255), 1.0f * zoom
            );
        }
        
        // Update pin positions and draw pins
        updatePinPositions(nodePos, zoom);
        drawPins(drawList, zoom);
    }

    void Node::drawEntityNodeContent(ImVec2 nodePos, ImVec2 nodeSize) {
        // Draw static text labels using ImDrawList
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        
        // Entity Selection Area (clickable)
        ImVec2 entitySelectPos = ImVec2(nodePos.x + 10, nodePos.y + 35);
        ImVec2 entitySelectSize = ImVec2(nodeSize.x - 20, 20);
        ImVec2 entitySelectMax = ImVec2(entitySelectPos.x + entitySelectSize.x, entitySelectPos.y + entitySelectSize.y);
        
        // Draw entity selection background
        ImU32 selectBgColor = IM_COL32(60, 60, 80, 255);
        drawList->AddRectFilled(entitySelectPos, entitySelectMax, selectBgColor, 3.0f);
        drawList->AddRect(entitySelectPos, entitySelectMax, IM_COL32(100, 100, 120, 255), 3.0f);
        
        // Draw entity selection text
        char entityText[64];
        if (associatedEntity != 0) {
            snprintf(entityText, sizeof(entityText), "Entity: %u", associatedEntity);
        } else {
            snprintf(entityText, sizeof(entityText), "Select Entity...");
        }
        drawList->AddText(ImVec2(entitySelectPos.x + 5, entitySelectPos.y + 3), 
                         IM_COL32(255, 255, 255, 255), entityText);
        
        // Show basic entity info if one is selected
        if (associatedEntity != 0) {
            // Show entity ID (already shown above, but add some additional info)
            char infoText[64];
            snprintf(infoText, sizeof(infoText), "ID: %u", associatedEntity);
            drawList->AddText(ImVec2(nodePos.x + 10, nodePos.y + 65), IM_COL32(200, 200, 200, 255), infoText);
            
            // Show connection status
            drawList->AddText(ImVec2(nodePos.x + 10, nodePos.y + 85), IM_COL32(150, 150, 200, 255), "Ready for connections");
        } else {
            // Show selection prompt
            drawList->AddText(ImVec2(nodePos.x + 10, nodePos.y + 65), IM_COL32(255, 200, 100, 255), "Click above to select");
        }
    }
    
    void Node::drawPins(ImDrawList* drawList, float zoom) {
        // Draw input pins
        for (auto& pin : inputPins) {
            ImU32 pinColor = pin.connected ? IM_COL32(100, 255, 100, 255) : IM_COL32(150, 150, 150, 255);
            drawList->AddCircleFilled(pin.position, 6.0f * zoom, pinColor);
            
            // Pin label
            ImVec2 labelPos = ImVec2(pin.position.x + 15 * zoom, pin.position.y - 8 * zoom);
            drawList->AddText(labelPos, IM_COL32(200, 200, 200, 255), pin.name.c_str());
        }
        
        // Draw output pins
        for (auto& pin : outputPins) {
            ImU32 pinColor = pin.connected ? IM_COL32(100, 255, 100, 255) : IM_COL32(150, 150, 150, 255);
            drawList->AddCircleFilled(pin.position, 6.0f * zoom, pinColor);
            
            // Pin label (right-aligned)
            ImVec2 textSize = ImGui::CalcTextSize(pin.name.c_str());
            ImVec2 labelPos = ImVec2(pin.position.x - textSize.x - 15 * zoom, pin.position.y - 8 * zoom);
            drawList->AddText(labelPos, IM_COL32(200, 200, 200, 255), pin.name.c_str());
        }
    }

    void Node::drawSpriteNodeContent(ImVec2 nodePos, ImVec2 nodeSize) {
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        
        // Texture preview area
        ImVec2 previewPos = ImVec2(nodePos.x + 10, nodePos.y + 30);
        ImVec2 previewSize = ImVec2(60, 60);
        ImVec2 previewMax = ImVec2(previewPos.x + previewSize.x, previewPos.y + previewSize.y);
        
        // Get the sprite component
        auto spriteComponent = std::static_pointer_cast<Sprite>(componentData);
        
        if (spriteComponent && spriteComponent->texture) {
            // Draw texture preview
            SDL_Texture* sdlTexture = spriteComponent->texture->getSDLTexture();
            if (sdlTexture) {
                // Convert SDL_Texture to ImTextureID (this is a simple cast for SDL)
                ImTextureID textureID = reinterpret_cast<ImTextureID>(sdlTexture);
                
                // Calculate aspect ratio to maintain proportions
                float textureWidth = static_cast<float>(spriteComponent->texture->getWidth());
                float textureHeight = static_cast<float>(spriteComponent->texture->getHeight());
                float aspectRatio = textureWidth / textureHeight;
                
                ImVec2 imageSize = previewSize;
                if (aspectRatio > 1.0f) {
                    imageSize.y = previewSize.x / aspectRatio;
                } else {
                    imageSize.x = previewSize.y * aspectRatio;
                }
                
                // Center the image in the preview area
                ImVec2 imagePos = ImVec2(
                    previewPos.x + (previewSize.x - imageSize.x) * 0.5f,
                    previewPos.y + (previewSize.y - imageSize.y) * 0.5f
                );
                
                drawList->AddImage(textureID, imagePos, ImVec2(imagePos.x + imageSize.x, imagePos.y + imageSize.y));
            }
        } else {
            // Draw placeholder for no texture
            drawList->AddRectFilled(previewPos, previewMax, IM_COL32(80, 80, 80, 255), 3.0f);
            drawList->AddRect(previewPos, previewMax, IM_COL32(120, 120, 120, 255), 3.0f);
            
            // Draw "No Texture" text
            ImVec2 textSize = ImGui::CalcTextSize("No\nTexture");
            ImVec2 textPos = ImVec2(
                previewPos.x + (previewSize.x - textSize.x) * 0.5f,
                previewPos.y + (previewSize.y - textSize.y) * 0.5f
            );
            drawList->AddText(textPos, IM_COL32(160, 160, 160, 255), "No\nTexture");
        }
        
        // Draw preview border
        drawList->AddRect(previewPos, previewMax, IM_COL32(200, 200, 200, 255), 3.0f, 0, 1.5f);
        
        // "Select Texture" button area
        ImVec2 buttonPos = ImVec2(nodePos.x + 80, nodePos.y + 40);
        ImVec2 buttonSize = ImVec2(100, 30);
        ImVec2 buttonMax = ImVec2(buttonPos.x + buttonSize.x, buttonPos.y + buttonSize.y);
        
        // Check if mouse is over the button area
        ImVec2 mousePos = ImGui::GetIO().MousePos;
        bool isButtonHovered = (mousePos.x >= buttonPos.x && mousePos.x <= buttonMax.x &&
                               mousePos.y >= buttonPos.y && mousePos.y <= buttonMax.y);
        
        // Draw button
        ImU32 buttonColor = isButtonHovered ? IM_COL32(70, 130, 180, 255) : IM_COL32(50, 100, 150, 255);
        drawList->AddRectFilled(buttonPos, buttonMax, buttonColor, 3.0f);
        drawList->AddRect(buttonPos, buttonMax, IM_COL32(200, 200, 200, 255), 3.0f);
        
        // Button text
        ImVec2 buttonTextSize = ImGui::CalcTextSize("Select Texture");
        ImVec2 buttonTextPos = ImVec2(
            buttonPos.x + (buttonSize.x - buttonTextSize.x) * 0.5f,
            buttonPos.y + (buttonSize.y - buttonTextSize.y) * 0.5f
        );
        drawList->AddText(buttonTextPos, IM_COL32(255, 255, 255, 255), "Select Texture");
        
        // Handle button click
        if (isButtonHovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
            ImGui::OpenPopup(("TextureSelector##" + std::to_string(id)).c_str());
        }
        
        // Texture selection popup
        if (ImGui::BeginPopup(("TextureSelector##" + std::to_string(id)).c_str())) {
            drawTextureSelectionPopup();
            ImGui::EndPopup();
        }
        
        // Display current texture filename
        if (spriteComponent && spriteComponent->texture && !spriteComponent->texture->getFilePath().empty()) {
            std::string filename = std::filesystem::path(spriteComponent->texture->getFilePath()).filename().string();
            ImVec2 filenamePos = ImVec2(nodePos.x + 10, nodePos.y + 95);
            
            // Truncate if too long
            if (filename.length() > 20) {
                filename = filename.substr(0, 17) + "...";
            }
            
            drawList->AddText(filenamePos, IM_COL32(180, 180, 180, 255), filename.c_str());
        }
    }

    void Node::drawRotationNodeContent(ImVec2 nodePos, ImVec2 nodeSize) {
        // Make sure we have component data
        if (!componentData) {
            printf("WARNING: Rotation node has no component data\n");
            return;
        }
        
        auto rotationComponent = std::static_pointer_cast<Rotation>(componentData);
        if (!rotationComponent) {
            printf("WARNING: Failed to cast rotation component data\n");
            return;
        }
        
        // Validate the node position and size
        if (nodeSize.x <= 0 || nodeSize.y <= 0) {
            printf("WARNING: Invalid node size for rotation content: %f, %f\n", nodeSize.x, nodeSize.y);
            return;
        }
        
        try {
            // Check if ImGui is initialized and ready for drawing
            ImGuiIO& io = ImGui::GetIO();
            if (!io.DisplaySize.x || !io.DisplaySize.y) {
                printf("WARNING: ImGui not properly initialized for rotation node\n");
                return;
            }
            
            // Set up ImGui for editing
            ImGui::SetCursorScreenPos(ImVec2(nodePos.x + 10, nodePos.y + 35));
            ImGui::PushItemWidth(nodeSize.x - 20);
            
            // Angle input
            float angle = rotationComponent->angle;
            std::string rotationId = "##rotation" + std::to_string(id);
            if (ImGui::SliderFloat(rotationId.c_str(), &angle, -180.0f, 180.0f, "%.1f°")) {
                rotationComponent->angle = angle;
            }
            
            ImGui::PopItemWidth();
        } catch (const std::exception& e) {
            printf("ERROR: Exception in drawRotationNodeContent: %s\n", e.what());
        }
    }

    void Node::drawScaleNodeContent(ImVec2 nodePos, ImVec2 nodeSize) {
        // Make sure we have component data
        if (!componentData) {
            printf("WARNING: Scale node has no component data\n");
            return;
        }
        
        auto scaleComponent = std::static_pointer_cast<Scale>(componentData);
        if (!scaleComponent) {
            printf("WARNING: Failed to cast scale component data\n");
            return;
        }
        
        // Validate the node position and size
        if (nodeSize.x <= 0 || nodeSize.y <= 0) {
            printf("WARNING: Invalid node size for scale content: %f, %f\n", nodeSize.x, nodeSize.y);
            return;
        }
        
        try {
            // Check if ImGui is initialized and ready for drawing
            ImGuiIO& io = ImGui::GetIO();
            if (!io.DisplaySize.x || !io.DisplaySize.y) {
                printf("WARNING: ImGui not properly initialized for scale node\n");
                return;
            }
            
            // Set up ImGui for editing
            ImGui::SetCursorScreenPos(ImVec2(nodePos.x + 10, nodePos.y + 35));
            ImGui::PushItemWidth((nodeSize.x - 30) * 0.5f);
            
            // Scale X input
            float scaleX = scaleComponent->scale.x;
            std::string scaleXId = "##scaleX" + std::to_string(id);
            if (ImGui::DragFloat(scaleXId.c_str(), &scaleX, 0.01f, 0.01f, 50.0f, "X: %.3f")) {
                scaleComponent->scale.x = scaleX;
            }
            
            ImGui::SameLine();
            
            // Scale Y input
            float scaleY = scaleComponent->scale.y;
            std::string scaleYId = "##scaleY" + std::to_string(id);
            if (ImGui::DragFloat(scaleYId.c_str(), &scaleY, 0.01f, 0.01f, 50.0f, "Y: %.3f")) {
                scaleComponent->scale.y = scaleY;
            }
            
            // Uniform scale checkbox and control
            ImGui::SetCursorScreenPos(ImVec2(nodePos.x + 10, nodePos.y + 60));
            
            // Use a per-node uniform scale state instead of static
            std::string uniformCheckboxId = "Uniform##" + std::to_string(id);
            bool uniformScale = (scaleComponent->scale.x == scaleComponent->scale.y);
            
            if (ImGui::Checkbox(uniformCheckboxId.c_str(), &uniformScale)) {
                if (uniformScale) {
                    // Set both to the same value (use X as reference)
                    scaleComponent->scale.y = scaleComponent->scale.x;
                }
            }
            
            if (uniformScale) {
                ImGui::SameLine();
                ImGui::PushItemWidth(nodeSize.x - 80);
                float uniform = scaleComponent->scale.x;
                std::string uniformDragId = "##uniform" + std::to_string(id);
                if (ImGui::DragFloat(uniformDragId.c_str(), &uniform, 0.01f, 0.1f, 5.0f, "%.2f")) {
                    scaleComponent->scale.x = uniform;
                    scaleComponent->scale.y = uniform;
                }
                ImGui::PopItemWidth();
            }
            
            ImGui::PopItemWidth();
        } catch (const std::exception& e) {
            printf("ERROR: Exception in drawScaleNodeContent: %s\n", e.what());
        }
    }

    void Node::drawTransformNodeContent(ImVec2 nodePos, ImVec2 nodeSize) {
        // Make sure we have component data
        if (!componentData) {
            return;
        }
        
        auto transformComponent = std::static_pointer_cast<Transform>(componentData);
        if (!transformComponent) {
            return;
        }
        
        // Draw static text labels using ImDrawList since ImGui controls don't work in this context
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        
        // Position labels and values
        char posText[64];
        snprintf(posText, sizeof(posText), "Pos: %.1f, %.1f", transformComponent->position.x, transformComponent->position.y);
        drawList->AddText(ImVec2(nodePos.x + 10, nodePos.y + 35), IM_COL32(200, 200, 200, 255), posText);
        
        // Scale labels and values
        char scaleText[64];
        snprintf(scaleText, sizeof(scaleText), "Scale: %.2f, %.2f", transformComponent->scale.x, transformComponent->scale.y);
        drawList->AddText(ImVec2(nodePos.x + 10, nodePos.y + 55), IM_COL32(200, 200, 200, 255), scaleText);
        
        // Rotation label and value
        char rotText[64];
        snprintf(rotText, sizeof(rotText), "Rot: %.1f°", transformComponent->rotation);
        drawList->AddText(ImVec2(nodePos.x + 10, nodePos.y + 75), IM_COL32(200, 200, 200, 255), rotText);
        
        // Add a note about editing
        drawList->AddText(ImVec2(nodePos.x + 10, nodePos.y + 100), IM_COL32(150, 150, 150, 255), "Double-click to edit");
    }

    void Node::drawParticleNodeContent(ImVec2 nodePos, ImVec2 nodeSize) {
        // Make sure we have component data
        if (!componentData) {
            return;
        }
        
        auto particleComponent = std::static_pointer_cast<ParticleEffect>(componentData);
        if (!particleComponent) {
            return;
        }
        
        // Draw static text labels using ImDrawList
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        
        // Target Entity Selection Area (clickable)
        ImVec2 entitySelectPos = ImVec2(nodePos.x + 10, nodePos.y + 35);
        ImVec2 entitySelectSize = ImVec2(nodeSize.x - 20, 20);
        ImVec2 entitySelectMax = ImVec2(entitySelectPos.x + entitySelectSize.x, entitySelectPos.y + entitySelectSize.y);
        
        // Draw entity selection background
        ImU32 selectBgColor = IM_COL32(60, 60, 80, 255);
        drawList->AddRectFilled(entitySelectPos, entitySelectMax, selectBgColor, 3.0f);
        drawList->AddRect(entitySelectPos, entitySelectMax, IM_COL32(100, 100, 120, 255), 3.0f);
        
        // Draw entity selection text
        char entityText[64];
        if (associatedEntity != 0) {
            snprintf(entityText, sizeof(entityText), "Entity: %u", associatedEntity);
        } else {
            snprintf(entityText, sizeof(entityText), "Select Entity...");
        }
        drawList->AddText(ImVec2(entitySelectPos.x + 5, entitySelectPos.y + 3), 
                         IM_COL32(255, 255, 255, 255), entityText);
        
        // Check if entity selection area is clicked
        if (ImGui::IsMouseHoveringRect(entitySelectPos, entitySelectMax)) {
            if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
                // Request entity selection popup through global NodeEditorWindow
                if (s_currentNodeEditor) {
                    s_currentNodeEditor->openEntitySelectionPopup(id);
                }
            }
        }
        
        // Emission info
        const char* shapeNames[] = {"Point", "Circle", "Box", "Cone"};
        const char* shapeName = shapeNames[static_cast<int>(particleComponent->shape)];
        
        char shapeText[64];
        snprintf(shapeText, sizeof(shapeText), "Shape: %s", shapeName);
        drawList->AddText(ImVec2(nodePos.x + 10, nodePos.y + 65), IM_COL32(255, 140, 0, 255), shapeText);
        
        // Particle count
        char countText[64];
        snprintf(countText, sizeof(countText), "Max: %d", particleComponent->maxParticles);
        drawList->AddText(ImVec2(nodePos.x + 10, nodePos.y + 85), IM_COL32(200, 200, 200, 255), countText);
        
        // Emission rate
        char rateText[64];
        snprintf(rateText, sizeof(rateText), "Rate: %.1f/s", particleComponent->emissionRate);
        drawList->AddText(ImVec2(nodePos.x + 10, nodePos.y + 105), IM_COL32(200, 200, 200, 255), rateText);
        
        // Status
        const char* status = particleComponent->isEmitting ? "Active" : "Stopped";
        ImU32 statusColor = particleComponent->isEmitting ? IM_COL32(0, 255, 0, 255) : IM_COL32(255, 100, 100, 255);
        drawList->AddText(ImVec2(nodePos.x + 10, nodePos.y + 125), statusColor, status);
    }

    void Node::drawKeyPressNodeContent(ImVec2 nodePos, ImVec2 nodeSize) {
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        
        // Key selection area (clickable)
        ImVec2 keySelectPos = ImVec2(nodePos.x + 10, nodePos.y + 35);
        ImVec2 keySelectSize = ImVec2(nodeSize.x - 20, 20);
        ImVec2 keySelectMax = ImVec2(keySelectPos.x + keySelectSize.x, keySelectPos.y + keySelectSize.y);
        
        // Draw key selection background
        ImU32 selectBgColor = IM_COL32(60, 80, 60, 255);
        drawList->AddRectFilled(keySelectPos, keySelectMax, selectBgColor, 3.0f);
        drawList->AddRect(keySelectPos, keySelectMax, IM_COL32(100, 120, 100, 255), 3.0f);
        
        // Draw key selection text
        char keyText[64];
        if (keyCode != 0) {
            // Convert key code to readable string
            const char* keyName = getKeyName(keyCode);
            snprintf(keyText, sizeof(keyText), "Key: %s", keyName);
        } else {
            snprintf(keyText, sizeof(keyText), "Select Key...");
        }
        drawList->AddText(ImVec2(keySelectPos.x + 5, keySelectPos.y + 3), 
                         IM_COL32(255, 255, 255, 255), keyText);
        
        // Check if key selection area is clicked
        if (ImGui::IsMouseHoveringRect(keySelectPos, keySelectMax)) {
            if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
                // Request key selection popup through global NodeEditorWindow
                printf("DEBUG: OnKeyPress node %d clicked, s_currentNodeEditor = %p\n", id, s_currentNodeEditor);
                if (s_currentNodeEditor) {
                    s_currentNodeEditor->openKeySelectionPopup(id);
                    printf("DEBUG: Called openKeySelectionPopup for node %d\n", id);
                } else {
                    printf("ERROR: s_currentNodeEditor is null!\n");
                }
            }
        }
        
        // Show current state
        if (keyCode != 0) {
            drawList->AddText(ImVec2(nodePos.x + 10, nodePos.y + 65), 
                             IM_COL32(200, 200, 200, 255), "Event triggers on key press");
        } else {
            drawList->AddText(ImVec2(nodePos.x + 10, nodePos.y + 65), 
                             IM_COL32(150, 150, 150, 255), "Click above to select key");
        }
    }

    bool Node::isImageFile(const std::string& extension) {
        std::string lowerExt = extension;
        std::transform(lowerExt.begin(), lowerExt.end(), lowerExt.begin(), ::tolower);
        return lowerExt == ".png" || lowerExt == ".jpg" || lowerExt == ".jpeg" || 
               lowerExt == ".bmp" || lowerExt == ".tga";
    }

    void Node::scanDirectoryForImages(const std::string& directory, std::vector<std::string>& imageFiles) {
        if (!std::filesystem::exists(directory)) return;
        
        for (const auto& entry : std::filesystem::directory_iterator(directory)) {
            if (entry.is_regular_file()) {
                std::string extension = entry.path().extension().string();
                if (isImageFile(extension)) {
                    imageFiles.push_back(directory + "/" + entry.path().filename().string());
                }
            }
        }
    }

    const char* Node::getKeyName(int keyCode) const {
        switch (keyCode) {
            case 87: return "W";
            case 65: return "A";
            case 83: return "S";
            case 68: return "D";
            case 32: return "Space";
            case 13: return "Enter";
            case 16: return "Shift";
            case 17: return "Ctrl";
            case 69: return "E";
            case 70: return "F";
            case 71: return "G";
            case 72: return "H";
            case 81: return "Q";
            case 82: return "R";
            case 84: return "T";
            case 89: return "Y";
            case 85: return "U";
            case 73: return "I";
            case 79: return "O";
            case 80: return "P";
            default: return "Unknown";
        }
    }

    void Node::drawTextureSelectionPopup() {
        ImGui::Text("Select Texture:");
        ImGui::Separator();
        
        // Get asset folder from config
        std::string assetFolder = ConfigManager::getInstance().getAssetFolder();
          // Collect image files
        std::vector<std::string> imageFiles;
        
        if (std::filesystem::exists(assetFolder)) {
            // Scan main assets folder
            scanDirectoryForImages(assetFolder, imageFiles);
            
            // Also scan tiles subfolder
            std::string tilesFolder = assetFolder + "/tiles";
            scanDirectoryForImages(tilesFolder, imageFiles);
        }
        
        if (!imageFiles.empty()) {
            ImGui::BeginChild("TextureList", ImVec2(300, 200), true);
            
            for (const std::string& fullPath : imageFiles) {
                std::string displayName = std::filesystem::path(fullPath).filename().string();
                  if (ImGui::Selectable(("🖼️ " + displayName).c_str())) {
                    // Load the selected texture
                    auto& engine = Engine::getInstance();
                    auto resourceManager = engine.getResourceManager();
                    auto texture = resourceManager->loadTexture(fullPath);
                    
                    if (texture) {
                        // Update the sprite component
                        auto spriteComponent = std::static_pointer_cast<Sprite>(componentData);
                        if (spriteComponent) {
                            spriteComponent->texture = texture;
                            spriteComponent->sourceRect = Rect(0, 0, texture->getWidth(), texture->getHeight());
                            
                            // If this node is connected to an entity and we have an active scene,
                            // immediately apply the texture to the entity
                            if (inputPins.size() > 0 && inputPins[0].connected) {
                                printf("DEBUG: Sprite texture updated, node is connected - triggering callback\n");
                                // Trigger the callback to notify the NodeEditorWindow
                                if (onComponentDataChanged) {
                                    onComponentDataChanged(this);
                                }
                            }
                        }
                    }
                    
                    ImGui::CloseCurrentPopup();
                }
            }
            
            ImGui::EndChild();
        } else {
            ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.0f, 1.0f), "📂 No images found in %s", assetFolder.c_str());
            ImGui::TextWrapped("Add .png, .jpg, .jpeg, .bmp, or .tga files to the assets folder");
        }
        
        if (ImGui::Button("Cancel")) {
            ImGui::CloseCurrentPopup();
        }
    }    bool Node::isInside(ImVec2 point) const {
        ImVec2 nodeSize = getNodeSize();
        return point.x >= position.x && point.x <= position.x + nodeSize.x &&
               point.y >= position.y && point.y <= position.y + nodeSize.y;
    }

    Pin* Node::getPinById(int pinId) {
        for (auto& pin : inputPins) {
            if (pin.id == pinId) return &pin;
        }
        for (auto& pin : outputPins) {
            if (pin.id == pinId) return &pin;
        }
        return nullptr;    }

    ImVec2 Node::getNodeSize() const {
        // Always return the actual size - constraints are handled in setSize()
        return size;
    }

    void Node::updatePinPositions(ImVec2 nodePos, float zoom) {
        ImVec2 nodeSize = ImVec2(getNodeSize().x * zoom, getNodeSize().y * zoom);
        
        float pinY = nodePos.y + 35 * zoom;
        for (auto& pin : inputPins) {
            pin.position = ImVec2(nodePos.x - 8 * zoom, pinY);
            pinY += 20 * zoom;
        }
        
        pinY = nodePos.y + 35 * zoom;
        for (auto& pin : outputPins) {
            pin.position = ImVec2(nodePos.x + nodeSize.x + 8 * zoom, pinY);
            pinY += 20 * zoom;
        }
    }

    bool Node::isOnResizeHandle(ImVec2 point) const {
        ImVec2 nodeSize = getNodeSize();
        ImVec2 resizeHandlePos = ImVec2(position.x + nodeSize.x - 10, position.y + nodeSize.y - 10);
        ImVec2 resizeHandleSize = ImVec2(10, 10);
        
        return point.x >= resizeHandlePos.x && point.x <= resizeHandlePos.x + resizeHandleSize.x &&
               point.y >= resizeHandlePos.y && point.y <= resizeHandlePos.y + resizeHandleSize.y;
    }

    void Node::setSize(ImVec2 newSize) {
        // Clamp to min/max size constraints
        size.x = std::max(minSize.x, std::min(maxSize.x, newSize.x));
        size.y = std::max(minSize.y, std::min(maxSize.y, newSize.y));
    }

    // ====== NODE EXECUTION SYSTEM ======
    void Node::execute() {
        if (executed) return;
        
        switch (type) {
            // ====== MATH NODES ======
            case NodeType::MathAdd: {
                float a = getFloatParam("A", 0.0f);
                float b = getFloatParam("B", 0.0f);
                float result = a + b;
                setFloatParam("Result", result);
                printf("DEBUG: MathAdd executed: %.2f + %.2f = %.2f\n", a, b, result);
                break;
            }
            
            case NodeType::MathSubtract: {
                float a = getFloatParam("A", 0.0f);
                float b = getFloatParam("B", 0.0f);
                float result = a - b;
                setFloatParam("Result", result);
                printf("DEBUG: MathSubtract executed: %.2f - %.2f = %.2f\n", a, b, result);
                break;
            }
            
            case NodeType::MathMultiply: {
                float a = getFloatParam("A", 1.0f);
                float b = getFloatParam("B", 1.0f);
                float result = a * b;
                setFloatParam("Result", result);
                printf("DEBUG: MathMultiply executed: %.2f * %.2f = %.2f\n", a, b, result);
                break;
            }
            
            case NodeType::MathDivide: {
                float a = getFloatParam("A", 1.0f);
                float b = getFloatParam("B", 1.0f);
                float result = (b != 0.0f) ? a / b : 0.0f;
                setFloatParam("Result", result);
                printf("DEBUG: MathDivide executed: %.2f / %.2f = %.2f\n", a, b, result);
                break;
            }
            
            case NodeType::MathMin: {
                float a = getFloatParam("A", 0.0f);
                float b = getFloatParam("B", 0.0f);
                float result = std::min(a, b);
                setFloatParam("Result", result);
                printf("DEBUG: MathMin executed: min(%.2f, %.2f) = %.2f\n", a, b, result);
                break;
            }
            
            case NodeType::MathMax: {
                float a = getFloatParam("A", 0.0f);
                float b = getFloatParam("B", 0.0f);
                float result = std::max(a, b);
                setFloatParam("Result", result);
                printf("DEBUG: MathMax executed: max(%.2f, %.2f) = %.2f\n", a, b, result);
                break;
            }
            
            case NodeType::MathSin: {
                float value = getFloatParam("Value", 0.0f);
                float result = std::sin(value);
                setFloatParam("Result", result);
                printf("DEBUG: MathSin executed: sin(%.2f) = %.2f\n", value, result);
                break;
            }
            
            case NodeType::MathCos: {
                float value = getFloatParam("Value", 0.0f);
                float result = std::cos(value);
                setFloatParam("Result", result);
                printf("DEBUG: MathCos executed: cos(%.2f) = %.2f\n", value, result);
                break;
            }
            
            case NodeType::MathAbs: {
                float value = getFloatParam("Value", 0.0f);
                float result = std::abs(value);
                setFloatParam("Result", result);
                printf("DEBUG: MathAbs executed: abs(%.2f) = %.2f\n", value, result);
                break;
            }
            
            case NodeType::MathClamp: {
                float value = getFloatParam("Value", 0.0f);
                float minVal = getFloatParam("Min", 0.0f);
                float maxVal = getFloatParam("Max", 1.0f);
                float result = std::clamp(value, minVal, maxVal);
                setFloatParam("Result", result);
                printf("DEBUG: MathClamp executed: clamp(%.2f, %.2f, %.2f) = %.2f\n", value, minVal, maxVal, result);
                break;
            }
            
            case NodeType::MathLerp: {
                float a = getFloatParam("A", 0.0f);
                float b = getFloatParam("B", 1.0f);
                float t = getFloatParam("T", 0.5f);
                float result = a + t * (b - a);
                setFloatParam("Result", result);
                printf("DEBUG: MathLerp executed: lerp(%.2f, %.2f, %.2f) = %.2f\n", a, b, t, result);
                break;
            }
            
            case NodeType::RandomFloat: {
                float minVal = getFloatParam("Min", 0.0f);
                float maxVal = getFloatParam("Max", 1.0f);
                static std::random_device rd;
                static std::mt19937 gen(rd());
                std::uniform_real_distribution<float> dis(minVal, maxVal);
                float result = dis(gen);
                setFloatParam("Random", result);
                printf("DEBUG: RandomFloat executed: random(%.2f, %.2f) = %.2f\n", minVal, maxVal, result);
                break;
            }
            
            case NodeType::RandomInt: {
                int minVal = getIntParam("Min", 0);
                int maxVal = getIntParam("Max", 100);
                static std::random_device rd;
                static std::mt19937 gen(rd());
                std::uniform_int_distribution<int> dis(minVal, maxVal);
                int result = dis(gen);
                setIntParam("Random", result);
                printf("DEBUG: RandomInt executed: random(%d, %d) = %d\n", minVal, maxVal, result);
                break;
            }
            
            // ====== LOGIC NODES ======
            case NodeType::LogicAND: {
                bool a = getBoolParam("A", false);
                bool b = getBoolParam("B", false);
                bool result = a && b;
                setBoolParam("Result", result);
                printf("DEBUG: LogicAND executed: %s AND %s = %s\n", 
                       a ? "true" : "false", b ? "true" : "false", result ? "true" : "false");
                break;
            }
            
            case NodeType::LogicOR: {
                bool a = getBoolParam("A", false);
                bool b = getBoolParam("B", false);
                bool result = a || b;
                setBoolParam("Result", result);
                printf("DEBUG: LogicOR executed: %s OR %s = %s\n", 
                       a ? "true" : "false", b ? "true" : "false", result ? "true" : "false");
                break;
            }
            
            case NodeType::LogicNOT: {
                bool input = getBoolParam("Input", false);
                bool result = !input;
                setBoolParam("Result", result);
                printf("DEBUG: LogicNOT executed: NOT %s = %s\n", 
                       input ? "true" : "false", result ? "true" : "false");
                break;
            }
            
            case NodeType::LogicXOR: {
                bool a = getBoolParam("A", false);
                bool b = getBoolParam("B", false);
                bool result = a != b;
                setBoolParam("Result", result);
                printf("DEBUG: LogicXOR executed: %s XOR %s = %s\n", 
                       a ? "true" : "false", b ? "true" : "false", result ? "true" : "false");
                break;
            }
            
            case NodeType::Compare: {
                float a = getFloatParam("A", 0.0f);
                float b = getFloatParam("B", 0.0f);
                setBoolParam("A > B", a > b);
                setBoolParam("A < B", a < b);
                setBoolParam("A == B", std::abs(a - b) < 0.001f);
                printf("DEBUG: Compare executed: A=%.2f, B=%.2f\n", a, b);
                break;
            }
            
            // ====== UTILITY NODES ======
            case NodeType::Print: {
                std::string message = getStringParam("Message", "Hello World");
                printf("PRINT NODE: %s\n", message.c_str());
                break;
            }
            
            case NodeType::Debug: {
                std::string message = getStringParam("Debug Message", "Debug output");
                printf("DEBUG NODE: %s\n", message.c_str());
                break;
            }
            
            // ====== COMPONENT NODES ======
            case NodeType::Transform:
            case NodeType::SpriteComponent:
            case NodeType::Collider:
            case NodeType::RigidBody:
            case NodeType::PlayerController:
            case NodeType::NPCController:
                printf("DEBUG: Component node %s executed (data applied to entity)\n", name.c_str());
                break;
                
            case NodeType::LightSource: {
                // Create and configure light source component
                if (componentData) {
                    auto lightSource = std::dynamic_pointer_cast<LightSource>(componentData);
                    if (lightSource) {
                        // Update light properties from node parameters
                        lightSource->intensity = getFloatParam("Intensity", 1.0f);
                        lightSource->range = getFloatParam("Range", 100.0f);
                        lightSource->color.r = static_cast<Uint8>(getFloatParam("Color.R", 255.0f));
                        lightSource->color.g = static_cast<Uint8>(getFloatParam("Color.G", 255.0f));
                        lightSource->color.b = static_cast<Uint8>(getFloatParam("Color.B", 255.0f));
                        lightSource->enabled = getBoolParam("Enabled", true);
                        lightSource->castShadows = getBoolParam("CastShadows", false);
                        lightSource->flicker = getBoolParam("Flicker", false);
                        lightSource->flickerSpeed = getFloatParam("FlickerSpeed", 5.0f);
                        
                        printf("DEBUG: LightSource executed - Intensity: %.2f, Range: %.2f, Color: (%d,%d,%d)\n",
                               lightSource->intensity, lightSource->range, 
                               lightSource->color.r, lightSource->color.g, lightSource->color.b);
                    }
                }
                break;
            }
            
            case NodeType::AudioSource: {
                // Create and configure audio source component
                if (componentData) {
                    auto audioSource = std::dynamic_pointer_cast<AudioSource>(componentData);
                    if (audioSource) {
                        // Update audio properties from node parameters
                        audioSource->audioFile = getStringParam("AudioFile", "");
                        audioSource->volume = getFloatParam("Volume", 1.0f);
                        audioSource->pitch = getFloatParam("Pitch", 1.0f);
                        audioSource->loop = getBoolParam("Loop", false);
                        audioSource->playOnStart = getBoolParam("PlayOnStart", false);
                        audioSource->is3D = getBoolParam("3D", false);
                        audioSource->minDistance = getFloatParam("MinDistance", 10.0f);
                        audioSource->maxDistance = getFloatParam("MaxDistance", 100.0f);
                        
                        printf("DEBUG: AudioSource executed - File: %s, Volume: %.2f, 3D: %s\n",
                               audioSource->audioFile.c_str(), audioSource->volume, 
                               audioSource->is3D ? "true" : "false");
                    }
                }
                break;
            }
            
            case NodeType::ParticleSystem: {
                // Create and configure particle effect component
                if (componentData) {
                    auto particleEffect = std::dynamic_pointer_cast<ParticleEffect>(componentData);
                    if (particleEffect) {
                        // Update particle properties from node parameters
                        particleEffect->isEmitting = getBoolParam("Emitting", true);
                        particleEffect->emissionRate = getFloatParam("EmissionRate", 10.0f);
                        particleEffect->maxParticles = getIntParam("MaxParticles", 100);
                        particleEffect->minLifetime = getFloatParam("MinLifetime", 1.0f);
                        particleEffect->maxLifetime = getFloatParam("MaxLifetime", 3.0f);
                        particleEffect->minSize = getFloatParam("MinSize", 1.0f);
                        particleEffect->maxSize = getFloatParam("MaxSize", 5.0f);
                        
                        printf("DEBUG: ParticleSystem executed - Rate: %.2f, Max: %d, Lifetime: %.2f-%.2f\n",
                               particleEffect->emissionRate, particleEffect->maxParticles,
                               particleEffect->minLifetime, particleEffect->maxLifetime);
                    }
                }
                break;
            }
                
            // ====== CONSTANT NODES ======
            case NodeType::ConstantFloat: {
                float value = getFloatParam("Value", 0.0f);
                setFloatParam("Output", value);
                printf("DEBUG: ConstantFloat executed: %.2f\n", value);
                break;
            }
            
            case NodeType::ConstantInt: {
                int value = getIntParam("Value", 0);
                setIntParam("Output", value);
                printf("DEBUG: ConstantInt executed: %d\n", value);
                break;
            }
            
            case NodeType::ConstantBool: {
                bool value = getBoolParam("Value", false);
                setBoolParam("Output", value);
                printf("DEBUG: ConstantBool executed: %s\n", value ? "true" : "false");
                break;
            }
            
            case NodeType::ConstantString: {
                std::string value = getStringParam("Value", "");
                setStringParam("Output", value);
                printf("DEBUG: ConstantString executed: %s\n", value.c_str());
                break;
            }
            
            // ====== VECTOR MATH NODES ======
            case NodeType::ConstantVector2: {
                float x = getFloatParam("X", 0.0f);
                float y = getFloatParam("Y", 0.0f);
                setFloatParam("Vector.X", x);
                setFloatParam("Vector.Y", y);
                printf("DEBUG: ConstantVector2 executed: (%.2f, %.2f)\n", x, y);
                break;
            }
            
            case NodeType::MathDistance: {
                float ax = getFloatParam("A.X", 0.0f);
                float ay = getFloatParam("A.Y", 0.0f);
                float bx = getFloatParam("B.X", 0.0f);
                float by = getFloatParam("B.Y", 0.0f);
                float dx = ax - bx;
                float dy = ay - by;
                float distance = std::sqrt(dx*dx + dy*dy);
                setFloatParam("Distance", distance);
                printf("DEBUG: MathDistance executed: distance((%.2f,%.2f), (%.2f,%.2f)) = %.2f\n", 
                       ax, ay, bx, by, distance);
                break;
            }
            
            case NodeType::MathNormalize: {
                float x = getFloatParam("Vector.X", 0.0f);
                float y = getFloatParam("Vector.Y", 0.0f);
                float length = std::sqrt(x*x + y*y);
                if (length > 0.0f) {
                    setFloatParam("Result.X", x / length);
                    setFloatParam("Result.Y", y / length);
                } else {
                    setFloatParam("Result.X", 0.0f);
                    setFloatParam("Result.Y", 0.0f);
                }
                printf("DEBUG: MathNormalize executed: normalize(%.2f,%.2f)\n", x, y);
                break;
            }
            
            case NodeType::MathDotProduct: {
                float ax = getFloatParam("A.X", 0.0f);
                float ay = getFloatParam("A.Y", 0.0f);
                float bx = getFloatParam("B.X", 0.0f);
                float by = getFloatParam("B.Y", 0.0f);
                float dot = ax * bx + ay * by;
                setFloatParam("DotProduct", dot);
                printf("DEBUG: MathDotProduct executed: dot((%.2f,%.2f), (%.2f,%.2f)) = %.2f\n", 
                       ax, ay, bx, by, dot);
                break;
            }
            
            // ====== CONDITIONAL NODES ======
            case NodeType::Branch: {
                bool condition = getBoolParam("Condition", false);
                if (condition) {
                    setBoolParam("True", true);
                    setBoolParam("False", false);
                } else {
                    setBoolParam("True", false);
                    setBoolParam("False", true);
                }
                printf("DEBUG: Branch executed: condition=%s\n", condition ? "true" : "false");
                break;
            }
            
            case NodeType::Condition: {
                float a = getFloatParam("A", 0.0f);
                float b = getFloatParam("B", 0.0f);
                std::string op = getStringParam("Operator", "==");
                bool result = false;
                
                if (op == "==") result = std::abs(a - b) < 0.001f;
                else if (op == "!=") result = std::abs(a - b) >= 0.001f;
                else if (op == ">") result = a > b;
                else if (op == ">=") result = a >= b;
                else if (op == "<") result = a < b;
                else if (op == "<=") result = a <= b;
                
                setBoolParam("Result", result);
                printf("DEBUG: Condition executed: %.2f %s %.2f = %s\n", 
                       a, op.c_str(), b, result ? "true" : "false");
                break;
            }
            
            // ====== GAME LOGIC NODES ======
            case NodeType::DealDamage: {
                float damage = getFloatParam("Damage", 10.0f);
                int targetEntity = getIntParam("Target", 0);
                printf("DEBUG: DealDamage executed: %d damage to entity %d\n", (int)damage, targetEntity);
                // In a real implementation, this would modify the target entity's health component
                break;
            }
            
            case NodeType::HealEntity: {
                float healing = getFloatParam("Healing", 10.0f);
                int targetEntity = getIntParam("Target", 0);
                printf("DEBUG: HealEntity executed: %d healing to entity %d\n", (int)healing, targetEntity);
                // In a real implementation, this would modify the target entity's health component
                break;
            }
            
            case NodeType::CheckHealth: {
                int targetEntity = getIntParam("Target", 0);
                // In a real implementation, this would check the entity's health component
                float currentHealth = 100.0f; // Placeholder
                setFloatParam("CurrentHealth", currentHealth);
                setBoolParam("IsAlive", currentHealth > 0.0f);
                printf("DEBUG: CheckHealth executed: entity %d has %.0f health\n", targetEntity, currentHealth);
                break;
            }
            
            // ====== TIMER AND DELAY NODES ======
            case NodeType::TimerNode: {
                float duration = getFloatParam("Duration", 1.0f);
                float elapsed = getFloatParam("Elapsed", 0.0f);
                elapsed += 0.016f; // Assume 60 FPS for now
                setFloatParam("Elapsed", elapsed);
                setBoolParam("Finished", elapsed >= duration);
                printf("DEBUG: TimerNode executed: %.2f/%.2f seconds\n", elapsed, duration);
                break;
            }
            
            case NodeType::Delay: {
                float delayTime = getFloatParam("Delay", 1.0f);
                float startTime = getFloatParam("StartTime", ImGui::GetTime());
                float currentTime = ImGui::GetTime();
                bool finished = (currentTime - startTime) >= delayTime;
                setBoolParam("Finished", finished);
                if (finished) {
                    printf("DEBUG: Delay executed: delay of %.2f seconds completed\n", delayTime);
                }
                break;
            }
            
            // ====== INPUT NODES ======
            case NodeType::OnKeyPress: {
                int keyCode = getIntParam("KeyCode", 0);
                // In a real implementation, this would check actual input state
                bool pressed = false; // Placeholder
                setBoolParam("Pressed", pressed);
                if (pressed) {
                    printf("DEBUG: OnKeyPress executed: key %d pressed\n", keyCode);
                }
                break;
            }
            
            case NodeType::OnMouseClick: {
                int button = getIntParam("Button", 0); // 0=left, 1=right, 2=middle
                // In a real implementation, this would check actual mouse state
                bool clicked = false; // Placeholder
                setBoolParam("Clicked", clicked);
                if (clicked) {
                    printf("DEBUG: OnMouseClick executed: button %d clicked\n", button);
                }
                break;
            }
            
            case NodeType::MathPower: {
                float base = getFloatParam("Base", 2.0f);
                float exponent = getFloatParam("Exponent", 2.0f);
                float result = std::pow(base, exponent);
                setFloatParam("Result", result);
                printf("DEBUG: MathPower executed: %.2f^%.2f = %.2f\n", base, exponent, result);
                break;
            }
            
            case NodeType::Switch: {
                int selector = getIntParam("Selector", 0);
                float input0 = getFloatParam("Input0", 0.0f);
                float input1 = getFloatParam("Input1", 0.0f);
                float input2 = getFloatParam("Input2", 0.0f);
                float input3 = getFloatParam("Input3", 0.0f);
                
                float result = input0;
                switch (selector) {
                    case 0: result = input0; break;
                    case 1: result = input1; break;
                    case 2: result = input2; break;
                    case 3: result = input3; break;
                }
                setFloatParam("Output", result);
                printf("DEBUG: Switch executed: selector=%d, output=%.2f\n", selector, result);
                break;
            }
            
            // ====== MOVEMENT AND ANIMATION NODES ======
            case NodeType::MoveTowards: {
                float currentX = getFloatParam("Current.X", 0.0f);
                float currentY = getFloatParam("Current.Y", 0.0f);
                float targetX = getFloatParam("Target.X", 0.0f);
                float targetY = getFloatParam("Target.Y", 0.0f);
                float speed = getFloatParam("Speed", 1.0f);
                float deltaTime = getFloatParam("DeltaTime", 0.016f);
                
                float dx = targetX - currentX;
                float dy = targetY - currentY;
                float distance = std::sqrt(dx*dx + dy*dy);
                
                if (distance > 0.0f) {
                    float moveDistance = speed * deltaTime;
                    if (moveDistance >= distance) {
                        // Reached target
                        setFloatParam("Result.X", targetX);
                        setFloatParam("Result.Y", targetY);
                        setBoolParam("ReachedTarget", true);
                    } else {
                        // Move towards target
                        float nx = dx / distance;
                        float ny = dy / distance;
                        setFloatParam("Result.X", currentX + nx * moveDistance);
                        setFloatParam("Result.Y", currentY + ny * moveDistance);
                        setBoolParam("ReachedTarget", false);
                    }
                }
                printf("DEBUG: MoveTowards executed: moving from (%.2f,%.2f) to (%.2f,%.2f)\n", 
                       currentX, currentY, targetX, targetY);
                break;
            }
            
            case NodeType::SpawnEntity: {
                float x = getFloatParam("Position.X", 0.0f);
                float y = getFloatParam("Position.Y", 0.0f);
                int templateId = getIntParam("TemplateID", 0);
                // In a real implementation, this would create a new entity
                printf("DEBUG: SpawnEntity executed: spawning entity at (%.2f,%.2f) with template %d\n", 
                       x, y, templateId);
                setIntParam("SpawnedEntityID", 999); // Placeholder
                break;
            }
            
            case NodeType::DestroyEntity: {
                int entityId = getIntParam("EntityID", 0);
                // In a real implementation, this would destroy the entity
                printf("DEBUG: DestroyEntity executed: destroying entity %d\n", entityId);
                setBoolParam("Destroyed", true);
                break;
            }
            
            // ====== SEQUENCE AND FLOW CONTROL ======
            case NodeType::Sequence: {
                int currentStep = getIntParam("CurrentStep", 0);
                int maxSteps = getIntParam("MaxSteps", 3);
                bool stepTriggered = getBoolParam("StepTrigger", false);
                
                if (stepTriggered) {
                    currentStep++;
                    if (currentStep >= maxSteps) {
                        currentStep = 0;
                    }
                    setIntParam("CurrentStep", currentStep);
                    setBoolParam("Step" + std::to_string(currentStep), true);
                    printf("DEBUG: Sequence executed: step %d of %d\n", currentStep, maxSteps);
                }
                break;
            }
            
            case NodeType::PlaySound: {
                std::string audioFile = getStringParam("AudioFile", "");
                float volume = getFloatParam("Volume", 1.0f);
                bool loop = getBoolParam("Loop", false);
                int entityId = getIntParam("EntityID", 0);
                
                if (!audioFile.empty()) {
                    printf("DEBUG: PlaySound executed - File: %s, Volume: %.2f, Entity: %d\n",
                           audioFile.c_str(), volume, entityId);
                    // In a real implementation, this would call the AudioSystem
                    setBoolParam("Playing", true);
                }
                break;
            }
            
            case NodeType::StopSound: {
                int entityId = getIntParam("EntityID", 0);
                printf("DEBUG: StopSound executed - Entity: %d\n", entityId);
                // In a real implementation, this would call the AudioSystem
                setBoolParam("Stopped", true);
                break;
            }
            
            case NodeType::Animate: {
                float startValue = getFloatParam("StartValue", 0.0f);
                float endValue = getFloatParam("EndValue", 1.0f);
                float duration = getFloatParam("Duration", 1.0f);
                float elapsed = getFloatParam("ElapsedTime", 0.0f);
                
                elapsed += 0.016f; // Assume 60 FPS
                float progress = std::min(elapsed / duration, 1.0f);
                float currentValue = startValue + (endValue - startValue) * progress;
                
                setFloatParam("ElapsedTime", elapsed);
                setFloatParam("CurrentValue", currentValue);
                setBoolParam("Finished", progress >= 1.0f);
                
                printf("DEBUG: Animate executed - Progress: %.2f%%, Value: %.2f\n", 
                       progress * 100.0f, currentValue);
                break;
            }
                
            default:
                printf("DEBUG: Node type %d execution not implemented yet\n", static_cast<int>(type));
                break;
        }
        
        executed = true;
        executionTime = ImGui::GetTime();
    }

    bool Node::canExecute() const {
        // Check if all required input pins have data or connections
        for (const auto& pin : inputPins) {
            if (pin.type == PinType::Input && !pin.connected) {
                // For some nodes, unconnected inputs are OK (they use default values)
                switch (type) {
                    case NodeType::ConstantFloat:
                    case NodeType::ConstantInt:
                    case NodeType::ConstantBool:
                    case NodeType::ConstantString:
                    case NodeType::Print:
                    case NodeType::Debug:
                        continue; // These don't need input connections
                    default:
                        // For most nodes, we can execute with default values
                        continue;
                }
            }
        }
        return true;
    }

    void Node::reset() {
        executed = false;
        executionTime = 0.0f;
    }

    // Parameter accessors
    void Node::setFloatParam(const std::string& name, float value) {
        floatParams[name] = value;
    }

    void Node::setIntParam(const std::string& name, int value) {
        intParams[name] = value;
    }

    void Node::setBoolParam(const std::string& name, bool value) {
        boolParams[name] = value;
    }

    void Node::setStringParam(const std::string& name, const std::string& value) {
        stringParams[name] = value;
    }

    float Node::getFloatParam(const std::string& name, float defaultValue) const {
        auto it = floatParams.find(name);
        return (it != floatParams.end()) ? it->second : defaultValue;
    }

    int Node::getIntParam(const std::string& name, int defaultValue) const {
        auto it = intParams.find(name);
        return (it != intParams.end()) ? it->second : defaultValue;
    }

    bool Node::getBoolParam(const std::string& name, bool defaultValue) const {
        auto it = boolParams.find(name);
        return (it != boolParams.end()) ? it->second : defaultValue;
    }

    std::string Node::getStringParam(const std::string& name, const std::string& defaultValue) const {
        auto it = stringParams.find(name);
        return (it != stringParams.end()) ? it->second : defaultValue;
    }

    NodeEditorWindow::NodeEditorWindow() {
    }

    NodeEditorWindow::~NodeEditorWindow() {
    }

    void NodeEditorWindow::show(bool* open, SceneWindow* activeScene) {
        m_activeScene = activeScene;
        s_currentNodeEditor = this;  // Set global pointer for Node access
        
        ImGui::SetNextWindowSize(ImVec2(800, 600), ImGuiCond_FirstUseEver);
        if (!ImGui::Begin("🔗 Node Editor", open, ImGuiWindowFlags_MenuBar)) {
            ImGui::End();
            return;
        }
        
        // Menu bar
        if (ImGui::BeginMenuBar()) {
            if (ImGui::BeginMenu("Create")) {
                drawNodeCreationMenu();
                ImGui::EndMenu();
            }
              if (ImGui::BeginMenu("Entity")) {
                if (ImGui::MenuItem("Apply to Selected Entity") && m_activeScene && m_activeScene->hasSelectedEntity()) {
                    applyNodesToEntity(m_activeScene->getSelectedEntity(), m_activeScene->getScene().get());
                }
                if (ImGui::MenuItem("Load Selected Entity") && m_activeScene && m_activeScene->hasSelectedEntity()) {
                    loadEntityAsNodes(m_activeScene->getSelectedEntity(), m_activeScene->getScene().get());
                }
                ImGui::Separator();
                if (ImGui::MenuItem("Clear All Nodes")) {
                    m_nodes.clear();
                    m_connections.clear();
                    m_nodeMap.clear();
                }
                ImGui::EndMenu();
            }
            
            if (ImGui::BeginMenu("Code")) {
                if (ImGui::MenuItem("Generate Game Code")) {
                    generateCodeFromNodes();
                }
                ImGui::Separator();
                if (ImGui::MenuItem("Export Node Graph")) {
                    exportNodeGraphAsCode();
                }
                ImGui::EndMenu();
            }
            
            if (ImGui::BeginMenu("Execute")) {
                if (ImGui::MenuItem("Execute Logic Graph")) {
                    executeLogicGraph();
                }
                if (ImGui::MenuItem("Reset All Nodes")) {
                    resetExecution();
                }
                ImGui::EndMenu();
            }
            
            ImGui::EndMenuBar();
        }
        
        // Canvas
        m_canvasPos = ImGui::GetCursorScreenPos();
        m_canvasSize = ImGui::GetContentRegionAvail();
        
        ImGui::InvisibleButton("canvas", m_canvasSize, ImGuiButtonFlags_MouseButtonLeft | ImGuiButtonFlags_MouseButtonRight);
        
        handleInput();
        
        // Set appropriate cursor based on current state
        if (m_resizing) {
            ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeNWSE);
        } else {
            // Check if hovering over a resize handle
            ImGuiIO& io = ImGui::GetIO();
            ImVec2 mousePos = io.MousePos;
            bool hoveringResizeHandle = false;
            
            if (ImGui::IsItemHovered()) {
                for (auto& node : m_nodes) {
                    if (node->selected) {
                        ImVec2 relativePos = ImVec2((mousePos.x - m_canvasPos.x - m_scrolling.x) / m_zoom, 
                                                   (mousePos.y - m_canvasPos.y - m_scrolling.y) / m_zoom);
                        if (node->isOnResizeHandle(relativePos)) {
                            hoveringResizeHandle = true;
                            break;
                        }
                    }
                }
            }
            
            if (hoveringResizeHandle) {
                ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeNWSE);
            }
        }
        
        drawGrid();
        drawNodes();
        drawConnections();
        drawConnectionInProgress();
        
        // Handle pending popup requests
        if (m_showKeySelectionPopup) {
            printf("DEBUG: Opening key selection popup for node %d\n", m_keySelectionNodeId);
            ImGui::OpenPopup("Select Key");
            m_showKeySelectionPopup = false;
        }
        
        // Handle transform edit popups
        for (auto& node : m_nodes) {
            if (node->type == NodeType::Transform) {
                std::string popupId = "EditTransform##" + std::to_string(node->id);
                if (ImGui::BeginPopup(popupId.c_str())) {
                    drawTransformEditPopup(node.get());
                    ImGui::EndPopup();
                }
            }
        }
        
        // Entity selection popup for ParticleEffect nodes
        if (ImGui::BeginPopup("Select Entity")) {
            ImGui::Text("Select Target Entity:");
            ImGui::Separator();
            
            // Option to clear selection
            if (ImGui::Selectable("None")) {
                if (m_entitySelectionNodeId != -1) {
                    auto nodeIt = std::find_if(m_nodes.begin(), m_nodes.end(),
                                             [this](const std::unique_ptr<Node>& node) {
                                                 return node->id == m_entitySelectionNodeId;
                                             });
                    if (nodeIt != m_nodes.end()) {
                        (*nodeIt)->associatedEntity = 0;
                    }
                }
                ImGui::CloseCurrentPopup();
            }
            
            // List all entities in the scene
            auto entities = getSceneEntities();
            for (const auto& entityPair : entities) {
                EntityID entityId = entityPair.first;
                const std::string& entityName = entityPair.second;
                
                if (ImGui::Selectable(entityName.c_str())) {
                    if (m_entitySelectionNodeId != -1) {
                        auto nodeIt = std::find_if(m_nodes.begin(), m_nodes.end(),
                                                 [this](const std::unique_ptr<Node>& node) {
                                                     return node->id == m_entitySelectionNodeId;
                                                 });
                        if (nodeIt != m_nodes.end()) {
                            (*nodeIt)->associatedEntity = entityId;
                        }
                    }
                    ImGui::CloseCurrentPopup();
                }
            }
            
            ImGui::EndPopup();
        }
        
        // Key selection popup for OnKeyPress nodes
        if (ImGui::BeginPopup("Select Key")) {
            printf("DEBUG: Key selection popup is being drawn\n");
            ImGui::Text("Select Key:");
            ImGui::Separator();
            
            // Common keys
            const struct { const char* name; int code; } commonKeys[] = {
                {"W", 87}, {"A", 65}, {"S", 83}, {"D", 68},
                {"Space", 32}, {"Enter", 13}, {"Shift", 16}, {"Ctrl", 17},
                {"E", 69}, {"F", 70}, {"G", 71}, {"H", 72},
                {"Q", 81}, {"R", 82}, {"T", 84}, {"Y", 89},
                {"U", 85}, {"I", 73}, {"O", 79}, {"P", 80}
            };
            
            for (const auto& key : commonKeys) {
                if (ImGui::Selectable(key.name)) {
                    printf("DEBUG: Key %s selected for node %d\n", key.name, m_keySelectionNodeId);
                    if (m_keySelectionNodeId != -1) {
                        auto nodeIt = std::find_if(m_nodes.begin(), m_nodes.end(),
                                                 [this](const std::unique_ptr<Node>& node) {
                                                     return node->id == m_keySelectionNodeId;
                                                 });
                        if (nodeIt != m_nodes.end()) {
                            (*nodeIt)->keyCode = key.code;
                            printf("DEBUG: Set keyCode %d on node %d\n", key.code, m_keySelectionNodeId);
                        }
                    }
                    ImGui::CloseCurrentPopup();
                }
            }
            
            ImGui::EndPopup();
        }
        
        ImGui::End();
    }

    void NodeEditorWindow::drawGrid() {
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        ImVec2 canvasMin = m_canvasPos;
        ImVec2 canvasMax = ImVec2(m_canvasPos.x + m_canvasSize.x, m_canvasPos.y + m_canvasSize.y);
        
        drawList->PushClipRect(canvasMin, canvasMax, true);
        
        // Grid
        const float gridStep = 50.0f * m_zoom;
        const ImU32 gridColor = IM_COL32(100, 100, 100, 40);
        
        for (float x = fmod(m_scrolling.x, gridStep); x < m_canvasSize.x; x += gridStep) {
            drawList->AddLine(ImVec2(canvasMin.x + x, canvasMin.y), 
                             ImVec2(canvasMin.x + x, canvasMax.y), gridColor);
        }
        
        for (float y = fmod(m_scrolling.y, gridStep); y < m_canvasSize.y; y += gridStep) {
            drawList->AddLine(ImVec2(canvasMin.x, canvasMin.y + y), 
                             ImVec2(canvasMax.x, canvasMin.y + y), gridColor);
        }
        
        drawList->PopClipRect();
    }    void NodeEditorWindow::drawNodes() {
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        drawList->PushClipRect(m_canvasPos, ImVec2(m_canvasPos.x + m_canvasSize.x, m_canvasPos.y + m_canvasSize.y), true);
        
        for (auto& node : m_nodes) {
            ImGui::PushID(node->id);
            
            // Calculate display position for this node
            ImVec2 displayPos = ImVec2(m_canvasPos.x + (node->position.x * m_zoom) + m_scrolling.x, 
                                      m_canvasPos.y + (node->position.y * m_zoom) + m_scrolling.y);
            
            // Draw the node at the display position without modifying the node's actual position
            node->draw(displayPos, m_zoom);
            
            ImGui::PopID();
        }
        
        drawList->PopClipRect();
    }

    void NodeEditorWindow::drawConnections() {
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        
        for (const auto& connection : m_connections) {
            Node* outputNode = nullptr;
            Node* inputNode = nullptr;
            Pin* outputPin = nullptr;
            Pin* inputPin = nullptr;
            
            // Find the nodes and pins for this connection
            for (auto& node : m_nodes) {
                if (auto pin = node->getPinById(connection.outputPinId)) {
                    outputNode = node.get();
                    outputPin = pin;
                }
                if (auto pin = node->getPinById(connection.inputPinId)) {
                    inputNode = node.get();
                    inputPin = pin;
                }
            }
            
            if (outputPin && inputPin) {
                ImVec2 p1 = outputPin->position;
                ImVec2 p2 = inputPin->position;
                
                // Bezier curve for the connection
                ImVec2 cp1 = ImVec2(p1.x + 50 * m_zoom, p1.y);
                ImVec2 cp2 = ImVec2(p2.x - 50 * m_zoom, p2.y);
                
                drawList->AddBezierCubic(p1, cp1, cp2, p2, IM_COL32(200, 200, 100, 255), 3.0f * m_zoom);
            }
        }
    }

    void NodeEditorWindow::drawConnectionInProgress() {
        if (m_creatingConnection) {
            ImDrawList* drawList = ImGui::GetWindowDrawList();
            
            // Find the start pin
            Pin* startPin = nullptr;
            for (auto& node : m_nodes) {
                if (auto pin = node->getPinById(m_connectionStartPinId)) {
                    startPin = pin;
                    break;
                }
            }
            
            if (startPin) {
                ImVec2 p1 = startPin->position;
                ImVec2 p2 = m_connectionEndPos;
                
                ImVec2 cp1 = ImVec2(p1.x + 50 * m_zoom, p1.y);
                ImVec2 cp2 = ImVec2(p2.x - 50 * m_zoom, p2.y);
                
                drawList->AddBezierCubic(p1, cp1, cp2, p2, IM_COL32(255, 255, 100, 200), 2.0f * m_zoom);
            }
        }
    }

    void NodeEditorWindow::drawTransformEditPopup(Node* node) {
        if (!node || !node->componentData) return;
        
        auto transformComponent = std::static_pointer_cast<Transform>(node->componentData);
        if (!transformComponent) return;
        
        ImGui::Text("Edit Transform Parameters");
        ImGui::Separator();
        
        // Position controls
        ImGui::Text("Position:");
        ImGui::DragFloat("X##pos", &transformComponent->position.x, 1.0f, -10000.0f, 10000.0f, "%.1f");
        ImGui::DragFloat("Y##pos", &transformComponent->position.y, 1.0f, -10000.0f, 10000.0f, "%.1f");
        
        ImGui::Separator();
        
        // Scale controls
        ImGui::Text("Scale:");
        ImGui::DragFloat("X##scale", &transformComponent->scale.x, 0.01f, 0.01f, 50.0f, "%.3f");
        ImGui::DragFloat("Y##scale", &transformComponent->scale.y, 0.01f, 0.01f, 50.0f, "%.3f");
        
        ImGui::Separator();
        
        // Rotation control
        ImGui::Text("Rotation:");
        ImGui::SliderFloat("Angle##rot", &transformComponent->rotation, -180.0f, 180.0f, "%.1f°");
        
        ImGui::Separator();
        
        if (ImGui::Button("Apply to Entity")) {
            if (m_activeScene && m_activeScene->hasSelectedEntity()) {
                applyComponentToEntity(m_activeScene->getSelectedEntity(), m_activeScene->getScene().get(), node);
            }
            ImGui::CloseCurrentPopup();
        }
        
        ImGui::SameLine();
        
        if (ImGui::Button("Close")) {
            ImGui::CloseCurrentPopup();
        }
    }

    void NodeEditorWindow::handleInput() {
        ImGuiIO& io = ImGui::GetIO();
        ImVec2 mousePos = io.MousePos;
        bool clickedOnPin = false;
        bool clickedOnConnection = false;
        
        if (ImGui::IsItemHovered()) {
            // Handle zoom with mouse wheel
            if (io.MouseWheel != 0.0f) {
                float zoomFactor = 0.1f;
                float oldZoom = m_zoom;
                
                // Update zoom
                m_zoom += io.MouseWheel * zoomFactor;
                m_zoom = std::max(m_minZoom, std::min(m_maxZoom, m_zoom));
                
                // Only adjust scrolling if zoom actually changed
                if (m_zoom != oldZoom) {
                    // Get mouse position relative to canvas
                    ImVec2 mouseCanvasPos = ImVec2(mousePos.x - m_canvasPos.x, mousePos.y - m_canvasPos.y);
                    
                    // Calculate the zoom change ratio
                    float zoomRatio = m_zoom / oldZoom;
                    
                    // Adjust scrolling to keep the point under the mouse cursor stationary
                    m_scrolling.x = mouseCanvasPos.x - (mouseCanvasPos.x - m_scrolling.x) * zoomRatio;
                    m_scrolling.y = mouseCanvasPos.y - (mouseCanvasPos.y - m_scrolling.y) * zoomRatio;
                }
            }
            
            // Check for pin clicks first
            for (auto& node : m_nodes) {
                // Check input pins
                for (auto& pin : node->inputPins) {
                    float distance = sqrt(pow(mousePos.x - pin.position.x, 2) + pow(mousePos.y - pin.position.y, 2));
                    if (distance <= 10.0f * m_zoom) {
                        if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
                            if (m_creatingConnection && canConnect(m_connectionStartPinId, pin.id)) {
                                // Complete connection
                                createConnection(m_connectionStartPinId, pin.id);
                                m_creatingConnection = false;
                                m_connectionStartPinId = -1;
                            }
                            clickedOnPin = true;
                        }
                        break;
                    }
                }
                
                // Check output pins
                for (auto& pin : node->outputPins) {
                    float distance = sqrt(pow(mousePos.x - pin.position.x, 2) + pow(mousePos.y - pin.position.y, 2));
                    if (distance <= 10.0f * m_zoom) {
                        if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
                            // Start connection
                            m_creatingConnection = true;
                            m_connectionStartPinId = pin.id;
                            m_connectionEndPos = mousePos;
                            clickedOnPin = true;
                        }
                        break;
                    }
                }
                
                if (clickedOnPin) break;
            }
            
            // Update connection end position while dragging
            if (m_creatingConnection) {
                m_connectionEndPos = mousePos;
                
                // Cancel connection on right-click or escape
                if (ImGui::IsMouseClicked(ImGuiMouseButton_Right) || ImGui::IsKeyPressed(ImGuiKey_Escape)) {
                    m_creatingConnection = false;
                    m_connectionStartPinId = -1;
                }
            }              // Check for right-click on connections (delete connection)
            if (ImGui::IsMouseClicked(ImGuiMouseButton_Right) && !m_creatingConnection) {
                // Check if hovering over a connection
                for (const auto& connection : m_connections) {
                    Node* outputNode = nullptr;
                    Node* inputNode = nullptr;
                    Pin* outputPin = nullptr;
                    Pin* inputPin = nullptr;
                    
                    // Find the nodes and pins for this connection
                    for (auto& node : m_nodes) {
                        if (auto pin = node->getPinById(connection.outputPinId)) {
                            outputNode = node.get();
                            outputPin = pin;
                        }
                        if (auto pin = node->getPinById(connection.inputPinId)) {
                            inputNode = node.get();
                            inputPin = pin;
                        }
                    }
                    
                    if (outputPin && inputPin) {
                        // Check if mouse is near the connection line
                        ImVec2 p1 = outputPin->position;
                        ImVec2 p2 = inputPin->position;
                          // Simple distance check to connection line
                        float distanceToLine = 0.0f;
                        ImVec2 lineDir = ImVec2(p2.x - p1.x, p2.y - p1.y);
                        float lineLength = sqrt(lineDir.x * lineDir.x + lineDir.y * lineDir.y);
                        
                        if (lineLength > 0) {
                            lineDir.x /= lineLength;
                            lineDir.y /= lineLength;
                            
                            ImVec2 mouseToStart = ImVec2(mousePos.x - p1.x, mousePos.y - p1.y);
                            float projLength = mouseToStart.x * lineDir.x + mouseToStart.y * lineDir.y;
                            projLength = std::max(0.0f, std::min(lineLength, projLength));
                            
                            ImVec2 closestPoint = ImVec2(p1.x + lineDir.x * projLength, p1.y + lineDir.y * projLength);
                            distanceToLine = sqrt(pow(mousePos.x - closestPoint.x, 2) + pow(mousePos.y - closestPoint.y, 2));
                        }
                        
                        if (distanceToLine <= 10.0f) {
                            deleteConnection(connection.id);
                            clickedOnConnection = true;
                            break;
                        }
                    }
                }
                
                // Right-click to create nodes (if not clicking on connection)
                if (!clickedOnConnection) {
                    ImGui::OpenPopup("CreateNode");
                }
            }
            
            // Handle node dragging and resizing
            if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !clickedOnPin) {
                // Check if clicking on a node or resize handle
                bool clickedOnNode = false;
                bool clickedOnResizeHandle = false;
                bool clickedOnNodeControl = false;
                
                for (auto& node : m_nodes) {
                    ImVec2 relativePos = ImVec2((mousePos.x - m_canvasPos.x - m_scrolling.x) / m_zoom, 
                                               (mousePos.y - m_canvasPos.y - m_scrolling.y) / m_zoom);
                    
                    if (node->isInside(relativePos)) {
                        clickedOnNode = true;
                        
                        // For Transform, Scale, and Rotation nodes, check if clicking in the control area
                        if (node->type == NodeType::Transform || node->type == NodeType::Scale || node->type == NodeType::Rotation) {
                            // Control area starts at y + 30 (below the title)
                            ImVec2 nodeSize = node->getNodeSize();
                            if (relativePos.y > node->position.y + 30 && 
                                relativePos.y < node->position.y + nodeSize.y - 10 &&
                                relativePos.x > node->position.x + 5 && 
                                relativePos.x < node->position.x + nodeSize.x - 5) {
                                clickedOnNodeControl = true;
                                
                                // Check for double-click to open edit popup
                                if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
                                    if (node->type == NodeType::Transform) {
                                        ImGui::OpenPopup(("EditTransform##" + std::to_string(node->id)).c_str());
                                    }
                                }
                            }
                        }
                        
                        // For Entity nodes, check if clicking in the entity selection area
                        if (node->type == NodeType::Entity) {
                            // Entity selection area: 10px from left, 35px from top, width-20px wide, 20px high
                            ImVec2 entitySelectPos = ImVec2(node->position.x + 10, node->position.y + 35);
                            ImVec2 entitySelectSize = ImVec2(node->getNodeSize().x - 20, 20);
                            ImVec2 entitySelectMax = ImVec2(entitySelectPos.x + entitySelectSize.x, entitySelectPos.y + entitySelectSize.y);
                            
                            if (relativePos.x >= entitySelectPos.x && relativePos.x <= entitySelectMax.x &&
                                relativePos.y >= entitySelectPos.y && relativePos.y <= entitySelectMax.y) {
                                clickedOnNodeControl = true;
                                
                                // Open entity selection popup
                                openEntitySelectionPopup(node->id);
                            }
                        }
                        
                        // Check if clicking on resize handle for already selected nodes
                        if (node->selected && node->isOnResizeHandle(relativePos)) {
                            m_resizing = true;
                            m_resizingNodeId = node->id;
                            m_resizeStartPos = relativePos;
                            m_resizeStartSize = node->getNodeSize();
                            clickedOnResizeHandle = true;
                        } else if (!clickedOnNodeControl) {
                            // Only start dragging if not clicking on controls
                            selectNode(node->id);
                            m_dragging = true;
                            m_draggedNodeId = node->id;
                            m_dragOffset = ImVec2(relativePos.x - node->position.x, relativePos.y - node->position.y);
                        } else {
                            // Just select the node if clicking on controls
                            selectNode(node->id);
                        }
                        break;
                    }
                }
                
                if (!clickedOnNode) {
                    clearSelection();
                }
            }
            
            // Canvas dragging (middle mouse or left mouse when not on node and not resizing)
            if (ImGui::IsMouseDragging(ImGuiMouseButton_Middle) || 
                (ImGui::IsMouseDragging(ImGuiMouseButton_Left) && !m_dragging && !m_creatingConnection && !m_resizing)) {
                // Only block canvas panning if actively editing a control (not just hovering)
                if (!ImGui::IsAnyItemActive()) {
                    m_scrolling.x += io.MouseDelta.x;
                    m_scrolling.y += io.MouseDelta.y;
                }
            }
        }
        
        // Handle node dragging
        if (m_dragging && ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
            for (auto& node : m_nodes) {
                if (node->id == m_draggedNodeId) {
                    ImVec2 relativePos = ImVec2((mousePos.x - m_canvasPos.x - m_scrolling.x) / m_zoom, 
                                               (mousePos.y - m_canvasPos.y - m_scrolling.y) / m_zoom);
                    node->position = ImVec2(relativePos.x - m_dragOffset.x, relativePos.y - m_dragOffset.y);
                    break;
                }
            }
        }
        
        // Handle node resizing
        if (m_resizing && ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
            for (auto& node : m_nodes) {
                if (node->id == m_resizingNodeId) {
                    ImVec2 relativePos = ImVec2((mousePos.x - m_canvasPos.x - m_scrolling.x) / m_zoom, 
                                               (mousePos.y - m_canvasPos.y - m_scrolling.y) / m_zoom);
                    
                    // Calculate new size based on mouse movement
                    ImVec2 sizeDelta = ImVec2(relativePos.x - m_resizeStartPos.x, relativePos.y - m_resizeStartPos.y);
                    ImVec2 newSize = ImVec2(m_resizeStartSize.x + sizeDelta.x, m_resizeStartSize.y + sizeDelta.y);
                    
                    // Apply the new size with constraints
                    node->setSize(newSize);
                    break;
                }
            }
        }
        
        if (ImGui::IsMouseReleased(ImGuiMouseButton_Left)) {
            m_dragging = false;
            m_draggedNodeId = -1;
            m_resizing = false;
            m_resizingNodeId = -1;
        }        
        // Create node popup
        if (ImGui::BeginPopup("CreateNode")) {
            drawNodeCreationMenu();
            ImGui::EndPopup();
        }
    }    int NodeEditorWindow::createConnection(int outputPinId, int inputPinId) {
        if (!canConnect(outputPinId, inputPinId)) return -1;
        
        int connectionId = m_nextConnectionId++;
        m_connections.emplace_back(connectionId, outputPinId, inputPinId);
        
        // Mark pins as connected
        for (auto& node : m_nodes) {
            if (auto pin = node->getPinById(outputPinId)) {
                pin->connected = true;
                pin->connectedPinId = inputPinId;
            }
            if (auto pin = node->getPinById(inputPinId)) {
                pin->connected = true;
                pin->connectedPinId = outputPinId;
            }
        }
          // Automatically apply the component to the entity when connected
        Node* outputNode = nullptr;
        Node* inputNode = nullptr;
        
        for (auto& node : m_nodes) {
            if (auto pin = node->getPinById(outputPinId)) {
                outputNode = node.get();
            }
            if (auto pin = node->getPinById(inputPinId)) {
                inputNode = node.get();
            }
        }
        
        // If we have an active scene and this is a valid entity-component connection,
        // automatically apply the component to the entity
        if (m_activeScene && outputNode && inputNode && 
            outputNode->type == NodeType::Entity && 
            inputNode->componentData) {
            
            // If the entity node doesn't have an associated entity yet, 
            // try to use the currently selected entity
            if (outputNode->associatedEntity == 0 && m_activeScene->hasSelectedEntity()) {
                outputNode->associatedEntity = m_activeScene->getSelectedEntity();
            }
            
            if (outputNode->associatedEntity != 0) {
                EntityID entity = outputNode->associatedEntity;
                Scene* scene = m_activeScene->getScene().get();
                  if (scene) {
                    // Apply the component using the helper method
                    applyComponentToEntity(entity, scene, inputNode);
                }
            }
        }
        
        // Handle Entity->EntitySpawner connections
        if (m_activeScene && outputNode && inputNode && 
            outputNode->type == NodeType::Entity && 
            inputNode->type == NodeType::EntitySpawner) {
            
            EntityID entityId = outputNode->associatedEntity;
            
            if (entityId != 0) {
                Scene* scene = m_activeScene->getScene().get();
                if (scene) {
                    // Find which pin was connected to determine behavior
                    Pin* connectedInputPin = nullptr;
                    for (auto& pin : inputNode->inputPins) {
                        if (pin.id == inputPinId) {
                            connectedInputPin = &pin;
                            break;
                        }
                    }
                    
                    if (connectedInputPin && connectedInputPin->name == "Target") {
                        // Entity connected to Target pin - this entity gains EntitySpawner component (spawning ability)
                        if (!scene->hasComponent<EntitySpawner>(entityId)) {
                            EntitySpawner spawner;
                            spawner.clearTemplates(); // Start with no templates
                            scene->addComponent<EntitySpawner>(entityId, spawner);
                            printf("DEBUG: Added EntitySpawner component to entity %u (gained spawning ability)\n", entityId);
                        }
                        
                    } else if (connectedInputPin && connectedInputPin->name == "Template") {
                        // Entity connected to Template pin - this entity becomes the spawn template
                        printf("DEBUG: Entity %u connected as template for EntitySpawner node %d\n", entityId, inputNode->id);
                        
                        // The EntitySpawner node now uses this entity as its template
                        inputNode->templateEntityId = entityId;
                        
                        // Create template info for the EntitySpawner node
                        std::string templateName = scene->getEntityName(entityId);
                        if (templateName.empty()) {
                            templateName = "Entity_" + std::to_string(entityId);
                        }
                        
                        printf("DEBUG: EntitySpawner will use Entity %u (%s) as spawn template\n", 
                               entityId, templateName.c_str());
                    }
                }
            }
        }
        
        return connectionId;
    }

    void NodeEditorWindow::deleteConnection(int connectionId) {
        // Find the connection to delete
        auto connectionIt = std::find_if(m_connections.begin(), m_connections.end(),
            [connectionId](const Connection& conn) { return conn.id == connectionId; });
        
        if (connectionIt != m_connections.end()) {
            // Get the connection details before deleting
            int outputPinId = connectionIt->outputPinId;
            int inputPinId = connectionIt->inputPinId;
            
            // Find the nodes and component type
            Node* outputNode = nullptr;
            Node* inputNode = nullptr;
            
            for (auto& node : m_nodes) {
                if (auto pin = node->getPinById(outputPinId)) {
                    outputNode = node.get();
                }
                if (auto pin = node->getPinById(inputPinId)) {
                    inputNode = node.get();
                }
            }
            
            // If we have an active scene and this is a valid entity-component connection,
            // remove the component from the entity
            if (m_activeScene && outputNode && inputNode && 
                outputNode->type == NodeType::Entity && 
                outputNode->associatedEntity != 0) {
                
                EntityID entity = outputNode->associatedEntity;
                Scene* scene = m_activeScene->getScene().get();
                  if (scene) {
                    // Remove the component using the helper method
                    removeComponentFromEntity(entity, scene, inputNode->type);
                }
            }
            
            // Mark pins as disconnected
            for (auto& node : m_nodes) {
                if (auto pin = node->getPinById(outputPinId)) {
                    pin->connected = false;
                    pin->connectedPinId = -1;
                }
                if (auto pin = node->getPinById(inputPinId)) {
                    pin->connected = false;
                    pin->connectedPinId = -1;
                }
            }
            
            // Remove the connection
            m_connections.erase(connectionIt);
        }
    }

    void NodeEditorWindow::drawNodeCreationMenu() {
        ImVec2 mousePos = ImGui::GetIO().MousePos;
        ImVec2 nodePos = ImVec2((mousePos.x - m_canvasPos.x - m_scrolling.x) / m_zoom, 
                               (mousePos.y - m_canvasPos.y - m_scrolling.y) / m_zoom);
        
        if (ImGui::MenuItem("Entity Node")) {
            createNode(NodeType::Entity, nodePos);
            ImGui::CloseCurrentPopup();
        }
        
        ImGui::Separator();
        
        if (ImGui::MenuItem("Sprite Component")) {
            createNode(NodeType::SpriteComponent, nodePos);
            ImGui::CloseCurrentPopup();
        }
        
        if (ImGui::MenuItem("Transform")) {
            createNode(NodeType::Transform, nodePos);
            ImGui::CloseCurrentPopup();
        }
        
        if (ImGui::MenuItem("Rotation")) {
            createNode(NodeType::Rotation, nodePos);
            ImGui::CloseCurrentPopup();
        }
        
        if (ImGui::MenuItem("Scale")) {
            createNode(NodeType::Scale, nodePos);
            ImGui::CloseCurrentPopup();
        }
        
        if (ImGui::MenuItem("Collider")) {
            createNode(NodeType::Collider, nodePos);
            ImGui::CloseCurrentPopup();
        }
        
        if (ImGui::MenuItem("RigidBody")) {
            createNode(NodeType::RigidBody, nodePos);
            ImGui::CloseCurrentPopup();
        }
        
        ImGui::Separator();
        
        if (ImGui::BeginMenu("Entity Management")) {
            if (ImGui::MenuItem("Entity Spawner")) {
                createNode(NodeType::EntitySpawner, nodePos);
                ImGui::CloseCurrentPopup();
            }
            if (ImGui::MenuItem("Entity Factory")) {
                createNode(NodeType::EntityFactory, nodePos);
                ImGui::CloseCurrentPopup();
            }
            if (ImGui::MenuItem("Spawn Entity")) {
                createNode(NodeType::SpawnEntity, nodePos);
                ImGui::CloseCurrentPopup();
            }
            if (ImGui::MenuItem("Destroy Entity")) {
                createNode(NodeType::DestroyEntity, nodePos);
                ImGui::CloseCurrentPopup();
            }
            if (ImGui::MenuItem("Entity Reference")) {
                createNode(NodeType::EntityReference, nodePos);
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndMenu();
        }
        
        if (ImGui::BeginMenu("Events & Input")) {
            if (ImGui::BeginMenu("Keyboard Events")) {
                if (ImGui::MenuItem("OnKeyPress")) {
                    createNode(NodeType::OnKeyPress, nodePos);
                    ImGui::CloseCurrentPopup();
                }
                if (ImGui::MenuItem("OnKeyRelease")) {
                    createNode(NodeType::OnKeyRelease, nodePos);
                    ImGui::CloseCurrentPopup();
                }
                ImGui::EndMenu();
            }
            
            if (ImGui::BeginMenu("Mouse Events")) {
                if (ImGui::MenuItem("OnMouseClick")) {
                    createNode(NodeType::OnMouseClick, nodePos);
                    ImGui::CloseCurrentPopup();
                }
                if (ImGui::MenuItem("OnMouseHover")) {
                    createNode(NodeType::OnMouseHover, nodePos);
                    ImGui::CloseCurrentPopup();
                }
                ImGui::EndMenu();
            }
            
            if (ImGui::BeginMenu("Collision Events")) {
                if (ImGui::MenuItem("OnCollision")) {
                    createNode(NodeType::OnCollision, nodePos);
                    ImGui::CloseCurrentPopup();
                }
                if (ImGui::MenuItem("OnTriggerEnter")) {
                    createNode(NodeType::OnTriggerEnter, nodePos);
                    ImGui::CloseCurrentPopup();
                }
                if (ImGui::MenuItem("OnTriggerExit")) {
                    createNode(NodeType::OnTriggerExit, nodePos);
                    ImGui::CloseCurrentPopup();
                }
                ImGui::EndMenu();
            }
            
            if (ImGui::BeginMenu("Entity Events")) {
                if (ImGui::MenuItem("OnEntitySpawned")) {
                    createNode(NodeType::OnEntitySpawned, nodePos);
                    ImGui::CloseCurrentPopup();
                }
                if (ImGui::MenuItem("OnEntityDestroyed")) {
                    createNode(NodeType::OnEntityDestroyed, nodePos);
                    ImGui::CloseCurrentPopup();
                }
                ImGui::EndMenu();
            }
            
            if (ImGui::MenuItem("Timer")) {
                createNode(NodeType::TimerNode, nodePos);
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndMenu();
        }
        
        if (ImGui::BeginMenu("Player Components")) {
            if (ImGui::MenuItem("Player Controller")) {
                createNode(NodeType::PlayerController, nodePos);
                ImGui::CloseCurrentPopup();
            }
            if (ImGui::MenuItem("Player Stats")) {
                createNode(NodeType::PlayerStats, nodePos);
                ImGui::CloseCurrentPopup();
            }
            if (ImGui::MenuItem("Player Physics")) {
                createNode(NodeType::PlayerPhysics, nodePos);
                ImGui::CloseCurrentPopup();
            }
            if (ImGui::MenuItem("Player Inventory")) {
                createNode(NodeType::PlayerInventory, nodePos);
                ImGui::CloseCurrentPopup();
            }
            if (ImGui::MenuItem("Player Abilities")) {
                createNode(NodeType::PlayerAbilities, nodePos);
                ImGui::CloseCurrentPopup();
            }
            if (ImGui::MenuItem("Player State")) {
                createNode(NodeType::PlayerState, nodePos);
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndMenu();
        }
        
        if (ImGui::BeginMenu("NPC & AI")) {
            if (ImGui::BeginMenu("NPC Components")) {
                if (ImGui::MenuItem("NPC Controller")) {
                    createNode(NodeType::NPCController, nodePos);
                    ImGui::CloseCurrentPopup();
                }
                if (ImGui::MenuItem("NPC Dialogue")) {
                    createNode(NodeType::NPCDialogue, nodePos);
                    ImGui::CloseCurrentPopup();    
                }
                if (ImGui::MenuItem("NPC Interaction")) {
                    createNode(NodeType::NPCInteraction, nodePos);
                    ImGui::CloseCurrentPopup();
                }
                ImGui::EndMenu();
            }
            
            if (ImGui::BeginMenu("AI Components")) {
                if (ImGui::MenuItem("AI Behavior")) {
                    createNode(NodeType::AIBehavior, nodePos);
                    ImGui::CloseCurrentPopup();
                }
                if (ImGui::MenuItem("AI State Machine")) {
                    createNode(NodeType::AIStateMachine, nodePos);
                    ImGui::CloseCurrentPopup();
                }
                if (ImGui::MenuItem("AI Pathfinding")) {
                    createNode(NodeType::AIPathfinding, nodePos);
                    ImGui::CloseCurrentPopup();
                }
                ImGui::EndMenu();
            }
            ImGui::EndMenu();
        }
        
        if (ImGui::BeginMenu("Environment")) {
            if (ImGui::MenuItem("Environment Collider")) {
                createNode(NodeType::EnvironmentCollider, nodePos);
                ImGui::CloseCurrentPopup();
            }
            if (ImGui::MenuItem("Environment Trigger")) {
                createNode(NodeType::EnvironmentTrigger, nodePos);
                ImGui::CloseCurrentPopup();
            }
            if (ImGui::MenuItem("Environment Hazard")) {
                createNode(NodeType::EnvironmentHazard, nodePos);
                ImGui::CloseCurrentPopup();
            }
            if (ImGui::MenuItem("Environment Door")) {
                createNode(NodeType::EnvironmentDoor, nodePos);
                ImGui::CloseCurrentPopup();
            }
            if (ImGui::MenuItem("Environment Switch")) {
                createNode(NodeType::EnvironmentSwitch, nodePos);
                ImGui::CloseCurrentPopup();
            }
            if (ImGui::MenuItem("Environment Platform")) {
                createNode(NodeType::EnvironmentPlatform, nodePos);
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndMenu();
        }
        
        if (ImGui::BeginMenu("Audio & Effects")) {
            if (ImGui::BeginMenu("Audio")) {
                if (ImGui::MenuItem("Audio Source")) {
                    createNode(NodeType::AudioSource, nodePos);
                    ImGui::CloseCurrentPopup();
                }
                if (ImGui::MenuItem("Audio Listener")) {
                    createNode(NodeType::AudioListener, nodePos);
                    ImGui::CloseCurrentPopup();
                }
                if (ImGui::MenuItem("Play Sound")) {
                    createNode(NodeType::PlaySound, nodePos);
                    ImGui::CloseCurrentPopup();
                }
                if (ImGui::MenuItem("Stop Sound")) {
                    createNode(NodeType::StopSound, nodePos);
                    ImGui::CloseCurrentPopup();
                }
                ImGui::EndMenu();
            }
            
            if (ImGui::BeginMenu("Visual Effects")) {
                if (ImGui::MenuItem("Particle System")) {
                    createNode(NodeType::ParticleSystem, nodePos);
                    ImGui::CloseCurrentPopup();
                }
                if (ImGui::MenuItem("Particle Emitter")) {
                    createNode(NodeType::ParticleEmitter, nodePos);
                    ImGui::CloseCurrentPopup();
                }
                if (ImGui::MenuItem("Visual Effect")) {
                    createNode(NodeType::VisualEffect, nodePos);
                    ImGui::CloseCurrentPopup();
                }
                if (ImGui::MenuItem("Light Source")) {
                    createNode(NodeType::LightSource, nodePos);
                    ImGui::CloseCurrentPopup();
                }
                ImGui::EndMenu();
            }
            ImGui::EndMenu();
        }
        
        if (ImGui::BeginMenu("UI Elements")) {
            if (ImGui::MenuItem("UI Element")) {
                createNode(NodeType::UIElement, nodePos);
                ImGui::CloseCurrentPopup();
            }
            if (ImGui::MenuItem("UI Button")) {
                createNode(NodeType::UIButton, nodePos);
                ImGui::CloseCurrentPopup();
            }
            if (ImGui::MenuItem("UI Text")) {
                createNode(NodeType::UIText, nodePos);
                ImGui::CloseCurrentPopup();
            }
            if (ImGui::MenuItem("UI Image")) {
                createNode(NodeType::UIImage, nodePos);
                ImGui::CloseCurrentPopup();
            }
            if (ImGui::MenuItem("UI Health Bar")) {
                createNode(NodeType::UIHealthBar, nodePos);
                ImGui::CloseCurrentPopup();
            }
            if (ImGui::MenuItem("UI Inventory Slot")) {
                createNode(NodeType::UIInventorySlot, nodePos);
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndMenu();
        }
        
        if (ImGui::BeginMenu("Math & Logic")) {
            if (ImGui::BeginMenu("Math Operations")) {
                if (ImGui::MenuItem("Add")) {
                    createNode(NodeType::MathAdd, nodePos);
                    ImGui::CloseCurrentPopup();
                }
                if (ImGui::MenuItem("Subtract")) {
                    createNode(NodeType::MathSubtract, nodePos);
                    ImGui::CloseCurrentPopup();
                }
                if (ImGui::MenuItem("Multiply")) {
                    createNode(NodeType::MathMultiply, nodePos);
                    ImGui::CloseCurrentPopup();
                }
                if (ImGui::MenuItem("Divide")) {
                    createNode(NodeType::MathDivide, nodePos);
                    ImGui::CloseCurrentPopup();
                }
                if (ImGui::MenuItem("Min")) {
                    createNode(NodeType::MathMin, nodePos);
                    ImGui::CloseCurrentPopup();
                }
                if (ImGui::MenuItem("Max")) {
                    createNode(NodeType::MathMax, nodePos);
                    ImGui::CloseCurrentPopup();
                }
                if (ImGui::MenuItem("Clamp")) {
                    createNode(NodeType::MathClamp, nodePos);
                    ImGui::CloseCurrentPopup();
                }
                if (ImGui::MenuItem("Lerp")) {
                    createNode(NodeType::MathLerp, nodePos);
                    ImGui::CloseCurrentPopup();
                }
                ImGui::EndMenu();
            }
            
            if (ImGui::BeginMenu("Vector Math")) {
                if (ImGui::MenuItem("Distance")) {
                    createNode(NodeType::MathDistance, nodePos);
                    ImGui::CloseCurrentPopup();
                }
                if (ImGui::MenuItem("Normalize")) {
                    createNode(NodeType::MathNormalize, nodePos);
                    ImGui::CloseCurrentPopup();
                }
                if (ImGui::MenuItem("Dot Product")) {
                    createNode(NodeType::MathDotProduct, nodePos);
                    ImGui::CloseCurrentPopup();
                }
                ImGui::EndMenu();
            }
            
            if (ImGui::BeginMenu("Random")) {
                if (ImGui::MenuItem("Random Float")) {
                    createNode(NodeType::RandomFloat, nodePos);
                    ImGui::CloseCurrentPopup();
                }
                if (ImGui::MenuItem("Random Int")) {
                    createNode(NodeType::RandomInt, nodePos);
                    ImGui::CloseCurrentPopup();
                }
                ImGui::EndMenu();
            }
            
            if (ImGui::BeginMenu("Logic")) {
                if (ImGui::MenuItem("AND")) {
                    createNode(NodeType::LogicAND, nodePos);
                    ImGui::CloseCurrentPopup();
                }
                if (ImGui::MenuItem("OR")) {
                    createNode(NodeType::LogicOR, nodePos);
                    ImGui::CloseCurrentPopup();
                }
                if (ImGui::MenuItem("NOT")) {
                    createNode(NodeType::LogicNOT, nodePos);
                    ImGui::CloseCurrentPopup();
                }
                if (ImGui::MenuItem("Compare")) {
                    createNode(NodeType::Compare, nodePos);
                    ImGui::CloseCurrentPopup();
                }
                ImGui::EndMenu();
            }
            ImGui::EndMenu();
        }
        
        if (ImGui::BeginMenu("Flow Control")) {
            if (ImGui::MenuItem("Branch")) {
                createNode(NodeType::Branch, nodePos);
                ImGui::CloseCurrentPopup();
            }
            if (ImGui::MenuItem("Sequence")) {
                createNode(NodeType::Sequence, nodePos);
                ImGui::CloseCurrentPopup();
            }
            if (ImGui::MenuItem("Parallel")) {
                createNode(NodeType::Parallel, nodePos);
                ImGui::CloseCurrentPopup();
            }
            if (ImGui::MenuItem("Delay")) {
                createNode(NodeType::Delay, nodePos);
                ImGui::CloseCurrentPopup();
            }
            if (ImGui::MenuItem("For Loop")) {
                createNode(NodeType::ForLoop, nodePos);
                ImGui::CloseCurrentPopup();
            }
            if (ImGui::MenuItem("While Loop")) {
                createNode(NodeType::WhileLoop, nodePos);
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndMenu();
        }
        
        if (ImGui::BeginMenu("Data")) {
            if (ImGui::BeginMenu("Constants")) {
                if (ImGui::MenuItem("Float")) {
                    createNode(NodeType::ConstantFloat, nodePos);
                    ImGui::CloseCurrentPopup();
                }
                if (ImGui::MenuItem("Int")) {
                    createNode(NodeType::ConstantInt, nodePos);
                    ImGui::CloseCurrentPopup();
                }
                if (ImGui::MenuItem("Bool")) {
                    createNode(NodeType::ConstantBool, nodePos);
                    ImGui::CloseCurrentPopup();
                }
                if (ImGui::MenuItem("String")) {
                    createNode(NodeType::ConstantString, nodePos);
                    ImGui::CloseCurrentPopup();
                }
                if (ImGui::MenuItem("Vector2")) {
                    createNode(NodeType::ConstantVector2, nodePos);
                    ImGui::CloseCurrentPopup();
                }
                ImGui::EndMenu();
            }
            
            if (ImGui::MenuItem("Variable")) {
                createNode(NodeType::Variable, nodePos);
                ImGui::CloseCurrentPopup();
            }
            if (ImGui::MenuItem("Global Variable")) {
                createNode(NodeType::GlobalVariable, nodePos);
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndMenu();
        }
        
        if (ImGui::BeginMenu("Physics")) {
            if (ImGui::MenuItem("Apply Force")) {
                createNode(NodeType::ApplyForce, nodePos);
                ImGui::CloseCurrentPopup();
            }
            if (ImGui::MenuItem("Set Velocity")) {
                createNode(NodeType::SetVelocity, nodePos);
                ImGui::CloseCurrentPopup();
            }
            if (ImGui::MenuItem("Raycast")) {
                createNode(NodeType::Raycast, nodePos);
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndMenu();
        }
        
        if (ImGui::BeginMenu("Game Systems")) {
            if (ImGui::BeginMenu("Game State")) {
                if (ImGui::MenuItem("Save Game")) {
                    createNode(NodeType::SaveGame, nodePos);
                    ImGui::CloseCurrentPopup();
                }
                if (ImGui::MenuItem("Load Game")) {
                    createNode(NodeType::LoadGame, nodePos);
                    ImGui::CloseCurrentPopup();
                }
                if (ImGui::MenuItem("Scene Loader")) {
                    createNode(NodeType::SceneLoader, nodePos);
                    ImGui::CloseCurrentPopup();
                }
                ImGui::EndMenu();
            }
            
            if (ImGui::BeginMenu("Combat & Health")) {
                if (ImGui::MenuItem("Deal Damage")) {
                    createNode(NodeType::DealDamage, nodePos);
                    ImGui::CloseCurrentPopup();
                }
                if (ImGui::MenuItem("Heal Entity")) {
                    createNode(NodeType::HealEntity, nodePos);
                    ImGui::CloseCurrentPopup();
                }
                if (ImGui::MenuItem("Check Health")) {
                    createNode(NodeType::CheckHealth, nodePos);
                    ImGui::CloseCurrentPopup();
                }
                ImGui::EndMenu();
            }
            
            if (ImGui::BeginMenu("Dialogue")) {
                if (ImGui::MenuItem("Dialogue Node")) {
                    createNode(NodeType::DialogueNode, nodePos);
                    ImGui::CloseCurrentPopup();
                }
                ImGui::EndMenu();
            }
            ImGui::EndMenu();
        }
        
        if (ImGui::BeginMenu("Movement")) {
            if (ImGui::MenuItem("Move Towards")) {
                createNode(NodeType::MoveTowards, nodePos);
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndMenu();
        }
        
        if (ImGui::BeginMenu("Debug")) {
            if (ImGui::MenuItem("Print")) {
                createNode(NodeType::Print, nodePos);
                ImGui::CloseCurrentPopup();
            }
            if (ImGui::MenuItem("Debug")) {
                createNode(NodeType::Debug, nodePos);
                ImGui::CloseCurrentPopup();
            }
            if (ImGui::MenuItem("Comment")) {
                createNode(NodeType::Comment, nodePos);
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndMenu();
        }
    }    int NodeEditorWindow::createNode(NodeType type, ImVec2 position) {
        int nodeId = m_nextNodeId++;
        std::string nodeName = getNodeTypeName(type);
        
        auto node = std::make_unique<Node>(nodeId, type, nodeName, position);
        
        // Set up callback for component data changes
        node->onComponentDataChanged = [this](Node* changedNode) {
            this->onNodeComponentDataChanged(changedNode);
        };
        
        // If creating an Entity node and we have a selected entity, associate them
        if (type == NodeType::Entity && m_activeScene && m_activeScene->hasSelectedEntity()) {
            node->associatedEntity = m_activeScene->getSelectedEntity();
            
            // Update the node name to show which entity it represents
            Scene* scene = m_activeScene->getScene().get();
            if (scene) {
                std::string entityName = scene->getEntityName(node->associatedEntity);
                node->name = "Entity: " + entityName;
            }
        }
        
        m_nodeMap[nodeId] = node.get();
        m_nodes.push_back(std::move(node));
        
        return nodeId;
    }

    void NodeEditorWindow::selectNode(int nodeId) {
        clearSelection();
        if (m_nodeMap.count(nodeId)) {
            m_nodeMap[nodeId]->selected = true;
        }
    }

    void NodeEditorWindow::clearSelection() {
        for (auto& node : m_nodes) {
            node->selected = false;
        }
    }    void NodeEditorWindow::updatePinPositions() {
        // Update pin positions for all nodes using display coordinates
        for (auto& node : m_nodes) {            // Calculate display position
            ImVec2 displayPos = ImVec2(m_canvasPos.x + node->position.x + m_scrolling.x, 
                                      m_canvasPos.y + node->position.y + m_scrolling.y);
            
            // Update pin positions using the node's helper method
            node->updatePinPositions(displayPos);
        }
    }

    void NodeEditorWindow::applyComponentToEntity(EntityID entity, Scene* scene, Node* componentNode) {
        if (!scene || !componentNode || !componentNode->componentData) return;
        
        switch (componentNode->type) {
            case NodeType::SpriteComponent:
                // Always apply/update the sprite component with the node's data (including texture)
                if (scene->hasComponent<Sprite>(entity)) {
                    scene->removeComponent<Sprite>(entity);
                }
                scene->addComponent<Sprite>(entity, *std::static_pointer_cast<Sprite>(componentNode->componentData));
                break;
            case NodeType::PlayerController:
                if (!scene->hasComponent<PlayerController>(entity)) {
                    scene->addComponent<PlayerController>(entity, *std::static_pointer_cast<PlayerController>(componentNode->componentData));
                    
                    // Auto-add required PlayerSystem components
                    if (!scene->hasComponent<PlayerStats>(entity)) {
                        scene->addComponent<PlayerStats>(entity, PlayerStats());
                    }
                    if (!scene->hasComponent<PlayerPhysics>(entity)) {
                        scene->addComponent<PlayerPhysics>(entity, PlayerPhysics());
                    }
                    if (!scene->hasComponent<PlayerState>(entity)) {
                        scene->addComponent<PlayerState>(entity, PlayerState());
                    }
                    if (!scene->hasComponent<PlayerAbilities>(entity)) {
                        scene->addComponent<PlayerAbilities>(entity, PlayerAbilities());
                    }
                }
                break;
            case NodeType::PlayerStats:
                if (!scene->hasComponent<PlayerStats>(entity)) {
                    scene->addComponent<PlayerStats>(entity, *std::static_pointer_cast<PlayerStats>(componentNode->componentData));
                }
                break;
            case NodeType::PlayerPhysics:
                if (!scene->hasComponent<PlayerPhysics>(entity)) {
                    scene->addComponent<PlayerPhysics>(entity, *std::static_pointer_cast<PlayerPhysics>(componentNode->componentData));
                }
                break;
            case NodeType::PlayerInventory:
                if (!scene->hasComponent<PlayerInventory>(entity)) {
                    scene->addComponent<PlayerInventory>(entity, *std::static_pointer_cast<PlayerInventory>(componentNode->componentData));
                }
                break;
            case NodeType::PlayerAbilities:
                if (!scene->hasComponent<PlayerAbilities>(entity)) {
                    scene->addComponent<PlayerAbilities>(entity, *std::static_pointer_cast<PlayerAbilities>(componentNode->componentData));
                }
                break;
            case NodeType::PlayerState:
                if (!scene->hasComponent<PlayerState>(entity)) {
                    scene->addComponent<PlayerState>(entity, *std::static_pointer_cast<PlayerState>(componentNode->componentData));
                }
                break;
            case NodeType::Transform:
                if (!scene->hasComponent<Transform>(entity)) {
                    scene->addComponent<Transform>(entity, *std::static_pointer_cast<Transform>(componentNode->componentData));
                } else {
                    // Update existing transform component
                    auto& transformComp = scene->getComponent<Transform>(entity);
                    transformComp = *std::static_pointer_cast<Transform>(componentNode->componentData);
                }
                break;
            case NodeType::Rotation:
                if (!scene->hasComponent<Rotation>(entity)) {
                    scene->addComponent<Rotation>(entity, *std::static_pointer_cast<Rotation>(componentNode->componentData));
                } else {
                    // Update existing rotation component
                    auto& rotComp = scene->getComponent<Rotation>(entity);
                    rotComp = *std::static_pointer_cast<Rotation>(componentNode->componentData);
                }
                break;
            case NodeType::Scale:
                if (!scene->hasComponent<Scale>(entity)) {
                    scene->addComponent<Scale>(entity, *std::static_pointer_cast<Scale>(componentNode->componentData));
                } else {
                    // Update existing scale component
                    auto& scaleComp = scene->getComponent<Scale>(entity);
                    scaleComp = *std::static_pointer_cast<Scale>(componentNode->componentData);
                }
                break;
            case NodeType::Collider:
                if (!scene->hasComponent<Collider>(entity)) {
                    scene->addComponent<Collider>(entity, *std::static_pointer_cast<Collider>(componentNode->componentData));
                }
                break;
            case NodeType::RigidBody:
                if (!scene->hasComponent<RigidBody>(entity)) {
                    scene->addComponent<RigidBody>(entity, *std::static_pointer_cast<RigidBody>(componentNode->componentData));
                }
                break;
            case NodeType::ParticleEffect:
                if (!scene->hasComponent<ParticleEffect>(entity)) {
                    scene->addComponent<ParticleEffect>(entity, *std::static_pointer_cast<ParticleEffect>(componentNode->componentData));
                }
                break;
            case NodeType::EntitySpawner:
                if (!scene->hasComponent<EntitySpawner>(entity)) {
                    scene->addComponent<EntitySpawner>(entity, *std::static_pointer_cast<EntitySpawner>(componentNode->componentData));
                }
                break;
            default:
                break;
        }
        
        // Mark the scene window as dirty to trigger a visual update
        if (m_activeScene) {
            m_activeScene->setDirty(true);
        }
    }

    void NodeEditorWindow::removeComponentFromEntity(EntityID entity, Scene* scene, NodeType componentType) {
        if (!scene) return;
        
        switch (componentType) {
            case NodeType::SpriteComponent:
                if (scene->hasComponent<Sprite>(entity)) {
                    scene->removeComponent<Sprite>(entity);
                }
                break;
            case NodeType::PlayerController:
                if (scene->hasComponent<PlayerController>(entity)) {
                    scene->removeComponent<PlayerController>(entity);
                }
                break;
            case NodeType::PlayerStats:
                if (scene->hasComponent<PlayerStats>(entity)) {
                    scene->removeComponent<PlayerStats>(entity);
                }
                break;
            case NodeType::PlayerPhysics:
                if (scene->hasComponent<PlayerPhysics>(entity)) {
                    scene->removeComponent<PlayerPhysics>(entity);
                }
                break;
            case NodeType::PlayerInventory:
                if (scene->hasComponent<PlayerInventory>(entity)) {
                    scene->removeComponent<PlayerInventory>(entity);
                }
                break;
            case NodeType::PlayerAbilities:
                if (scene->hasComponent<PlayerAbilities>(entity)) {
                    scene->removeComponent<PlayerAbilities>(entity);
                }
                break;
            case NodeType::PlayerState:
                if (scene->hasComponent<PlayerState>(entity)) {
                    scene->removeComponent<PlayerState>(entity);
                }
                break;
            case NodeType::Transform:
                if (scene->hasComponent<Transform>(entity)) {
                    scene->removeComponent<Transform>(entity);
                }
                break;
            case NodeType::Rotation:
                if (scene->hasComponent<Rotation>(entity)) {
                    scene->removeComponent<Rotation>(entity);
                }
                break;
            case NodeType::Scale:
                if (scene->hasComponent<Scale>(entity)) {
                    scene->removeComponent<Scale>(entity);
                }
                break;
            case NodeType::Collider:
                if (scene->hasComponent<Collider>(entity)) {
                    scene->removeComponent<Collider>(entity);
                }
                break;
            case NodeType::RigidBody:
                if (scene->hasComponent<RigidBody>(entity)) {
                    scene->removeComponent<RigidBody>(entity);
                }
                break;
            case NodeType::EntitySpawner:
                if (scene->hasComponent<EntitySpawner>(entity)) {
                    scene->removeComponent<EntitySpawner>(entity);
                    printf("DEBUG: Removed EntitySpawner component from entity %u\n", entity);
                }
                break;
            case NodeType::ParticleEffect:
                if (scene->hasComponent<ParticleEffect>(entity)) {
                    scene->removeComponent<ParticleEffect>(entity);
                }
                break;
            default:
                break;
        }
    }

    std::string NodeEditorWindow::getNodeTypeName(NodeType type) {
        switch (type) {
            case NodeType::Entity: return "Entity";
            case NodeType::SpriteComponent: return "Sprite";
            case NodeType::Transform: return "Transform";
            case NodeType::Rotation: return "Rotation";
            case NodeType::Scale: return "Scale";
            case NodeType::Collider: return "Collider";
            case NodeType::RigidBody: return "RigidBody";
            
            // Player Components
            case NodeType::PlayerController: return "Controller";
            case NodeType::PlayerStats: return "Stats";
            case NodeType::PlayerPhysics: return "Physics";
            case NodeType::PlayerInventory: return "Inventory";
            case NodeType::PlayerAbilities: return "Abilities";
            case NodeType::PlayerState: return "State";
            
            // NPC and AI Components
            case NodeType::NPCController: return "NPC Controller";
            case NodeType::AIBehavior: return "AI Behavior";
            case NodeType::AIStateMachine: return "AI State Machine";
            case NodeType::AIPathfinding: return "AI Pathfinding";
            case NodeType::NPCDialogue: return "NPC Dialogue";
            case NodeType::NPCInteraction: return "NPC Interaction";
            
            // Environment Components
            case NodeType::EnvironmentCollider: return "Env Collider";
            case NodeType::EnvironmentTrigger: return "Env Trigger";
            case NodeType::EnvironmentHazard: return "Env Hazard";
            case NodeType::EnvironmentDoor: return "Env Door";
            case NodeType::EnvironmentSwitch: return "Env Switch";
            case NodeType::EnvironmentPlatform: return "Env Platform";
            
            // Audio and Effects
            case NodeType::AudioSource: return "Audio Source";
            case NodeType::AudioListener: return "Audio Listener";
            case NodeType::ParticleSystem: return "Particle System";
            case NodeType::ParticleEmitter: return "Particle Emitter";
            case NodeType::VisualEffect: return "Visual Effect";
            case NodeType::LightSource: return "Light Source";
            
            // UI Components
            case NodeType::UIElement: return "UI Element";
            case NodeType::UIButton: return "UI Button";
            case NodeType::UIText: return "UI Text";
            case NodeType::UIImage: return "UI Image";
            case NodeType::UIHealthBar: return "UI Health Bar";
            case NodeType::UIInventorySlot: return "UI Inventory Slot";
            
            // Math nodes
            case NodeType::MathAdd: return "Add";
            case NodeType::MathSubtract: return "Subtract";
            case NodeType::MathMultiply: return "Multiply";
            case NodeType::MathDivide: return "Divide";
            case NodeType::MathPower: return "Power";
            case NodeType::MathSin: return "Sin";
            case NodeType::MathCos: return "Cos";
            case NodeType::MathAbs: return "Abs";
            case NodeType::MathMin: return "Min";
            case NodeType::MathMax: return "Max";
            case NodeType::MathClamp: return "Clamp";
            case NodeType::MathLerp: return "Lerp";
            case NodeType::MathDistance: return "Distance";
            case NodeType::MathNormalize: return "Normalize";
            case NodeType::MathDotProduct: return "Dot Product";
            case NodeType::RandomFloat: return "Random Float";
            case NodeType::RandomInt: return "Random Int";
            
            // Logic nodes
            case NodeType::LogicAND: return "AND";
            case NodeType::LogicOR: return "OR";
            case NodeType::LogicNOT: return "NOT";
            case NodeType::LogicXOR: return "XOR";
            case NodeType::Compare: return "Compare";
            case NodeType::Branch: return "Branch";
            case NodeType::Condition: return "Condition";
            case NodeType::Switch: return "Switch";
            
            // Flow control
            case NodeType::Sequence: return "Sequence";
            case NodeType::Parallel: return "Parallel";
            case NodeType::Delay: return "Delay";
            case NodeType::Loop: return "Loop";
            case NodeType::ForLoop: return "For Loop";
            case NodeType::WhileLoop: return "While Loop";
            
            // Event nodes
            case NodeType::EventTrigger: return "Event Trigger";
            case NodeType::EventListener: return "Event Listener";
            case NodeType::OnCollision: return "OnCollision";
            case NodeType::OnKeyPress: return "OnKeyPress";
            case NodeType::OnKeyRelease: return "OnKeyRelease";
            case NodeType::OnMouseClick: return "OnMouseClick";
            case NodeType::OnMouseHover: return "OnMouseHover";
            case NodeType::OnTriggerEnter: return "OnTriggerEnter";
            case NodeType::OnTriggerExit: return "OnTriggerExit";
            case NodeType::OnEntityDestroyed: return "OnEntityDestroyed";
            case NodeType::OnEntitySpawned: return "OnEntitySpawned";
            case NodeType::TimerNode: return "Timer";
            
            // Data nodes
            case NodeType::ConstantFloat: return "Float";
            case NodeType::ConstantInt: return "Int";
            case NodeType::ConstantString: return "String";
            case NodeType::ConstantBool: return "Bool";
            case NodeType::ConstantVector2: return "Vector2";
            case NodeType::Variable: return "Variable";
            case NodeType::GlobalVariable: return "Global Variable";
            case NodeType::EntityReference: return "Entity Reference";
            
            // Game-specific nodes
            case NodeType::MoveTowards: return "Move Towards";
            case NodeType::FollowPath: return "Follow Path";
            case NodeType::Animate: return "Animate";
            case NodeType::PlaySound: return "Play Sound";
            case NodeType::StopSound: return "Stop Sound";
            case NodeType::SpawnEntity: return "Spawn Entity";
            case NodeType::DestroyEntity: return "Destroy Entity";
            case NodeType::EntitySpawner: return "Entity Spawner";
            case NodeType::EntityFactory: return "Entity Factory";
            case NodeType::ParticleEffect: return "Particle Effect";
            
            // Script and Behavior nodes
            case NodeType::ScriptNode: return "Script Node";
            case NodeType::BehaviorTree: return "Behavior Tree";
            case NodeType::StateMachine: return "State Machine";
            case NodeType::CustomScript: return "Custom Script";
            
            // Physics and Movement
            case NodeType::ApplyForce: return "Apply Force";
            case NodeType::SetVelocity: return "Set Velocity";
            case NodeType::Raycast: return "Raycast";
            case NodeType::OverlapCheck: return "Overlap Check";
            case NodeType::PhysicsConstraint: return "Physics Constraint";
            
            // Game State and Management
            case NodeType::SceneLoader: return "Scene Loader";
            case NodeType::GameStateManager: return "Game State Manager";
            case NodeType::SaveGame: return "Save Game";
            case NodeType::LoadGame: return "Load Game";
            case NodeType::CheckGameState: return "Check Game State";
            
            // Inventory and Items
            case NodeType::ItemPickup: return "Item Pickup";
            case NodeType::ItemDrop: return "Item Drop";
            case NodeType::InventoryAdd: return "Inventory Add";
            case NodeType::InventoryRemove: return "Inventory Remove";
            case NodeType::InventoryCheck: return "Inventory Check";
            
            // Health and Combat
            case NodeType::DealDamage: return "Deal Damage";
            case NodeType::HealEntity: return "Heal Entity";
            case NodeType::CheckHealth: return "Check Health";
            case NodeType::ApplyStatusEffect: return "Apply Status Effect";
            case NodeType::RemoveStatusEffect: return "Remove Status Effect";
            
            // Dialogue and Narrative
            case NodeType::DialogueNode: return "Dialogue";
            case NodeType::DialogueChoice: return "Dialogue Choice";
            case NodeType::DialogueCondition: return "Dialogue Condition";
            case NodeType::QuestStart: return "Quest Start";
            case NodeType::QuestComplete: return "Quest Complete";
            case NodeType::QuestCheck: return "Quest Check";
            
            // Utility nodes
            case NodeType::Print: return "Print";
            case NodeType::Debug: return "Debug";
            case NodeType::Comment: return "Comment";
            
            default: return "Unknown";
        }
    }

    bool NodeEditorWindow::isECSComponentNode(NodeType type) const {
        // Return true for node types that correspond to ECS components
        switch (type) {
            // Core components
            case NodeType::SpriteComponent:
            case NodeType::Transform:
            case NodeType::Rotation:
            case NodeType::Scale:
            case NodeType::Collider:
            case NodeType::RigidBody:
            
            // Player components
            case NodeType::PlayerController:
            case NodeType::PlayerStats:
            case NodeType::PlayerPhysics:
            case NodeType::PlayerInventory:
            case NodeType::PlayerAbilities:
            case NodeType::PlayerState:
            
            // NPC and AI components
            case NodeType::NPCController:
            case NodeType::AIBehavior:
            case NodeType::AIStateMachine:
            case NodeType::AIPathfinding:
            case NodeType::NPCDialogue:
            case NodeType::NPCInteraction:
            
            // Environment components
            case NodeType::EnvironmentCollider:
            case NodeType::EnvironmentTrigger:
            case NodeType::EnvironmentHazard:
            case NodeType::EnvironmentDoor:
            case NodeType::EnvironmentSwitch:
            case NodeType::EnvironmentPlatform:
            
            // Audio and Effects components
            case NodeType::AudioSource:
            case NodeType::AudioListener:
            case NodeType::ParticleSystem:
            case NodeType::ParticleEmitter:
            case NodeType::VisualEffect:
            case NodeType::LightSource:
            
            // UI components
            case NodeType::UIElement:
            case NodeType::UIButton:
            case NodeType::UIText:
            case NodeType::UIImage:
            case NodeType::UIHealthBar:
            case NodeType::UIInventorySlot:
            
            // Special components
            case NodeType::ParticleEffect:
            case NodeType::EntitySpawner:
                return true;
            
            // Non-ECS node types (event nodes, logic nodes, math nodes, etc.)
            case NodeType::Entity:
            case NodeType::OnKeyPress:
            case NodeType::OnKeyRelease:
            case NodeType::OnMouseClick:
            case NodeType::OnMouseHover:
            case NodeType::OnCollision:
            case NodeType::OnTriggerEnter:
            case NodeType::OnTriggerExit:
            case NodeType::OnEntityDestroyed:
            case NodeType::OnEntitySpawned:
            case NodeType::TimerNode:
            case NodeType::EntityFactory:
            case NodeType::MathAdd:
            case NodeType::MathSubtract:
            case NodeType::MathMultiply:
            case NodeType::MathDivide:
            case NodeType::MathPower:
            case NodeType::MathSin:
            case NodeType::MathCos:
            case NodeType::MathAbs:
            case NodeType::MathMin:
            case NodeType::MathMax:
            case NodeType::MathClamp:
            case NodeType::MathLerp:
            case NodeType::MathDistance:
            case NodeType::MathNormalize:
            case NodeType::MathDotProduct:
            case NodeType::RandomFloat:
            case NodeType::RandomInt:
            case NodeType::LogicAND:
            case NodeType::LogicOR:
            case NodeType::LogicNOT:
            case NodeType::LogicXOR:
            case NodeType::Compare:
            case NodeType::Branch:
            case NodeType::Condition:
            case NodeType::Switch:
            case NodeType::Sequence:
            case NodeType::Parallel:
            case NodeType::Delay:
            case NodeType::Loop:
            case NodeType::ForLoop:
            case NodeType::WhileLoop:
            case NodeType::ConstantFloat:
            case NodeType::ConstantInt:
            case NodeType::ConstantString:
            case NodeType::ConstantBool:
            case NodeType::ConstantVector2:
            case NodeType::Variable:
            case NodeType::GlobalVariable:
            case NodeType::EntityReference:
            case NodeType::MoveTowards:
            case NodeType::FollowPath:
            case NodeType::Animate:
            case NodeType::PlaySound:
            case NodeType::StopSound:
            case NodeType::SpawnEntity:
            case NodeType::DestroyEntity:
            case NodeType::ScriptNode:
            case NodeType::BehaviorTree:
            case NodeType::StateMachine:
            case NodeType::CustomScript:
            case NodeType::ApplyForce:
            case NodeType::SetVelocity:
            case NodeType::Raycast:
            case NodeType::OverlapCheck:
            case NodeType::PhysicsConstraint:
            case NodeType::SceneLoader:
            case NodeType::GameStateManager:
            case NodeType::SaveGame:
            case NodeType::LoadGame:
            case NodeType::CheckGameState:
            case NodeType::ItemPickup:
            case NodeType::ItemDrop:
            case NodeType::InventoryAdd:
            case NodeType::InventoryRemove:
            case NodeType::InventoryCheck:
            case NodeType::DealDamage:
            case NodeType::HealEntity:
            case NodeType::CheckHealth:
            case NodeType::ApplyStatusEffect:
            case NodeType::RemoveStatusEffect:
            case NodeType::DialogueNode:
            case NodeType::DialogueChoice:
            case NodeType::DialogueCondition:
            case NodeType::QuestStart:
            case NodeType::QuestComplete:
            case NodeType::QuestCheck:
            case NodeType::Print:
            case NodeType::Debug:
            case NodeType::Comment:
                return false;
                
            default:
                return false;
        }
    }    void NodeEditorWindow::applyNodesToEntity(EntityID entity, Scene* scene) {
        if (!scene) return;
        
        // Associate the entity with any Entity nodes first
        for (auto& node : m_nodes) {
            if (node->type == NodeType::Entity) {
                node->associatedEntity = entity;
            }
        }
        
        // Find connected component nodes and apply them to the entity
        for (auto& node : m_nodes) {
            if (node->type == NodeType::Entity) continue;
            
            // Special handling for EntitySpawner nodes
            if (node->type == NodeType::EntitySpawner) {
                // Apply EntitySpawner component to the entity
                if (!scene->hasComponent<EntitySpawner>(entity)) {
                    EntitySpawner spawner;
                    spawner.clearTemplates();
                    
                    // If this EntitySpawner node has a template entity, configure it
                    if (node->templateEntityId != 0) {
                        std::string templateName = scene->getEntityName(node->templateEntityId);
                        if (templateName.empty()) {
                            templateName = "Entity_" + std::to_string(node->templateEntityId);
                        }
                        
                        // Create template based on the template entity
                        EntitySpawner::SpawnTemplate newTemplate(templateName, "", ::Vector2(20, 0), ::Vector2(100, 0));
                        newTemplate.scale = 1.0f;
                        newTemplate.lifeTime = 0.0f;
                        newTemplate.hasCollider = scene->hasComponent<Collider>(node->templateEntityId);
                        newTemplate.hasRigidBody = scene->hasComponent<RigidBody>(node->templateEntityId);
                        
                        // Store the template entity ID for spawning
                        newTemplate.spriteFile = "TEMPLATE_ENTITY_" + std::to_string(node->templateEntityId);
                        
                        spawner.templates.push_back(newTemplate);
                        spawner.selectedTemplate = 0;
                        
                        printf("DEBUG: EntitySpawner applied to entity %u with template from entity %u (%s)\n", 
                               entity, node->templateEntityId, templateName.c_str());
                    }
                    
                    scene->addComponent<EntitySpawner>(entity, spawner);
                }
                continue;
            }
            
            // Check if this component node is connected to an entity
            bool isConnected = false;
            for (const auto& connection : m_connections) {
                for (auto& node2 : m_nodes) {
                    if (node2->type == NodeType::Entity) {
                        for (const auto& outputPin : node2->outputPins) {
                            if (outputPin.id == connection.outputPinId) {
                                for (const auto& inputPin : node->inputPins) {
                                    if (inputPin.id == connection.inputPinId) {
                                        isConnected = true;
                                        break;
                                    }
                                }
                            }
                        }
                    }
                }
            }
              if (isConnected && node->componentData) {
                // Apply the component using the helper method
                applyComponentToEntity(entity, scene, node.get());
            }
        }
        
        // Save the current node layout for this entity
        saveNodeLayout(entity);
        printf("DEBUG: Applied nodes to entity %u and saved layout\n", entity);
    }

    void NodeEditorWindow::loadEntityAsNodes(EntityID entity, Scene* scene) {
        if (!scene) return;
        
        printf("DEBUG: Starting loadEntityAsNodes for entity %d\n", entity);
        
        try {
            // Clear existing nodes
            m_nodes.clear();
            m_connections.clear();
            m_nodeMap.clear();
            
            printf("DEBUG: Cleared existing nodes\n");
            
            // Create entity node
            int entityNodeId = createNode(NodeType::Entity, ImVec2(100, 100));
            printf("DEBUG: Created entity node with ID %d\n", entityNodeId);
            
            Node* entityNode = m_nodeMap[entityNodeId];
            if (!entityNode) {
                printf("ERROR: Failed to find entity node in node map\n");
                return;
            }
            
            entityNode->associatedEntity = entity;
            printf("DEBUG: Associated entity %d with node\n", entity);
            
            // Update the entity node name to reflect the correct entity
            if (scene) {
                std::string entityName = scene->getEntityName(entity);
                entityNode->name = "Entity: " + entityName;
                printf("DEBUG: Set entity node name to '%s'\n", entityNode->name.c_str());
            }
            
            // Create component nodes based on what the entity has
            std::vector<int> componentNodeIds;
            
            printf("DEBUG: Checking components for entity %u\n", entity);
            
          if (scene->hasComponent<Sprite>(entity)) {
            printf("DEBUG: Found Sprite component\n");
            int nodeId = createNode(NodeType::SpriteComponent, ImVec2(300, 50));
            // Copy the actual sprite component data from the entity
            auto& entitySprite = scene->getComponent<Sprite>(entity);
            auto nodeSprite = std::static_pointer_cast<Sprite>(m_nodeMap[nodeId]->componentData);
            if (nodeSprite) {
                *nodeSprite = entitySprite; // Copy all sprite data including texture
            }
            componentNodeIds.push_back(nodeId);
        }
        
        if (scene->hasComponent<PlayerController>(entity)) {
            printf("DEBUG: Found PlayerController component\n");
            int nodeId = createNode(NodeType::PlayerController, ImVec2(300, 120));
            // Copy component data
            auto& entityController = scene->getComponent<PlayerController>(entity);
            auto nodeController = std::static_pointer_cast<PlayerController>(m_nodeMap[nodeId]->componentData);
            if (nodeController) {
                *nodeController = entityController;
            }
            componentNodeIds.push_back(nodeId);
        }
        
        if (scene->hasComponent<PlayerStats>(entity)) {
            printf("DEBUG: Found PlayerStats component\n");
            int nodeId = createNode(NodeType::PlayerStats, ImVec2(300, 190));
            // Copy component data
            auto& entityStats = scene->getComponent<PlayerStats>(entity);
            auto nodeStats = std::static_pointer_cast<PlayerStats>(m_nodeMap[nodeId]->componentData);
            if (nodeStats) {
                *nodeStats = entityStats;
            }
            componentNodeIds.push_back(nodeId);
        }
        
        if (scene->hasComponent<PlayerPhysics>(entity)) {
            printf("DEBUG: Found PlayerPhysics component\n");
            int nodeId = createNode(NodeType::PlayerPhysics, ImVec2(300, 260));
            // Copy component data
            auto& entityPhysics = scene->getComponent<PlayerPhysics>(entity);
            auto nodePhysics = std::static_pointer_cast<PlayerPhysics>(m_nodeMap[nodeId]->componentData);
            if (nodePhysics) {
                *nodePhysics = entityPhysics;
            }
            componentNodeIds.push_back(nodeId);
        }
        
        if (scene->hasComponent<PlayerInventory>(entity)) {
            printf("DEBUG: Found PlayerInventory component\n");
            int nodeId = createNode(NodeType::PlayerInventory, ImVec2(300, 330));
            // Copy component data
            auto& entityInventory = scene->getComponent<PlayerInventory>(entity);
            auto nodeInventory = std::static_pointer_cast<PlayerInventory>(m_nodeMap[nodeId]->componentData);
            if (nodeInventory) {
                *nodeInventory = entityInventory;
            }
            componentNodeIds.push_back(nodeId);
        }
        
        if (scene->hasComponent<PlayerAbilities>(entity)) {
            printf("DEBUG: Found PlayerAbilities component\n");
            int nodeId = createNode(NodeType::PlayerAbilities, ImVec2(300, 400));
            // Copy component data
            auto& entityAbilities = scene->getComponent<PlayerAbilities>(entity);
            auto nodeAbilities = std::static_pointer_cast<PlayerAbilities>(m_nodeMap[nodeId]->componentData);
            if (nodeAbilities) {
                *nodeAbilities = entityAbilities;
            }
            componentNodeIds.push_back(nodeId);
        }
        
        if (scene->hasComponent<PlayerState>(entity)) {
            printf("DEBUG: Found PlayerState component\n");
            int nodeId = createNode(NodeType::PlayerState, ImVec2(300, 470));
            // Copy component data
            auto& entityState = scene->getComponent<PlayerState>(entity);
            auto nodeState = std::static_pointer_cast<PlayerState>(m_nodeMap[nodeId]->componentData);
            if (nodeState) {
                *nodeState = entityState;
            }
            componentNodeIds.push_back(nodeId);
        }
        
        if (scene->hasComponent<Transform>(entity)) {
            int nodeId = createNode(NodeType::Transform, ImVec2(300, 540));
            // Copy the actual transform component data from the entity
            auto& entityTransform = scene->getComponent<Transform>(entity);
            auto nodeTransform = std::static_pointer_cast<Transform>(m_nodeMap[nodeId]->componentData);
            if (nodeTransform) {
                *nodeTransform = entityTransform;
            }
            componentNodeIds.push_back(nodeId);
        }
        
        if (scene->hasComponent<Rotation>(entity)) {
            printf("DEBUG: Found Rotation component\n");
            try {
                int nodeId = createNode(NodeType::Rotation, ImVec2(500, 100));
                
                // Verify the node was created correctly and copy component data
                if (m_nodeMap.find(nodeId) != m_nodeMap.end() && m_nodeMap[nodeId] && m_nodeMap[nodeId]->componentData) {
                    auto& entityRotation = scene->getComponent<Rotation>(entity);
                    auto nodeRotation = std::static_pointer_cast<Rotation>(m_nodeMap[nodeId]->componentData);
                    if (nodeRotation) {
                        *nodeRotation = entityRotation;
                    }
                }
                componentNodeIds.push_back(nodeId);
            } catch (const std::exception& e) {
                printf("ERROR: Exception while creating Rotation node: %s\n", e.what());
            }
        }
        
        if (scene->hasComponent<Scale>(entity)) {
            printf("DEBUG: Found Scale component\n");
            try {
                int nodeId = createNode(NodeType::Scale, ImVec2(500, 200));
                
                // Verify the node was created correctly and copy component data
                if (m_nodeMap.find(nodeId) != m_nodeMap.end() && m_nodeMap[nodeId] && m_nodeMap[nodeId]->componentData) {
                    auto& entityScale = scene->getComponent<Scale>(entity);
                    auto nodeScale = std::static_pointer_cast<Scale>(m_nodeMap[nodeId]->componentData);
                    if (nodeScale) {
                        *nodeScale = entityScale;
                    }
                }
                componentNodeIds.push_back(nodeId);
            } catch (const std::exception& e) {
                printf("ERROR: Exception while creating Scale node: %s\n", e.what());
            }
        }
        
        if (scene->hasComponent<Collider>(entity)) {
            printf("DEBUG: Found Collider component\n");
            int nodeId = createNode(NodeType::Collider, ImVec2(300, 610));
            // Copy component data
            auto& entityCollider = scene->getComponent<Collider>(entity);
            auto nodeCollider = std::static_pointer_cast<Collider>(m_nodeMap[nodeId]->componentData);
            if (nodeCollider) {
                *nodeCollider = entityCollider;
            }
            componentNodeIds.push_back(nodeId);
        }
        
        if (scene->hasComponent<RigidBody>(entity)) {
            printf("DEBUG: Found RigidBody component\n");
            int nodeId = createNode(NodeType::RigidBody, ImVec2(300, 680));
            // Copy component data
            auto& entityRigidBody = scene->getComponent<RigidBody>(entity);
            auto nodeRigidBody = std::static_pointer_cast<RigidBody>(m_nodeMap[nodeId]->componentData);
            if (nodeRigidBody) {
                *nodeRigidBody = entityRigidBody;
            }
            componentNodeIds.push_back(nodeId);
        }
        
        if (scene->hasComponent<ParticleEffect>(entity)) {
            printf("DEBUG: Found ParticleEffect component\n");
            int nodeId = createNode(NodeType::ParticleEffect, ImVec2(300, 750));
            // Copy the actual ParticleEffect component data from the entity
            auto& entityParticle = scene->getComponent<ParticleEffect>(entity);
            auto nodeParticle = std::static_pointer_cast<ParticleEffect>(m_nodeMap[nodeId]->componentData);
            if (nodeParticle) {
                *nodeParticle = entityParticle;
            }
            componentNodeIds.push_back(nodeId);
        }
        
        if (scene->hasComponent<EntitySpawner>(entity)) {
            printf("DEBUG: Found EntitySpawner component\n");
            int nodeId = createNode(NodeType::EntitySpawner, ImVec2(300, 820));
            // Copy the actual EntitySpawner component data from the entity
            auto& entitySpawner = scene->getComponent<EntitySpawner>(entity);
            auto nodeSpawner = std::static_pointer_cast<EntitySpawner>(m_nodeMap[nodeId]->componentData);
            if (nodeSpawner) {
                *nodeSpawner = entitySpawner; // Copy all spawner data including templates
            }
            componentNodeIds.push_back(nodeId);
        }
        
        printf("DEBUG: Found %zu components for entity %u\n", componentNodeIds.size(), entity);
        
          // Create connections with proper pin matching
        printf("DEBUG: Starting connection creation, %zu component nodes\n", componentNodeIds.size());
        for (int componentNodeId : componentNodeIds) {
            printf("DEBUG: Processing component node %d\n", componentNodeId);
            Node* componentNode = m_nodeMap[componentNodeId];
            if (!componentNode) {
                printf("ERROR: Component node %d not found in map\n", componentNodeId);
                continue;
            }
            
            if (!entityNode->outputPins.empty() && !componentNode->inputPins.empty()) {
                printf("DEBUG: Entity has %zu output pins, component has %zu input pins\n", 
                       entityNode->outputPins.size(), componentNode->inputPins.size());
                
                // Use the single Entity output pin for all connections
                int outputPinId = entityNode->id * 100 + 1; // Single Entity pin
                printf("DEBUG: Using Entity pin %d for connection to %s\n", outputPinId, getNodeTypeName(componentNode->type).c_str());
                
                printf("DEBUG: Creating connection from pin %d to pin %d\n", outputPinId, componentNode->inputPins[0].id);
                int connectionId = m_nextConnectionId++;
                m_connections.emplace_back(connectionId, outputPinId, componentNode->inputPins[0].id);
                
                // Mark pins as connected
                bool outputPinFound = false, inputPinFound = false;
                for (auto& node : m_nodes) {
                    if (auto pin = node->getPinById(outputPinId)) {
                        pin->connected = true;
                        pin->connectedPinId = componentNode->inputPins[0].id;
                        outputPinFound = true;
                    }
                    if (auto pin = node->getPinById(componentNode->inputPins[0].id)) {
                        pin->connected = true;
                        pin->connectedPinId = outputPinId;
                        inputPinFound = true;
                    }
                }
                printf("DEBUG: Pin marking - Output found: %s, Input found: %s\n", 
                       outputPinFound ? "true" : "false", inputPinFound ? "true" : "false");
            } else {
                printf("WARNING: Entity has no output pins or component has no input pins\n");
            }
        }
        
        printf("DEBUG: loadEntityAsNodes completed successfully\n");
        
        // Load saved node layout to restore positions
        loadNodeLayout(entity);
        
        } catch (const std::exception& e) {
            printf("ERROR: Exception in loadEntityAsNodes: %s\n", e.what());
        }
    }    bool NodeEditorWindow::canConnect(int outputPinId, int inputPinId) {
        // Find the pins
        Pin* outputPin = nullptr;
        Pin* inputPin = nullptr;
        Node* outputNode = nullptr;
        Node* inputNode = nullptr;
        
        for (auto& node : m_nodes) {
            if (auto pin = node->getPinById(outputPinId)) {
                outputPin = pin;
                outputNode = node.get();
            }
            if (auto pin = node->getPinById(inputPinId)) {
                inputPin = pin;
                inputNode = node.get();
            }
        }
        
        if (!outputPin || !inputPin || !outputNode || !inputNode) {
            printf("DEBUG: canConnect failed - missing pins or nodes. OutputPin: %p, InputPin: %p, OutputNode: %p, InputNode: %p\n", 
                   outputPin, inputPin, outputNode, inputNode);
            return false;
        }
        
        printf("DEBUG: canConnect attempt - %s (%s pin) -> %s (%s pin)\n", 
               getNodeTypeName(outputNode->type).c_str(), outputPin->name.c_str(),
               getNodeTypeName(inputNode->type).c_str(), inputPin->name.c_str());
        
        // Can't connect node to itself
        if (outputNode == inputNode) return false;
        
        // Check if input pin is already connected
        if (inputPin->connected) return false;
        
        // Special handling for new node types (EntitySpawner, EntityFactory, ParticleEffect)
        if (outputNode->type == NodeType::EntitySpawner || outputNode->type == NodeType::EntityFactory) {
            // EntitySpawner/EntityFactory can connect to Entity nodes to give them spawning behavior
            if (inputNode->type == NodeType::Entity) {
                // Allow Event connections (to trigger spawning) and Entity connections (to provide template)
                return (outputPin->dataType == PinDataType::Event && inputPin->dataType == PinDataType::Event) ||
                       (outputPin->dataType == PinDataType::Entity && inputPin->dataType == PinDataType::Entity);
            }
            // EntitySpawner/EntityFactory can connect to ParticleEffect nodes
            if (inputNode->type == NodeType::ParticleEffect) {
                // Check if connecting Entity output to Target Entity input (now handled by dropdown)
                return (outputPin->dataType == PinDataType::Event && inputPin->dataType == PinDataType::Event);
            }
            // Allow connections to component nodes for backward compatibility
            if (inputNode->type != NodeType::EntitySpawner && inputNode->type != NodeType::EntityFactory) {
                return true; // Allow EntitySpawner/Factory to connect to components
            }
            return false;
        }
        
        // Original Entity node connection rules
        if (outputNode->type == NodeType::Entity) {
            // Allow Entity nodes to connect to EntitySpawner nodes to gain spawning ability
            if (inputNode->type == NodeType::EntitySpawner) {
                // Entity(1) -> EntitySpawner: Entity(1) gains spawning ability
                printf("DEBUG: Entity->EntitySpawner connection check. OutputPin dataType: %d, InputPin dataType: %d\n", 
                       static_cast<int>(outputPin->dataType), static_cast<int>(inputPin->dataType));
                bool canConnect = outputPin->dataType == PinDataType::Entity && inputPin->dataType == PinDataType::Entity;
                printf("DEBUG: Entity->EntitySpawner connection result: %s\n", canConnect ? "ALLOWED" : "DENIED");
                return canConnect;
            }
            
            // Allow Entity to connect to other Entities that have spawning ability
            if (inputNode->type == NodeType::Entity) {
                // Entity(2) -> Entity(1): Entity(2) spawns through Entity(1) if Entity(1) has spawning ability
                // We'll check spawning ability in createConnection
                bool canConnect = outputPin->dataType == PinDataType::Entity && inputPin->dataType == PinDataType::Entity;
                printf("DEBUG: Entity->Entity connection result: %s\n", canConnect ? "ALLOWED" : "DENIED");
                return canConnect;
            }
            
            // Don't allow Entity to connect to ParticleEffect directly (use dropdown instead)
            if (inputNode->type == NodeType::ParticleEffect) {
                return false;
            }
            
            // Allow Entity nodes to connect to all component types through the single Entity pin
            // The Entity pin can connect to any component that accepts Entity input
            switch (inputNode->type) {
                // Basic components
                case NodeType::SpriteComponent:
                case NodeType::Transform:
                case NodeType::Rotation:
                case NodeType::Scale:
                case NodeType::Collider:
                case NodeType::RigidBody:
                
                // Player components
                case NodeType::PlayerController:
                case NodeType::PlayerStats:
                case NodeType::PlayerPhysics:
                case NodeType::PlayerInventory:
                case NodeType::PlayerAbilities:
                case NodeType::PlayerState:
                
                // NPC and AI components
                case NodeType::NPCController:
                case NodeType::AIBehavior:
                case NodeType::AIStateMachine:
                case NodeType::AIPathfinding:
                case NodeType::NPCDialogue:
                case NodeType::NPCInteraction:
                
                // Environment components
                case NodeType::EnvironmentCollider:
                case NodeType::EnvironmentTrigger:
                case NodeType::EnvironmentHazard:
                case NodeType::EnvironmentDoor:
                case NodeType::EnvironmentSwitch:
                case NodeType::EnvironmentPlatform:
                
                // Audio and Effects components
                case NodeType::AudioSource:
                case NodeType::AudioListener:
                case NodeType::ParticleSystem:
                case NodeType::ParticleEmitter:
                case NodeType::VisualEffect:
                case NodeType::LightSource:
                
                // UI components
                case NodeType::UIElement:
                case NodeType::UIButton:
                case NodeType::UIText:
                case NodeType::UIImage:
                case NodeType::UIHealthBar:
                case NodeType::UIInventorySlot:
                    // All component nodes accept Entity input through their Entity input pin
                    printf("DEBUG: Entity->%s connection check - ALLOWED\n", getNodeTypeName(inputNode->type).c_str());
                    return outputPin->dataType == PinDataType::Entity && inputPin->dataType == PinDataType::Entity;
                    
                default:
                    // Unknown component type - don't allow connection
                    printf("DEBUG: Entity->%s connection check - DENIED (unknown component type)\n", getNodeTypeName(inputNode->type).c_str());
                    return false;
            }
        }
        
        // For other node types, allow connections if data types match
        return outputPin->dataType == inputPin->dataType;
    }
    
    bool NodeEditorWindow::wouldCreateCycle(int outputPinId, int inputPinId) {
        // For this simple system, cycles shouldn't be possible since
        // we only connect Entity -> Component (one direction)
        return false;
    }

    void NodeEditorWindow::onNodeComponentDataChanged(Node* node) {
        if (!node || !m_activeScene) return;
        
        // Check if this node is connected to an entity
        bool isConnected = false;
        EntityID connectedEntity = 0;
        
        // Look for connections where this node's input pin is connected to an entity's output pin
        for (const auto& connection : m_connections) {
            // Check if this node has the input pin for this connection
            if (node->getPinById(connection.inputPinId)) {
                // Find the entity node that has the output pin
                for (auto& entityNode : m_nodes) {
                    if (entityNode->type == NodeType::Entity && 
                        entityNode->getPinById(connection.outputPinId)) {
                        isConnected = true;
                        connectedEntity = entityNode->associatedEntity;
                        break;
                    }
                }
            }
            if (isConnected) break;
        }
        
        // If connected to an entity, apply the updated component data
        if (isConnected && connectedEntity != 0) {
            Scene* scene = m_activeScene->getScene().get();
            if (scene) {
                printf("DEBUG: Node component data changed, updating entity %d\n", connectedEntity);
                applyComponentToEntity(connectedEntity, scene, node);
            }
        }
    }
    
    void Node::setupNodeAppearance(NodeType nodeType) {
        switch (nodeType) {
            case NodeType::SpriteComponent:
                headerColor = IM_COL32(70, 180, 70, 255);   // Green
                description = "Sprite rendering component";
                minSize = ImVec2(200, 120);
                size = ImVec2(240, 160);
                break;
                
            case NodeType::Transform:
                headerColor = IM_COL32(180, 70, 70, 255);   // Red
                description = "Position, rotation, and scale";
                minSize = ImVec2(200, 140);
                size = ImVec2(240, 140);
                break;
                
            case NodeType::Collider:
                headerColor = IM_COL32(180, 180, 70, 255);  // Yellow
                description = "Collision detection component";
                break;
                
            case NodeType::RigidBody:
                headerColor = IM_COL32(70, 70, 180, 255);   // Blue
                description = "Physics simulation component";
                break;
                
            // Player components
            case NodeType::PlayerController:
            case NodeType::PlayerStats:
            case NodeType::PlayerPhysics:
            case NodeType::PlayerInventory:
            case NodeType::PlayerAbilities:
            case NodeType::PlayerState:
                headerColor = IM_COL32(180, 70, 180, 255);  // Purple
                description = "Player-specific component";
                break;
                
            // Math nodes
            case NodeType::MathAdd:
            case NodeType::MathSubtract:
            case NodeType::MathMultiply:
            case NodeType::MathDivide:
            case NodeType::MathSin:
            case NodeType::MathCos:
            case NodeType::MathAbs:
            case NodeType::MathMin:
            case NodeType::MathMax:
            case NodeType::MathClamp:
            case NodeType::MathLerp:
                headerColor = IM_COL32(100, 150, 100, 255); // Dark green
                description = "Mathematical operation";
                break;
                
            // Logic nodes
            case NodeType::LogicAND:
            case NodeType::LogicOR:
            case NodeType::LogicXOR:
            case NodeType::LogicNOT:
            case NodeType::Branch:
                headerColor = IM_COL32(150, 100, 100, 255); // Dark red
                description = "Logical operation";
                break;
                
            // Constant nodes
            case NodeType::ConstantFloat:
            case NodeType::ConstantInt:
            case NodeType::ConstantBool:
            case NodeType::ConstantString:
            case NodeType::ConstantVector2:
                headerColor = IM_COL32(100, 100, 150, 255); // Dark blue
                description = "Constant value";
                break;
                
            // Event nodes
            case NodeType::OnKeyPress:
            case NodeType::OnMouseClick:
            case NodeType::OnCollision:
                headerColor = IM_COL32(150, 150, 100, 255); // Dark yellow
                description = "Event trigger";
                break;
                
            // Utility nodes
            case NodeType::Print:
                headerColor = IM_COL32(120, 120, 120, 255); // Gray
                description = "Debug output";
                break;
                
            // Entity creation nodes
            case NodeType::EntitySpawner:
                headerColor = IM_COL32(70, 130, 200, 255);  // Blue
                description = "Spawns entities from templates";
                minSize = ImVec2(160, 120);
                size = ImVec2(180, 140);
                break;
                
            case NodeType::EntityFactory:
                headerColor = IM_COL32(50, 150, 200, 255);  // Light blue
                description = "Creates new entities dynamically";
                minSize = ImVec2(160, 120);
                size = ImVec2(180, 140);
                break;
                
            case NodeType::ParticleEffect:
                headerColor = IM_COL32(255, 140, 0, 255);   // Orange
                description = "Adds particle effects to entities";
                minSize = ImVec2(160, 120);
                size = ImVec2(180, 140);
                break;
                
            case NodeType::Comment:
                headerColor = IM_COL32(80, 80, 80, 255);    // Dark gray
                description = "Comment node";
                minSize = ImVec2(200, 100);
                size = ImVec2(200, 100);
                break;
                
            default:
                headerColor = IM_COL32(100, 100, 100, 255); // Default gray
                description = "Unknown node type";
                break;
        }
    }
    
    void Node::createComponentData(NodeType nodeType) {
        switch (nodeType) {
            case NodeType::SpriteComponent:
                componentData = std::make_shared<Sprite>();
                break;
            case NodeType::Transform:
                componentData = std::make_shared<Transform>();
                break;
            case NodeType::Collider:
                componentData = std::make_shared<Collider>();
                break;
            case NodeType::RigidBody:
                componentData = std::make_shared<RigidBody>();
                break;
            case NodeType::PlayerController:
                componentData = std::make_shared<PlayerController>();
                break;
            case NodeType::PlayerStats:
                componentData = std::make_shared<PlayerStats>();
                break;
            case NodeType::PlayerPhysics:
                componentData = std::make_shared<PlayerPhysics>();
                break;
            case NodeType::PlayerInventory:
                componentData = std::make_shared<PlayerInventory>();
                break;
            case NodeType::PlayerAbilities:
                componentData = std::make_shared<PlayerAbilities>();
                break;
            case NodeType::PlayerState:
                componentData = std::make_shared<PlayerState>();
                break;
            case NodeType::ParticleEffect:
                componentData = std::make_shared<ParticleEffect>();
                break;
            case NodeType::ParticleSystem:
                componentData = std::make_shared<ParticleEffect>();
                break;
            case NodeType::LightSource:
                componentData = std::make_shared<LightSource>();
                break;
            case NodeType::AudioSource:
                componentData = std::make_shared<AudioSource>();
                break;
            default:
                // Non-component nodes don't have component data
                break;
        }
    }

    std::vector<std::pair<EntityID, std::string>> NodeEditorWindow::getSceneEntities() const {
        std::vector<std::pair<EntityID, std::string>> entities;
        
        if (m_activeScene && m_activeScene->getScene()) {
            auto scene = m_activeScene->getScene();
            auto allEntities = scene->getAllLivingEntities();
            
            for (EntityID entity : allEntities) {
                std::string entityName = scene->getEntityName(entity);
                if (entityName.empty()) {
                    entityName = "Entity " + std::to_string(entity);
                }
                entities.emplace_back(entity, entityName);
            }
            
            // Sort by entity ID for consistent ordering
            std::sort(entities.begin(), entities.end(), 
                     [](const std::pair<EntityID, std::string>& a, const std::pair<EntityID, std::string>& b) {
                         return a.first < b.first;
                     });
        }
        
        return entities;
    }

    void NodeEditorWindow::openEntitySelectionPopup(int nodeId) {
        m_entitySelectionNodeId = nodeId;
        m_showEntitySelectionPopup = true;
        ImGui::OpenPopup("Select Entity");
    }

    void NodeEditorWindow::openKeySelectionPopup(int nodeId) {
        printf("DEBUG: openKeySelectionPopup called for node %d\n", nodeId);
        m_keySelectionNodeId = nodeId;
        m_showKeySelectionPopup = true;
        printf("DEBUG: Set key selection request for node %d\n", nodeId);
    }

    const char* NodeEditorWindow::getKeyName(int keyCode) const {
        switch (keyCode) {
            case 87: return "W";
            case 65: return "A"; 
            case 83: return "S";
            case 68: return "D";
            case 32: return "Space";
            case 13: return "Enter";
            case 16: return "Shift";
            case 17: return "Ctrl";
            case 69: return "E";
            case 70: return "F";
            case 71: return "G";
            case 72: return "H";
            case 81: return "Q";
            case 82: return "R";
            case 84: return "T";
            case 89: return "Y";
            case 85: return "U";
            case 73: return "I";
            case 79: return "O";
            case 80: return "P";
            default: return "Unknown";
        }
    }
    
    
    void NodeEditorWindow::setCodeRefreshCallback(CodeRefreshCallback callback) {
        m_codeRefreshCallback = callback;
    }
    
    void NodeEditorWindow::saveNodeLayout(EntityID entity) {
        try {
            std::filesystem::create_directories("node_layouts");
            std::string layoutFile = getLayoutFilePath(entity);
            
            std::ofstream file(layoutFile);
            if (!file.is_open()) {
                printf("ERROR: Failed to open layout file for writing: %s\n", layoutFile.c_str());
                return;
            }
            
            // Save node positions and types
            file << "# Node Layout for Entity " << entity << "\n";
            file << "nodes=" << m_nodes.size() << "\n";
            
            for (const auto& node : m_nodes) {
                file << "node_id=" << node->id << "\n";
                file << "node_type=" << static_cast<int>(node->type) << "\n";
                file << "node_name=" << node->name << "\n";
                file << "position_x=" << node->position.x << "\n";
                file << "position_y=" << node->position.y << "\n";
                file << "associated_entity=" << node->associatedEntity << "\n";
                
                // Save additional node-specific data
                if (node->type == NodeType::OnKeyPress) {
                    file << "key_code=" << node->keyCode << "\n";
                }
                
                file << "---\n";
            }
            
            // Save connections
            file << "connections=" << m_connections.size() << "\n";
            for (const auto& connection : m_connections) {
                file << "connection_id=" << connection.id << "\n";
                file << "output_pin=" << connection.outputPinId << "\n";
                file << "input_pin=" << connection.inputPinId << "\n";
                file << "---\n";
            }
            
            file.close();
            printf("DEBUG: Saved node layout for entity %u to %s\n", entity, layoutFile.c_str());
            
        } catch (const std::exception& e) {
            printf("ERROR: Exception saving node layout: %s\n", e.what());
        }
    }
    
    void NodeEditorWindow::loadNodeLayout(EntityID entity) {
        try {
            std::string layoutFile = getLayoutFilePath(entity);
            
            if (!std::filesystem::exists(layoutFile)) {
                printf("DEBUG: No saved layout found for entity %u\n", entity);
                return;
            }
            
            std::ifstream file(layoutFile);
            if (!file.is_open()) {
                printf("ERROR: Failed to open layout file for reading: %s\n", layoutFile.c_str());
                return;
            }
            
            std::string line;
            std::vector<std::tuple<int, NodeType, std::string, ImVec2, EntityID, int>> savedNodes; // Added keyCode as 6th element
            std::vector<std::tuple<int, int, int>> savedConnections;
            
            // Parse saved nodes
            while (std::getline(file, line)) {
                if (line.substr(0, 6) == "nodes=") {
                    int nodeCount = std::stoi(line.substr(6));
                    printf("DEBUG: Loading %d nodes from layout\n", nodeCount);
                    
                    for (int i = 0; i < nodeCount; i++) {
                        int nodeId = 0, nodeType = 0, keyCode = -1; // Default keyCode to -1 (not set)
                        std::string nodeName;
                        float posX = 0, posY = 0;
                        EntityID assocEntity = 0;
                        
                        // Read node properties
                        while (std::getline(file, line) && line != "---") {
                            if (line.substr(0, 8) == "node_id=") {
                                nodeId = std::stoi(line.substr(8));
                            }
                            else if (line.substr(0, 10) == "node_type=") {
                                nodeType = std::stoi(line.substr(10));
                            }
                            else if (line.substr(0, 10) == "node_name=") {
                                nodeName = line.substr(10);
                            }
                            else if (line.substr(0, 11) == "position_x=") {
                                posX = std::stof(line.substr(11));
                            }
                            else if (line.substr(0, 11) == "position_y=") {
                                posY = std::stof(line.substr(11));
                            }
                            else if (line.substr(0, 18) == "associated_entity=") {
                                assocEntity = std::stoul(line.substr(18));
                            }
                            else if (line.substr(0, 9) == "key_code=") {
                                keyCode = std::stoi(line.substr(9));
                            }
                        }
                        
                        savedNodes.emplace_back(nodeId, static_cast<NodeType>(nodeType), nodeName, ImVec2(posX, posY), assocEntity, keyCode);
                        printf("DEBUG: Parsed node: %s (type %d) at (%.1f, %.1f), keyCode=%d\n", 
                               nodeName.c_str(), nodeType, posX, posY, keyCode);
                    }
                    break;
                }
            }
            
            // Parse saved connections
            while (std::getline(file, line)) {
                if (line.substr(0, 12) == "connections=") {
                    int connectionCount = std::stoi(line.substr(12));
                    printf("DEBUG: Loading %d connections from layout\n", connectionCount);
                    
                    for (int i = 0; i < connectionCount; i++) {
                        int connectionId = 0, outputPin = 0, inputPin = 0;
                        
                        if (std::getline(file, line) && line.substr(0, 14) == "connection_id=") {
                            connectionId = std::stoi(line.substr(14));
                        }
                        if (std::getline(file, line) && line.substr(0, 11) == "output_pin=") {
                            outputPin = std::stoi(line.substr(11));
                        }
                        if (std::getline(file, line) && line.substr(0, 10) == "input_pin=") {
                            inputPin = std::stoi(line.substr(10));
                        }
                        
                        // Skip separator
                        std::getline(file, line); // "---"
                        
                        savedConnections.emplace_back(connectionId, outputPin, inputPin);
                    }
                    break;
                }
            }
            
            file.close();
            
            // Apply saved layout to current nodes and create missing nodes
            for (const auto& [savedId, savedType, savedName, savedPos, savedEntity, savedKeyCode] : savedNodes) {
                bool nodeFound = false;
                
                // First, try to find existing node to restore position and properties
                for (auto& currentNode : m_nodes) {
                    if (currentNode->type == savedType && 
                        (savedType == NodeType::Entity ? currentNode->associatedEntity == savedEntity : true)) {
                        currentNode->position = savedPos;
                        if (savedType == NodeType::OnKeyPress && savedKeyCode != -1) {
                            currentNode->keyCode = savedKeyCode;
                        }
                        printf("DEBUG: Restored position (%.1f, %.1f) for existing %s node\n", 
                               savedPos.x, savedPos.y, savedName.c_str());
                        nodeFound = true;
                        break;
                    }
                }
                
                // If node wasn't found and it's a non-ECS node type, create it
                if (!nodeFound && !isECSComponentNode(savedType)) {
                    printf("DEBUG: Creating missing non-ECS node: %s (type %d)\n", savedName.c_str(), static_cast<int>(savedType));
                    int newNodeId = createNode(savedType, savedPos);
                    
                    // Restore specific properties for the created node
                    if (newNodeId != -1 && m_nodeMap.count(newNodeId)) {
                        Node* newNode = m_nodeMap[newNodeId];
                        if (savedType == NodeType::OnKeyPress && savedKeyCode != -1) {
                            newNode->keyCode = savedKeyCode;
                            printf("DEBUG: Restored keyCode %d for OnKeyPress node\n", savedKeyCode);
                        }
                        if (savedType == NodeType::Entity && savedEntity != 0) {
                            newNode->associatedEntity = savedEntity;
                            printf("DEBUG: Restored associated entity %u for Entity node\n", savedEntity);
                        }
                    }
                }
            }
            
            printf("SUCCESS: Loaded node layout for entity %u\n", entity);
            
        } catch (const std::exception& e) {
            printf("ERROR: Exception loading node layout: %s\n", e.what());
        }
    }
    
    std::string NodeEditorWindow::getLayoutFilePath(EntityID entity) const {
        return "node_layouts/entity_" + std::to_string(entity) + "_layout.txt";
    }

    void NodeEditorWindow::generateCodeFromNodes() {
        if (m_nodes.empty()) {
            printf("INFO: No nodes to generate code from\n");
            return;
        }
        
        printf("INFO: Generating C++ code from %zu nodes\n", m_nodes.size());
        saveGeneratedCodeToFiles();
    }
    
    void NodeEditorWindow::exportNodeGraphAsCode() {
        generateCodeFromNodes();
    }
    
    std::string NodeEditorWindow::generateEntitySpawnerCode() {
        std::stringstream code;
        
        code << "#pragma once\n\n";
        code << "#include \"../components/Components.h\"\n";
        code << "#include \"../scene/Scene.h\"\n";
        code << "#include \"../core/Engine.h\"\n\n";
        code << "// Auto-generated from Node Editor\n";
        code << "// This file contains EntitySpawner systems created visually\n\n";
        code << "class GeneratedEntitySpawners {\n";
        code << "public:\n";
        code << "    static void setupAllSpawners(Scene* scene) {\n";
        code << "        if (!scene) return;\n\n";
        
        // Find all EntitySpawner nodes and generate setup code
        int spawnerCount = 0;
        for (const auto& node : m_nodes) {
            if (node->type == NodeType::EntitySpawner) {
                spawnerCount++;
                
                // Find connected entity nodes
                EntityID targetEntity = 0;
                EntityID templateEntity = 0;
                
                for (const auto& connection : m_connections) {
                    // Check if this spawner node is connected
                    for (const auto& inputPin : node->inputPins) {
                        if (inputPin.id == connection.inputPinId) {
                            // Find the connected output node
                            for (const auto& otherNode : m_nodes) {
                                for (const auto& outputPin : otherNode->outputPins) {
                                    if (outputPin.id == connection.outputPinId && otherNode->type == NodeType::Entity) {
                                        if (inputPin.name == "Target") {
                                            targetEntity = otherNode->associatedEntity;
                                        } else if (inputPin.name == "Template") {
                                            templateEntity = otherNode->associatedEntity;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
                
                code << "        // EntitySpawner " << spawnerCount << "\n";
                if (targetEntity != 0) {
                    code << "        setupEntitySpawner" << spawnerCount << "(scene, " << targetEntity;
                    if (templateEntity != 0) {
                        code << ", " << templateEntity;
                    }
                    code << ");\n\n";
                }
            }
        }
        
        code << "    }\n\n";
        
        // Generate individual spawner setup methods
        spawnerCount = 0;
        for (const auto& node : m_nodes) {
            if (node->type == NodeType::EntitySpawner) {
                spawnerCount++;
                
                code << "private:\n";
                code << "    static void setupEntitySpawner" << spawnerCount << "(Scene* scene, EntityID targetEntity";
                
                // Check if we have a template entity
                bool hasTemplate = false;
                for (const auto& connection : m_connections) {
                    for (const auto& inputPin : node->inputPins) {
                        if (inputPin.id == connection.inputPinId && inputPin.name == "Template") {
                            hasTemplate = true;
                            break;
                        }
                    }
                    if (hasTemplate) break;
                }
                
                if (hasTemplate) {
                    code << ", EntityID templateEntity";
                }
                code << ") {\n";
                
                code << "        if (!scene->hasComponent<EntitySpawner>(targetEntity)) {\n";
                code << "            EntitySpawner spawner;\n";
                code << "            spawner.clearTemplates();\n";
                code << "            \n";
                
                if (hasTemplate) {
                    code << "            // Create template from template entity\n";
                    code << "            std::string templateName = scene->getEntityName(templateEntity);\n";
                    code << "            if (templateName.empty()) {\n";
                    code << "                templateName = \"Entity_\" + std::to_string(templateEntity);\n";
                    code << "            }\n";
                    code << "            \n";
                    code << "            EntitySpawner::SpawnTemplate newTemplate(templateName, \"\", Vector2(20, 0), Vector2(100, 0));\n";
                    code << "            newTemplate.scale = 1.0f;\n";
                    code << "            newTemplate.lifeTime = 0.0f;\n";
                    code << "            newTemplate.hasCollider = scene->hasComponent<Collider>(templateEntity);\n";
                    code << "            newTemplate.hasRigidBody = scene->hasComponent<RigidBody>(templateEntity);\n";
                    code << "            newTemplate.spriteFile = \"TEMPLATE_ENTITY_\" + std::to_string(templateEntity);\n";
                    code << "            \n";
                    code << "            spawner.templates.push_back(newTemplate);\n";
                    code << "            spawner.selectedTemplate = 0;\n";
                } else {
                    code << "            // Default arrow template\n";
                    code << "            spawner.addTemplate(\"Arrow\", \"arrow.png\", Vector2(20, 0), Vector2(100, 0));\n";
                }
                
                code << "            \n";
                code << "            scene->addComponent<EntitySpawner>(targetEntity, spawner);\n";
                code << "        }\n";
                code << "    }\n\n";
            }
        }
        
        code << "};\n\n";
        code << "// Usage: Call GeneratedEntitySpawners::setupAllSpawners(scene) in your game initialization\n";
        
        return code.str();
    }
    
    std::string NodeEditorWindow::generateComponentSystemCode() {
        std::stringstream code;
        
        code << "#pragma once\n\n";
        code << "#include \"../components/Components.h\"\n";
        code << "#include \"../scene/Scene.h\"\n\n";
        code << "// Auto-generated from Node Editor\n";
        code << "// This file contains component systems created visually\n\n";
        code << "class GeneratedComponentSystems {\n";
        code << "public:\n";
        code << "    static void setupAllComponents(Scene* scene) {\n";
        code << "        if (!scene) return;\n\n";
        
        // Find all Entity nodes and their connected components
        int entityCount = 0;
        for (const auto& node : m_nodes) {
            if (node->type == NodeType::Entity && node->associatedEntity != 0) {
                entityCount++;
                
                code << "        // Entity " << entityCount << ": " << node->name << "\n";
                code << "        setupEntity" << entityCount << "(scene, " << node->associatedEntity << ");\n\n";
            }
        }
        
        code << "    }\n\n";
        
        // Generate individual entity setup methods
        entityCount = 0;
        for (const auto& node : m_nodes) {
            if (node->type == NodeType::Entity && node->associatedEntity != 0) {
                entityCount++;
                
                code << "private:\n";
                code << "    static void setupEntity" << entityCount << "(Scene* scene, EntityID entity) {\n";
                code << "        // Components for " << node->name << "\n";
                
                // Find all components connected to this entity
                std::vector<std::string> components;
                for (const auto& connection : m_connections) {
                    // Check if this is an output from our entity node
                    for (const auto& outputPin : node->outputPins) {
                        if (outputPin.id == connection.outputPinId) {
                            // Find the connected component node
                            for (const auto& componentNode : m_nodes) {
                                for (const auto& inputPin : componentNode->inputPins) {
                                    if (inputPin.id == connection.inputPinId) {
                                        std::string componentType = this->getNodeTypeName(componentNode->type);
                                        components.push_back(componentType);
                                        
                                        // Generate component-specific setup code
                                        switch (componentNode->type) {
                                            case NodeType::SpriteComponent:
                                                code << "        if (!scene->hasComponent<Sprite>(entity)) {\n";
                                                code << "            Sprite sprite;\n";
                                                code << "            sprite.visible = true;\n";
                                                code << "            sprite.layer = 1;\n";
                                                code << "            scene->addComponent<Sprite>(entity, sprite);\n";
                                                code << "        }\n";
                                                break;
                                                
                                            case NodeType::Transform:
                                                code << "        if (!scene->hasComponent<Transform>(entity)) {\n";
                                                code << "            Transform transform;\n";
                                                code << "            transform.position = Vector2(0, 0);\n";
                                                code << "            transform.scale = Vector2(1, 1);\n";
                                                code << "            transform.rotation = 0.0f;\n";
                                                code << "            scene->addComponent<Transform>(entity, transform);\n";
                                                code << "        }\n";
                                                break;
                                                
                                            case NodeType::PlayerController:
                                                code << "        if (!scene->hasComponent<PlayerController>(entity)) {\n";
                                                code << "            scene->addComponent<PlayerController>(entity, PlayerController());\n";
                                                code << "            // Auto-add required components\n";
                                                code << "            if (!scene->hasComponent<PlayerStats>(entity)) {\n";
                                                code << "                scene->addComponent<PlayerStats>(entity, PlayerStats());\n";
                                                code << "            }\n";
                                                code << "            if (!scene->hasComponent<PlayerPhysics>(entity)) {\n";
                                                code << "                scene->addComponent<PlayerPhysics>(entity, PlayerPhysics());\n";
                                                code << "            }\n";
                                                code << "        }\n";
                                                break;
                                                
                                            case NodeType::Collider:
                                                code << "        if (!scene->hasComponent<Collider>(entity)) {\n";
                                                code << "            Collider collider;\n";
                                                code << "            collider.size = Vector2(32, 32);\n";
                                                code << "            scene->addComponent<Collider>(entity, collider);\n";
                                                code << "        }\n";
                                                break;
                                                
                                            case NodeType::RigidBody:
                                                code << "        if (!scene->hasComponent<RigidBody>(entity)) {\n";
                                                code << "            scene->addComponent<RigidBody>(entity, RigidBody());\n";
                                                code << "        }\n";
                                                break;
                                                
                                            default:
                                                code << "        // " << componentType << " component setup\n";
                                                break;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
                
                if (components.empty()) {
                    code << "        // No components connected to this entity\n";
                }
                
                code << "    }\n\n";
            }
        }
        
        code << "};\n\n";
        code << "// Usage: Call GeneratedComponentSystems::setupAllComponents(scene) in your game initialization\n";
        
        return code.str();
    }

    std::string NodeEditorWindow::generateEventSystemCode() {
        std::stringstream code;
        
        code << "#pragma once\n\n";
        code << "#include \"../components/Components.h\"\n";
        code << "#include \"../scene/Scene.h\"\n";
        code << "#include \"../core/Engine.h\"\n";
        code << "#include \"../input/InputManager.h\"\n\n";
        code << "// Auto-generated from Node Editor\n";
        code << "// Event systems for input handling and game events\n\n";
        code << "class GeneratedEventSystems {\n";
        code << "public:\n";
        code << "    static void setupAllEvents(Scene* scene) {\n";
        code << "        if (!scene) return;\n\n";
        
        // Count event nodes for setup
        int keyEventCount = 0;
        int mouseEventCount = 0;
        int collisionEventCount = 0;
        
        for (const auto& node : m_nodes) {
            if (node->type == NodeType::OnKeyPress) keyEventCount++;
            else if (node->type == NodeType::OnMouseClick) mouseEventCount++;
            else if (node->type == NodeType::OnCollision) collisionEventCount++;
        }
        
        code << "        // Event system setup from Node Editor design\n";
        code << "        printf(\"INFO: Setting up event systems - %d key events, %d mouse events, %d collision events\\n\", " 
             << keyEventCount << ", " << mouseEventCount << ", " << collisionEventCount << ");\n";
        code << "    }\n\n";
        
        code << "    static void updateEvents(Scene* scene, float deltaTime) {\n";
        code << "        if (!scene) return;\n\n";
        code << "        auto& engine = Engine::getInstance();\n";
        code << "        auto* inputManager = engine.getInputManager();\n";
        code << "        if (!inputManager) return;\n\n";
        
        // Generate OnKeyPress event handlers with actual logic
        bool hasKeyEvents = false;
        for (const auto& node : m_nodes) {
            if (node->type == NodeType::OnKeyPress) {
                if (!hasKeyEvents) {
                    code << "        // OnKeyPress event handling from Node Editor\n";
                    hasKeyEvents = true;
                }
                
                code << "        // Key " << getKeyName(node->keyCode) << " (" << node->keyCode << ") event handler\n";
                code << "        if (inputManager->isKeyPressed(" << node->keyCode << ")) {\n";
                
                // Find connected entities and generate specific actions
                bool foundConnections = false;
                for (const auto& connection : m_connections) {
                    for (const auto& inputPin : node->inputPins) {
                        if (inputPin.id == connection.inputPinId) {
                            // Find the connected entity
                            for (const auto& otherNode : m_nodes) {
                                if (otherNode->type == NodeType::Entity) {
                                    for (const auto& outputPin : otherNode->outputPins) {
                                        if (outputPin.id == connection.outputPinId) {
                                            foundConnections = true;
                                            EntityID targetEntity = otherNode->associatedEntity;
                                            code << "            // Action for entity " << targetEntity << " (" << otherNode->name << ")\n";
                                            code << "            EntityID entity = " << targetEntity << ";\n";
                                            code << "            if (scene->isEntityValid(entity)) {\n";
                                            
                                            // Generate specific actions based on connected components
                                            generateKeyPressActions(code, targetEntity, otherNode.get());
                                            
                                            code << "            }\n";
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
                
                if (!foundConnections) {
                    code << "            printf(\"DEBUG: Key " << getKeyName(node->keyCode) << " pressed but no entities connected\\n\");\n";
                }
                
                code << "        }\n\n";
            }
        }
        
        // Generate OnMouseClick handlers
        bool hasMouseEvents = false;
        for (const auto& node : m_nodes) {
            if (node->type == NodeType::OnMouseClick) {
                if (!hasMouseEvents) {
                    code << "        // OnMouseClick event handling from Node Editor\n";
                    hasMouseEvents = true;
                }
                
                code << "        // Mouse click event handler\n";
                code << "        if (inputManager->isMouseButtonPressed(0)) { // Left mouse button\n";
                code << "            Vector2 mousePos = inputManager->getMousePosition();\n";
                code << "            printf(\"INFO: Mouse clicked at (%.2f, %.2f)\\n\", mousePos.x, mousePos.y);\n";
                code << "            // Add mouse click logic here based on connected nodes\n";
                code << "        }\n\n";
            }
        }
        
        // Generate OnCollision handlers
        bool hasCollisionEvents = false;
        for (const auto& node : m_nodes) {
            if (node->type == NodeType::OnCollision) {
                if (!hasCollisionEvents) {
                    code << "        // OnCollision event handling from Node Editor\n";
                    hasCollisionEvents = true;
                }
                
                code << "        // Collision detection for entities\n";
                code << "        auto entities = scene->getAllLivingEntities();\n";
                code << "        for (EntityID entity : entities) {\n";
                code << "            if (scene->hasComponent<Collider>(entity)) {\n";
                code << "                // Check for collisions and trigger collision events\n";
                code << "                // This would integrate with the physics system\n";
                code << "            }\n";
                code << "        }\n\n";
            }
        }
        
        if (!hasKeyEvents && !hasMouseEvents && !hasCollisionEvents) {
            code << "        // No event nodes found in Node Editor design\n";
        }
        
        code << "    }\n\n";
        code << "private:\n";
        code << "    // Helper method to apply movement to entity\n";
        code << "    static void applyMovement(Scene* scene, EntityID entity, float deltaX, float deltaY, float speed) {\n";
        code << "        if (scene->hasComponent<Transform>(entity)) {\n";
        code << "            auto& transform = scene->getComponent<Transform>(entity);\n";
        code << "            transform.position.x += deltaX * speed;\n";
        code << "            transform.position.y += deltaY * speed;\n";
        code << "        }\n";
        code << "    }\n";
        code << "};\n\n";
        
        return code.str();
    }

    void NodeEditorWindow::generateKeyPressActions(std::stringstream& code, EntityID entityId, Node* entityNode) {
        // Analyze what components this entity has to generate appropriate actions
        bool hasPlayerController = false;
        bool hasTransform = false;
        bool hasSprite = false;
        bool hasEntitySpawner = false;
        
        // Check connected components
        for (const auto& connection : m_connections) {
            for (const auto& node : m_nodes) {
                if (node->type != NodeType::Entity) {
                    for (const auto& inputPin : node->inputPins) {
                        if (inputPin.id == connection.inputPinId) {
                            for (const auto& outputPin : entityNode->outputPins) {
                                if (outputPin.id == connection.outputPinId) {
                                    switch (node->type) {
                                        case NodeType::PlayerController:
                                            hasPlayerController = true;
                                            break;
                                        case NodeType::Transform:
                                            hasTransform = true;
                                            break;
                                        case NodeType::SpriteComponent:
                                            hasSprite = true;
                                            break;
                                        case NodeType::EntitySpawner:
                                            hasEntitySpawner = true;
                                            break;
                                        default:
                                            break;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        
        // Generate actions based on components
        if (hasPlayerController) {
            code << "                // Player movement actions\n";
            code << "                if (scene->hasComponent<PlayerController>(entity)) {\n";
            code << "                    auto& controller = scene->getComponent<PlayerController>(entity);\n";
            code << "                    float moveSpeed = controller.speed * deltaTime;\n";
            code << "                    \n";
            code << "                    // Apply directional movement based on key\n";
            code << "                    switch (" << getKeyCodeFromNode(entityNode) << ") {\n";
            code << "                        case 87: // W key - move up\n";
            code << "                            applyMovement(scene, entity, 0, -moveSpeed, 1.0f);\n";
            code << "                            break;\n";
            code << "                        case 83: // S key - move down\n";
            code << "                            applyMovement(scene, entity, 0, moveSpeed, 1.0f);\n";
            code << "                            break;\n";
            code << "                        case 65: // A key - move left\n";
            code << "                            applyMovement(scene, entity, -moveSpeed, 0, 1.0f);\n";
            code << "                            break;\n";
            code << "                        case 68: // D key - move right\n";
            code << "                            applyMovement(scene, entity, moveSpeed, 0, 1.0f);\n";
            code << "                            break;\n";
            code << "                        case 32: // Space key - jump or action\n";
            code << "                            controller.jumpPressed = true;\n";
            code << "                            printf(\"INFO: Player jump action triggered\\n\");\n";
            code << "                            break;\n";
            code << "                        default:\n";
            code << "                            printf(\"INFO: Key action for player entity %u\\n\", entity);\n";
            code << "                            break;\n";
            code << "                    }\n";
            code << "                }\n";
        }
        
        if (hasEntitySpawner) {
            code << "                // Entity spawning actions\n";
            code << "                if (scene->hasComponent<EntitySpawner>(entity)) {\n";
            code << "                    auto& spawner = scene->getComponent<EntitySpawner>(entity);\n";
            code << "                    // Trigger spawning on key press\n";
            code << "                    spawner.shouldSpawn = true;\n";
            code << "                    spawner.lastSpawnTime = 0.0f; // Force immediate spawn\n";
            code << "                    printf(\"INFO: Spawner triggered by key press\\n\");\n";
            code << "                }\n";
        }
        
        if (hasTransform && !hasPlayerController) {
            code << "                // Basic transform movement\n";
            code << "                if (scene->hasComponent<Transform>(entity)) {\n";
            code << "                    float moveSpeed = 100.0f * deltaTime;\n";
            code << "                    switch (" << getKeyCodeFromNode(entityNode) << ") {\n";
            code << "                        case 87: // W\n";
            code << "                            applyMovement(scene, entity, 0, -moveSpeed, 1.0f);\n";
            code << "                            break;\n";
            code << "                        case 83: // S\n";
            code << "                            applyMovement(scene, entity, 0, moveSpeed, 1.0f);\n";
            code << "                            break;\n";
            code << "                        case 65: // A\n";
            code << "                            applyMovement(scene, entity, -moveSpeed, 0, 1.0f);\n";
            code << "                            break;\n";
            code << "                        case 68: // D\n";
            code << "                            applyMovement(scene, entity, moveSpeed, 0, 1.0f);\n";
            code << "                            break;\n";
            code << "                        default:\n";
            code << "                            printf(\"INFO: Transform action for entity %u\\n\", entity);\n";
            code << "                            break;\n";
            code << "                    }\n";
            code << "                }\n";
        }
        
        if (hasSprite) {
            code << "                // Sprite-related actions (visual feedback)\n";
            code << "                if (scene->hasComponent<Sprite>(entity)) {\n";
            code << "                    auto& sprite = scene->getComponent<Sprite>(entity);\n";
            code << "                    // Could change sprite appearance, animate, etc.\n";
            code << "                    printf(\"INFO: Sprite action triggered for entity %u\\n\", entity);\n";
            code << "                }\n";
        }
        
        // Default action if no specific components
        if (!hasPlayerController && !hasEntitySpawner && !hasTransform && !hasSprite) {
            code << "                printf(\"INFO: Generic key action for entity %u\\n\", entity);\n";
        }
    }

    int NodeEditorWindow::getKeyCodeFromNode(Node* entityNode) {
        // Find connected OnKeyPress nodes to get the key code
        for (const auto& connection : m_connections) {
            for (const auto& node : m_nodes) {
                if (node->type == NodeType::OnKeyPress) {
                    for (const auto& inputPin : node->inputPins) {
                        if (inputPin.id == connection.inputPinId) {
                            for (const auto& outputPin : entityNode->outputPins) {
                                if (outputPin.id == connection.outputPinId) {
                                    return node->keyCode;
                                }
                            }
                        }
                    }
                }
            }
        }
        return 0; // Default key code
    }

    std::string NodeEditorWindow::generateGameLogicCode() {
        std::stringstream code;
        
        code << "#pragma once\n\n";
        code << "#include \"../components/Components.h\"\n";
        code << "#include \"../scene/Scene.h\"\n";
        code << "#include \"../systems/CoreSystems.h\"\n\n";
        code << "// Auto-generated from Node Editor\n";
        code << "// Complete game logic systems\n\n";
        code << "class GeneratedGameLogic {\n";
        code << "public:\n";
        code << "    static void initializeGameSystems(Scene* scene) {\n";
        code << "        if (!scene) return;\n\n";
        code << "        // Initialize core game systems based on Node Editor design\n";
        code << "        auto* systemManager = scene->getSystemManager();\n";
        code << "        if (systemManager) {\n";
        code << "            // Core rendering and physics systems\n";
        code << "            // systemManager->addSystem<RenderSystem>();\n";
        code << "            // systemManager->addSystem<PhysicsSystem>();\n";
        code << "            // systemManager->addSystem<PlayerControllerSystem>();\n";
        code << "            // systemManager->addSystem<EntitySpawnerSystem>();\n";
        code << "        }\n\n";
        code << "        printf(\"INFO: Game logic systems initialized from Node Editor\\n\");\n";
        code << "    }\n\n";
        
        code << "    static void updateGameLogic(Scene* scene, float deltaTime) {\n";
        code << "        if (!scene) return;\n\n";
        code << "        // Update entity spawners\n";
        code << "        updateEntitySpawners(scene, deltaTime);\n\n";
        code << "        // Update player logic\n";  
        code << "        updatePlayerLogic(scene, deltaTime);\n";
        code << "    }\n\n";
        
        code << "private:\n";
        code << "    static void updateEntitySpawners(Scene* scene, float deltaTime) {\n";
        code << "        // EntitySpawner update logic handled by engine's spawner system\n";
        code << "    }\n\n";
        
        code << "    static void updatePlayerLogic(Scene* scene, float deltaTime) {\n";
        code << "        // Player-specific game logic updates\n";
        code << "        auto entities = scene->getAllLivingEntities();\n";
        code << "        for (auto entity : entities) {\n";
        code << "            if (scene->hasComponent<PlayerController>(entity)) {\n";
        code << "                // Player entity logic here\n";
        code << "            }\n";
        code << "        }\n";
        code << "    }\n";
        code << "};\n\n";
        
        return code.str();
    }

    std::string NodeEditorWindow::generateCompleteSceneCode() {
        std::stringstream code;
        
        code << "#include \"NodeEditorGenerated.h\"\n";
        code << "#include \"../scene/Scene.h\"\n";
        code << "#include \"../core/Engine.h\"\n\n";
        code << "// Auto-generated complete scene implementation\n";
        code << "// This file provides a ready-to-use game scene based on your Node Editor design\n\n";
        
        code << "class GeneratedGameScene {\n";
        code << "public:\n";
        code << "    static std::shared_ptr<Scene> createGameScene() {\n";
        code << "        auto scene = std::make_shared<Scene>();\n";
        code << "        if (!scene) return nullptr;\n\n";
        
        code << "        // Initialize the scene with ECS components\n";
        code << "        scene->initialize();\n\n";
        
        code << "        // Apply all Node Editor generated systems\n";
        code << "        NodeEditorGenerated::initializeScene(scene.get());\n\n";
        
        code << "        printf(\"INFO: Generated game scene created with all systems\\n\");\n";
        code << "        return scene;\n";
        code << "    }\n\n";
        
        code << "    static void updateScene(Scene* scene, float deltaTime) {\n";
        code << "        if (!scene) return;\n\n";
        code << "        // Update all Node Editor generated systems\n";
        code << "        NodeEditorGenerated::updateGame(scene, deltaTime);\n\n";
        code << "        // Update core scene systems\n";
        code << "        scene->update(deltaTime);\n";
        code << "    }\n\n";
        
        code << "    static void renderScene(Scene* scene, Renderer* renderer) {\n";
        code << "        if (!scene || !renderer) return;\n\n";
        code << "        // Render the scene\n";
        code << "        scene->render(renderer);\n";
        code << "    }\n";
        code << "};\n\n";
        
        code << "// Usage Example:\n";
        code << "// auto gameScene = GeneratedGameScene::createGameScene();\n";
        code << "// In game loop: GeneratedGameScene::updateScene(gameScene.get(), deltaTime);\n";
        code << "// In render loop: GeneratedGameScene::renderScene(gameScene.get(), renderer);\n";
        
        return code.str();
    }

    bool NodeEditorWindow::isECSComponentNode(NodeType type) {
        switch (type) {
            case NodeType::Transform:
            case NodeType::SpriteComponent:
            case NodeType::PlayerController:
            case NodeType::EntitySpawner:
            case NodeType::Collider:
            case NodeType::RigidBody:
                return true;
            case NodeType::OnKeyPress:
            case NodeType::OnMouseClick:
            case NodeType::OnCollision:
            case NodeType::EventTrigger:
            case NodeType::EventListener:
                return false; // Event nodes are not ECS components
            default:
                return false;
        }
    }
    
    void NodeEditorWindow::saveGeneratedCodeToFiles() {
        try {
            // Create game directory if it doesn't exist
            std::filesystem::create_directories("game");
            
            // Generate EntitySpawner code
            std::string spawnerCode = generateEntitySpawnerCode();
            if (!spawnerCode.empty()) {
                std::ofstream spawnerFile("game/GeneratedEntitySpawners.h");
                if (spawnerFile.is_open()) {
                    spawnerFile << spawnerCode;
                    spawnerFile.close();
                    printf("INFO: Generated EntitySpawner code saved to game/GeneratedEntitySpawners.h\n");
                }
            }
            
            // Generate Component System code
            std::string componentCode = generateComponentSystemCode();
            if (!componentCode.empty()) {
                std::ofstream componentFile("game/GeneratedComponentSystems.h");
                if (componentFile.is_open()) {
                    componentFile << componentCode;
                    componentFile.close();
                    printf("INFO: Generated Component System code saved to game/GeneratedComponentSystems.h\n");
                }
            }
            
            // Generate Event System code (NEW)
            std::string eventCode = generateEventSystemCode();
            if (!eventCode.empty()) {
                std::ofstream eventFile("game/GeneratedEventSystems.h");
                if (eventFile.is_open()) {
                    eventFile << eventCode;
                    eventFile.close();
                    printf("INFO: Generated Event System code saved to game/GeneratedEventSystems.h\n");
                }
            }
            
            // Generate Game Logic code (NEW)
            std::string gameLogicCode = generateGameLogicCode();
            if (!gameLogicCode.empty()) {
                std::ofstream gameLogicFile("game/GeneratedGameLogic.h");
                if (gameLogicFile.is_open()) {
                    gameLogicFile << gameLogicCode;
                    gameLogicFile.close();
                    printf("INFO: Generated Game Logic code saved to game/GeneratedGameLogic.h\n");
                }
            }
            
            // Generate Complete Game Scene code (NEW)
            std::string sceneCode = generateCompleteSceneCode();
            if (!sceneCode.empty()) {
                std::ofstream sceneFile("game/GeneratedScene.cpp");
                if (sceneFile.is_open()) {
                    sceneFile << sceneCode;
                    sceneFile.close();
                    printf("INFO: Generated Complete Scene code saved to game/GeneratedScene.cpp\n");
                }
            }
            
            // Generate main game logic integration file
            std::stringstream mainCode;
            mainCode << "#pragma once\n\n";
            mainCode << "#include \"GeneratedEntitySpawners.h\"\n";
            mainCode << "#include \"GeneratedComponentSystems.h\"\n";
            mainCode << "#include \"GeneratedEventSystems.h\"\n";
            mainCode << "#include \"GeneratedGameLogic.h\"\n";
            mainCode << "#include \"../scene/Scene.h\"\n\n";
            mainCode << "// Auto-generated from Node Editor\n";
            mainCode << "// Main integration file for all generated systems\n\n";
            mainCode << "class NodeEditorGenerated {\n";
            mainCode << "public:\n";
            mainCode << "    static void initializeScene(Scene* scene) {\n";
            mainCode << "        if (!scene) return;\n\n";
            mainCode << "        // Setup all component systems from Node Editor\n";
            mainCode << "        GeneratedComponentSystems::setupAllComponents(scene);\n\n";
            mainCode << "        // Setup all EntitySpawner systems from Node Editor\n";
            mainCode << "        GeneratedEntitySpawners::setupAllSpawners(scene);\n\n";
            mainCode << "        // Setup all Event systems from Node Editor\n";
            mainCode << "        GeneratedEventSystems::setupAllEvents(scene);\n\n";
            mainCode << "        // Initialize game logic systems\n";
            mainCode << "        GeneratedGameLogic::initializeGameSystems(scene);\n\n";
            mainCode << "        printf(\"INFO: Initialized complete game scene with Node Editor generated systems\\n\");\n";
            mainCode << "    }\n\n";
            mainCode << "    static void updateGame(Scene* scene, float deltaTime) {\n";
            mainCode << "        if (!scene) return;\n\n";
            mainCode << "        // Update event systems\n";
            mainCode << "        GeneratedEventSystems::updateEvents(scene, deltaTime);\n\n";
            mainCode << "        // Update game logic\n";
            mainCode << "        GeneratedGameLogic::updateGameLogic(scene, deltaTime);\n";
            mainCode << "    }\n";
            mainCode << "};\n\n";
            mainCode << "// Usage: \n";
            mainCode << "// 1. Call NodeEditorGenerated::initializeScene(scene) in your game initialization\n";
            mainCode << "// 2. Call NodeEditorGenerated::updateGame(scene, deltaTime) in your game loop\n";
            mainCode << "// This provides a complete game framework from your visual node designs\n";
            
            std::ofstream mainFile("game/NodeEditorGenerated.h");
            if (mainFile.is_open()) {
                mainFile << mainCode.str();
                mainFile.close();
                printf("INFO: Generated main integration file saved to game/NodeEditorGenerated.h\n");
            }
            
            printf("SUCCESS: Complete game code generation finished! Files saved to game/ directory\n");
            printf("INFO: Include 'game/NodeEditorGenerated.h' in your project and call:\n");
            printf("      - NodeEditorGenerated::initializeScene(scene) in setup\n");
            printf("      - NodeEditorGenerated::updateGame(scene, deltaTime) in game loop\n");
            
            // Refresh the Game Code Viewer
            if (m_codeRefreshCallback) {
                m_codeRefreshCallback();
                printf("INFO: Game Code Viewer refreshed with complete game systems\n");
            }
            
        } catch (const std::exception& e) {
            printf("ERROR: Failed to save generated code files: %s\n", e.what());
        }
    }

    // ====== NODE EXECUTION SYSTEM ======
    void NodeEditorWindow::executeLogicGraph() {
        printf("DEBUG: Executing logic graph with %zu nodes\n", m_nodes.size());
        
        // First, reset all nodes
        resetExecution();
        
        // Execute nodes in dependency order
        std::set<int> executedNodes;
        std::queue<Node*> nodesToExecute;
        
        // Start with nodes that have no input connections (source nodes)
        for (auto& node : m_nodes) {
            bool hasInputConnections = false;
            for (const auto& pin : node->inputPins) {
                if (pin.connected) {
                    hasInputConnections = true;
                    break;
                }
            }
            
            if (!hasInputConnections) {
                switch (node->type) {
                    case NodeType::ConstantFloat:
                    case NodeType::ConstantInt:
                    case NodeType::ConstantBool:
                    case NodeType::ConstantString:
                    case NodeType::OnKeyPress:
                    case NodeType::OnMouseClick:
                    case NodeType::TimerNode:
                        nodesToExecute.push(node.get());
                        break;
                    default:
                        break;
                }
            }
        }
        
        // Execute nodes in waves, propagating data through connections
        int maxIterations = 100; // Prevent infinite loops
        int iteration = 0;
        
        while (!nodesToExecute.empty() && iteration < maxIterations) {
            Node* currentNode = nodesToExecute.front();
            nodesToExecute.pop();
            
            if (executedNodes.find(currentNode->id) == executedNodes.end()) {
                executeNode(currentNode);
                executedNodes.insert(currentNode->id);
                
                // Add connected nodes to execution queue
                for (const auto& pin : currentNode->outputPins) {
                    if (pin.connected) {
                        // Find the connected node
                        for (auto& targetNode : m_nodes) {
                            for (const auto& inputPin : targetNode->inputPins) {
                                if (inputPin.id == pin.connectedPinId) {
                                    if (executedNodes.find(targetNode->id) == executedNodes.end()) {
                                        nodesToExecute.push(targetNode.get());
                                    }
                                    break;
                                }
                            }
                        }
                    }
                }
            }
            
            iteration++;
        }
        
        if (iteration >= maxIterations) {
            printf("WARNING: Node execution hit max iterations limit\n");
        }
        
        printf("DEBUG: Logic graph execution completed in %d iterations\n", iteration);
    }

    void NodeEditorWindow::resetExecution() {
        for (auto& node : m_nodes) {
            node->reset();
        }
        printf("DEBUG: All nodes reset for execution\n");
    }

    void NodeEditorWindow::executeNode(Node* node) {
        if (!node || node->executed) {
            return;
        }
        
        printf("DEBUG: Executing node %d (%s) of type %d\n", 
               node->id, node->name.c_str(), static_cast<int>(node->type));
        
        // Copy data from connected input pins
        for (auto& inputPin : node->inputPins) {
            if (inputPin.connected) {
                // Find the source pin and copy its data
                for (const auto& sourceNode : m_nodes) {
                    for (const auto& outputPin : sourceNode->outputPins) {
                        if (outputPin.id == inputPin.connectedPinId) {
                            // Copy data based on pin type
                            switch (inputPin.dataType) {
                                case PinDataType::Float:
                                    if (sourceNode->floatParams.find("Output") != sourceNode->floatParams.end()) {
                                        node->setFloatParam(inputPin.name, sourceNode->getFloatParam("Output", 0.0f));
                                    }
                                    break;
                                case PinDataType::Int:
                                    if (sourceNode->intParams.find("Output") != sourceNode->intParams.end()) {
                                        node->setIntParam(inputPin.name, sourceNode->getIntParam("Output", 0));
                                    }
                                    break;
                                case PinDataType::Bool:
                                    if (sourceNode->boolParams.find("Output") != sourceNode->boolParams.end()) {
                                        node->setBoolParam(inputPin.name, sourceNode->getBoolParam("Output", false));
                                    }
                                    break;
                                case PinDataType::String:
                                    if (sourceNode->stringParams.find("Output") != sourceNode->stringParams.end()) {
                                        node->setStringParam(inputPin.name, sourceNode->getStringParam("Output", ""));
                                    }
                                    break;
                                default:
                                    break;
                            }
                            break;
                        }
                    }
                }
            }
        }
        
        // Execute the node
        node->execute();
    }

} // namespace NodeEditor
