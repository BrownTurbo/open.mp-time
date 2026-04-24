#pragma once
#include <Server/Components/Pawn/pawn.hpp>
#include <Server/Components/Pawn/Impl/pawn_natives.hpp>
#include <Server/Components/Pawn/Impl/pawn_impl.hpp>

#include "../lib/ntp/NTPClient.h"
#include "../lib/ntp/UDP/UDP.h"

#include "natives.hpp"

class OMPTime final : public IComponent, public PawnEventHandler, public PlayerConnectEventHandler
{
public:
    PROVIDE_UID(0x55347878BAA3C18A);

    StringView componentName() const override;

	SemanticVersion componentVersion() const override;

	void onLoad(ICore *c) override;

	void onInit(IComponentList *components) override;

	void onAmxLoad(IPawnScript &script) override;

	void onAmxUnload(IPawnScript &script) override;

	void onTick(Microseconds elapsed, TimePoint now);

	void onFree(IComponent *component) override;

	void reset() override;

	void free() override;

	static ICore *&getCore();

	static OMPTime *&get();

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

private:
	ICore *core{};
	IPawnComponent *pawnComponent{};
};
