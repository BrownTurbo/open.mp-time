#include "./UDP.h"
#include <cstring>

static int winsockRefCount = 0;

void UDPSocket::initSockets()
{
    if (winsockRefCount++ == 0) {
#ifdef _WIN32
        WSADATA wsaData;
        WSAStartup(MAKEWORD(2, 2), &wsaData);
#endif
    }
}

void UDPSocket::cleanupSockets()
{
    if (--winsockRefCount == 0) {
#ifdef _WIN32
        WSACleanup();
#endif
    }
}

UDPSocket::UDPSocket() : _socket(INVALID_SOCKET), _isNonBlocking(false) {
    initSockets();
}

UDPSocket::~UDPSocket() {
    stop();
    cleanupSockets();
}

bool UDPSocket::begin(uint16_t port) {
    if (_socket != INVALID_SOCKET) {
        stop();
    }

    _socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (_socket == INVALID_SOCKET) return false;

    struct sockaddr_in local;
    local.sin_family = AF_INET;
    local.sin_port = htons(port);
    local.sin_addr.s_addr = INADDR_ANY;

    if (bind(_socket, (struct sockaddr*)&local, sizeof(local)) == SOCKET_ERROR) {
        stop();
        return false;
    }
    return true;
}

void UDPSocket::setNonBlocking(bool enable) {
    _isNonBlocking = enable;
#ifdef _WIN32
    u_long mode = enable ? 1 : 0;
    ioctlsocket(_socket, FIONBIO, &mode);
#else
    int flags = fcntl(_socket, F_GETFL, 0);
    fcntl(_socket, F_SETFL, enable ? (flags | O_NONBLOCK) : (flags & ~O_NONBLOCK));
#endif
}

int UDPSocket::beginPacket(const char* host, uint16_t port) {
    struct addrinfo hints = {0}, *res;
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;

    if (getaddrinfo(host, nullptr, &hints, &res) != 0) return 0;

    memcpy(&_remoteAddr, res->ai_addr, sizeof(struct sockaddr_in));
    _remoteAddr.sin_port = htons(port);

    freeaddrinfo(res);
    return 1;
}

int UDPSocket::beginPacket(uint32_t rawip, uint16_t port) {
    struct addrinfo hints = {0}, *res;
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;

    char host[INET_ADDRSTRLEN];
    uint32_t netIp = htonl(rawip);

    if (inet_ntop(AF_INET, &netIp, host, INET_ADDRSTRLEN) == nullptr) {
        perror("inet_ntop");
        return 0;
    }

    if (getaddrinfo(host, nullptr, &hints, &res) != 0) return 0;

    memcpy(&_remoteAddr, res->ai_addr, sizeof(struct sockaddr_in));
    _remoteAddr.sin_port = htons(port);

    freeaddrinfo(res);
    return 1;
}

size_t UDPSocket::write(const uint8_t* buffer, size_t size) {
    int sent = sendto(_socket, (const char*)buffer, (int)size, 0,
                      (struct sockaddr*)&_remoteAddr, sizeof(_remoteAddr));
    return (sent == SOCKET_ERROR) ? 0 : (size_t)sent;
}

int UDPSocket::parsePacket() {
    // Check if data is available using recv with MSG_PEEK
    char peek;
    struct sockaddr_in from;
    socklen_t fromLen = sizeof(from);
    int result = recvfrom(_socket, &peek, 128, MSG_PEEK, reinterpret_cast<struct sockaddr*>(&from), &fromLen);

    if (result > 0) return result;
    return 0;
}

int UDPSocket::read(unsigned char* buffer, size_t len) {
    return recv(_socket, (char*)buffer, (int)len, 0);
}

void UDPSocket::stop() {
    if (_socket != INVALID_SOCKET) {
#ifdef _WIN32
        closesocket(_socket);
#else
        close(_socket);
#endif
        _socket = INVALID_SOCKET;
    }
}

void UDPSocket::flush() {
    if (_socket == INVALID_SOCKET) return;

    unsigned char dummy[512];
    int bytes;
    // Keep reading until the incoming buffer is empty
    while ((bytes = recv(_socket, (char*)dummy, sizeof(dummy), 0)) > 0) {
        // Discarding data
    }
}

void UDPSocket::setTimeout(int milliseconds) {
    if (_socket == INVALID_SOCKET) return;
#ifdef _WIN32
    DWORD timeout = (DWORD)milliseconds;
    setsockopt(_socket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout));
#else
    struct timeval tv;
    tv.tv_sec = milliseconds / 1000;
    tv.tv_usec = (milliseconds % 1000) * 1000;
    setsockopt(_socket, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
#endif
}
