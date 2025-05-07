#pragma once
#include "Kargono/Core/Types.h"
#include "Kargono/Core/DataStructures.h"
#include "FileSystemModule/FileSystem.h"

#include <functional>
#include <unordered_set>
#include <limits>

namespace Kargono
{
	// Unique identifier for each system
	using SystemID = uint32_t;
	constexpr SystemID k_InvalidSystemID{ std::numeric_limits<SystemID>::max() };

	// Value between 0 - 254 that indicates callback calling order (lower is better)
	using CallbackPriority = uint8_t;
	constexpr CallbackPriority k_InvalidCallbackPriority
	{ 
		std::numeric_limits<CallbackPriority>::max() 
	};

	// Callback aliases
	using InitCallback = std::function<bool()>;
	using TerminateCallback = std::function<bool()>;
	using UpdateCallback = std::function<void(float)>;

	struct SystemInfo
	{
		// System identifier
		SystemID m_SystemID{ k_InvalidSystemID };
		// Callback priorities for organization
		CallbackPriority m_InitPriority{ k_InvalidCallbackPriority };
		CallbackPriority m_TerminatePriority{ k_InvalidCallbackPriority };
		CallbackPriority m_UpdatePriority{ k_InvalidCallbackPriority };
	};

	class ISystem
	{
	public:
		//==============================
		// System Information
		//==============================
		[[nodiscard]] virtual SystemInfo GetSystemInfo() const = 0;

	public:
		//==============================
		// System Functions
		//==============================
		[[nodiscard]] virtual Optional<InitCallback> GetInit() const = 0;
		[[nodiscard]] virtual Optional<TerminateCallback> GetTerminate() const = 0;
		[[nodiscard]] virtual Optional<UpdateCallback> GetUpdate() const = 0;
	};

	template<typename t_CallbackType>
	struct HandlerCallback
	{
		SystemID m_ID{ k_InvalidSystemID };
		t_CallbackType m_Callback{ nullptr };
	};

	class SystemHandler
	{
	public:
		//==============================
		// Constructors/Destructors
		//==============================
		SystemHandler() = default;
		~SystemHandler() = default;
		
		//==============================
		// Call System Functions
		//==============================
		[[nodiscard]] bool InitSystems() const
		{
			bool initSuccess{ true };
			for (const auto& [id, initCallback] : m_InitCallbacks)
			{
				if (!initCallback())
				{
					initSuccess = false;
				}
			}

			return initSuccess;
		}

		[[nodiscard]] bool TerminateSystems() const
		{
			bool terminateSuccess{ true };
			for (const auto& [id, terminateCallback] : m_TerminateCallbacks)
			{
				if (!terminateCallback())
				{
					terminateSuccess = false;
				}
				terminateCallback();
			}

			return terminateSuccess;
		}

		void UpdateSystems(float ts) const
		{
			for (const auto& [id, updateCallback] : m_UpdateCallbacks)
			{
				updateCallback(ts);
			}
		}

		//==============================
		// Manage Systems
		//==============================
		[[nodiscard]] bool RegisterSystem(ISystem* system)
		{
			KG_ASSERT(system);

			SystemInfo info{ system->GetSystemInfo()};

			// Is system already registered?
			if (m_ActiveSystems.contains(info.m_SystemID))
			{
				return false;
			}

			// Add all the relevant callbacks
			Optional<InitCallback> initCallback{ system->GetInit() };
			if (initCallback)
			{
				m_InitCallbacks.AddItem
				(
					{ info.m_SystemID, initCallback.value() }, 
					info.m_InitPriority
				);
			}

			Optional<TerminateCallback> terminateCallback{ system->GetTerminate() };
			if (terminateCallback)
			{
				m_TerminateCallbacks.AddItem
				(
					{ info.m_SystemID, terminateCallback.value() },
					info.m_TerminatePriority
				);
			}

			Optional<UpdateCallback> updateCallback{ system->GetUpdate() };
			if (updateCallback)
			{
				m_UpdateCallbacks.AddItem
				(
					{ info.m_SystemID, updateCallback.value() },
					info.m_UpdatePriority
				);
			}

			// Register the system ID
			m_ActiveSystems.insert(info.m_SystemID);
		}

		[[nodiscard]] bool RemoveSystem(SystemID systemID)
		{
			// Ensure the system exists
			if (!m_ActiveSystems.contains(systemID))
			{
				return false;
			}

			// Remove system from function callbacks

			// Search for init callback
			PriorityListIndex index = m_InitCallbacks.FindItem([systemID](const HandlerCallback<InitCallback>& callback) 
			{
				if (systemID == callback.m_ID)
				{
					return true;
				}

				return false;
			});
			
			// Remove callback if found
			if (index != k_InvalidPriorityListIndex)
			{
				if (!m_InitCallbacks.RemoveIndex(index))
				{
					KG_ERROR("Failed to remove init callback.");
				}
			}

			// Search for Terminate callback
			PriorityListIndex index = m_TerminateCallbacks.FindItem([systemID](const HandlerCallback<TerminateCallback>& callback)
			{
				if (systemID == callback.m_ID)
				{
					return true;
				}

				return false;
			});

			// Remove callback if found
			if (index != k_InvalidPriorityListIndex)
			{
				if (!m_TerminateCallbacks.RemoveIndex(index))
				{
					KG_ERROR("Failed to remove Terminate callback.");
				}
			}

			// Search for Update callback
			PriorityListIndex index = m_UpdateCallbacks.FindItem([systemID](const HandlerCallback<UpdateCallback>& callback)
			{
				if (systemID == callback.m_ID)
				{
					return true;
				}

				return false;
			});

			// Remove callback if found
			if (index != k_InvalidPriorityListIndex)
			{
				if (!m_UpdateCallbacks.RemoveIndex(index))
				{
					KG_ERROR("Failed to remove Update callback.");
				}
			}

			// Remove system
			m_ActiveSystems.erase(systemID);
			return true;
		}

		void ClearSystems()
		{
			// Clear callbacks
			m_InitCallbacks.Clear();
			m_TerminateCallbacks.Clear();
			m_UpdateCallbacks.Clear();

			// Clear systems
			m_ActiveSystems.clear();
		}

		void SortSystems()
		{
			m_InitCallbacks.SortList();
			m_TerminateCallbacks.SortList();
			m_UpdateCallbacks.SortList();
		}
	private:
		//==============================
		// Internal Fields
		//==============================
		// Callbacks
		PriorityList<HandlerCallback<InitCallback>, CallbackPriority> m_InitCallbacks{};
		PriorityList<HandlerCallback<TerminateCallback>, CallbackPriority> m_TerminateCallbacks{};
		PriorityList<HandlerCallback<UpdateCallback>, CallbackPriority> m_UpdateCallbacks{};
		// Registered systems
		std::unordered_set<SystemID> m_ActiveSystems{};
	};
}

namespace Kargono::Utility
{
	consteval SystemID GetSystemID(const char* systemName)
	{
		return static_cast<SystemID>(FileSystem::CRCFromString(systemName));
	}
}