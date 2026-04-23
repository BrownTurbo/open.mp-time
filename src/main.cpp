#include <Server/Components/Pawn/pawn.hpp>
#include <Server/Components/Pawn/Impl/pawn_natives.hpp>
#include <Server/Components/Pawn/Impl/pawn_impl.hpp>

#include "../lib/ntp/NTPClient.h"
#include "../lib/ntp/UDP/UDP.h"

#include "natives.hpp"

struct OMPTime;
OMPTime* OMPTime = nullptr;

struct IPawnExtension : IExtension
{
    PROVIDE_EXT_UID(0xB92A8CF204F9CF02);

    virtual void setData(int value) = 0;
};

class PawnExtension final : public IPawnExtension
{
private:
    int data_ = 0;

public:
    void setData(int value) override
    {v
        data_ = value;
    }

    int getData() const
    {
        return data_;
    }

    // Implement the core of the extensions API.
    void freeExtension() override
    {
        delete this;
    }

    void reset() override
    {
        data_ = 0;
    }
};

class PawnTemplate final : public IComponent, public PawnEventHandler, public PlayerConnectEventHandler
{
private:
    ICore* core = nullptr;
    IPawnComponent* pawnComponent;

private:
    PROVIDE_UID(0x55347878BAA3C18A);

    StringView componentName() const override
    {
        return "OMP-Time";
    }

    SemanticVersion componentVersion() const override
    {
        return SemanticVersion(1, 1, 2 0);
    }

    void onLoad(ICore* c) override
    {
        core = c;
    }

    void onInit(IComponentList* components) override
    {
        pawnComponent = components->queryComponent<IPawnComponent>();

        if (pawnComponent == nullptr) {
            StringView name = componentName();
            core->logLn(
                LogLevel::Error,
                "Error loading component %.*s: Pawn component not loaded",
                name.length(),
                name.data());
            return;
        }

        setAmxFunctions(pawnComponent->getAmxFunctions());
        setAmxLookups(components);
        pawnComponent->getEventDispatcher().addEventHandler(this);
    }

    void onReady() override { }

	~OMPTime()
	{
		if (pawnComponent)
		{
			pawnComponent->getEventDispatcher().removeEventHandler(this);
		}
		if (core)
		{
			core->getPlayers().getPlayerConnectDispatcher().removeEventHandler(this);
		}
	}

    void onAmxLoad(IPawnScript& script) override
    {
        pawnComponentnatives::AmxLoad(script.GetAMX());
    }

    void onAmxUnload(IPawnScript& script) override
    {
        if (NTPClient)
        {
            NTPClient->end();
            NTPClient.reset();
        }
        if (UDPSocket)
        {
            UDPSocket->stop();
            UDPSocket.reset();
        }
        CleanupSockets();
        ClientInitialised = false;
    }

    void onFree(IComponent* component) override
    {
		if (component == pawnComponent)
		{
			pawnComponent = nullptr;
			setAmxFunctions();
			setAmxLookups();
		}
    }

    void onTick(Microseconds elapsed, TimePoint now) override
    {
        if (ClientInitialised && NTPClient)
        {
            NTPClient->update();
        }
    }

    void free() override
    {
        if (pawnComponent != nullptr)
            pawnComponent->getEventDispatcher().removeEventHandler(this);

        delete this;
    }

    void reset() override
    {
        // Nothing to reset for now.
    }

    ICore *&getCore()
    {
        static ICore *core{};

        return core;
    }

    OMPTime *&get()
    {
        static OMPTime *component{};

        return component;
    }
};

COMPONENT_ENTRY_POINT()
{
    OMPTime = new OMPTime();
    return OMPTime;
}
