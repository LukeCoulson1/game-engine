#include "NodeEditor.h"
#include "SceneWindow.h"
#include "../core/Engine.h"
#include "../utils/ConfigManager.h"
#include "../utils/ResourceManager.h"
#include <algorithm>
#include <cmath>
#include <filesystem>

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
        
        // Sprite nodes are larger to accommodate texture preview
        ImVec2 nodeSize = size;
        if (type == NodeType::SpriteComponent) {
            nodeSize = ImVec2(200, 120);
        }
        ImVec2 nodeRectMax = ImVec2(nodePos.x + nodeSize.x, nodePos.y + nodeSize.y);
        
        // Node background
        ImU32 nodeColor = selected ? IM_COL32(100, 150, 255, 200) : IM_COL32(60, 60, 60, 200);
        drawList->AddRectFilled(nodeRectMin, nodeRectMax, nodeColor, 5.0f);
        drawList->AddRect(nodeRectMin, nodeRectMax, IM_COL32(200, 200, 200, 255), 5.0f, 0, 2.0f);
        
        // Node title
        ImVec2 titlePos = ImVec2(nodePos.x + 10, nodePos.y + 10);
        drawList->AddText(titlePos, IM_COL32(255, 255, 255, 255), name.c_str());
        
        // Special handling for Sprite nodes
        if (type == NodeType::SpriteComponent) {
            drawSpriteNodeContent(nodePos, nodeSize);
        }
        
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
            pin.position = ImVec2(nodePos.x + nodeSize.x + 8, pinY);
            ImU32 pinColor = pin.connected ? IM_COL32(100, 255, 100, 255) : IM_COL32(150, 150, 150, 255);
            drawList->AddCircleFilled(pin.position, 6.0f, pinColor);
            
            // Pin label (right-aligned)
            ImVec2 textSize = ImGui::CalcTextSize(pin.name.c_str());
            ImVec2 labelPos = ImVec2(pin.position.x - textSize.x - 15, pin.position.y - 8);
            drawList->AddText(labelPos, IM_COL32(200, 200, 200, 255), pin.name.c_str());
            
            pinY += 20;
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

    void Node::drawTextureSelectionPopup() {
        ImGui::Text("Select Texture:");
        ImGui::Separator();
        
        // Get asset folder from config
        std::string assetFolder = ConfigManager::getInstance().getAssetFolder();
        
        // Collect image files
        std::vector<std::string> imageFiles;
        
        if (std::filesystem::exists(assetFolder)) {
            // Scan main assets folder
            for (const auto& entry : std::filesystem::directory_iterator(assetFolder)) {
                if (entry.is_regular_file()) {
                    std::string filename = entry.path().filename().string();
                    std::string extension = entry.path().extension().string();
                    std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);
                    
                    if (extension == ".png" || extension == ".jpg" || extension == ".jpeg" || 
                        extension == ".bmp" || extension == ".tga") {
                        imageFiles.push_back(assetFolder + "/" + filename);
                    }
                }
            }
            
            // Also scan tiles subfolder
            std::string tilesFolder = assetFolder + "/tiles";
            if (std::filesystem::exists(tilesFolder)) {
                for (const auto& entry : std::filesystem::directory_iterator(tilesFolder)) {
                    if (entry.is_regular_file()) {
                        std::string filename = entry.path().filename().string();
                        std::string extension = entry.path().extension().string();
                        std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);
                        
                        if (extension == ".png" || extension == ".jpg" || extension == ".jpeg" || 
                            extension == ".bmp" || extension == ".tga") {
                            imageFiles.push_back(tilesFolder + "/" + filename);
                        }
                    }
                }
            }
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
                                // This node is connected, we should find the associated entity and update it
                                // The NodeEditorWindow will handle this through the normal apply process
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
        ImVec2 nodeSize = size;
        if (type == NodeType::SpriteComponent) {
            nodeSize = ImVec2(200, 120);
        }
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
        bool clickedOnPin = false;
        bool clickedOnConnection = false;
        
        if (ImGui::IsItemHovered()) {
            // Check for pin clicks first
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
                
                if (scene) {                    // Apply the component based on the input node type
                    switch (inputNode->type) {
                        case NodeType::SpriteComponent:
                            // Always apply/update the sprite component with the node's data (including texture)
                            if (scene->hasComponent<Sprite>(entity)) {
                                scene->removeComponent<Sprite>(entity);
                            }
                            scene->addComponent<Sprite>(entity, *std::static_pointer_cast<Sprite>(inputNode->componentData));
                            break;
                        case NodeType::PlayerController:
                            if (!scene->hasComponent<PlayerController>(entity)) {
                                scene->addComponent<PlayerController>(entity, *std::static_pointer_cast<PlayerController>(inputNode->componentData));
                            }
                            break;
                        case NodeType::PlayerStats:
                            if (!scene->hasComponent<PlayerStats>(entity)) {
                                scene->addComponent<PlayerStats>(entity, *std::static_pointer_cast<PlayerStats>(inputNode->componentData));
                            }
                            break;
                        case NodeType::PlayerPhysics:
                            if (!scene->hasComponent<PlayerPhysics>(entity)) {
                                scene->addComponent<PlayerPhysics>(entity, *std::static_pointer_cast<PlayerPhysics>(inputNode->componentData));
                            }
                            break;
                        case NodeType::PlayerInventory:
                            if (!scene->hasComponent<PlayerInventory>(entity)) {
                                scene->addComponent<PlayerInventory>(entity, *std::static_pointer_cast<PlayerInventory>(inputNode->componentData));
                            }
                            break;
                        case NodeType::PlayerAbilities:
                            if (!scene->hasComponent<PlayerAbilities>(entity)) {
                                scene->addComponent<PlayerAbilities>(entity, *std::static_pointer_cast<PlayerAbilities>(inputNode->componentData));
                            }
                            break;
                        case NodeType::PlayerState:
                            if (!scene->hasComponent<PlayerState>(entity)) {
                                scene->addComponent<PlayerState>(entity, *std::static_pointer_cast<PlayerState>(inputNode->componentData));
                            }
                            break;
                        case NodeType::Transform:
                            if (!scene->hasComponent<Transform>(entity)) {
                                scene->addComponent<Transform>(entity, *std::static_pointer_cast<Transform>(inputNode->componentData));
                            }
                            break;
                        case NodeType::Collider:
                            if (!scene->hasComponent<Collider>(entity)) {
                                scene->addComponent<Collider>(entity, *std::static_pointer_cast<Collider>(inputNode->componentData));
                            }
                            break;
                        case NodeType::RigidBody:
                            if (!scene->hasComponent<RigidBody>(entity)) {
                                scene->addComponent<RigidBody>(entity, *std::static_pointer_cast<RigidBody>(inputNode->componentData));
                            }
                            break;
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
                    // Remove the component based on the input node type
                    switch (inputNode->type) {
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
                    }
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
    }    int NodeEditorWindow::createNode(NodeType type, ImVec2 position) {
        int nodeId = m_nextNodeId++;
        std::string nodeName = getNodeTypeName(type);
        
        auto node = std::make_unique<Node>(nodeId, type, nodeName, position);
        
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
        for (auto& node : m_nodes) {
            // Calculate display position
            ImVec2 displayPos = ImVec2(m_canvasPos.x + node->position.x + m_scrolling.x, 
                                      m_canvasPos.y + node->position.y + m_scrolling.y);
            
            // Get node size (sprite nodes are larger)
            ImVec2 nodeSize = node->size;
            if (node->type == NodeType::SpriteComponent) {
                nodeSize = ImVec2(200, 120);
            }
            
            float pinY = displayPos.y + 35;
            for (auto& pin : node->inputPins) {
                pin.position = ImVec2(displayPos.x - 8, pinY);
                pinY += 20;
            }
            
            pinY = displayPos.y + 35;
            for (auto& pin : node->outputPins) {
                pin.position = ImVec2(displayPos.x + nodeSize.x + 8, pinY);
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
            // Copy the actual sprite component data from the entity
            auto& entitySprite = scene->getComponent<Sprite>(entity);
            auto nodeSprite = std::static_pointer_cast<Sprite>(m_nodeMap[nodeId]->componentData);
            if (nodeSprite) {
                *nodeSprite = entitySprite; // Copy all sprite data including texture
            }
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
          // Create connections with proper pin matching
        Node* entityNode = m_nodeMap[entityNodeId];
        for (int componentNodeId : componentNodeIds) {
            Node* componentNode = m_nodeMap[componentNodeId];
            if (!entityNode->outputPins.empty() && !componentNode->inputPins.empty()) {
                // Find the correct output pin based on component type
                int outputPinId = -1;
                int entityNodeIdForPin = entityNode->id;
                
                switch (componentNode->type) {
                    case NodeType::SpriteComponent:
                        outputPinId = entityNodeIdForPin * 100 + 2; // Sprite pin
                        break;
                    case NodeType::PlayerController:
                    case NodeType::PlayerStats:
                    case NodeType::PlayerPhysics:
                    case NodeType::PlayerInventory:
                    case NodeType::PlayerAbilities:
                    case NodeType::PlayerState:
                        outputPinId = entityNodeIdForPin * 100 + 3; // Player pin
                        break;
                    case NodeType::Transform:
                    case NodeType::Collider:
                    case NodeType::RigidBody:
                        outputPinId = entityNodeIdForPin * 100 + 1; // Transform pin
                        break;
                }
                
                if (outputPinId != -1) {
                    int connectionId = m_nextConnectionId++;
                    m_connections.emplace_back(connectionId, outputPinId, componentNode->inputPins[0].id);
                    
                    // Mark pins as connected
                    for (auto& node : m_nodes) {
                        if (auto pin = node->getPinById(outputPinId)) {
                            pin->connected = true;
                            pin->connectedPinId = componentNode->inputPins[0].id;
                        }
                        if (auto pin = node->getPinById(componentNode->inputPins[0].id)) {
                            pin->connected = true;
                            pin->connectedPinId = outputPinId;
                        }
                    }
                }
            }
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
        
        if (!outputPin || !inputPin || !outputNode || !inputNode) return false;
        
        // Can't connect node to itself
        if (outputNode == inputNode) return false;
        
        // Output must be from Entity node, input must be to Component node
        if (outputNode->type != NodeType::Entity) return false;
        if (inputNode->type == NodeType::Entity) return false;
        
        // Check if input pin is already connected
        if (inputPin->connected) return false;
        
        // Check component-specific connection rules
        // Entity output pins: Transform (id+1), Sprite (id+2), Player (id+3)
        int entityNodeId = outputNode->id;
        int transformPinId = entityNodeId * 100 + 1;
        int spritePinId = entityNodeId * 100 + 2;
        int playerPinId = entityNodeId * 100 + 3;
        
        switch (inputNode->type) {
            case NodeType::SpriteComponent:
                // Sprite components can only connect to the Sprite output pin
                return (outputPinId == spritePinId);
                
            case NodeType::PlayerController:
            case NodeType::PlayerStats:
            case NodeType::PlayerPhysics:
            case NodeType::PlayerInventory:
            case NodeType::PlayerAbilities:
            case NodeType::PlayerState:
                // Player components can only connect to the Player output pin
                return (outputPinId == playerPinId);
                
            case NodeType::Transform:
            case NodeType::Collider:
            case NodeType::RigidBody:
                // Basic components can only connect to the Transform output pin
                return (outputPinId == transformPinId);
                
            default:
                // Unknown component type
                return false;
        }
    }
    
    bool NodeEditorWindow::wouldCreateCycle(int outputPinId, int inputPinId) {
        // For this simple system, cycles shouldn't be possible since
        // we only connect Entity -> Component (one direction)
        return false;
    }

} // namespace NodeEditor
