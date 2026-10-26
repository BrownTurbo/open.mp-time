#include "./NTPClient.h"

#include <chrono>

NTPClient::NTPClient(UDP &udp)
{
  this->_udp = &udp;
}

NTPClient::NTPClient(UDP &udp, long timeOffset)
{
  this->_udp = &udp;
  this->_timeOffset = timeOffset;
}

NTPClient::NTPClient(UDP &udp, const char *poolServerName)
{
  this->_udp = &udp;
  this->_poolServerName = poolServerName ? poolServerName : "";
  this->_useIP = false;
}

NTPClient::NTPClient(UDP &udp, IPAddress poolServerIP)
{
  this->_udp = &udp;
  this->_poolServerIP = poolServerIP;
  this->_useIP = true;
}

NTPClient::NTPClient(UDP &udp, const char *poolServerName, long timeOffset)
{
  this->_udp = &udp;
  this->_timeOffset = timeOffset;
  this->_poolServerName = poolServerName ? poolServerName : "";
  this->_useIP = false;
}

NTPClient::NTPClient(UDP &udp, IPAddress poolServerIP, long timeOffset)
{
  this->_udp = &udp;
  this->_timeOffset = timeOffset;
  this->_poolServerIP = poolServerIP;
  this->_useIP = true;
}

NTPClient::NTPClient(UDP &udp, const char *poolServerName, long timeOffset, unsigned long updateInterval)
{
  this->_udp = &udp;
  this->_timeOffset = timeOffset;
  this->_poolServerName = poolServerName ? poolServerName : "";
  this->_useIP = false;
  this->_updateInterval = updateInterval;
}

NTPClient::NTPClient(UDP &udp, IPAddress poolServerIP, long timeOffset, unsigned long updateInterval)
{
  this->_udp = &udp;
  this->_timeOffset = timeOffset;
  this->_poolServerIP = poolServerIP;
  this->_useIP = true;
  this->_updateInterval = updateInterval;
}

NTPClient::NTPClient(UDP &udp, IPAddress poolServerIP, unsigned int port, long timeOffset, unsigned long updateInterval)
{
  this->_udp = &udp;
  this->_timeOffset = timeOffset;
  this->_poolServerIP = poolServerIP;
  this->_useIP = true;
  this->_updateInterval = updateInterval;
  this->_NTPport = port;
}

NTPClient::NTPClient(UDP &udp, const char *poolServerName, unsigned int port, long timeOffset, unsigned long updateInterval)
{
  this->_udp = &udp;
  this->_timeOffset = timeOffset;
  this->_poolServerName = poolServerName ? poolServerName : "";
  this->_useIP = false;
  this->_updateInterval = updateInterval;
  this->_NTPport = port;
}

NTPClient::NTPClient(UDP &udp, const char *poolServerName, unsigned int port)
{
  this->_udp = &udp;
  this->_poolServerName = poolServerName ? poolServerName : "";
  this->_useIP = false;
  this->_NTPport = port;
}

NTPClient::NTPClient(UDP &udp, IPAddress poolServerIP, unsigned int port)
{
  this->_udp = &udp;
  this->_poolServerIP = poolServerIP;
  this->_useIP = true;
  this->_NTPport = port;
}

NTPClient::NTPClient(UDP &udp, const char *poolServerName, unsigned int port, long timeOffset)
{
  this->_udp = &udp;
  this->_timeOffset = timeOffset;
  this->_poolServerName = poolServerName ? poolServerName : "";
  this->_useIP = false;
  this->_NTPport = port;
}

NTPClient::NTPClient(UDP &udp, IPAddress poolServerIP, unsigned int port, long timeOffset)
{
  this->_udp = &udp;
  this->_timeOffset = timeOffset;
  this->_poolServerIP = poolServerIP;
  this->_useIP = true;
  this->_NTPport = port;
}

void NTPClient::begin()
{
  this->begin(this->_port);
}

