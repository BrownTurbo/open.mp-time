#include "natives.hpp"
#include "main.hpp"

bool ClientInitialised = false;
std::unique_ptr<UDPSocket> udpSocket;
std::unique_ptr<NTPClient> ntpClient;

std::map<std::string, chrono::milliseconds> unitMap = {
    {"ms", chrono::milliseconds(1)},
    {"s", chrono::seconds(1)},
    {"m", chrono::minutes(1)},
    {"h", chrono::hours(1)},
    {"d", chrono::hours(24)}};

SCRIPT_API(Now, int())
{
    return static_cast<int>(std::chrono::seconds(std::time(NULL)).count());
}

SCRIPT_API(TimeFormat, int(int unix_timestamp, const std::string &fmt, cell &output, int outputSize))
{
    if (outputSize <= 0)
    {
        return 1;
    }

    try
    {
        date::sys_seconds since_epoch(std::chrono::seconds{unix_timestamp});

        std::ostringstream os;
        date::to_stream(os, fmt.c_str(), since_epoch);

        std::string result = os.str();

        return amx_SetString(&output, result.c_str(), 0, 0, outputSize);
    }
    catch (const std::exception &e)
    {
        ICore *core = OMPTime::getCore();
        if (core)
        {
            core->logLn(LogLevel::Error, "ERROR: date::to_stream failed: %s", e.what());
        }
        return 1;
    }
    catch (...)
    {
        return 1;
    }
}

SCRIPT_API(TimeParse, int(const std::string &string, const std::string &fmt, cell &output))

{
    std::istringstream is(string);
    date::sys_seconds d;

    try
    {
        if (date::from_stream(is, fmt.c_str(), d).fail())
        {
            return 1;
        }
    }
    catch (std::exception &e)
    {
        ICore *core = OMPTime::getCore();
        if (core)
        {
            core->logLn(LogLevel::Error, "ERROR: date::from_stream failed: %s", e.what());
        }
        return 1;
    }

    output = static_cast<cell>(d.time_since_epoch().count());

    return 0;
}

SCRIPT_API(DurationParse, int(const std::string &input, cell &output))
{
    size_t idx = 0,
           length = input.length();

    if (length == 0)
    {
        return 1;
    }

    bool negative = false;
    if (input[idx] == '-')
    {
        negative = true;
        idx++;
    }
    else if (input[idx] == '+')
    {
        negative = false;
        idx++;
    }

    if (idx >= length)
    {
        return 1;
    }

    // The next character must be [0-9.]
    if (!(input[idx] == '.' || ('0' <= input[idx] && input[idx] <= '9')))
    {
        return 1;
    }

    int numberBegin = -1,
        value = 0;

    bool gotValue = false,
         gotUnit = false;

    std::string unit;
    int resultDuration = 0;

    while (idx <= length)
    {
        if (!gotValue)
        {
            if (numberBegin == -1)
            {
                numberBegin = static_cast<int>(idx);
                idx++;
                continue;
            }
            else
            {
                if (idx < length && ('0' <= input[idx] && input[idx] <= '9'))
                {
                    idx++;
                    continue;
                }

                gotValue = true;
                try
                {
                    value = std::stoi(input.substr(numberBegin, idx - numberBegin));
                }
                catch (...)
                {
                    return 1;
                }
                numberBegin = -1;
                continue;
            }
        }
        if (!gotUnit)
        {
            if (idx == length || input[idx] == 0 || !('a' <= input[idx] && input[idx] <= 'z'))
            {
                gotUnit = true;
                continue;
            }

            unit.push_back(input[idx]);
            idx++;
            continue;
        }

        if (unitMap.find(unit) == unitMap.end())
        {
            return 2;
        }
        resultDuration += static_cast<int>(value * unitMap.at(unit).count());

        if (idx >= length || input[idx] == 0)
        {
            break;
        }

        gotValue = false;
        gotUnit = false;
        unit.clear();
    }

    output = negative ? -resultDuration : resultDuration;

    return 0;
}

