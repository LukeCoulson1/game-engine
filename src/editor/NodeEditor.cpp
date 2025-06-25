#include "NodeEditor.h"
#include "SceneWindow.h"
#include "../core/Engine.h"
#include <algorithm>
#include <cmath>

namespace NodeEditor {

    Node::Node(int nodeId, NodeType nodeType, const std::string& nodeName, ImVec2 pos)
        : id(nodeId), type(nodeType), name(nodeName), position(pos), size(120, 80) {
        
        // Set up pins based on node type
        switch (type) {
            case NodeType::Entity:
                // Entity nodes have multiple output pins for each component
                outputPins.push_back({nodeId * 100 + 1, PinType::Output, "Transform", ImVec2()});
                outputPins.push_back({nodeId * 100 + 2, PinType::Output, "Sprite", ImVec2()});
                outputPins.push_back({nodeId * 100 + 3, PinType::Output, "Player", ImVec2()});
                break;
                
            case NodeType::SpriteComponent:
                inputPins.push_back({nodeId * 100 + 1, PinType::Input, "Entity", ImVec2()});
                componentData = std::make_shared<Sprite>();
                break;
                
            case NodeType::PlayerController:
                inputPins.push_back({nodeId * 100 + 1, PinType::Input, "Entity", ImVec2()});
                componentData = std::make_shared<PlayerController>();
                break;
                
            case NodeType::PlayerStats:
                inputPins.push_back({nodeId * 100 + 1, PinType::Input, "Entity", ImVec2()});
                componentData = std::make_shared<PlayerStats>();
                break;
                
            case NodeType::PlayerPhysics:
                inputPins.push_back({nodeId * 100 + 1, PinType::Input, "Entity", ImVec2()});
                componentData = std::make_shared<PlayerPhysics>();
                break;
                
            case NodeType::PlayerInventory:
                inputPins.push_back({nodeId * 100 + 1, PinType::Input, "Entity", ImVec2()});
                componentData = std::make_shared<PlayerInventory>();
                break;
                
            case NodeType::PlayerAbilities:
                inputPins.push_back({nodeId * 100 + 1, PinType::Input, "Entity", ImVec2()});
                componentData = std::make_shared<PlayerAbilities>();
                break;
                
            case NodeType::PlayerState:
                inputPins.push_back({nodeId * 100 + 1, PinType::Input, "Entity", ImVec2()});
                componentData = std::make_shared<PlayerState>();
                break;
                
            case NodeType::Transform:
                inputPins.push_back({nodeId * 100 + 1, PinType::Input, "Entity", ImVec2()});
                componentData = std::make_shared<Transform>();
                break;
                
            case NodeType::Collider:
                inputPins.push_back({nodeId * 100 + 1, PinType::Input, "Entity", ImVec2()});
                componentData = std::make_shared<Collider>();
                break;
                
            case NodeType::RigidBody:
                inputPins.push_back({nodeId * 100 + 1, PinType::Input, "Entity", ImVec2()});
                componentData = std::make_shared<RigidBody>();
                break;
        }
    }    void Node::draw(ImVec2 displayPos) {
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        
        // Use provided display position, or fallback to stored position
        ImVec2 nodePos = (displayPos.x != 0 || displayPos.y != 0) ? displayPos : position;
        ImVec2 nodeRectMin = nodePos;
        ImVec2 nodeRectMax = ImVec2(nodePos.x + size.x, nodePos.y + size.y);
        
        // Node background
        ImU32 nodeColor = selected ? IM_COL32(100, 150, 255, 200) : IM_COL32(60, 60, 60, 200);
        drawList->AddRectFilled(nodeRectMin, nodeRectMax, nodeColor, 5.0f);
        drawList->AddRect(nodeRectMin, nodeRectMax, IM_COL32(200, 200, 200, 255), 5.0f, 0, 2.0f);
        
        // Node title
        ImVec2 titlePos = ImVec2(nodePos.x + 10, nodePos.y + 10);
        drawList->AddText(titlePos, IM_COL32(255, 255, 255, 255), name.c_str());
        
        // Draw input pins
        float pinY = nodePos.y + 35;
        for (auto& pin : inputPins) {
            pin.position = ImVec2(nodePos.x - 8, pinY);
            ImU32 pinColor = pin.connected ? IM_COL32(100, 255, 100, 255) : IM_COL32(150, 150, 150, 255);
            drawList->AddCircleFilled(pin.position, 6.0f, pinColor);
            
            // Pin label
            ImVec2 labelPos = ImVec2(pin.position.x + 15, pin.position.y - 8);
            drawList->AddText(labelPos, IM_COL32(200, 200, 200, 255), pin.name.c_str());
            
            pinY += 20;
        }        
        // Draw output pins
        pinY = nodePos.y + 35;
        for (auto& pin : outputPins) {
            pin.position = ImVec2(nodePos.x + size.x + 8, pinY);
            ImU32 pinColor = pin.connected ? IM_COL32(100, 255, 100, 255) : IM_COL32(150, 150, 150, 255);
            drawList->AddCircleFilled(pin.position, 6.0f, pinColor);
            
            // Pin label (right-aligned)
            ImVec2 textSize = ImGui::CalcTextSize(pin.name.c_str());
            ImVec2 labelPos = ImVec2(pin.position.x - textSize.x - 15, pin.position.y - 8);
            drawList->AddText(labelPos, IM_COL32(200, 200, 200, 255), pin.name.c_str());
            
            pinY += 20;
        }
    }

