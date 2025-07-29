#pragma once

#include <imgui.h>
#include <vector>
#include <memory>
#include <string>
#include <unordered_map>
#include <functional>
#include "../components/Components.h"
#include "../scene/Scene.h"

// Forward declarations
class SceneWindow;

namespace NodeEditor {
    
    // Simple Vector2 struct for node editor use
    struct Vector2 {
        float x, y;
        Vector2() : x(0), y(0) {}
        Vector2(float x, float y) : x(x), y(y) {}
    };

    // Constants for pin ID calculation
    static constexpr int PIN_ID_MULTIPLIER = 100;
    static constexpr int TRANSFORM_PIN_OFFSET = 1;
    static constexpr int SPRITE_PIN_OFFSET = 2;
    static constexpr int PLAYER_PIN_OFFSET = 3;

    enum class NodeType {
        // Entity nodes
        Entity,
        
        // Component nodes  
        SpriteComponent,
        Transform,
        Collider,
        RigidBody,
        
        // Player-specific components
        PlayerController,
        PlayerStats,
        PlayerPhysics,
        PlayerInventory,
        PlayerAbilities,
        PlayerState,
        
        // NPC and AI components
        NPCController,
        AIBehavior,
        AIStateMachine,
        AIPathfinding,
        NPCDialogue,
        NPCInteraction,
        
        // Environment components
        EnvironmentCollider,
        EnvironmentTrigger,
        EnvironmentHazard,
        EnvironmentDoor,
        EnvironmentSwitch,
        EnvironmentPlatform,
        
        // Audio and Effects
        AudioSource,
        AudioListener,
        ParticleSystem,
        ParticleEmitter,
        VisualEffect,
        LightSource,
        
        // UI and Interface
        UIElement,
        UIButton,
        UIText,
        UIImage,
        UIHealthBar,
        UIInventorySlot,
        
        // Transform sub-nodes
        Rotation,
        Scale,
        
        // Logic nodes
        LogicAND,
        LogicOR,
        LogicNOT,
        LogicXOR,
        Condition,
        Branch,
        Switch,
        Compare,
        
        // Math nodes
        MathAdd,
        MathSubtract,
        MathMultiply,
        MathDivide,
        MathPower,
        MathSin,
        MathCos,
        MathAbs,
        MathMin,
        MathMax,
        MathClamp,
        MathLerp,
        MathDistance,
        MathNormalize,
        MathDotProduct,
        RandomFloat,
        RandomInt,
        
        // Event nodes
        EventTrigger,
        EventListener,
        OnCollision,
        OnKeyPress,
        OnKeyRelease,
        OnMouseClick,
        OnMouseHover,
        OnTriggerEnter,
        OnTriggerExit,
        OnEntityDestroyed,
        OnEntitySpawned,
        TimerNode,
        
        // Flow control
        Sequence,
        Parallel,
        Delay,
        Loop,
        ForLoop,
        WhileLoop,
        
        // Data nodes
        ConstantFloat,
        ConstantInt,
        ConstantString,
        ConstantBool,
        ConstantVector2,
        Variable,
        GlobalVariable,
        EntityReference,
        
        // Game-specific nodes
        MoveTowards,
        FollowPath,
        Animate,
        PlaySound,
        StopSound,
        SpawnEntity,
        DestroyEntity,
        EntitySpawner,
        EntityFactory,
        ParticleEffect,
        
        // Script and Behavior nodes
        ScriptNode,
        BehaviorTree,
        StateMachine,
        CustomScript,
        
        // Physics and Movement
        ApplyForce,
        SetVelocity,
        Raycast,
        OverlapCheck,
        PhysicsConstraint,
        
        // Game State and Management
        SceneLoader,
        GameStateManager,
        SaveGame,
        LoadGame,
        CheckGameState,
        
        // Inventory and Items
        ItemPickup,
        ItemDrop,
        InventoryAdd,
        InventoryRemove,
        InventoryCheck,
        
        // Health and Combat
        DealDamage,
        HealEntity,
        CheckHealth,
        ApplyStatusEffect,
        RemoveStatusEffect,
        
        // Dialogue and Narrative
        DialogueNode,
        DialogueChoice,
        DialogueCondition,
        QuestStart,
        QuestComplete,
        QuestCheck,
        
        // Utility nodes
        Print,
        Debug,
        Comment
    };

    enum class PinType {
        Input,
        Output
    };
    