void NTPClient::begin(unsigned int port)
{
  this->_port = port;

  if (!this->_udp->isBound())
  {
    if (!this->_udp->begin(static_cast<uint16_t>(_port)))
    {
      return;
    }
    this->_udp->setNonBlocking(true);
  }

  this->_udpSetup = true;
}

bool NTPClient::forceUpdate()
{
  // Flush any existing packets
  this->_udp->flush();

  if (!this->sendNTPPacket())
  {
    return false;
  }

  // Wait till data is there or timeout...
  auto start = std::chrono::steady_clock::now();
  while (this->_udp->parsePacket() < NTP_PACKET_SIZE)
  {
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start).count();
    if (elapsed > 2000)
      return false;            // timeout
    std::this_thread::yield(); // give other threads CPU time
  }

  auto now = std::chrono::steady_clock::now();
  int bytesRead = this->_udp->read(this->_packetBuffer, NTP_PACKET_SIZE);
  if (bytesRead < NTP_PACKET_SIZE)
    return false;

  uint32_t secsSince1900 = (static_cast<uint32_t>(this->_packetBuffer[40]) << 24) |
                           (static_cast<uint32_t>(this->_packetBuffer[41]) << 16) |
                           (static_cast<uint32_t>(this->_packetBuffer[42]) << 8)  |
                           (static_cast<uint32_t>(this->_packetBuffer[43]));

  if (secsSince1900 < SEVENZYYEARS)
    return false;

  this->_currentEpoc = secsSince1900 - SEVENZYYEARS;
  this->_lastUpdate = now;
  this->_state = NTPState::IDLE;

  return true;
}

bool NTPClient::update()
{
  auto now = std::chrono::steady_clock::now();

  // Case 1: We are idle, check if it's time to request a new sync
  if (this->_state == NTPState::IDLE)
  {
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - this->_lastUpdate).count();
    if (this->_lastUpdate == std::chrono::steady_clock::time_point() || elapsed >= this->_updateInterval)
    {
      this->requestUpdate();
    }
    return false;
  }

  // Case 2: We are waiting for a response
  if (this->_state == NTPState::WAITING_FOR_RESPONSE)
  {
    int cb = this->_udp->parsePacket();

    if (cb >= NTP_PACKET_SIZE)
    {
      // Packet arrived! Process it.
      int bytesRead = this->_udp->read(this->_packetBuffer, NTP_PACKET_SIZE);
      if (bytesRead >= NTP_PACKET_SIZE)
      {
        uint32_t secsSince1900 = (static_cast<uint32_t>(this->_packetBuffer[40]) << 24) |
                                 (static_cast<uint32_t>(this->_packetBuffer[41]) << 16) |
                                 (static_cast<uint32_t>(this->_packetBuffer[42]) << 8)  |
                                 (static_cast<uint32_t>(this->_packetBuffer[43]));

        if (secsSince1900 >= SEVENZYYEARS)
        {
          this->_currentEpoc = secsSince1900 - SEVENZYYEARS;
          this->_lastUpdate = now; // Store the exact sync time
          this->_state = NTPState::IDLE;
          return true;
        }
      }
    }

    // Handle Timeout (e.g., 2 seconds)
    auto waitTime = std::chrono::duration_cast<std::chrono::milliseconds>(now - this->_requestTime).count();
    if (waitTime > 2000)
    {
      this->_state = NTPState::IDLE; // Reset to try again later
      if (this->_lastUpdate == std::chrono::steady_clock::time_point() && this->_updateInterval > 5000)
      {
        // If not synced yet, retry in 5 seconds rather than waiting full interval
        this->_lastUpdate = std::chrono::steady_clock::now() - std::chrono::milliseconds(this->_updateInterval - 5000);
      }
    }
  }

  return false;
}

