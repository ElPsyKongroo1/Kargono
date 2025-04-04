#pragma once

// Define the current platform
#define PLATFORM_WINDOWS 1
#define PLATFORM_MAC 2
#define PLATFORM_UNIX 3

#if defined(_WIN32)
#define PLATFORM PLATFORM_WINDOWS
#elif defined(__APPLE__)
#define PLATFORM PLATFORM_MAC
#else
#define PLATFORM PLATFORM_UNIX
#endif

#ifdef PLATFORM_WINDOWS
#ifndef NOMINMAX
#define NOMINMAX
#endif
#endif

// Define the socket headers relevent for "this" platform
#if PLATFORM == PLATFORM_WINDOWS
#include <WinSock2.h>
#elif PLATFORM == PLATFORM_MAC || PLATFORM == PLATFORM_UNIX
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#endif

// TODO: Link the winsock library in the actual engine plz TODO TODO TODO
#if PLATFORM == PLATFORM_WINDOWS
#pragma comment( lib, "wsock32.lib" )
#pragma comment ( lib, "Ws2_32.lib" )
#endif

