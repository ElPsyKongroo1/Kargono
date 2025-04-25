#pragma once

#if defined(KG_PLATFORM_WINDOWS)
	#ifndef NOMINMAX
	#define NOMINMAX
	#endif
#endif

// Define the socket headers relevent for "this" platform
#if defined(KG_PLATFORM_WINDOWS)
	#include <WinSock2.h>
#elif defined(KG_PLATFORM_LINUX) || defined(KG_PLATFORM_MAC)
	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <fcntl.h>
#else
	#error "Platform is not supported for networking!"
#endif

// TODO: Link the winsock library in the actual engine plz TODO TODO TODO
#if defined(KG_PLATFORM_WINDOWS)
	#pragma comment( lib, "wsock32.lib" )
	#pragma comment ( lib, "Ws2_32.lib" )
#endif

