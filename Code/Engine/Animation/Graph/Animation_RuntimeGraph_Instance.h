#pragma once
#include "Animation_RuntimeGraph_Definition.h"
#include "Animation_RuntimeGraph_RootMotionDebugger.h"
#include "Animation_RuntimeGraph_Contexts.h"
#include "System/Types/PointerID.h"

//-------------------------------------------------------------------------

namespace EE::Physics
{
    class Scene;
}

//-------------------------------------------------------------------------

namespace EE::Animation
{
    class GraphContext;
    class TaskSystem;
    class GraphNode;
    class PoseNode;
    enum class TaskSystemDebugMode;

    //-------------------------------------------------------------------------

    #if EE_DEVELOPMENT_TOOLS
    enum class GraphDebugMode
    {
        Off,
        On,
    };
    #endif

    //-------------------------------------------------------------------------

    class EE_ENGINE_API GraphInstance
    {
        friend class AnimationDebugView;

    public:

        struct ChildGraph
        {
            int16_t             m_nodeIdx = InvalidIndex;
            GraphInstance*      m_pInstance = nullptr;
        };

        struct ExternalGraph
        {
            StringID            m_slotID;
            int16_t             m_nodeIdx = InvalidIndex;
            GraphInstance*      m_pInstance = nullptr;
        };

        #if EE_DEVELOPMENT_TOOLS
        struct DebuggableChildGraph
        {
            PointerID GetID() const { return PointerID( m_pInstance ); }

            String              m_pathToInstance;
            GraphInstance*      m_pInstance = nullptr;
        };
        #endif

    public:

        // Main instance
        inline GraphInstance( GraphVariation const* pGraphVariation, uint64_t ownerID ) : GraphInstance( pGraphVariation, ownerID, nullptr ) {}
        ~GraphInstance();

        // Info 
        //-------------------------------------------------------------------------

        inline StringID const& GetVariationID() const { return m_pGraphVariation->m_dataSet.m_variationID; }
        inline ResourceID const& GetResourceID() const { return m_pGraphVariation->GetResourceID(); }
        inline ResourceID const& GetDefinitionResourceID() const { return m_pGraphVariation->m_pGraphDefinition->GetResourceID(); }

        // Pose
        //-------------------------------------------------------------------------

        // Get the final pose from the task system
        Pose const* GetPose();

        // Does the task system has unexecuted pose tasks
        bool DoesTaskSystemNeedUpdate() const;

        // Graph State
        //-------------------------------------------------------------------------

        // Is this a valid instance that has been correctly initialized
        bool IsInitialized() const { return m_pRootNode != nullptr && m_pRootNode->IsValid(); }

        // Run the graph logic - returns the root motion delta for the update
        GraphPoseNodeResult EvaluateGraph( Seconds const deltaTime, Transform const& startWorldTransform, Physics::Scene* pPhysicsScene, bool resetGraphState = false );

        // Run the graph logic synchronized (needed for external graph support) - returns the root motion delta for the update
        GraphPoseNodeResult EvaluateGraph( Seconds const deltaTime, Transform const& startWorldTransform, Physics::Scene* pPhysicsScene, SyncTrackTimeRange const& updateRange, bool resetGraphState = false );

        // Execute any pre-physics pose tasks (assumes the character is at its final position for this frame)
        void ExecutePrePhysicsPoseTasks( Transform const& endWorldTransform );

        // Execute any post-physics pose tasks
        void ExecutePostPhysicsPoseTasks();

        // Get the sampled events for the last update
        SampledEventsBuffer const& GetSampledEvents() const { return m_graphContext.m_sampledEventsBuffer; }

        // General Node Info
        //-------------------------------------------------------------------------

        inline PoseNode const* GetRootNode() const { return m_pRootNode; }

        // Was this node active in the last update
        inline bool IsNodeActive( int16_t nodeIdx ) const
        {
            EE_ASSERT( IsValidNodeIndex( nodeIdx ) );
            return m_nodes[nodeIdx]->IsNodeActive( const_cast<GraphContext&>( m_graphContext ) );
        }

        inline bool IsValidNodeIndex( int16_t nodeIdx ) const 
        {
            return nodeIdx < m_pGraphVariation->m_pGraphDefinition->m_nodeSettings.size();
        }

        // Control Parameters
        //-------------------------------------------------------------------------

        inline int32_t GetNumControlParameters() const { return (int32_t) m_pGraphVariation->m_pGraphDefinition->m_controlParameterIDs.size(); }

