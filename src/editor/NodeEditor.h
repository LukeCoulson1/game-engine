#pragma once

#include <imgui.h>
#include <vector>
#include <memory>
#include <string>
#include <unordered_map>
#include "../components/Components.h"
#include "../scene/Scene.h"

// Forward declarations
class SceneWindow;

namespace NodeEditor {

    // Constants for pin ID calculation
    static constexpr int PIN_ID_MULTIPLIER = 100;
    static constexpr int TRANSFORM_PIN_OFFSET = 1;
    static constexpr int SPRITE_PIN_OFFSET = 2;
    static constexpr int PLAYER_PIN_OFFSET = 3;

    enum class NodeType {
        Entity,
        SpriteComponent,
        PlayerController,
        PlayerStats,
        PlayerPhysics,
        PlayerInventory,
        PlayerAbilities,
        PlayerState,
        Transform,
        Rotation,
        Scale,
        Collider,
        RigidBody
    };

    enum class PinType {
        Input,
        Output
    };

    struct Pin {
        int id;
        PinType type;
        std::string name;
        ImVec2 position;
        bool connected = false;
        int connectedPinId = -1;
    };

    struct Node {
        int id;
        NodeType type;
        std::string name;
        ImVec2 position;
        ImVec2 size;
        std::vector<Pin> inputPins;
        std::vector<Pin> outputPins;
        bool selected = false;
        EntityID associatedEntity = 0;
        
        // Resize functionality
        bool resizing = false;
        ImVec2 minSize = ImVec2(120, 80);
        ImVec2 maxSize = ImVec2(400, 300);
        
        // Component-specific data
        std::shared_ptr<Component> componentData;        Node(int nodeId, NodeType nodeType, const std::string& nodeName, ImVec2 pos);
        void draw(ImVec2 displayPos = ImVec2(0, 0), float zoom = 1.0f);
        void drawSpriteNodeContent(ImVec2 nodePos, ImVec2 nodeSize);
        void drawRotationNodeContent(ImVec2 nodePos, ImVec2 nodeSize);
        void drawScaleNodeContent(ImVec2 nodePos, ImVec2 nodeSize);
        void drawTextureSelectionPopup();
        void drawPins(ImDrawList* drawList, float zoom = 1.0f);
        bool isInside(ImVec2 point) const;
        bool isOnResizeHandle(ImVec2 point) const;
        Pin* getPinById(int pinId);
          // Helper methods
        ImVec2 getNodeSize() const;
        void updatePinPositions(ImVec2 nodePos, float zoom = 1.0f);
        void setSize(ImVec2 newSize);
        
        // Image file utilities
        static bool isImageFile(const std::string& extension);
        static void scanDirectoryForImages(const std::string& directory, std::vector<std::string>& imageFiles);
    };

    struct Connection {
        int id;
        int outputPinId;
        int inputPinId;
        ImVec2 startPos;
        ImVec2 endPos;
        
        Connection(int connId, int outPin, int inPin) 
            : id(connId), outputPinId(outPin), inputPinId(inPin) {}
    };

    class NodeEditorWindow {
    public:
        NodeEditorWindow();
        ~NodeEditorWindow();
        
        void show(bool* open, SceneWindow* activeScene);
        void update();
        
        // Node management
        int createNode(NodeType type, ImVec2 position);
        void deleteNode(int nodeId);
        void selectNode(int nodeId);
        void clearSelection();
        
        // Connection management
        int createConnection(int outputPinId, int inputPinId);
        void deleteConnection(int connectionId);
        
        // Entity operations
        void applyNodesToEntity(EntityID entity, Scene* scene);
        void loadEntityAsNodes(EntityID entity, Scene* scene);
        
    private:
        std::vector<std::unique_ptr<Node>> m_nodes;
        std::vector<Connection> m_connections;
        std::unordered_map<int, Node*> m_nodeMap;
        
        int m_nextNodeId = 1;
        int m_nextPinId = 1;
        int m_nextConnectionId = 1;
        
        ImVec2 m_canvasPos;
        ImVec2 m_canvasSize;
        ImVec2 m_scrolling{0.0f, 0.0f};
        float m_zoom = 1.0f;
        float m_minZoom = 0.2f;
        float m_maxZoom = 3.0f;
        bool m_dragging = false;
        int m_draggedNodeId = -1;
        ImVec2 m_dragOffset;
        
        // Node resizing
        bool m_resizing = false;
        int m_resizingNodeId = -1;
        ImVec2 m_resizeStartPos;
        ImVec2 m_resizeStartSize;
        
        // Connection creation
        bool m_creatingConnection = false;
        int m_connectionStartPinId = -1;
        ImVec2 m_connectionEndPos;
        
        SceneWindow* m_activeScene = nullptr;
        
        // Helper methods
        void drawGrid();
        void drawNodes();
        void drawConnections();
        void handleInput();
        void drawNodeCreationMenu();
        void drawConnectionInProgress();
        
        std::string getNodeTypeName(NodeType type);
        ImU32 getNodeColor(NodeType type);
        ImU32 getPinColor(NodeType nodeType);
        
        // Pin position calculation
        ImVec2 calculatePinPosition(const Node* node, const Pin* pin);
        void updatePinPositions();
          // Validation
        bool canConnect(int outputPinId, int inputPinId);
        bool wouldCreateCycle(int outputPinId, int inputPinId);
        
        // Component management helpers
        void applyComponentToEntity(EntityID entity, Scene* scene, Node* componentNode);
        void removeComponentFromEntity(EntityID entity, Scene* scene, NodeType componentType);
    };

} // namespace NodeEditor
