#pragma once

#include "EngineTools/Animation/ToolsGraph/Animation_ToolsGraph_Definition.h"
#include "EngineTools/Animation/ToolsGraph/Animation_ToolsGraph_Compilation.h"
#include "EngineTools/Core/Workspace.h"
#include "EngineTools/Core/VisualGraph/VisualGraph_View.h"
#include "EngineTools/Core/Helpers/CategoryTree.h"
#include "Engine/Animation/Graph/Animation_RuntimeGraph_Definition.h"
#include "Engine/Animation/TaskSystem/Animation_TaskSystem.h"
#include "System/Time/Timers.h"

//-------------------------------------------------------------------------

namespace EE::Render { class SkeletalMeshComponent; }
namespace EE::Physics { class PhysicsSystem; }

//-------------------------------------------------------------------------

namespace EE::Animation
{
    class AnimationGraphComponent;
    class ControlParameterPreviewState;
    class VariationHierarchy;
    class GraphRecorder;

    namespace GraphNodes { class VirtualParameterToolsNode; class ControlParameterToolsNode; }

    //-------------------------------------------------------------------------

    class AnimationGraphWorkspace final : public TWorkspace<GraphDefinition>
    {
        friend class GraphUndoableAction;

    public:

        struct GraphData
        {
            ToolsGraphDefinition                    m_graphDefinition;
            StringID                                m_selectedVariationID = Variation::s_defaultVariationID;
            VisualGraph::BaseNode*                  m_pParentNode = nullptr;

            //-------------------------------------------------------------------------

            GraphInstance*                          m_pGraphInstance = nullptr;
            THashMap<UUID, int16_t>                 m_nodeIDtoIndexMap;
            THashMap<int16_t, UUID>                 m_nodeIndexToIDMap;
        };

    private:

        enum class DebugMode
        {
            None,
            Preview,
            ReviewRecording,
            LiveDebug,
        };

        enum class DebugTargetType
        {
            None,
            MainGraph,
            ChildGraph,
            ExternalGraph,
            Recording
        };

        enum class ParameterType
        {
            Default,
            Virtual
        };

        struct DebugTarget
        {
            bool IsValid() const;

            DebugTargetType                     m_type = DebugTargetType::None;
            AnimationGraphComponent*            m_pComponentToDebug = nullptr;
            PointerID                           m_childGraphID;
            StringID                            m_externalSlotID;
        };

        struct NavigationTarget
        {
            NavigationTarget( GraphNodes::FlowToolsNode const* pNode, String&& path )
                : m_pNode( pNode )
                , m_path( eastl::move( path ) )
            {}

            GraphNodes::FlowToolsNode const*    m_pNode;
            String                              m_path;
        };

        enum class GraphOperationType
        {
            None,
            Navigate,
            CreateParameter,
            RenameParameter,
            DeleteParameter,
            CreateVariation,
            RenameVariation,
            DeleteVariation
        };

    public:

        AnimationGraphWorkspace( ToolsContext const* pToolsContext, EntityWorld* pWorld, ResourceID const& resourceID );
        ~AnimationGraphWorkspace();

        virtual bool IsWorkingOnResource( ResourceID const& resourceID ) const override;

    private:

        virtual void Initialize( UpdateContext const& context ) override;
        virtual void Shutdown( UpdateContext const& context ) override;
        virtual void InitializeDockingLayout( ImGuiID dockspaceID ) const override;
        virtual void PreUpdateWorld( EntityWorldUpdateContext const& updateContext ) override;

