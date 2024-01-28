#pragma once
#include <cstdint>

enum CustomMsgTypes : uint32_t
{
	AcceptConnection = 0,
	DenyConnection,
	ServerPing,
	MessageAll,
	ServerMessage,
	ClientChat,
	ServerChat,
	KeepAlive,
	UDPInit,

	RequestUserCount,
	UpdateUserCount,
	UpdateSessionUserSlot,
	LeaveCurrentSession,
	UserLeftSession,

	// Session Messages
	RequestJoinSession,
	ApproveJoinSession,
	DenyJoinSession,
	CurrentSessionInit,
	StartSession,
	InitSyncPing,
	SessionReadyCheck,
	SessionReadyCheckConfirm,
	EnableReadyCheck,

	// Entity Updates
	SendAllEntityLocation,
	UpdateEntityLocation,
	SendAllEntityPhysics,
	UpdateEntityPhysics,
	SignalAll,
	ReceiveSignal
};
