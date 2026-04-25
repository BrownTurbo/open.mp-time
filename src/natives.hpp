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

namespace chrono = std::chrono;

extern bool ClientInitialised;
extern std::unique_ptr<UDPSocket> udpSocket;
extern std::unique_ptr<NTPClient> ntpClient;

#endif