    bool Node::isInside(ImVec2 point) const {
        return point.x >= position.x && point.x <= position.x + size.x &&
               point.y >= position.y && point.y <= position.y + size.y;
    }

    Pin* Node::getPinById(int pinId) {
        for (auto& pin : inputPins) {
            if (pin.id == pinId) return &pin;
        }
        for (auto& pin : outputPins) {
            if (pin.id == pinId) return &pin;
        }
        return nullptr;
    }

    NodeEditorWindow::NodeEditorWindow() {
    }

    NodeEditorWindow::~NodeEditorWindow() {
    }

    void NodeEditorWindow::show(bool* open, SceneWindow* activeScene) {
        m_activeScene = activeScene;
        
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
            ImGui::EndMenuBar();
        }
        
        // Canvas
        m_canvasPos = ImGui::GetCursorScreenPos();
        m_canvasSize = ImGui::GetContentRegionAvail();
        
        ImGui::InvisibleButton("canvas", m_canvasSize, ImGuiButtonFlags_MouseButtonLeft | ImGuiButtonFlags_MouseButtonRight);
        
        handleInput();
        drawGrid();
        drawNodes();
        drawConnections();
        drawConnectionInProgress();
        
        ImGui::End();
    }

    void NodeEditorWindow::drawGrid() {
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        ImVec2 canvasMin = m_canvasPos;
        ImVec2 canvasMax = ImVec2(m_canvasPos.x + m_canvasSize.x, m_canvasPos.y + m_canvasSize.y);
        
        drawList->PushClipRect(canvasMin, canvasMax, true);
        
        // Grid
        const float gridStep = 50.0f;
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
            ImVec2 displayPos = ImVec2(m_canvasPos.x + node->position.x + m_scrolling.x, 
                                      m_canvasPos.y + node->position.y + m_scrolling.y);
            
            // Draw the node at the display position without modifying the node's actual position
            node->draw(displayPos);
            
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
                ImVec2 cp1 = ImVec2(p1.x + 50, p1.y);
                ImVec2 cp2 = ImVec2(p2.x - 50, p2.y);
                
                drawList->AddBezierCubic(p1, cp1, cp2, p2, IM_COL32(200, 200, 100, 255), 3.0f);
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
                
                ImVec2 cp1 = ImVec2(p1.x + 50, p1.y);
                ImVec2 cp2 = ImVec2(p2.x - 50, p2.y);
                
                drawList->AddBezierCubic(p1, cp1, cp2, p2, IM_COL32(255, 255, 100, 200), 2.0f);
            }
        }
    }    void NodeEditorWindow::handleInput() {
        ImGuiIO& io = ImGui::GetIO();
        ImVec2 mousePos = io.MousePos;
        
        if (ImGui::IsItemHovered()) {
            // Check for pin clicks first
            bool clickedOnPin = false;
            for (auto& node : m_nodes) {
                // Check input pins
                for (auto& pin : node->inputPins) {
                    float distance = sqrt(pow(mousePos.x - pin.position.x, 2) + pow(mousePos.y - pin.position.y, 2));
                    if (distance <= 10.0f) {
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
                    if (distance <= 10.0f) {
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
            }
            
            // Right-click to create nodes (if not creating connection)
            if (ImGui::IsMouseClicked(ImGuiMouseButton_Right) && !m_creatingConnection) {
                ImGui::OpenPopup("CreateNode");
            }
            
            // Handle node dragging
            if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !clickedOnPin) {
                // Check if clicking on a node
                bool clickedOnNode = false;
                for (auto& node : m_nodes) {
                    ImVec2 relativePos = ImVec2(mousePos.x - m_canvasPos.x - m_scrolling.x, 
                                               mousePos.y - m_canvasPos.y - m_scrolling.y);
                    if (node->isInside(relativePos)) {
                        selectNode(node->id);
                        m_dragging = true;
                        m_draggedNodeId = node->id;
                        m_dragOffset = ImVec2(relativePos.x - node->position.x, relativePos.y - node->position.y);
                        clickedOnNode = true;
                        break;
                    }
                }
                
                if (!clickedOnNode) {
                    clearSelection();
                }
            }
            
            // Canvas dragging (middle mouse or left mouse when not on node)
            if (ImGui::IsMouseDragging(ImGuiMouseButton_Middle) || 
                (ImGui::IsMouseDragging(ImGuiMouseButton_Left) && !m_dragging && !m_creatingConnection)) {
                m_scrolling.x += io.MouseDelta.x;
                m_scrolling.y += io.MouseDelta.y;
            }
        }
        
        // Handle node dragging
        if (m_dragging && ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
            for (auto& node : m_nodes) {
                if (node->id == m_draggedNodeId) {
                    ImVec2 relativePos = ImVec2(mousePos.x - m_canvasPos.x - m_scrolling.x, 
                                               mousePos.y - m_canvasPos.y - m_scrolling.y);
                    node->position = ImVec2(relativePos.x - m_dragOffset.x, relativePos.y - m_dragOffset.y);
                    break;
                }
            }
        }
        
        if (ImGui::IsMouseReleased(ImGuiMouseButton_Left)) {
            m_dragging = false;
            m_draggedNodeId = -1;
        }        
        // Create node popup
        if (ImGui::BeginPopup("CreateNode")) {
            drawNodeCreationMenu();
            ImGui::EndPopup();
        }
    }

    int NodeEditorWindow::createConnection(int outputPinId, int inputPinId) {
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
        
        return connectionId;
    }

    void NodeEditorWindow::drawNodeCreationMenu() {
        ImVec2 mousePos = ImGui::GetIO().MousePos;
        ImVec2 nodePos = ImVec2(mousePos.x - m_canvasPos.x - m_scrolling.x, 
                               mousePos.y - m_canvasPos.y - m_scrolling.y);
        
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
        
        if (ImGui::MenuItem("Collider")) {
            createNode(NodeType::Collider, nodePos);
            ImGui::CloseCurrentPopup();
        }
        
        if (ImGui::MenuItem("RigidBody")) {
            createNode(NodeType::RigidBody, nodePos);
            ImGui::CloseCurrentPopup();
        }
        
        ImGui::Separator();
        
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
    }

    int NodeEditorWindow::createNode(NodeType type, ImVec2 position) {
        int nodeId = m_nextNodeId++;
        std::string nodeName = getNodeTypeName(type);
        
        auto node = std::make_unique<Node>(nodeId, type, nodeName, position);
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
        for (auto& node : m_nodes) {
            // Calculate display position
            ImVec2 displayPos = ImVec2(m_canvasPos.x + node->position.x + m_scrolling.x, 
                                      m_canvasPos.y + node->position.y + m_scrolling.y);
            
            float pinY = displayPos.y + 35;
            for (auto& pin : node->inputPins) {
                pin.position = ImVec2(displayPos.x - 8, pinY);
                pinY += 20;
            }
            
            pinY = displayPos.y + 35;
            for (auto& pin : node->outputPins) {
                pin.position = ImVec2(displayPos.x + node->size.x + 8, pinY);
                pinY += 20;
            }
        }
    }

    std::string NodeEditorWindow::getNodeTypeName(NodeType type) {
        switch (type) {
            case NodeType::Entity: return "Entity";
            case NodeType::SpriteComponent: return "Sprite";
            case NodeType::PlayerController: return "Controller";
            case NodeType::PlayerStats: return "Stats";
            case NodeType::PlayerPhysics: return "Physics";
            case NodeType::PlayerInventory: return "Inventory";
            case NodeType::PlayerAbilities: return "Abilities";
            case NodeType::PlayerState: return "State";
            case NodeType::Transform: return "Transform";
            case NodeType::Collider: return "Collider";
            case NodeType::RigidBody: return "RigidBody";
            default: return "Unknown";
        }
    }

    void NodeEditorWindow::applyNodesToEntity(EntityID entity, Scene* scene) {
        if (!scene) return;
        
        // Find connected component nodes and apply them to the entity
        for (auto& node : m_nodes) {
            if (node->type == NodeType::Entity) continue;
            
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
                // Apply the component to the entity based on type
                switch (node->type) {
                    case NodeType::SpriteComponent:
                        if (!scene->hasComponent<Sprite>(entity)) {
                            scene->addComponent<Sprite>(entity, *std::static_pointer_cast<Sprite>(node->componentData));
                        }
                        break;
                    case NodeType::PlayerController:
                        if (!scene->hasComponent<PlayerController>(entity)) {
                            scene->addComponent<PlayerController>(entity, *std::static_pointer_cast<PlayerController>(node->componentData));
                        }
                        break;
                    case NodeType::PlayerStats:
                        if (!scene->hasComponent<PlayerStats>(entity)) {
                            scene->addComponent<PlayerStats>(entity, *std::static_pointer_cast<PlayerStats>(node->componentData));
                        }
                        break;
                    case NodeType::PlayerPhysics:
                        if (!scene->hasComponent<PlayerPhysics>(entity)) {
                            scene->addComponent<PlayerPhysics>(entity, *std::static_pointer_cast<PlayerPhysics>(node->componentData));
                        }
                        break;
                    case NodeType::PlayerInventory:
                        if (!scene->hasComponent<PlayerInventory>(entity)) {
                            scene->addComponent<PlayerInventory>(entity, *std::static_pointer_cast<PlayerInventory>(node->componentData));
                        }
                        break;
                    case NodeType::PlayerAbilities:
                        if (!scene->hasComponent<PlayerAbilities>(entity)) {
                            scene->addComponent<PlayerAbilities>(entity, *std::static_pointer_cast<PlayerAbilities>(node->componentData));
                        }
                        break;
                    case NodeType::PlayerState:
                        if (!scene->hasComponent<PlayerState>(entity)) {
                            scene->addComponent<PlayerState>(entity, *std::static_pointer_cast<PlayerState>(node->componentData));
                        }
                        break;
                    case NodeType::Transform:
                        if (!scene->hasComponent<Transform>(entity)) {
                            scene->addComponent<Transform>(entity, *std::static_pointer_cast<Transform>(node->componentData));
                        }
                        break;
                    case NodeType::Collider:
                        if (!scene->hasComponent<Collider>(entity)) {
                            scene->addComponent<Collider>(entity, *std::static_pointer_cast<Collider>(node->componentData));
                        }
                        break;
                    case NodeType::RigidBody:
                        if (!scene->hasComponent<RigidBody>(entity)) {
                            scene->addComponent<RigidBody>(entity, *std::static_pointer_cast<RigidBody>(node->componentData));
                        }
                        break;
                }
            }
        }
    }

    void NodeEditorWindow::loadEntityAsNodes(EntityID entity, Scene* scene) {
        if (!scene) return;
        
        // Clear existing nodes
        m_nodes.clear();
        m_connections.clear();
        m_nodeMap.clear();
        
        // Create entity node
        int entityNodeId = createNode(NodeType::Entity, ImVec2(100, 100));
        m_nodeMap[entityNodeId]->associatedEntity = entity;
        
        // Create component nodes based on what the entity has
        std::vector<int> componentNodeIds;
        
        if (scene->hasComponent<Sprite>(entity)) {
            int nodeId = createNode(NodeType::SpriteComponent, ImVec2(300, 50));
            componentNodeIds.push_back(nodeId);
        }
        
        if (scene->hasComponent<PlayerController>(entity)) {
            int nodeId = createNode(NodeType::PlayerController, ImVec2(300, 120));
            componentNodeIds.push_back(nodeId);
        }
        
        if (scene->hasComponent<PlayerStats>(entity)) {
            int nodeId = createNode(NodeType::PlayerStats, ImVec2(300, 190));
            componentNodeIds.push_back(nodeId);
        }
        
        if (scene->hasComponent<PlayerPhysics>(entity)) {
            int nodeId = createNode(NodeType::PlayerPhysics, ImVec2(300, 260));
            componentNodeIds.push_back(nodeId);
        }
        
        if (scene->hasComponent<Transform>(entity)) {
            int nodeId = createNode(NodeType::Transform, ImVec2(300, 330));
            componentNodeIds.push_back(nodeId);
        }
        
        if (scene->hasComponent<Collider>(entity)) {
            int nodeId = createNode(NodeType::Collider, ImVec2(300, 400));
            componentNodeIds.push_back(nodeId);
        }
        
        // Create connections (simplified - would need proper pin matching)
        // This is a basic implementation that connects the first output pin to first input pin
        Node* entityNode = m_nodeMap[entityNodeId];
        for (int componentNodeId : componentNodeIds) {
            Node* componentNode = m_nodeMap[componentNodeId];
            if (!entityNode->outputPins.empty() && !componentNode->inputPins.empty()) {
                int connectionId = m_nextConnectionId++;
                m_connections.emplace_back(connectionId, entityNode->outputPins[0].id, componentNode->inputPins[0].id);
            }
        }
    }

    bool NodeEditorWindow::canConnect(int outputPinId, int inputPinId) {
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
        
        if (!outputPin || !inputPin || !outputNode || !inputNode) return false;
        
        // Can't connect node to itself
        if (outputNode == inputNode) return false;
        
        // Output must be from Entity node, input must be to Component node
        if (outputNode->type != NodeType::Entity) return false;
        if (inputNode->type == NodeType::Entity) return false;
        
        // Check if input pin is already connected
        if (inputPin->connected) return false;
        
        return true;
    }
    
    bool NodeEditorWindow::wouldCreateCycle(int outputPinId, int inputPinId) {
        // For this simple system, cycles shouldn't be possible since
        // we only connect Entity -> Component (one direction)
        return false;
    }

} // namespace NodeEditor
