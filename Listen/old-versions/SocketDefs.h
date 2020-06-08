// Copyright (c) 2015-2019 Josh Blum
// SPDX-License-Identifier: BSL-1.0

// ** Modified from SoapyRemote/common/SoapySocketDefs.hpp
// ** This header should be included first, to avoid compile errors.
// ** At least in the case of the windows header files.

// This header helps to abstract network differences between platforms.
// Including the correct headers for various network APIs.
// And providing various typedefs and definitions when missing.

#pragma once

/***********************************************************************
 * Windows socket headers
 **********************************************************************/
#ifdef _MSC_VER
#include <winsock2.h> //htonll

#include <ws2tcpip.h> //addrinfo
typedef int socklen_t;

#include <io.h> //read/write
#else

/***********************************************************************
 * unix socket headers
 **********************************************************************/
#ifndef __has_include
#error "Compiler missing __has_include macro!"
#endif

#if __has_include(<unistd.h>)
#include <unistd.h> //close
#define closesocket close
#endif

#if __has_include(<netdb.h>)
#include <netdb.h> //addrinfo
#endif

#if __has_include(<netinet/in.h>)
#include <netinet/in.h>
#endif

#if __has_include(<netinet/tcp.h>)
#include <netinet/tcp.h>
#endif

#if __has_include(<sys/types.h>)
#include <sys/types.h>
#endif

#if __has_include(<sys/socket.h>)
#include <sys/socket.h>
#endif

#if __has_include(<arpa/inet.h>)
#include <arpa/inet.h> //inet_ntop
#endif

#if __has_include(<ifaddrs.h>)
#include <ifaddrs.h> //getifaddrs
#endif

#if __has_include(<net/if.h>)
#include <net/if.h> //if_nametoindex
#endif

#if __has_include(<fcntl.h>)
#include <fcntl.h> //fcntl and constants
#endif
#endif

/***********************************************************************
 * socket type definitions
 **********************************************************************/
#ifndef INVALID_SOCKET
#define INVALID_SOCKET -1
#endif //INVALID_SOCKET

#ifndef _MSC_VER
#define SOCKET int
#endif //SOCKET

/***********************************************************************
 * socket errno
 **********************************************************************/
#ifdef _MSC_VER
#define SOCKET_ERRNO WSAGetLastError()
#define SOCKET_EINPROGRESS WSAEWOULDBLOCK
#define SOCKET_ETIMEDOUT WSAETIMEDOUT
#define SOCKET_ECONNREFUSED WSAECONNREFUSED
static inline const char *socket_strerror(const int err)
{
    static thread_local char buff[1024];
    FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, err, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&buff, sizeof(buff), NULL);
    return buff;
}
#else
#define SOCKET_ERRNO errno
#define SOCKET_EINPROGRESS EINPROGRESS
#define SOCKET_ETIMEDOUT ETIMEDOUT
#define SOCKET_ECONNREFUSED ECONNREFUSED
#define socket_strerror strerror
#endif

/***********************************************************************
 * socket flag definitions
 **********************************************************************/
#ifndef MSG_NOSIGNAL
#define MSG_NOSIGNAL 0
#endif