        virtual bool HasTitlebarIcon() const override { return true; }
        virtual char const* GetTitlebarIcon() const override { EE_ASSERT( HasTitlebarIcon() ); return EE_ICON_STATE_MACHINE; }
        virtual bool HasViewportToolbarTimeControls() const override { return true; }
        virtual void DrawViewportOverlayElements( UpdateContext const& context, Render::Viewport const* pViewport ) override;
        virtual void DrawWorkspaceToolbarItems( UpdateContext const& context ) override;
        virtual void Update( UpdateContext const& context, ImGuiWindowClass* pWindowClass, bool isFocused ) override;
        virtual void PreUndoRedo( UndoStack::Operation operation ) override;
        virtual void PostUndoRedo( UndoStack::Operation operation, IUndoableAction const* pAction ) override;
        virtual bool IsDirty() const override;
        virtual bool AlwaysAllowSaving() const override { return true; }
        virtual bool Save() override;

        virtual void DrawDialogs( UpdateContext const& context );

        // Graph Operations
        //-------------------------------------------------------------------------

        inline GraphData* GetMainGraphData() { return m_graphStack[0]; }
        inline GraphData const* GetMainGraphData() const { return m_graphStack[0]; }

        void OnBeginGraphModification( VisualGraph::BaseGraph* pRootGraph );
        void OnEndGraphModification( VisualGraph::BaseGraph* pRootGraph );

        void GraphDoubleClicked( VisualGraph::BaseGraph* pGraph );
        void PostPasteNodes( TInlineVector<VisualGraph::BaseNode*, 20> const& pastedNodes );

        // Variations
        //-------------------------------------------------------------------------

        inline bool IsDefaultVariationSelected() const { return GetMainGraphData()->m_selectedVariationID == Variation::s_defaultVariationID; }

        inline StringID GetSelectedVariationID() const { return GetMainGraphData()->m_selectedVariationID; }

        // Sets the current selected variation. Assumes a valid variation ID!
        void SetSelectedVariation( StringID variationID );

        // Tries to case-insensitively match a supplied variation name to the various variations we have
        void TrySetSelectedVariation( String const& variationName );

        // Selection
        //-------------------------------------------------------------------------

        void SetSelectedNodes( TVector<VisualGraph::SelectedNode> const& selectedNodes ) { m_selectedNodes = selectedNodes; }
        void ClearSelection() { m_selectedNodes.clear(); }

        // User Context
        //-------------------------------------------------------------------------

        void InitializeUserContext();
        void UpdateUserContext();
        void ShutdownUserContext();


        // Graph View
        //-------------------------------------------------------------------------

        void DrawGraphView( UpdateContext const& context, ImGuiWindowClass* pWindowClass );
        void DrawGraphViewNavigationBar();
        void UpdateSecondaryViewState();

        inline bool IsInReadOnlyState() const;

        inline bool IsViewingMainGraph() const { return m_graphStack.empty(); }
        
        inline ToolsGraphDefinition* GetCurrentlyViewedGraphDefinition() { return m_graphStack.empty() ? &GetMainGraphData()->m_graphDefinition : &m_graphStack.back()->m_graphDefinition; }
        inline ToolsGraphDefinition const* GetCurrentlyViewedGraphDefinition() const { return m_graphStack.empty() ? &GetMainGraphData()->m_graphDefinition : &m_graphStack.back()->m_graphDefinition; }

        // Get the stack index for the specified node!
        int32_t GetStackIndexForNode( VisualGraph::BaseNode* pNode ) const;

        // Get the stack index for the specified primary graph!
        int32_t GetStackIndexForGraph( VisualGraph::BaseGraph* pGraph ) const;

        // Create a new view stack based on a provided child-graph
        void PushOnGraphStack( VisualGraph::BaseNode* pSourceNode, ResourceID const& graphID );

        // Pop child graph view from stack
        void PopGraphStack();

        // Clear the entire view stack - except for the edited graph!
        void ClearGraphStack();

        // Generate the necessary debug data for the graph stack
        bool GenerateGraphStackDebugData();

        // Clear all graph stack debug data
        void ClearGraphStackDebugData();

        // Navigation
        //-------------------------------------------------------------------------

