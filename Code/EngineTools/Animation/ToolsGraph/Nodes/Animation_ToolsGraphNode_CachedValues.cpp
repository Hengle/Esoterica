#include "Animation_ToolsGraphNode_CachedValues.h"
#include "EngineTools/Animation/ToolsGraph/Animation_ToolsGraph_Compilation.h"

//-------------------------------------------------------------------------

namespace EE::Animation::GraphNodes
{
    void CachedBoolToolsNode::Initialize( VisualGraph::BaseGraph* pParent )
    {
        FlowToolsNode::Initialize( pParent );
        CreateOutputPin( "Result", GraphValueType::Bool, true );
        CreateInputPin( "Value", GraphValueType::Bool );
    }

    int16_t CachedBoolToolsNode::Compile( GraphCompilationContext& context ) const
    {
        CachedBoolNode::Settings* pSettings = nullptr;
        NodeCompilationState const state = context.GetSettings<CachedBoolNode>( this, pSettings );
        if ( state == NodeCompilationState::NeedCompilation )
        {
            auto pInputNode = GetConnectedInputNode<FlowToolsNode>( 0 );
            if ( pInputNode != nullptr )
            {
                int16_t const compiledNodeIdx = pInputNode->Compile( context );
                if ( compiledNodeIdx != InvalidIndex )
                {
                    pSettings->m_inputValueNodeIdx = compiledNodeIdx;
                }
                else
                {
                    return InvalidIndex;
                }
            }
            else
            {
                context.LogError( this, "Disconnected input pin!" );
                return InvalidIndex;
            }

            //-------------------------------------------------------------------------

            pSettings->m_mode = m_mode;
        }

        return pSettings->m_nodeIdx;
    }

    void CachedBoolToolsNode::DrawInfoText( VisualGraph::DrawContext const& ctx )
    {
        ImGui::Text( m_mode == CachedValueMode::OnEntry ? "Cache On Entry" : "Cache On Exit" );
    }

    //-------------------------------------------------------------------------

    void CachedIDToolsNode::Initialize( VisualGraph::BaseGraph* pParent )
    {
        FlowToolsNode::Initialize( pParent );
        CreateOutputPin( "Result", GraphValueType::ID, true );
        CreateInputPin( "Value", GraphValueType::ID );
    }

    int16_t CachedIDToolsNode::Compile( GraphCompilationContext& context ) const
    {
        CachedIDNode::Settings* pSettings = nullptr;
        if ( context.GetSettings<CachedIDNode>( this, pSettings ) == NodeCompilationState::NeedCompilation )
        {
            auto pInputNode = GetConnectedInputNode<FlowToolsNode>( 0 );
            if ( pInputNode != nullptr )
            {
                int16_t const compiledNodeIdx = pInputNode->Compile( context );
                if ( compiledNodeIdx != InvalidIndex )
                {
                    pSettings->m_inputValueNodeIdx = compiledNodeIdx;
                }
                else
                {
                    return InvalidIndex;
                }
            }
            else
            {
                context.LogError( this, "Disconnected input pin!" );
                return InvalidIndex;
            }

            //-------------------------------------------------------------------------

            pSettings->m_mode = m_mode;
        }
        return pSettings->m_nodeIdx;
    }

    void CachedIDToolsNode::DrawInfoText( VisualGraph::DrawContext const& ctx )
    {
        ImGui::Text( m_mode == CachedValueMode::OnEntry ? "Cache On Entry" : "Cache On Exit" );
    }

    //-------------------------------------------------------------------------

    void CachedIntToolsNode::Initialize( VisualGraph::BaseGraph* pParent )
    {
        FlowToolsNode::Initialize( pParent );
        CreateOutputPin( "Result", GraphValueType::Int, true );
        CreateInputPin( "Value", GraphValueType::Int );
    }

    int16_t CachedIntToolsNode::Compile( GraphCompilationContext& context ) const
    {
        CachedIntNode::Settings* pSettings = nullptr;
        if ( context.GetSettings<CachedIntNode>( this, pSettings ) == NodeCompilationState::NeedCompilation )
        {
            auto pInputNode = GetConnectedInputNode<FlowToolsNode>( 0 );
            if ( pInputNode != nullptr )
            {
                int16_t const compiledNodeIdx = pInputNode->Compile( context );
                if ( compiledNodeIdx != InvalidIndex )
                {
                    pSettings->m_inputValueNodeIdx = compiledNodeIdx;
                }
                else
                {
                    return InvalidIndex;
                }
            }
            else
            {
                context.LogError( this, "Disconnected input pin!" );
                return InvalidIndex;
            }

            //-------------------------------------------------------------------------

            pSettings->m_mode = m_mode;
        }
        return pSettings->m_nodeIdx;
    }

