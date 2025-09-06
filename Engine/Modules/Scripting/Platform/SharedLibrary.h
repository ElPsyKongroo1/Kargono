#pragma once

#include "Modules/Scripting/Platform/SharedLibraryTypes.h"

namespace Kargono::Scripting
{
	class SharedLib
	{
	public:
		//==============================
		// Constructor/Destructor
		//==============================
		SharedLib() = default;
		~SharedLib() = default;
	public:
		//==============================
		// Load/Close Shared Library
		//==============================
		bool LoadSharedLib(const char* libLocation);
		bool CloseSharedLib();
	public:
		//==============================
		// Query Func Pointer State
		//==============================
		bool IsActive();
	public:
		//==============================
		// Get FuncPtr's From Shared Library
		//==============================
		template<typename t_FuncType>
		t_FuncType GetFuncPointer(const char* funcIdentifier)
		{
			return reinterpret_cast<t_FuncType>(GetFuncPtrRaw(funcIdentifier));
		}
		SharedLibFuncPtr GetFuncPtrRaw(const char* funcIdentifier);
	public:
		//==============================
		// Public Fields
		//==============================
		SharedLibHandle m_Handle{ nullptr };
	};
}