// native NTP_Init(const server[] = "pool.ntp.org", port = 123);
SCRIPT_API(NTP_Init, int(const std::string &NTPserver, int port))
{
    ICore *core = OMPTime::getCore();
    if (!core)
    {
        return 0;
    }

    std::string serverName = NTPserver.empty() ? "pool.ntp.org" : NTPserver;

    if (ClientInitialised && ntpClient)
    {
        ntpClient->setPoolServerName(serverName);
        ntpClient->setPoolServerPort(static_cast<unsigned int>(port));
        ntpClient->requestUpdate();
        core->logLn(LogLevel::Debug, "NTP plugin: re-initialised with server %s:%d", serverName.c_str(), port);
        return 1;
    }

    ntpClient.reset();
    udpSocket.reset();

    // Create UDP socket and NTP client
    udpSocket = std::make_unique<UDPSocket>();
    if (!udpSocket->begin(0))
    {
        core->logLn(LogLevel::Error, "NTP plugin: failed to bind UDP socket");
        return 0;
    }
    udpSocket->setNonBlocking(true);

    ntpClient = std::make_unique<NTPClient>(*udpSocket, serverName.c_str(), static_cast<unsigned int>(port));
    ntpClient->begin();

    core->logLn(LogLevel::Debug, "NTP plugin: initialised with server %s:%d", serverName.c_str(), port);
    ClientInitialised = true;
    return 1;
}

// native NTP_Update();  // Called from ProcessTick, but also exposed to scripts
SCRIPT_API(NTP_Update, bool())
{
    if (!ClientInitialised || !ntpClient)
        return 0;
    return ntpClient->update() ? 1 : 0;
}

// native NTP_ForceUpdate();
SCRIPT_API(NTP_ForceUpdate, bool())
{
    if (!ClientInitialised || !ntpClient)
        return 0;
    return ntpClient->forceUpdate() ? 1 : 0;
}

// native NTP_IsSynced();
SCRIPT_API(NTP_IsSynced, bool())
{
    if (!ClientInitialised || !ntpClient)
        return 0;
    return ntpClient->isTimeSet() ? 1 : 0;
}

// native NTP_GetTime(&hour, &minute, &second);
SCRIPT_API(NTP_GetTime, bool(int &hour, int &minute, int &second))
{
    if (!ClientInitialised || !ntpClient || !ntpClient->isTimeSet())
    {
        return 0;
    }

    hour = ntpClient->getHours();
    minute = ntpClient->getMinutes();
    second = ntpClient->getSeconds();

    return 1;
}

// native Timestamp:NTP_GetTimestamp();
SCRIPT_API(NTP_GetTimestamp, int())
{
    if (!ClientInitialised || !ntpClient || !ntpClient->isTimeSet())
    {
        return 0;
    }

    return static_cast<int>(ntpClient->getEpochTime());
}

// native NTP_SetServer(const server[], port = 123);
SCRIPT_API(NTP_SetServer, bool(const std::string &server, int port))
{
    if (!ClientInitialised || !ntpClient)
    {
        return 0;
    }

    std::string serverName = server.empty() ? "pool.ntp.org" : server;
    ntpClient->setPoolServerName(serverName);
    ntpClient->setPoolServerPort(static_cast<unsigned int>(port));
    ntpClient->requestUpdate();

    ICore *core = OMPTime::getCore();
    if (core)
    {
        core->logLn(LogLevel::Debug, "NTP plugin: server changed to %s:%d", serverName.c_str(), port);
    }
    return 1;
}

// native NTP_SetUpdateInterval(interval_ms);
SCRIPT_API(NTP_SetUpdateInterval, bool(int interval_ms))
{
    if (!ClientInitialised || !ntpClient || interval_ms <= 0)
    {
        return 0;
    }

    ntpClient->setUpdateInterval(static_cast<unsigned long>(interval_ms));
    return 1;
}

// native NTP_SetOffset(offset);
SCRIPT_API(NTP_SetOffset, bool(int offset))
{
    if (!ClientInitialised || !ntpClient)
        return 0;
    ntpClient->setTimeOffset(offset);
    return 1;
}

// native GetTimezoneOffset(const timezone[], Timestamp:timestamp = Timestamp:0);
SCRIPT_API(GetTimezoneOffset, int(const std::string &timezone, int timestamp))
{
    return GetTimezoneOffsetSeconds(timezone, static_cast<std::time_t>(timestamp));
}
