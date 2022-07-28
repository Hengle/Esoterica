#include "Animation_EditorGraphNode_Warping.h"
#include "Animation_EditorGraphNode_AnimationClip.h"
#include "EngineTools/Animation/GraphEditor/EditorGraph/Animation_EditorGraph_Compilation.h"
#include "Engine/Animation/Graph/Nodes/Animation_RuntimeGraphNode_Warping.h"

//-------------------------------------------------------------------------

namespace EE::Animation::GraphNodes
{
    void OrientationWarpEditorNode::Initialize( VisualGraph::BaseGraph* pParent )
    {
        EditorGraphNode::Initialize( pParent );
        CreateOutputPin( "Result", GraphValueType::Pose, false );
        CreateInputPin( "Input", GraphValueType::Pose );
        CreateInputPin( "Angle Offset", GraphValueType::Float );
    }

    int16_t OrientationWarpEditorNode::Compile( GraphCompilationContext& context ) const
    {
        OrientationWarpNode::Settings* pSettings = nullptr;
        NodeCompilationState const state = context.GetSettings<OrientationWarpNode>( this, pSettings );
        if ( state == NodeCompilationState::NeedCompilation )
        {
            auto pInputNode = GetConnectedInputNode<EditorGraphNode>( 0 );
            if ( pInputNode != nullptr )
            {
                int16_t const compiledNodeIdx = pInputNode->Compile( context );
                if ( compiledNodeIdx != InvalidIndex )
                {
                    pSettings->m_clipReferenceNodeIdx = compiledNodeIdx;
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

            pInputNode = GetConnectedInputNode<EditorGraphNode>( 1 );
            if ( pInputNode != nullptr )
            {
                int16_t const compiledNodeIdx = pInputNode->Compile( context );
                if ( compiledNodeIdx != InvalidIndex )
                {
                    pSettings->m_angleOffsetValueNodeIdx = compiledNodeIdx;
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
        }
        return pSettings->m_nodeIdx;
    }

    bool OrientationWarpEditorNode::IsValidConnection( UUID const& inputPinID, Node const* pOutputPinNode, UUID const& outputPinID ) const
    {
        int32_t const pinIdx = GetInputPinIndex( inputPinID );
        if ( pinIdx == 0 )
        {
            return IsOfType<AnimationClipEditorNode>( pOutputPinNode ) || IsOfType<AnimationClipReferenceEditorNode>( pOutputPinNode );
        }

        return EditorGraphNode::IsValidConnection( inputPinID, pOutputPinNode, outputPinID );
    }

    //-------------------------------------------------------------------------

    void TargetWarpEditorNode::Initialize( VisualGraph::BaseGraph* pParent )
    {
        EditorGraphNode::Initialize( pParent );
        CreateOutputPin( "Result", GraphValueType::Pose, false );
        CreateInputPin( "Input", GraphValueType::Pose );
        CreateInputPin( "World Target", GraphValueType::Target );
    }

    int16_t TargetWarpEditorNode::Compile( GraphCompilationContext& context ) const
    {
        TargetWarpNode::Settings* pSettings = nullptr;
        NodeCompilationState const state = context.GetSettings<TargetWarpNode>( this, pSettings );
        if ( state == NodeCompilationState::NeedCompilation )
        {
            auto pInputNode = GetConnectedInputNode<EditorGraphNode>( 0 );
            if ( pInputNode != nullptr )
            {
                int16_t const compiledNodeIdx = pInputNode->Compile( context );
                if ( compiledNodeIdx != InvalidIndex )
                {
                    pSettings->m_clipReferenceNodeIdx = compiledNodeIdx;
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

            pInputNode = GetConnectedInputNode<EditorGraphNode>( 1 );
            if ( pInputNode != nullptr )
            {
                int16_t const compiledNodeIdx = pInputNode->Compile( context );
                if ( compiledNodeIdx != InvalidIndex )
                {
                    pSettings->m_targetValueNodeIdx = compiledNodeIdx;
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
        }

        //-------------------------------------------------------------------------

        pSettings->m_allowTargetUpdate = m_allowTargetUpdate;
        pSettings->m_samplingMode = m_samplingMode;
        pSettings->m_samplingPositionErrorThresholdSq = Math::Sqr( m_samplingPositionErrorThreshold );

        return pSettings->m_nodeIdx;
    }

    bool TargetWarpEditorNode::IsValidConnection( UUID const& inputPinID, Node const* pOutputPinNode, UUID const& outputPinID ) const
    {
        int32_t const pinIdx = GetInputPinIndex( inputPinID );
        if ( pinIdx == 0 )
        {
            return IsOfType<AnimationClipEditorNode>( pOutputPinNode ) || IsOfType<AnimationClipReferenceEditorNode>( pOutputPinNode );
        }

        return EditorGraphNode::IsValidConnection( inputPinID, pOutputPinNode, outputPinID );
    }
}