        void NavigateTo( VisualGraph::BaseNode* pNode, bool focusViewOnNode = true );
        void NavigateTo( VisualGraph::BaseGraph* pGraph );
        void OpenChildGraph( VisualGraph::BaseNode* pNode, ResourceID const& graphID, bool openInNewWorkspace );
        void StartNavigationOperation();
        void DrawNavigationDialogWindow( UpdateContext const& context );
        void GenerateNavigationTargetList();
        void GenerateActiveTargetList();

        // Property Grid
        //-------------------------------------------------------------------------

        void DrawPropertyGrid( UpdateContext const& context, ImGuiWindowClass* pWindowClass );

        void InitializePropertyGrid();
        void ShutdownPropertyGrid();

        // Compilation Log
        //-------------------------------------------------------------------------

        void DrawGraphLog( UpdateContext const& context, ImGuiWindowClass* pWindowClass );

        // Control Parameter Editor
        //-------------------------------------------------------------------------

        void DrawControlParameterEditor( UpdateContext const& context, ImGuiWindowClass* pWindowClass );

        void InitializeControlParameterEditor();
        void ShutdownControlParameterEditor();

        void CreateControlParameterPreviewStates();
        void DestroyControlParameterPreviewStates();

        void RefreshControlParameterCache();
        void RefreshParameterCategoryTree();
        void DrawParameterList();
        void DrawPreviewParameterList( UpdateContext const& context );

        void DrawCreateOrRenameParameterDialogWindow();
        void DrawDeleteParameterDialogWindow();

        void ControlParameterCategoryDragAndDropHandler( Category<GraphNodes::FlowToolsNode*>& category );

        void StartParameterCreate( GraphValueType type, ParameterType parameterType );
        void StartParameterRename( UUID const& parameterID );
        void StartParameterDelete( UUID const& parameterID );

        GraphNodes::ControlParameterToolsNode* FindControlParameter( UUID parameterID ) const;
        GraphNodes::VirtualParameterToolsNode* FindVirtualParameter( UUID parameterID ) const;

        void CreateParameter( ParameterType parameterType, GraphValueType valueType, String const& desiredParameterName, String const& desiredCategoryName );
        void RenameParameter( UUID parameterID, String const& desiredParameterName, String const& desiredCategoryName );
        void DestroyParameter( UUID parameterID );

        void EnsureUniqueParameterName( String& desiredParameterName ) const;
        void ResolveCategoryName( String& desiredCategoryName ) const;

        // Variation Editor
        //-------------------------------------------------------------------------

        void DrawVariationEditor( UpdateContext const& context, ImGuiWindowClass* pWindowClass );

        void StartCreate( StringID variationID );
        void StartRename( StringID variationID );
        void StartDelete( StringID variationID );

        void CreateVariation( StringID newVariationID, StringID parentVariationID );
        void RenameVariation( StringID oldVariationID, StringID newVariationID );
        void DeleteVariation( StringID variationID );

        void DrawVariationSelector( float width = -1 );
        void DrawVariationTreeNode( VariationHierarchy const& variationHierarchy, StringID variationID );
        void DrawOverridesTable();

        bool DrawVariationNameEditor();
        void DrawCreateVariationDialogWindow();
        void DrawRenameVariationDialogWindow();
        void DrawDeleteVariationDialogWindow();

        // Debugging
        //-------------------------------------------------------------------------

        void DrawDebuggerWindow( UpdateContext const& context, ImGuiWindowClass* pWindowClass );

        inline bool IsDebugging() const { return m_debugMode != DebugMode::None; }
        inline bool IsPreviewing() const { return m_debugMode == DebugMode::Preview; }
        inline bool IsLiveDebugging() const { return m_debugMode == DebugMode::LiveDebug; }
        inline bool IsReviewingRecording() const { return m_debugMode == DebugMode::ReviewRecording; }
        inline bool IsPreviewOrReview() const { return m_debugMode == DebugMode::Preview || m_debugMode == DebugMode::ReviewRecording; }