    enum class PinDataType {
        Entity,
        Transform,
        Sprite,
        Float,
        Int,
        String,
        Bool,
        Vector2,
        Event,
        Logic,
        Any
    };

    struct Pin {
        int id;
        PinType type;
        PinDataType dataType;
        std::string name;
        ImVec2 position;
        bool connected = false;
        int connectedPinId = -1;
        std::string tooltip = "";
        
        // Data storage for constant nodes
        union {
            float floatValue;
            int intValue;
            bool boolValue;
        };
        std::string stringValue;
        Vector2 vector2Value;
    };

    struct Node {
        int id;
        NodeType type;
        std::string name;
        std::string description = "";
        ImVec2 position;
        ImVec2 size;
        std::vector<Pin> inputPins;
        std::vector<Pin> outputPins;
        bool selected = false;
        EntityID associatedEntity = 0;
        int keyCode = 0;  // For OnKeyPress nodes - stores the key code to detect
        EntityID templateEntityId = 0;  // For EntitySpawner nodes - stores the template entity ID
        
        // Visual properties
        ImU32 headerColor = IM_COL32(60, 60, 60, 200);
        ImU32 bodyColor = IM_COL32(40, 40, 40, 200);
        bool collapsed = false;
        
        // Resize functionality
        bool resizing = false;
        ImVec2 minSize = ImVec2(120, 80);
        ImVec2 maxSize = ImVec2(400, 300);
        
        // Component-specific data
        std::shared_ptr<Component> componentData;
        
        // Node-specific data for logic/math nodes
        std::unordered_map<std::string, float> floatParams;
        std::unordered_map<std::string, int> intParams;
        std::unordered_map<std::string, bool> boolParams;
        std::unordered_map<std::string, std::string> stringParams;
        
        // Execution state for logic nodes
        bool executed = false;
        float executionTime = 0.0f;
        
        // Callback for when component data changes
        std::function<void(Node*)> onComponentDataChanged;
        
        Node(int nodeId, NodeType nodeType, const std::string& nodeName, ImVec2 pos);
        void draw(ImVec2 displayPos = ImVec2(0, 0), float zoom = 1.0f);
        
        // Helper methods for node setup
        void setupBasicPins(NodeType nodeType);
        void setupNodeAppearance(NodeType nodeType);
        void createComponentData(NodeType nodeType);
        void drawHeader(ImDrawList* drawList, ImVec2 nodePos, ImVec2 nodeSize, float zoom);
        void drawBody(ImDrawList* drawList, ImVec2 nodePos, ImVec2 nodeSize, float zoom);
        void drawSpriteNodeContent(ImVec2 nodePos, ImVec2 nodeSize);
        void drawRotationNodeContent(ImVec2 nodePos, ImVec2 nodeSize);
        void drawScaleNodeContent(ImVec2 nodePos, ImVec2 nodeSize);
        void drawTransformNodeContent(ImVec2 nodePos, ImVec2 nodeSize);
        void drawParticleNodeContent(ImVec2 nodePos, ImVec2 nodeSize);
        void drawKeyPressNodeContent(ImVec2 nodePos, ImVec2 nodeSize);
        void drawMathNodeContent(ImVec2 nodePos, ImVec2 nodeSize);
        void drawLogicNodeContent(ImVec2 nodePos, ImVec2 nodeSize);
        void drawConstantNodeContent(ImVec2 nodePos, ImVec2 nodeSize);
        void drawEventNodeContent(ImVec2 nodePos, ImVec2 nodeSize);
        void drawEntityNodeContent(ImVec2 nodePos, ImVec2 nodeSize);
        void drawTextureSelectionPopup();
        void drawPins(ImDrawList* drawList, float zoom = 1.0f);
        bool isInside(ImVec2 point) const;
        bool isOnResizeHandle(ImVec2 point) const;
        bool isOnHeader(ImVec2 point) const;
        Pin* getPinById(int pinId);
        
        // Execution methods for logic nodes
        void execute();
        bool canExecute() const;
        void reset();
        
        // Parameter accessors
        void setFloatParam(const std::string& name, float value);
        void setIntParam(const std::string& name, int value);
        void setBoolParam(const std::string& name, bool value);
        void setStringParam(const std::string& name, const std::string& value);
        