    void CachedIntToolsNode::DrawInfoText( VisualGraph::DrawContext const& ctx )
    {
        ImGui::Text( m_mode == CachedValueMode::OnEntry ? "Cache On Entry" : "Cache On Exit" );
    }

    //-------------------------------------------------------------------------

    void CachedFloatToolsNode::Initialize( VisualGraph::BaseGraph* pParent )
    {
        FlowToolsNode::Initialize( pParent );
        CreateOutputPin( "Result", GraphValueType::Float, true );
        CreateInputPin( "Value", GraphValueType::Float );
    }

    int16_t CachedFloatToolsNode::Compile( GraphCompilationContext& context ) const
    {
        CachedFloatNode::Settings* pSettings = nullptr;
        if ( context.GetSettings<CachedFloatNode>( this, pSettings ) == NodeCompilationState::NeedCompilation )
        {
            auto pInputNode = GetConnectedInputNode<FlowToolsNode>( 0 );
            if ( pInputNode != nullptr )
            {
                int16_t const compiledNodeIdx = pInputNode->Compile( context );
                if ( compiledNodeIdx != InvalidIndex )
                {
                    pSettings->m_inputValueNodeIdx = compiledNodeIdx;
                }
                else
                {
                    return InvalidIndex;
                }
            }
            else
            {
                context.LogError( this, "Disconnected input pin!" );
                return InvalidIndex;
            }

            //-------------------------------------------------------------------------

            pSettings->m_mode = m_mode;
        }
        return pSettings->m_nodeIdx;
    }

    void CachedFloatToolsNode::DrawInfoText( VisualGraph::DrawContext const& ctx )
    {
        ImGui::Text( m_mode == CachedValueMode::OnEntry ? "Cache On Entry" : "Cache On Exit" );
    }

    //-------------------------------------------------------------------------

    void CachedVectorToolsNode::Initialize( VisualGraph::BaseGraph* pParent )
    {
        FlowToolsNode::Initialize( pParent );
        CreateOutputPin( "Result", GraphValueType::Vector, true );
        CreateInputPin( "Value", GraphValueType::Vector );
    }

    int16_t CachedVectorToolsNode::Compile( GraphCompilationContext& context ) const
    {
        CachedVectorNode::Settings* pSettings = nullptr;
        if ( context.GetSettings<CachedVectorNode>( this, pSettings ) == NodeCompilationState::NeedCompilation )
        {
            auto pInputNode = GetConnectedInputNode<FlowToolsNode>( 0 );
            if ( pInputNode != nullptr )
            {
                int16_t const compiledNodeIdx = pInputNode->Compile( context );
                if ( compiledNodeIdx != InvalidIndex )
                {
                    pSettings->m_inputValueNodeIdx = compiledNodeIdx;
                }
                else
                {
                    return InvalidIndex;
                }
            }
            else
            {
                context.LogError( this, "Disconnected input pin!" );
                return InvalidIndex;
            }

            //-------------------------------------------------------------------------

            pSettings->m_mode = m_mode;
        }
        return pSettings->m_nodeIdx;
    }

    void CachedVectorToolsNode::DrawInfoText( VisualGraph::DrawContext const& ctx )
    {
        ImGui::Text( m_mode == CachedValueMode::OnEntry ? "Cache On Entry" : "Cache On Exit" );
    }

    //-------------------------------------------------------------------------

    void CachedTargetToolsNode::Initialize( VisualGraph::BaseGraph* pParent )
    {
        FlowToolsNode::Initialize( pParent );
        CreateOutputPin( "Result", GraphValueType::Target, true );
        CreateInputPin( "Value", GraphValueType::Target );
    }

    int16_t CachedTargetToolsNode::Compile( GraphCompilationContext& context ) const
    {
        CachedTargetNode::Settings* pSettings = nullptr;
        if ( context.GetSettings<CachedTargetNode>( this, pSettings ) == NodeCompilationState::NeedCompilation )
        {
            auto pInputNode = GetConnectedInputNode<FlowToolsNode>( 0 );
            if ( pInputNode != nullptr )
            {
                int16_t const compiledNodeIdx = pInputNode->Compile( context );
                if ( compiledNodeIdx != InvalidIndex )
                {
                    pSettings->m_inputValueNodeIdx = compiledNodeIdx;
                }
                else
                {
                    return InvalidIndex;
                }
            }
            else
            {
                context.LogError( this, "Disconnected input pin!" );
                return InvalidIndex;
            }

            //-------------------------------------------------------------------------

            pSettings->m_mode = m_mode;
        }
        return pSettings->m_nodeIdx;
    }

    void CachedTargetToolsNode::DrawInfoText( VisualGraph::DrawContext const& ctx )
    {
        ImGui::Text( m_mode == CachedValueMode::OnEntry ? "Cache On Entry" : "Cache On Exit" );
    }
}