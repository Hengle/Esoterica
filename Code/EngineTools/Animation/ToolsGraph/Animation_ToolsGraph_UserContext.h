#pragma once

#include "Engine/Animation/Graph/Animation_RuntimeGraph_Instance.h"
#include "EngineTools/Core/VisualGraph/VisualGraph_UserContext.h"
#include "EngineTools/Core/Helpers/CategoryTree.h"
#include "System/Types/HashMap.h"

//-------------------------------------------------------------------------

namespace EE::TypeSystem { class TypeInfo; }

//-------------------------------------------------------------------------

namespace EE::Animation
{
    class VariationHierarchy;

    namespace GraphNodes
    {
        class ControlParameterToolsNode;
        class VirtualParameterToolsNode;
    }

    //-------------------------------------------------------------------------

    struct ToolsGraphUserContext : public VisualGraph::UserContext
    {
        // Node Helpers
        //-------------------------------------------------------------------------

        inline TInlineVector<GraphNodes::ControlParameterToolsNode*, 20> const& GetControlParameters() const { EE_ASSERT( m_pControlParameters != nullptr ); return *m_pControlParameters; }
        inline TInlineVector<GraphNodes::VirtualParameterToolsNode*, 20> const& GetVirtualParameters() const { EE_ASSERT( m_pVirtualParameters != nullptr ); return *m_pVirtualParameters; }
        inline Category<TypeSystem::TypeInfo const*> const& GetCategorizedNodeTypes() const { return *m_pCategorizedNodeTypes; }

        // Navigation
        //-------------------------------------------------------------------------

        void OpenChildGraph( VisualGraph::BaseNode* pSourceNode, ResourceID const& graphID, bool openInNewWorkspace );

        inline TEventHandle<VisualGraph::BaseNode*, ResourceID const&, bool> OnRequestOpenChildGraph() { return m_navigateToChildGraphEvent; }

        // Debug Data
        //-------------------------------------------------------------------------

        inline bool HasDebugData() const
        {
            return m_pGraphInstance != nullptr && m_pGraphInstance->IsInitialized();
        }

        inline int16_t GetRuntimeGraphNodeIndex( UUID const& nodeID ) const
        {
            auto const foundIter = m_nodeIDtoIndexMap.find( nodeID );
            if ( foundIter != m_nodeIDtoIndexMap.end() )
            {
                return foundIter->second;
            }

            return InvalidIndex;
        }

        inline UUID GetGraphNodeUUID( int16_t const& runtimeNodeIdx ) const
        {
            auto const foundIter = m_nodeIndexToIDMap.find( runtimeNodeIdx );
            if ( foundIter != m_nodeIndexToIDMap.end() )
            {
                return foundIter->second;
            }

            return UUID();
        }

        TVector<int16_t> const& GetActiveNodes() const;

        bool IsNodeActive( int16_t nodeIdx ) const;

        PoseNodeDebugInfo GetPoseNodeDebugInfo( int16_t runtimeNodeIdx ) const;

        template<typename T>
        inline T GetRuntimeNodeDebugValue( int16_t runtimeNodeIdx ) const
        {
            return m_pGraphInstance->GetRuntimeNodeDebugValue<T>( runtimeNodeIdx );
        }

    public:

        StringID                                                            m_selectedVariationID;
        VariationHierarchy const*                                           m_pVariationHierarchy = nullptr;
        GraphInstance*                                                      m_pGraphInstance = nullptr;
        THashMap<UUID, int16_t>                                             m_nodeIDtoIndexMap;
        THashMap<int16_t, UUID>                                             m_nodeIndexToIDMap;
        TInlineVector<GraphNodes::ControlParameterToolsNode*, 20> const*    m_pControlParameters = nullptr;
        TInlineVector<GraphNodes::VirtualParameterToolsNode*, 20> const*    m_pVirtualParameters = nullptr;
        Category<TypeSystem::TypeInfo const*> const*                        m_pCategorizedNodeTypes = nullptr;
        TypeSystem::TypeRegistry const*                                     m_pTypeRegistry = nullptr;
        TEvent<VisualGraph::BaseNode*, ResourceID const&, bool>             m_navigateToChildGraphEvent;
    };
}