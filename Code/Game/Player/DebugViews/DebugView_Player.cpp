#include "DebugView_Player.h"
#include "Game/Player/Systems/EntitySystem_PlayerController.h"
#include "Game/Player/Physics/PlayerPhysicsController.h"
#include "Engine/Entity/EntityWorld.h"
#include "Engine/Entity/EntitySystem.h"
#include "Engine/Entity/EntityWorldUpdateContext.h"
#include "Engine/Player/Systems/WorldSystem_PlayerManager.h"
#include "Engine/UpdateContext.h"
#include "System/Imgui/ImguiX.h"

// HACK
#include "Game/Player/StateMachine/Actions/PlayerAction_Jump.h"
#include "Game/Player/StateMachine/Actions/PlayerAction_Interact.h"
#include "Game/Player/Components/Component_MainPlayer.h"
#include "Engine/Camera/Systems/WorldSystem_CameraManager.h"

//-------------------------------------------------------------------------

#if EE_DEVELOPMENT_TOOLS
namespace EE::Player
{
    PlayerDebugView::PlayerDebugView()
    {
        m_menus.emplace_back( DebugMenu( "Game/Player", [this] ( EntityWorldUpdateContext const& context ) { DrawMenu( context ); } ) );
    }

    void PlayerDebugView::Initialize( SystemRegistry const& systemRegistry, EntityWorld const* pWorld )
    {
        m_pWorld = pWorld;
        m_pPlayerManager = pWorld->GetWorldSystem<PlayerManager>();
    }

    void PlayerDebugView::Shutdown()
    {
        m_pPlayerManager = nullptr;
        m_pWorld = nullptr;
    }

    void PlayerDebugView::DrawWindows( EntityWorldUpdateContext const& context, ImGuiWindowClass* pWindowClass )
    {
        EE_ASSERT( m_pWorld != nullptr );

        // HACK
        m_isActionDebugWindowOpen = true;
        m_isCharacterControllerDebugWindowOpen = true;

        PlayerController* pPlayerController = nullptr;
        if ( m_isActionDebugWindowOpen || m_isCharacterControllerDebugWindowOpen )
        {
            if ( m_pPlayerManager->HasPlayer() )
            {
                auto pPlayerEntity = m_pWorld->GetPersistentMap()->FindEntity( m_pPlayerManager->GetPlayerEntityID() );
                for ( auto pSystem : pPlayerEntity->GetSystems() )
                {
                    pPlayerController = TryCast<PlayerController>( pSystem );
                    if ( pPlayerController != nullptr )
                    {
                        break;
                    }
                }
            }
        }

        //-------------------------------------------------------------------------

        if ( pPlayerController != nullptr )
        {
            if ( m_isActionDebugWindowOpen )
            {
                if ( pWindowClass != nullptr ) ImGui::SetNextWindowClass( pWindowClass );
                
                ImGui::SetNextWindowBgAlpha( 0.9f );
                if ( ImGui::Begin( "Player Actions", &m_isActionDebugWindowOpen ) )
                {
                    if ( pPlayerController == nullptr )
                    {
                        ImGui::Text( "No Player Controller Found" );
                    }
                    else
                    {
                        ActionStateMachine const* pStateMachine = &pPlayerController->m_actionStateMachine;
                        pStateMachine->DrawDebugUI();
                    }
                }
                ImGui::End();
            }

            if ( m_isCharacterControllerDebugWindowOpen )
            {
                if ( pWindowClass != nullptr ) ImGui::SetNextWindowClass( pWindowClass );

                ImGui::SetNextWindowBgAlpha( 0.9f );
                if ( ImGui::Begin( "Player Character Controller", &m_isCharacterControllerDebugWindowOpen ) )
                {
                    auto pCharacterController = pPlayerController->m_actionContext.m_pCharacterController;
                    if ( pCharacterController != nullptr )
                    {
                        pCharacterController->DrawDebugUI();
                    }
                    else
                    {
                        ImGui::Text( "No valid physics controller on player!" );
                    }
                }
                ImGui::End();
            }
        }
    }

