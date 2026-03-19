#pragma once

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <winsock2.h>
#include <ws2tcpip.h>

using socket_t = SOCKET;
using socklen_t = int;
// ssize_t is already defined by MinGW's corecrt.h as __int64
constexpr socket_t invalid_socket_value = INVALID_SOCKET;

inline int socket_last_error()
{
    return WSAGetLastError();
}

inline int socket_close(socket_t socketFd)
{
    return closesocket(socketFd);
}

inline int socket_set_recv_timeout(socket_t socketFd, int milliseconds)
{
    DWORD timeout = static_cast<DWORD>(milliseconds);
    return setsockopt(socketFd, SOL_SOCKET, SO_RCVTIMEO, reinterpret_cast<const char*>(&timeout), sizeof(timeout));
}

inline int socket_set_send_timeout(socket_t socketFd, int milliseconds)
{
    DWORD timeout = static_cast<DWORD>(milliseconds);
    return setsockopt(socketFd, SOL_SOCKET, SO_SNDTIMEO, reinterpret_cast<const char*>(&timeout), sizeof(timeout));
}

inline int socket_set_reuseaddr(socket_t socketFd, int enabled)
{
    return setsockopt(socketFd, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char*>(&enabled), sizeof(enabled));
}

inline bool socket_would_block(int errorCode)
{
    return errorCode == WSAEWOULDBLOCK || errorCode == WSAETIMEDOUT;
}

#else
#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

using socket_t = int;
constexpr socket_t invalid_socket_value = -1;

inline int socket_last_error()
{
    return errno;
}

inline int socket_close(socket_t socketFd)
{
    return close(socketFd);
}

inline int socket_set_recv_timeout(socket_t socketFd, int milliseconds)
{
    timeval timeout{};
    timeout.tv_sec = milliseconds / 1000;
    timeout.tv_usec = (milliseconds % 1000) * 1000;
    return setsockopt(socketFd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
}

inline int socket_set_send_timeout(socket_t socketFd, int milliseconds)
{
    timeval timeout{};
    timeout.tv_sec = milliseconds / 1000;
    timeout.tv_usec = (milliseconds % 1000) * 1000;
    return setsockopt(socketFd, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));
}

inline int socket_set_reuseaddr(socket_t socketFd, int enabled)
{
    return setsockopt(socketFd, SOL_SOCKET, SO_REUSEADDR, &enabled, sizeof(enabled));
}

inline bool socket_would_block(int errorCode)
{
    return errorCode == EAGAIN || errorCode == EWOULDBLOCK;
}

#endif