void NTPClient::requestUpdate()
{
  // Flush old data
  this->_udp->flush();

  if (this->sendNTPPacket())
  {
    this->_state = NTPState::WAITING_FOR_RESPONSE;
    this->_requestTime = std::chrono::steady_clock::now();
  }
  else
  {
    this->_state = NTPState::IDLE;
    if (this->_updateInterval > 5000)
    {
      this->_lastUpdate = std::chrono::steady_clock::now() - std::chrono::milliseconds(this->_updateInterval - 5000);
    }
  }
}

bool NTPClient::isTimeSet() const
{
  return (this->_lastUpdate != std::chrono::steady_clock::time_point());
}

unsigned long NTPClient::getEpochTime() const
{
  if (this->_lastUpdate == std::chrono::steady_clock::time_point())
    return 0;

  auto now = std::chrono::steady_clock::now();
  auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - this->_lastUpdate).count();

  return this->_timeOffset + this->_currentEpoc + (unsigned long)elapsed;
}

int NTPClient::getDay() const
{
  return (((this->getEpochTime() / 86400L) + 4) % 7); // 0 is Sunday
}
int NTPClient::getHours() const
{
  return ((this->getEpochTime() % 86400L) / 3600);
}
int NTPClient::getMinutes() const
{
  return ((this->getEpochTime() % 3600) / 60);
}
int NTPClient::getSeconds() const
{
  return (this->getEpochTime() % 60);
}

std::string NTPClient::getFormattedTime() const
{
  unsigned long rawTime = this->getEpochTime();
  unsigned long hours = (rawTime % 86400L) / 3600;
  unsigned long minutes = (rawTime % 3600) / 60;
  unsigned long seconds = rawTime % 60;

  char buffer[16];
  snprintf(buffer, sizeof(buffer), "%02lu:%02lu:%02lu", hours, minutes, seconds);
  return std::string(buffer);
}

void NTPClient::end()
{
  this->_udp->stop();
  this->_udpSetup = false;
}

void NTPClient::setTimeOffset(int timeOffset)
{
  this->_timeOffset = timeOffset;
}

void NTPClient::setUpdateInterval(unsigned long updateInterval)
{
  this->_updateInterval = updateInterval;
}

void NTPClient::setPoolServerName(const char *poolServerName)
{
  this->_poolServerName = poolServerName ? poolServerName : "";
  this->_useIP = false;
}

void NTPClient::setPoolServerName(const std::string &poolServerName)
{
  this->_poolServerName = poolServerName;
  this->_useIP = false;
}

void NTPClient::setPoolServerIP(IPAddress poolServerIP)
{
  this->_poolServerIP = poolServerIP;
  this->_useIP = true;
}

void NTPClient::setPoolServerPort(unsigned int port)
{
  this->_NTPport = port;
}

bool NTPClient::sendNTPPacket()
{
  // set all bytes in the buffer to 0
  memset(this->_packetBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request (LI=3, Version=4, Mode=3 Client)
  this->_packetBuffer[0] = 0b11100011; // LI, Version, Mode
  this->_packetBuffer[1] = 0;          // Stratum, or type of clock
  this->_packetBuffer[2] = 6;          // Polling Interval
  this->_packetBuffer[3] = 0xEC;       // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  this->_packetBuffer[12] = 49;
  this->_packetBuffer[13] = 0x4E;
  this->_packetBuffer[14] = 49;
  this->_packetBuffer[15] = 52;

  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:
  int ok = 0;
  if (!this->_useIP && !this->_poolServerName.empty())
  {
    ok = this->_udp->beginPacket(this->_poolServerName.c_str(), this->_NTPport);
  }
  else
  {
    ok = this->_udp->beginPacket(this->_poolServerIP, this->_NTPport);
  }
  if (!ok)
  {
    return false;
  }
  return this->_udp->write(this->_packetBuffer, NTP_PACKET_SIZE) == NTP_PACKET_SIZE;
}

void NTPClient::setRandomPort(unsigned int minValue, unsigned int maxValue)
{
  static std::random_device rd;
  static std::mt19937 gen(rd());
  std::uniform_int_distribution<> dis(minValue, maxValue);
  this->_port = dis(gen);
}