    void PlayerDebugView::DrawMenu( EntityWorldUpdateContext const& context )
    {
        PlayerController* pPlayerController = nullptr;
        if ( m_pPlayerManager->HasPlayer() )
        {
            auto pPlayerEntity = m_pWorld->GetPersistentMap()->FindEntity( m_pPlayerManager->GetPlayerEntityID() );
            for ( auto pSystem : pPlayerEntity->GetSystems() )
            {
                pPlayerController = TryCast<PlayerController>( pSystem );
                if ( pPlayerController != nullptr )
                {
                    break;
                }
            }
        }

        if ( pPlayerController == nullptr )
        {
            return;
        }

        //-------------------------------------------------------------------------

        if ( ImGui::MenuItem( "Action Debugger" ) )
        {
            m_isActionDebugWindowOpen = true;
        }

        if ( ImGui::MenuItem( "Physics State Debugger" ) )
        {
            m_isCharacterControllerDebugWindowOpen = true;
        }
    }

    // Called within the context of a large overlay window allowing you to draw helpers and widgets over a viewport
    void PlayerDebugView::DrawOverlayElements( EntityWorldUpdateContext const& context )
    {
        EE_ASSERT( m_pWorld != nullptr );

        PlayerController* pPlayerController = nullptr;

        if ( m_pPlayerManager->HasPlayer() )
        {
            auto pPlayerEntity = m_pWorld->GetPersistentMap()->FindEntity( m_pPlayerManager->GetPlayerEntityID() );
            for ( auto pSystem : pPlayerEntity->GetSystems() )
            {
                pPlayerController = TryCast<PlayerController>( pSystem );
                if ( pPlayerController != nullptr )
                {
                    break;
                }
            }
        }

        if ( pPlayerController == nullptr )
        {
            return;
        }

        //-------------------------------------------------------------------------

        // Draw fake hud when using the player camera
        auto pCameraManager = m_pWorld->GetWorldSystem<CameraManager>();
        if ( m_pPlayerManager->GetPlayerCamera() == pCameraManager->GetActiveCamera() )
        {
            HACK_DrawPlayerHUD( context, pPlayerController );
        }
    }

