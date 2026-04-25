#pragma once
#ifndef NTPCLIENT_H
#define NTPCLIENT_H
#include <cstdint>
#include <cstdio>
#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <chrono>
#include <random>
#include <cstring>
#include <cstdio>

#include "./UDP/UDP.h"

typedef uint32_t IPAddress;
typedef UDPSocket UDP;
#define SEVENZYYEARS 2208988800UL
#define NTP_PACKET_SIZE 48
#define NTP_DEFAULT_LOCAL_PORT 1337

inline uint16_t makeWord(uint8_t h, uint8_t l) {
    return (h << 8) | l;
}

class NTPClient {
  private:
    UDP*          _udp;
    bool          _udpSetup       = false;

    const char*   _poolServerName = "pool.ntp.org"; // Default time server
    IPAddress     _poolServerIP;
    unsigned int  _port           = NTP_DEFAULT_LOCAL_PORT;
    long          _timeOffset     = 0;

    unsigned long _updateInterval = 60000;  // In ms

    unsigned long _currentEpoc    = 0;
    std::chrono::steady_clock::time_point _lastUpdate;
    bool          _isSynced       = false;

    enum class NTPState { IDLE, WAITING_FOR_RESPONSE };
    NTPState _state = NTPState::IDLE;
    std::chrono::steady_clock::time_point _requestTime;

    uint8_t       _packetBuffer[NTP_PACKET_SIZE];

    void          sendNTPPacket();

    unsigned int _NTPport = 123;

  public:
    explicit NTPClient(UDP &udp);
    NTPClient(UDP &udp, long timeOffset);
    NTPClient(UDP &udp, const char *poolServerName);
    NTPClient(UDP &udp, const char *poolServerName, long timeOffset);
    NTPClient(UDP &udp, const char *poolServerName, long timeOffset, unsigned long updateInterval);
    NTPClient(UDP &udp, const char *poolServerName, unsigned int port);
    NTPClient(UDP &udp, const char *poolServerName, unsigned int port, long timeOffset);
    NTPClient(UDP &udp, const char *poolServerName, unsigned int port, long timeOffset, unsigned long updateInterval);
    NTPClient(UDP &udp, IPAddress poolServerIP);
    NTPClient(UDP &udp, IPAddress poolServerIP, long timeOffset);
    NTPClient(UDP &udp, IPAddress poolServerIP, long timeOffset, unsigned long updateInterval);
    NTPClient(UDP &udp, IPAddress poolServerIP, unsigned int port);
    NTPClient(UDP &udp, IPAddress poolServerIP, unsigned int port, long timeOffset);
    NTPClient(UDP &udp, IPAddress poolServerIP, unsigned int port, long timeOffset, unsigned long updateInterval);

    /**
     * Starts the underlying UDP client on the default local port.
     * Will not re-bind if the socket was already set up externally.
     */
    void begin();

    /**
     * Starts the underlying UDP client on the given local port.
     * Will not re-bind if the socket was already set up externally.
     */
    void begin(unsigned int port);

    /**
     * Stops the underlying UDP client.
     */
    void end();

    /**
     * Non-blocking update — call from the main server tick.
     * Returns true only when a fresh timestamp has just been received.
     */
    bool update();

    /**
     * Triggers an immediate NTP request (non-blocking; response arrives later).
     */
    void requestUpdate();

    /**
     * Blocking update — waits up to 1 s for a response.
     * Avoid in the server tick; use update() instead.
     */
    bool forceUpdate();

    /**
     * Returns true once at least one successful sync has occurred.
     */
    bool isTimeSet() const;

    // ...
    int getDay() const;
    int getHours() const;
    int getMinutes() const;
    int getSeconds() const;

    /** @return time in seconds since Jan. 1, 1970 */
    unsigned long getEpochTime() const;

    /** @return time formatted as "hh:mm:ss" */
    std::string getFormattedTime() const;

    // ...
    void setPoolServerName(const char *poolServerName);
    void setPoolServerIP(IPAddress poolServerIP);
    void setPoolServerPort(unsigned int port);
    void setTimeOffset(int timeOffset);
    void setUpdateInterval(unsigned long updateInterval);

    /** Pick a random local bind port in [minValue, maxValue]. */
    void setRandomPort(unsigned int minValue = 49152, unsigned int maxValue = 65535);
};
#endif