        inline int16_t GetControlParameterIndex( StringID parameterID ) const
        {
            int32_t const numParams = GetNumControlParameters();
            for ( int16_t i = 0; i < numParams; i++ )
            {
                if ( m_pGraphVariation->m_pGraphDefinition->m_controlParameterIDs[i] == parameterID )
                {
                    return i;
                }
            }

            return InvalidIndex;
        }

        inline StringID GetControlParameterID( int16_t parameterNodeIdx )
        {
            EE_ASSERT( IsControlParameter( parameterNodeIdx ) );
            return m_pGraphVariation->m_pGraphDefinition->m_controlParameterIDs[parameterNodeIdx];
        }

        inline GraphValueType GetControlParameterType( int16_t parameterNodeIdx )
        {
            EE_ASSERT( IsControlParameter( parameterNodeIdx ) );
            return reinterpret_cast<ValueNode*>( m_nodes[parameterNodeIdx] )->GetValueType();
        }

        template<typename T>
        inline void SetControlParameterValue( int16_t parameterNodeIdx, T const& value )
        {
            EE_ASSERT( IsControlParameter( parameterNodeIdx ) );
            reinterpret_cast<ValueNode*>( m_nodes[parameterNodeIdx] )->SetValue<T>( value );
        }

        template<typename T>
        inline T GetControlParameterValue( int16_t parameterNodeIdx ) const
        {
            EE_ASSERT( IsControlParameter( parameterNodeIdx ) );
            return reinterpret_cast<ValueNode*>( m_nodes[parameterNodeIdx] )->GetValue<T>( const_cast<GraphContext&>( m_graphContext ) );
        }

        // External Graphs
        //-------------------------------------------------------------------------

        // Check if a given slot ID is valid
        inline bool IsValidExternalGraphSlotID( StringID slotID ) const { return GetExternalGraphNodeIndex( slotID ) != InvalidIndex; }

        // Is the specified external graph slot node active
        inline bool IsExternalGraphSlotNodeActive( StringID slotID ) const { return IsNodeActive( GetExternalGraphNodeIndex( slotID ) ); }

        // Is the specified external graph slot node filled
        inline bool IsExternalGraphSlotFilled( StringID slotID ) const { return GetConnectedExternalGraphIndex( slotID ) != InvalidIndex; }

        // Connects a supplied external graph to the specified slot. Note, it is the callers responsibility to ensure that the slot ID is valid!
        GraphInstance* ConnectExternalGraph( StringID slotID, GraphVariation const* pGraphVariation );

        // Disconnects an external graph, will destroy the created instance
        void DisconnectExternalGraph( StringID slotID );

        // Debug Information
        //-------------------------------------------------------------------------
        
        #if EE_DEVELOPMENT_TOOLS
        // Get the list of active node indices for this instance
        inline TVector<int16_t> const& GetActiveNodes() const { return m_activeNodes; }

        // Get the graph debug mode
        inline GraphDebugMode GetGraphDebugMode() const { return m_debugMode; }

        // Set the graph debug mode
        inline void SetGraphDebugMode( GraphDebugMode mode ) { m_debugMode = mode; }

        // Get the root motion debug mode
        inline RootMotionDebugMode GetRootMotionDebugMode() const { return m_rootMotionDebugger.GetDebugMode(); }

        // Set the root motion debug mode
        inline void SetRootMotionDebugMode( RootMotionDebugMode mode ) { m_rootMotionDebugger.SetDebugMode( mode ); }

        // Get the root motion debugger for this instance
        inline RootMotionDebugger const* GetRootMotionDebugger() const { return &m_rootMotionDebugger; }

        // Manually end the root motion debugger update (call this if you explicitly skip executing the pose tasks)
        inline void EndRootMotionDebuggerUpdate( Transform const& endWorldTransform ) { m_rootMotionDebugger.EndCharacterUpdate( endWorldTransform ); }

        // Get the task system debug mode
        inline TaskSystemDebugMode GetTaskSystemDebugMode() const;

        // Set the task system debug mode
        inline void SetTaskSystemDebugMode( TaskSystemDebugMode mode );

        // Get the debug world transform that the task system used to execute
        Transform GetTaskSystemDebugWorldTransform();

        // Set the list of the debugs that we wish to explicitly debug. Set an empty list to debug everything!
        inline void SetNodeDebugFilterList( TVector<int16_t> const& filterList ) { m_debugFilterNodes = filterList; }