    void PlayerDebugView::HACK_DrawPlayerHUD( EntityWorldUpdateContext const& context, PlayerController* pController )
    {
        InlineString statusString;

        bool const isChargedJumpReady = static_cast<JumpAction const*>( pController->m_actionStateMachine.m_baseActions[ActionStateMachine::Jump] )->IsChargedJumpReady();
        if ( isChargedJumpReady )
        {
            statusString += "Charged Jump Ready";
        }

        bool const canInteract = static_cast<InteractAction const*>( pController->m_actionStateMachine.m_baseActions[ActionStateMachine::Interact] )->CanInteract();
        if ( canInteract )
        {
            statusString += "Interact";
        }

        //-------------------------------------------------------------------------

        auto window = ImGui::GetCurrentWindow();
        auto drawList = window->DrawList;

        ImVec2 const windowPos = ImGui::GetWindowPos();
        ImVec2 const windowDimensions = ImGui::GetWindowSize();
        ImVec2 const windowCenter = windowDimensions / 2;

        {
            ImGuiX::ScopedFont sf( ImGuiX::Font::Medium );

            ImVec2 const crosshairSize = ImGui::CalcTextSize( EE_ICON_CROSSHAIRS );
            ImVec2 const crosshairCursorPos( windowCenter - ( crosshairSize / 2 ) );
            ImGui::SetCursorPos( crosshairCursorPos );
            ImGui::Text( EE_ICON_CROSSHAIRS );
        }

        if ( !statusString.empty() )
        {
            ImGuiX::ScopedFont sf( ImGuiX::Font::LargeBold );

            ImVec2 const statusTextSize = ImGui::CalcTextSize( statusString.c_str() );
            ImVec2 const statusTextCursorPos( windowCenter.x - ( statusTextSize.x / 2 ), windowCenter.y + 8 );
            ImGui::SetCursorPos( statusTextCursorPos );
            ImGui::TextColored( ImGuiX::ImColors::Red, statusString.c_str() );
        }

        {
            float const startX = windowDimensions.x - 600;
            float const startY = windowDimensions.y - 40;
            float const EndX = windowDimensions.x - 10;
            float const EndY = windowDimensions.y - 10;
            float const rectWidth = EndX - startX;

            ImVec2 const rectangleStart = ImVec2( startX, startY ) + windowPos;
            ImVec2 const rectangleEnd = ImVec2( EndX, EndY ) + windowPos;
            ImVec2 const rectangleSize = rectangleEnd - rectangleStart;

            auto const color = ImGui::GetColorU32( ImVec4( 1.0f, 1.0f, 1.0f, 1.0f ) );
            auto const colorLevel = ImGui::GetColorU32( ImVec4( 0.5f, 1.0f, 0.5f, 1.0f ) );

            auto GetStartPosForLevel = [rectangleStart, rectWidth] ( int32_t level )
            {
                float paddingX = level == 1 ? 5.f : 2.5f;
                return rectangleStart + ( ImVec2( float( level - 1 ), float( level - 1 ) ) * ImVec2( rectWidth / 3.f, 0.f ) ) + ImVec2( paddingX, 5.f );
            };
            auto GetEndPosForLevel = [rectangleEnd, rectWidth] ( int32_t level )
            {
                float paddingX = level == 3 ? 5.f : 2.5f;
                return rectangleEnd - ( ImVec2( float( 3 - level ), float( 3 - level ) ) * ImVec2( rectWidth / 3.f, 0.f ) ) - ImVec2( paddingX, 5.f );
            };

            drawList->AddRect( rectangleStart, rectangleEnd, color );
            drawList->AddRect( GetStartPosForLevel( 1 ), GetEndPosForLevel( 1 ), colorLevel );
            drawList->AddRect( GetStartPosForLevel( 2 ), GetEndPosForLevel( 2 ), colorLevel );
            drawList->AddRect( GetStartPosForLevel( 3 ), GetEndPosForLevel( 3 ), colorLevel );

            float const energyLevel = pController->m_actionContext.m_pPlayerComponent->GetEnergyLevel();
            if( energyLevel < 1.0 )
            {
                ImVec2 const start = GetStartPosForLevel( 1 );
                ImVec2 const end = GetEndPosForLevel( 1 );
                float const width = end.x - start.x;

                drawList->AddRectFilled( start, ImVec2( start.x + energyLevel * width, end.y ), colorLevel );
            }
            else if( energyLevel < 2.0 )
            {
                ImVec2 const start = GetStartPosForLevel( 2 );
                ImVec2 const end = GetEndPosForLevel( 2 );
                float const width = end.x - start.x;

                drawList->AddRectFilled( GetStartPosForLevel( 1 ), GetEndPosForLevel( 1 ), colorLevel );
                drawList->AddRectFilled( start, ImVec2( start.x + ( energyLevel - 1.f ) * width, end.y ), colorLevel );
            }
            else if( energyLevel < 3.0 )
            {
                ImVec2 const start = GetStartPosForLevel( 3 );
                ImVec2 const end = GetEndPosForLevel( 3 );
                float const width = end.x - start.x;

                drawList->AddRectFilled( GetStartPosForLevel( 1 ), GetEndPosForLevel( 1 ), colorLevel );
                drawList->AddRectFilled( GetStartPosForLevel( 2 ), GetEndPosForLevel( 2 ), colorLevel );
                drawList->AddRectFilled( start, ImVec2( start.x + ( energyLevel - 2.f ) * width, end.y ), colorLevel );
            }
            else
            {
                drawList->AddRectFilled( GetStartPosForLevel( 1 ), GetEndPosForLevel( 1 ), colorLevel );
                drawList->AddRectFilled( GetStartPosForLevel( 2 ), GetEndPosForLevel( 2 ), colorLevel );
                drawList->AddRectFilled( GetStartPosForLevel( 3 ), GetEndPosForLevel( 3 ), colorLevel );
            }
        }
    }
}
#endif