        // Hot Reload
        virtual void OnHotReloadStarted( bool descriptorNeedsReload, TInlineVector<Resource::ResourcePtr*, 10> const& resourcesToBeReloaded ) override;

        // Starts a debugging session. If a target component is provided we assume we are attaching to a live game 
        void StartDebugging( UpdateContext const& context, DebugTarget target );

        // Ends the current debug session
        void StopDebugging();

        // Set's the preview graph parameters to their default preview values
        void ReflectInitialPreviewParameterValues( UpdateContext const& context );

        // Draw all the debug options for this graph
        void DrawLiveDebugTargetsMenu( UpdateContext const& context );

        // Calculate the offset at which to place the camera when tracking
        void CalculateCameraOffset();

        // Recording
        //-------------------------------------------------------------------------

        // Draw all the recorder controls
        void DrawRecorderUI( UpdateContext const& context );

        // Clear recorded data
        void ClearRecordedData();

        // Start recorder the live preview state
        void StartRecording();

        // Stop recording the live preview state
        void StopRecording();

        // Set the currently reviewed frame, this is set on all clients in the review
        void SetFrameToReview( int32_t newFrameIdx );

        // Step the review forwards one frame
        void StepReviewForward();

        // Step the review backwards one frame
        void StepReviewBackward();

    private:

        String                                                          m_controlParametersWindowName;
        String                                                          m_graphViewWindowName;
        String                                                          m_propertyGridWindowName;
        String                                                          m_variationEditorWindowName;
        String                                                          m_graphLogWindowName;
        String                                                          m_debuggerWindowName;
        PropertyGrid                                                    m_propertyGrid;
        GraphOperationType                                              m_activeOperation = GraphOperationType::None;

        EventBindingID                                                  m_requestOpenChildGraphBindingID;
        EventBindingID                                                  m_rootGraphBeginModificationBindingID;
        EventBindingID                                                  m_rootGraphEndModificationBindingID;
        EventBindingID                                                  m_preEditEventBindingID;
        EventBindingID                                                  m_postEditEventBindingID;
        EventBindingID                                                  m_postPasteNodesEventBindingID;
        EventBindingID                                                  m_graphDoubleClickedEventBindingID;

        // Graph Type Data
        TVector<TypeSystem::TypeInfo const*>                            m_registeredNodeTypes;
        CategoryTree<TypeSystem::TypeInfo const*>                       m_categorizedNodeTypes;

        // Graph Data
        FileSystem::Path                                                m_graphFilePath;
        TVector<GraphData*>                                             m_graphStack;
        TVector<VisualGraph::SelectedNode>                              m_selectedNodes;
        TVector<VisualGraph::SelectedNode>                              m_selectedNodesPreUndoRedo;

        // User Context
        ToolsGraphUserContext                                           m_userContext;
        EventBindingID                                                  m_navigateToNodeEventBindingID;
        EventBindingID                                                  m_navigateToGraphEventBindingID;
        EventBindingID                                                  m_resourceOpenRequestEventBindingID;

        // Graph view
        float                                                           m_primaryGraphViewHeight = 300;
        VisualGraph::GraphView                                          m_primaryGraphView;
        VisualGraph::GraphView                                          m_secondaryGraphView;
        VisualGraph::GraphView*                                         m_pFocusedGraphView = nullptr;
        UUID                                                            m_primaryViewGraphID;
        VisualGraph::BaseNode*                                          m_pBreadcrumbPopupContext = nullptr;

        // Navigation
        TVector<NavigationTarget>                                       m_navigationTargetNodes;
        TVector<NavigationTarget>                                       m_navigationActiveTargetNodes;
        ImGuiX::FilterWidget                                            m_navigationFilter;

        // Compilation Log
        TVector<NodeCompilationLogEntry>                                m_visualLog;