        // Get the runtime time info for a specified pose node
        inline PoseNodeDebugInfo GetPoseNodeDebugInfo( int16_t nodeIdx ) const
        {
            EE_ASSERT( IsValidNodeIndex( nodeIdx ) );
            EE_ASSERT( m_nodes[nodeIdx]->GetValueType() == GraphValueType::Pose );
            auto pNode = reinterpret_cast<PoseNode const*>( m_nodes[nodeIdx] );
            return pNode->GetDebugInfo();
        }

        // Get the connected external graph instance
        GraphInstance const* GetChildGraphDebugInstance( int16_t nodeIdx ) const;

        // Get the connected external graph instance
        GraphInstance const* GetChildGraphDebugInstance( PointerID childGraphInstanceID ) const;

        // Get all child graphs - this will return all child graph instance (recursively)
        void GetChildGraphsForDebug( TVector<DebuggableChildGraph>& outChildGraphInstances, String const& pathPrefix = String() ) const;

        // Get the connected external graph instance
        GraphInstance const* GetExternalGraphDebugInstance( StringID slotID ) const;

        // Get all connected external graphs
        inline TVector<ExternalGraph> const& GetExternalGraphsForDebug() const { return m_externalGraphs; }

        // Get the value of a specified value node
        template<typename T>
        inline T GetRuntimeNodeDebugValue( int16_t nodeIdx ) const
        {
            EE_ASSERT( IsValidNodeIndex( nodeIdx ) );
            auto pValueNode = reinterpret_cast<ValueNode*>( const_cast<GraphNode*>( m_nodes[nodeIdx] ) );
            return pValueNode->GetValue<T>( const_cast<GraphContext&>( m_graphContext ) );
        }

        // Get the runtime log for this graph instance
        TVector<GraphLogEntry> const& GetLog() const { return m_log; }

        // Draw graph debug visualizations
        void DrawDebug( Drawing::DrawContext& drawContext );
        #endif

        // Recording
        //-------------------------------------------------------------------------

        #if EE_DEVELOPMENT_TOOLS
        inline bool IsRecording() const { return m_pUpdateRecorder != nullptr; }
        void StartRecording( RecordedGraphState& outState, GraphUpdateRecorder* pUpdateRecorder = nullptr );
        void StopRecording();
        void SetToRecordedState( RecordedGraphState const& recordedState );
        void SetRecordedUpdateData( RecordedGraphFrameData const& recordedUpdateData );
        #endif

    private:

        explicit GraphInstance( GraphVariation const* pGraphVariation, uint64_t ownerID, TaskSystem* pTaskSystem );

        EE_FORCE_INLINE bool IsControlParameter( int16_t nodeIdx ) const { return nodeIdx < GetNumControlParameters(); }
        int32_t GetExternalGraphSlotIndex( StringID slotID ) const;
        int16_t GetExternalGraphNodeIndex( StringID slotID ) const;
        int32_t GetConnectedExternalGraphIndex( StringID slotID ) const;

        GraphInstance( GraphInstance const& ) = delete;
        GraphInstance( GraphInstance&& ) = delete;
        GraphInstance& operator=( GraphInstance const& ) = delete;
        GraphInstance& operator=( GraphInstance&& ) = delete;

        // Recording
        //-------------------------------------------------------------------------

        #if EE_DEVELOPMENT_TOOLS
        void RecordGraphUpdateData( Seconds const deltaTime, Transform const& startWorldTransform );
        #endif

    private:

        GraphVariation const* const             m_pGraphVariation = nullptr;
        TVector<GraphNode*>                     m_nodes;
        uint8_t*                                m_pAllocatedInstanceMemory = nullptr;
        PoseNode*                               m_pRootNode = nullptr;
        uint64_t                                m_userID = 0; // An idea identifying the owner of this instance (usually the entity ID)

        TaskSystem*                             m_pTaskSystem = nullptr;
        GraphContext                            m_graphContext;
        TVector<ChildGraph>                     m_childGraphs;
        TVector<ExternalGraph>                  m_externalGraphs;

        #if EE_DEVELOPMENT_TOOLS
        TVector<int16_t>                        m_activeNodes;
        GraphDebugMode                          m_debugMode = GraphDebugMode::Off;
        RootMotionDebugger                      m_rootMotionDebugger; // Allows nodes to record root motion operations
        TVector<int16_t>                        m_debugFilterNodes; // The list of nodes that are allowed to debug draw (if this is empty all nodes will draw)
        TVector<GraphLogEntry>                  m_log;
        GraphUpdateRecorder*                    m_pUpdateRecorder = nullptr;
        #endif
    };
}
