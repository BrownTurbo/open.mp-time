#pragma once
#ifndef CROSSUDPLIB_H
#define CROSSUDPLIB_H
#include <string>
#include <vector>
#include <cstdint>

#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    typedef int socklen_t;
#else
    #include <sys/socket.h>
    #include <arpa/inet.h>
    #include <netdb.h>
    #include <unistd.h>
    #include <fcntl.h>
    #define INVALID_SOCKET -1
    #define SOCKET_ERROR -1
    typedef int SOCKET;
#endif

class UDPSocket {
private:
    SOCKET _socket;
    struct sockaddr_in _remoteAddr;
    bool _isNonBlocking;

public:
    UDPSocket();
    ~UDPSocket();

    void initSockets();
    void cleanupSockets();

    bool begin(uint16_t port);
    void stop();

    // Sending
    int beginPacket(const char* host, uint16_t port);
    int beginPacket(uint32_t rawip, uint16_t port);
    size_t write(const uint8_t* buffer, size_t size);

    // Receiving
    int parsePacket();
    int read(unsigned char* buffer, size_t len);
    void flush();

    void setNonBlocking(bool enable);
	void setTimeout(int milliseconds);
};
#endif
