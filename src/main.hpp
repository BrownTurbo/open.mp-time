#pragma once
#ifndef MAINFLE_H
#define MAINFLE_H

#include "sdk.hpp"
#include <Server/Components/Pawn/Impl/pawn_natives.hpp>
#include <Server/Components/Pawn/pawn.hpp>
#include <Server/Components/Pawn/Impl/pawn_impl.hpp>

#include "../lib/ntp/NTPClient.h"
#include "../lib/ntp/UDP/UDP.h"

#include "natives.hpp"

#include <iostream>
#include <sstream>
#include <streambuf>
#include <string>
#include <mutex>

// Core Logging
class CoreLogStreambuf : public std::streambuf
{
public:
    CoreLogStreambuf(ICore *core, LogLevel level)
        : core_(core), level_(level) {}

protected:
    int overflow(int ch) override
    {
        if (ch == traits_type::eof())
            return !traits_type::eof();
        buffer_ += static_cast<char>(ch);
        if (ch == '\n')
            sync();
        return ch;
    }

    int sync() override
    {
        if (!buffer_.empty() && core_)
        {
            // remove trailing newline
            if (buffer_.back() == '\n')
                buffer_.pop_back();
            core_->logLn(level_, "%.*s", static_cast<int>(buffer_.size()), buffer_.data());
            buffer_.clear();
        }
        return 0;
    }

private:
    ICore *core_;
    LogLevel level_;
    std::string buffer_;
};

// The main component class
class OMPTime final : public IComponent, public PawnEventHandler, public CoreEventHandler
{
public:
    PROVIDE_UID(0x55347878BAA3C18A);

    StringView componentName() const override;

    SemanticVersion componentVersion() const override;

    void onLoad(ICore *c) override;

    void onInit(IComponentList *components) override;

    void onAmxLoad(IPawnScript &script) override;

    void onAmxUnload(IPawnScript &script) override;

    void onTick(Microseconds elapsed, TimePoint now) override;

    void onFree(IComponent *component) override;

    void reset() override;

    void free() override;

    static ICore *&getCore();

    static OMPTime *&get();

    private:
        std::streambuf * oldCout = nullptr;
        std::streambuf * oldCerr = nullptr;
        std::streambuf * oldClog = nullptr;

    ~OMPTime()
    {
        if (pawnComponent)
        {
            pawnComponent->getEventDispatcher().removeEventHandler(this);
        }

        // Restore streams
        if (oldCerr) std::cerr.rdbuf(oldCerr);
        if (oldClog) std::clog.rdbuf(oldClog);
        if (oldCout) std::cout.rdbuf(oldCout);
    }

private:
    ICore *core{};
    IPawnComponent *pawnComponent{};
};
#endif