        float getFloatParam(const std::string& name, float defaultValue = 0.0f) const;
        int getIntParam(const std::string& name, int defaultValue = 0) const;
        bool getBoolParam(const std::string& name, bool defaultValue = false) const;
        std::string getStringParam(const std::string& name, const std::string& defaultValue = "") const;
          // Helper methods
        ImVec2 getNodeSize() const;
        void updatePinPositions(ImVec2 nodePos, float zoom = 1.0f);
        void setSize(ImVec2 newSize);
        
        // Image file utilities
        static bool isImageFile(const std::string& extension);
        static void scanDirectoryForImages(const std::string& directory, std::vector<std::string>& imageFiles);
        
        // Key name helper
        const char* getKeyName(int keyCode) const;
    };

    struct NodeCategory {
        std::string name;
        std::vector<NodeType> nodeTypes;
        bool expanded = true;
    };
    
    struct UndoRedoAction {
        enum Type {
            CreateNode,
            DeleteNode,
            MoveNode,
            CreateConnection,
            DeleteConnection,
            ModifyNode
        };
        
        Type type;
        std::vector<int> nodeIds;
        std::vector<ImVec2> positions;
        std::vector<int> connectionIds;
        std::string data;
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
        
        // Callback for when code files are generated
        using CodeRefreshCallback = std::function<void()>;
        void setCodeRefreshCallback(CodeRefreshCallback callback);
        
        void show(bool* open, SceneWindow* activeScene);
        void update();
        
        // Node management
        int createNode(NodeType type, ImVec2 position);
        void deleteNode(int nodeId);
        void selectNode(int nodeId);
        void clearSelection();
        void selectMultiple(const std::vector<int>& nodeIds);
        void deleteSelectedNodes();
        
        // Connection management
        int createConnection(int outputPinId, int inputPinId);
        void deleteConnection(int connectionId);
        
        // Copy/Paste
        void copySelectedNodes();
        void pasteNodes();
        bool canPaste() const;
        
        // Undo/Redo
        void undo();
        void redo();
        bool canUndo() const;
        bool canRedo() const;
        void pushUndoAction(const UndoRedoAction& action);
        
        // Templates and presets
        void saveAsTemplate(const std::string& name);
        void loadTemplate(const std::string& name);
        std::vector<std::string> getAvailableTemplates() const;
        
        // Search and navigation
        void showSearchDialog();
        void focusOnNode(int nodeId);
        void frameAllNodes();
        void frameSelectedNodes();
        
        // Entity operations
        void applyNodesToEntity(EntityID entity, Scene* scene);
        void loadEntityAsNodes(EntityID entity, Scene* scene);
        
        // Helper methods
        std::string getNodeTypeName(NodeType type);
        
        // Code generation
        void generateCodeFromNodes();
        void exportNodeGraphAsCode();
        std::string generateEntitySpawnerCode();
        std::string generateComponentSystemCode();
        std::string generateEventSystemCode();
        std::string generateGameLogicCode();
        std::string generateCompleteSceneCode();
        void saveGeneratedCodeToFiles();
        
        // Helper methods for code generation
        void generateKeyPressActions(std::stringstream& code, EntityID entityId, Node* entityNode);
        int getKeyCodeFromNode(Node* entityNode);
        
        // Entity dropdown helpers
        std::vector<std::pair<EntityID, std::string>> getSceneEntities() const;
        void openEntitySelectionPopup(int nodeId);
        
        // Key selection helpers
        void openKeySelectionPopup(int nodeId);
        const char* getKeyName(int keyCode) const;
        
        // Node layout persistence
        void saveNodeLayout(EntityID entity);
        void loadNodeLayout(EntityID entity);
        std::string getLayoutFilePath(EntityID entity) const;
        bool isECSComponentNode(NodeType type);
        
        // Node execution (for logic nodes)
        void executeLogicGraph();
        void resetExecution();
        
    private:
        std::vector<std::unique_ptr<Node>> m_nodes;
        std::vector<Connection> m_connections;
        std::unordered_map<int, Node*> m_nodeMap;
        
        // ID generators
        int m_nextNodeId = 1;
        int m_nextPinId = 1;
        int m_nextConnectionId = 1;
        
        // View state
        ImVec2 m_canvasPos;
        ImVec2 m_canvasSize;
        ImVec2 m_scrolling{0.0f, 0.0f};
        float m_zoom = 1.0f;
        float m_minZoom = 0.2f;
        float m_maxZoom = 3.0f;
        
