#ifndef CHRONO_NATIVES_H
#define CHRONO_NATIVES_H

#include <chrono>
#include <ctime>
#include <date/date.h>
#include <map>
#include <string>
#include <memory>

#include "./main.hpp"

#include "../lib/ntp/NTPClient.h"
#include "../lib/ntp/UDP/UDP.h"

#include <Server/Components/Pawn/pawn.hpp>
#include <Server/Components/Pawn/Impl/pawn_natives.hpp>

namespace chrono = std::chrono;

bool ClientInitialised = false;
std::unique_ptr<UDPSocket> udpSocket;
std::unique_ptr<NTPClient> ntpClient;

#endif
