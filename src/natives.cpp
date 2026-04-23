#include "natives.hpp"

std::map<std::string, chrono::milliseconds> unitMap = {
    { "ms", chrono::milliseconds(1) },
    { "s", chrono::seconds(1) },
    { "m", chrono::minutes(1) },
    { "h", chrono::hours(1) },
    { "d", chrono::hours(24) }
};

SCRIPT_API(Now, int())
{
    return static_cast<int>(std::chrono::seconds(std::time(NULL)).count());
}

SCRIPT_API(TimeFormat, ())
{
    int unix_timestamp = static_cast<int>(params[1]);
    std::string fmt = amx_GetCppString(amx, params[2]);

    date::sys_seconds since_epoch(chrono::seconds{ unix_timestamp });

    std::ostringstream os;
    date::to_stream(os, fmt.c_str(), since_epoch);
    std::string output(os.str());

    return amx_SetCppString(amx, params[3], output, params[4]);
}

SCRIPT_API(TimeParse, ())
{
    std::string string = amx_GetCppString(amx, params[1]);
    std::string fmt = amx_GetCppString(amx, params[2]);
    cell* output;
    amx_GetAddr(amx, params[3], &output);

    std::istringstream is(string);
    date::sys_seconds d;

	try {
		if (date::from_stream(is, fmt.c_str(), d).fail()) {
			return 1;
		}
	} catch (std::exception& e) {
		core->logLn(LogLevel::Error, "ERROR: date::from_stream failed: %s", e.what());
		return 1;
	}

    *output = static_cast<cell>(d.time_since_epoch().count());

    return 0;
}

SCRIPT_API(DurationParse, ()) {
    std::string input = amx_GetCppString(amx, params[1]);
    cell* output;
    amx_GetAddr(amx, params[2], &output);

    size_t idx = 0,
           length = input.length();

    bool negative = false;
    if (input[idx] == '-') {
        negative = true;
        idx++;
    } else if (input[idx] == '+') {
        negative = false;
        idx++;
    }

    // The next character must be [0-9.]
    if (!(input[idx] == '.' || '0' <= input[idx] && input[idx] <= '9')) {
        return 1;
    }

    int numberBegin = -1,
        value;

    bool gotValue = false,
         gotUnit = false;

    std::string unit;
    int resultDuration = 0;

    while (idx <= length) {
        if (!gotValue) {
            if (numberBegin == -1) {
                numberBegin = idx;

                idx++;
                continue;
            } else {
                if (!('0' <= input[idx] && input[idx] <= '9')) {
                    gotValue = true;
                    value = std::stoi(input.substr(numberBegin, idx - numberBegin));
                    numberBegin = -1;

                    continue;
                }

                idx++;
                continue;
            }
        }
        if (!gotUnit) {
            if (input[idx] == 0 || !('a' <= input[idx] && input[idx] <= 'z')) {
                gotUnit = true;
                continue;
            }

            unit.push_back(input[idx]);

            idx++;
            continue;
        }

        if (unitMap.find(unit) == unitMap.end()) {
            return 2;
        }
        resultDuration += static_cast<int>(value * unitMap.at(unit).count());

        if (input[idx] == 0) {
            break;
        }

        gotValue = false;
        gotUnit = false;
        unit = std::string();
    }

    *output = resultDuration;

    return 0;
}

// native NTP_Init(const server[] = "pool.ntp.org", port = 123);
SCRIPT_API(NTP_Init, ()) {
    if (ClientInitialised) {
        core->logLn(LogLevel::Debug, "NTP plugin: already initialised.");
        return 1;
    }

    // Get parameters: server name, port
    char serverName[256];
    amx_GetString(serverName, params[1], 0, sizeof(serverName));
    uint16_t port = static_cast<uint16_t>(params[2]);

    // Create UDP socket and NTP client
    InitSockets();
    UDPSocket = std::make_unique<UDPSocket>();
    if (!UDPSocket->begin(0)) {   // bind to any free port
        core->logLn(LogLevel::Debug, "NTP plugin: UDP socket creation failed.");
        return 0;
    }
    UDPSocket->setTimeout(100);   // 100 ms recv timeout

    NTPClient = std::make_unique<NTPClient>(*UDPSocket, serverName);
    NTPClient->begin();

    core->logLn(LogLevel::Debug, "NTP plugin: initialised with server %s:%d", serverName, port);
    ClientInitialised = true;
    return 1;
}

// native NTP_Update();  // Called from ProcessTick, but also exposed to scripts
SCRIPT_API(NTP_Update, ()) {
    if (!ClientInitialised || !NTPClient) return 0;
    return NTPClient->update() ? 1 : 0;
}

// native NTP_ForceUpdate();
SCRIPT_API(NTP_ForceUpdate, ()) {
    if (!ClientInitialised || !NTPClient) return 0;
    return NTPClient->forceUpdate() ? 1 : 0;
}

// native NTP_IsSynced();
SCRIPT_API(NTP_IsSynced, ()) {
    if (!ClientInitialised || !NTPClient) return 0;
    return NTPClient->isTimeSet() ? 1 : 0;
}

// native NTP_GetTime(&hour, &minute, &second);
SCRIPT_API(NTP_GetTime, ()) {
    if (!ClientInitialised || !NTPClient || !NTPClient->isTimeSet()) {
        return 0;
    }

    cell *hour   = nullptr;
    cell *minute = nullptr;
    cell *second = nullptr;
    amx_GetAddr(amx, params[1], &hour);
    amx_GetAddr(amx, params[2], &minute);
    amx_GetAddr(amx, params[3], &second);

    *hour   = NTPClient->getHours();
    *minute = NTPClient->getMinutes();
    *second = NTPClient->getSeconds();

    return 1;
}

// native NTP_SetOffset(offset);
SCRIPT_API(NTP_SetOffset, ())
{
    if (!ClientInitialised || !NTPClient) return 0;
    NTPClient->setTimeOffset(params[1]);
    return 1;
}