        // Control Parameter Editor
        TInlineVector<GraphNodes::ControlParameterToolsNode*, 20>       m_controlParameters;
        TInlineVector<GraphNodes::VirtualParameterToolsNode*, 20>       m_virtualParameters;
        UUID                                                            m_currentOperationParameterID;
        ParameterType                                                   m_currentOperationParameterType;
        GraphValueType                                                  m_currentOperationParameterValueType = GraphValueType::Unknown;
        char                                                            m_parameterNameBuffer[255];
        char                                                            m_parameterCategoryBuffer[255];
        THashMap<UUID, int32_t>                                         m_cachedNumUses;
        CategoryTree<GraphNodes::FlowToolsNode*>                        m_parameterCategoryTree;
        TVector<ControlParameterPreviewState*>                          m_previewParameterStates;
        CategoryTree<ControlParameterPreviewState*>                     m_previewParameterCategoryTree;

        // Variation Editor
        StringID                                                        m_activeOperationVariationID;
        char                                                            m_nameBuffer[255] = { 0 };
        ImGuiX::FilterWidget                                            m_variationFilter;
        Resource::ResourcePicker                                        m_resourcePicker;

        // Preview/Debug
        DebugMode                                                       m_debugMode = DebugMode::None;
        EntityID                                                        m_debuggedEntityID; // This is needed to ensure that we dont try to debug a destroyed entity
        ComponentID                                                     m_debuggedComponentID;
        AnimationGraphComponent*                                        m_pDebugGraphComponent = nullptr;
        Render::SkeletalMeshComponent*                                  m_pDebugMeshComponent = nullptr;
        GraphInstance*                                                  m_pDebugGraphInstance = nullptr;
        StringID                                                        m_debugExternalGraphSlotID = StringID();
        GraphDebugMode                                                  m_graphDebugMode = GraphDebugMode::On;
        RootMotionDebugMode                                             m_rootMotionDebugMode = RootMotionDebugMode::Off;
        TaskSystemDebugMode                                             m_taskSystemDebugMode = TaskSystemDebugMode::Off;
        bool                                                            m_showPreviewCapsule = false;
        float                                                           m_previewCapsuleHalfHeight = 0.65f;
        float                                                           m_previewCapsuleRadius = 0.35f;
        TResourcePtr<GraphVariation>                                    m_previewGraphVariationPtr;
        Physics::PhysicsSystem*                                         m_pPhysicsSystem = nullptr;
        Entity*                                                         m_pPreviewEntity = nullptr;
        Transform                                                       m_previewStartTransform = Transform::Identity;
        Transform                                                       m_characterTransform = Transform::Identity;
        Transform                                                       m_cameraOffsetTransform = Transform::Identity;
        Transform                                                       m_previousCameraTransform = Transform::Identity;
        bool                                                            m_startPaused = false;
        bool                                                            m_isFirstPreviewFrame = false;
        bool                                                            m_isCameraTrackingEnabled = false;

        // Recording
        RecordedGraphState                                              m_recordedGraphState;
        GraphUpdateRecorder                                             m_updateRecorder;
        int32_t                                                         m_currentReviewFrameIdx = InvalidIndex;
        bool                                                            m_isRecording = false;
        bool                                                            m_reviewStarted = false;
        bool                                                            m_drawExtraRecordingInfo = false;
    };

    //-------------------------------------------------------------------------

    class GraphUndoableAction final : public IUndoableAction
    {
        EE_REGISTER_TYPE( IUndoableAction );

    public:

        GraphUndoableAction() = default;
        GraphUndoableAction( AnimationGraphWorkspace* pWorkspace );

        virtual void Undo() override;
        virtual void Redo() override;
        void SerializeBeforeState();
        void SerializeAfterState();

    private:

        AnimationGraphWorkspace*                                        m_pWorkspace = nullptr;
        String                                                          m_valueBefore;
        String                                                          m_valueAfter;
    };
}