        // Selection
        std::vector<int> m_selectedNodes;
        bool m_multiSelect = false;
        ImVec2 m_selectionBoxStart;
        ImVec2 m_selectionBoxEnd;
        bool m_drawingSelectionBox = false;
        
        // Interaction state
        bool m_dragging = false;
        int m_draggedNodeId = -1;
        ImVec2 m_dragOffset;
        std::vector<ImVec2> m_dragOffsets; // For multi-selection dragging
        
        // Node resizing
        bool m_resizing = false;
        int m_resizingNodeId = -1;
        ImVec2 m_resizeStartPos;
        ImVec2 m_resizeStartSize;
        
        // Connection creation
        bool m_creatingConnection = false;
        int m_connectionStartPinId = -1;
        ImVec2 m_connectionEndPos;
        
        // Copy/Paste
        std::string m_clipboardData;
        
        // Undo/Redo
        std::vector<UndoRedoAction> m_undoStack;
        std::vector<UndoRedoAction> m_redoStack;
        static const size_t MAX_UNDO_STACK_SIZE = 100;
        
        // Search
        bool m_showSearchDialog = false;
        char m_searchBuffer[256] = "";
        std::vector<int> m_searchResults;
        int m_currentSearchResult = -1;
        
        // Categories
        std::vector<NodeCategory> m_nodeCategories;
        
        // Templates
        std::unordered_map<std::string, std::string> m_nodeTemplates;
        
        // Context menu
        bool m_showContextMenu = false;
        ImVec2 m_contextMenuPos;
        
        // Code refresh callback
        CodeRefreshCallback m_codeRefreshCallback;
        
        // Entity selection for ParticleEffect nodes
        int m_entitySelectionNodeId = -1;
        bool m_showEntitySelectionPopup = false;
        
        // Key selection for OnKeyPress nodes
        int m_keySelectionNodeId = -1;
        bool m_showKeySelectionPopup = false;
        
        SceneWindow* m_activeScene = nullptr;
        
        // Helper methods
        void drawGrid();
        void drawNodes();
        void drawConnections();
        void drawSelectionBox();
        void handleInput();
        void handleKeyboardShortcuts();
        void drawNodeCreationMenu();
        void drawContextMenu();
        void drawSearchDialog();
        void drawConnectionInProgress();
        void drawTransformEditPopup(Node* node);
        void drawNodeTooltip(Node* node);
        
        // Node creation helpers
        void initializeNodeCategories();
        void setupNodePins(Node* node);
        void setupNodeVisuals(Node* node);
        
        // Utility methods
        std::string getNodeDescription(NodeType type);
        ImU32 getNodeColor(NodeType type);
        ImU32 getPinColor(PinDataType dataType);
        std::string getCategoryName(NodeType type);
        
        // Pin position calculation
        ImVec2 calculatePinPosition(const Node* node, const Pin* pin);
        void updatePinPositions();
        
        // Selection helpers
        bool isNodeSelected(int nodeId) const;
        void addToSelection(int nodeId);
        void removeFromSelection(int nodeId);
        Node* getNodeAt(ImVec2 position);
        std::vector<int> getNodesInRect(ImVec2 min, ImVec2 max);
        
        // Clipboard helpers
        std::string serializeNodes(const std::vector<int>& nodeIds);
        void deserializeNodes(const std::string& data, ImVec2 offset);
        
        // Template helpers
        void saveTemplatesToFile();
        void loadTemplatesFromFile();
        
        // Search helpers
        void performSearch(const std::string& query);
        void highlightSearchResults();
        
        // Validation
        bool canConnect(int outputPinId, int inputPinId);
        bool wouldCreateCycle(int outputPinId, int inputPinId);
        bool isDataTypeCompatible(PinDataType outputType, PinDataType inputType);
        
        // Component management helpers
        void applyComponentToEntity(EntityID entity, Scene* scene, Node* componentNode);
        void removeComponentFromEntity(EntityID entity, Scene* scene, NodeType componentType);
        void applyTransformChangesIfConnected(Node* transformNode);
        void onNodeComponentDataChanged(Node* node);
        bool isECSComponentNode(NodeType type) const;
        
        // Execution helpers (for logic nodes)
        void executeNode(Node* node);
        void propagateExecution(Node* node);
        std::vector<Node*> getExecutionOrder();
        bool hasExecutionPath(Node* from, Node* to);
    };

} // namespace NodeEditor
