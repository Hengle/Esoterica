#pragma once

#include "ResourceNetworkMessages.h"
#include "System/Resource/ResourceProvider.h"
#include "System/Network/IPC/IPCMessageClient.h"
#include "System/Time/Timers.h"
#include "System/Threading/Threading.h"

//-------------------------------------------------------------------------

#if EE_DEVELOPMENT_TOOLS
namespace EE::Resource
{
    class ResourceSettings;

    //-------------------------------------------------------------------------

    class EE_SYSTEM_API NetworkResourceProvider final : public ResourceProvider
    {

    public:

        NetworkResourceProvider( ResourceSettings const& settings ) : ResourceProvider( settings ) {}
        virtual bool IsReady() const override final;

    private:

        virtual bool Initialize() override final;
        virtual void Shutdown() override final;
        virtual void Update() override final;

        virtual void RequestRawResource( ResourceRequest* pRequest ) override;
        virtual void CancelRequest( ResourceRequest* pRequest ) override;

        virtual TVector<ResourceID> const& GetExternallyUpdatedResources() const override { return m_externallyUpdatedResources; }

    private:

        Network::IPC::Client                                m_networkClient;
        String                                              m_address;
        TVector<NetworkResourceResponse>                    m_serverReponses;
        Threading::LockFreeQueue<Network::IPC::Message>     m_messagesToSend;
        bool                                                m_networkFailureDetected = false;

        TVector<ResourceRequest*>                           m_requests;

        #if EE_DEVELOPMENT_TOOLS
        TVector<ResourceID>                                 m_externallyUpdatedResources;
        #endif
    };
}
#endif