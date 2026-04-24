#include "./main.hpp"

StringView OMPTime::componentName() const
{
    return "OMP-Time";
}

SemanticVersion OMPTime::componentVersion() const
{
    return SemanticVersion(1, 1, 2, 0);
}

void OMPTime::onLoad(ICore* c)
{
    getCore() = core = c;
}

void OMPTime::onInit(IComponentList* components)
{
    get() = this;
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

void OMPTime::onAmxLoad(IPawnScript& script)
{
    pawn_natives::AmxLoad(script.GetAMX());
}

void OMPTime::onAmxUnload(IPawnScript& script)
{
    if (ntpClient)
    {
        ntpClient->end();
    }
    if (udpSocket)
    {
        udpSocket->stop();
    }
    udpSocket->cleanupSockets();
    ClientInitialised = false;
}

void OMPTime::onFree(IComponent* component)
{
    if (component == pawnComponent)
    {
        pawnComponent = nullptr;
        setAmxFunctions();
        setAmxLookups();
    }
}

void OMPTime::onTick(Microseconds elapsed, TimePoint now)
{
    if (ClientInitialised && ntpClient)
    {
        ntpClient->update();
    }
}

void OMPTime::free()
{
    if (pawnComponent != nullptr)
        pawnComponent->getEventDispatcher().removeEventHandler(this);

    delete this;
}

void OMPTime::reset()
{
    // Nothing to reset for now.
}

ICore *&OMPTime::getCore()
{
    static ICore *core{};

    return core;
}

OMPTime *&OMPTime::get()
{
    static OMPTime *component{};

    return component;
}

COMPONENT_ENTRY_POINT()
{
    return new OMPTime();
}
