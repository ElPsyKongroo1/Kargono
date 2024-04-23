#pragma once

#include "Kargono/Core/Base.h"
#include "Kargono/Core/WrappedData.h"
#include "Kargono/Assets/Asset.h"

#include <unordered_map>
#include <string>

namespace Kargono::Assets { class AssetManager; }

namespace Kargono
{
	class GameState
	{
	public:
		template<typename T>
		T& GetField(const std::string& fieldName)
		{
			if (!m_Fields.contains(fieldName))
			{
				KG_CRITICAL("Could not get field from game state {}", fieldName);
				return nullptr;
			}
			return m_Fields.at(fieldName)->GetWrappedValue<T>();
		}

		std::string GetName()
		{
			return m_Name;
		}

		void SetName(const std::string& name)
		{
			m_Name = name;
		}
	public:
		static Ref<GameState> s_GameState;
		static Assets::AssetHandle s_GameStateHandle;
	private:
		std::string m_Name{};
		std::unordered_map<std::string, Ref<WrappedVariable>> m_Fields {};
		friend class Assets::AssetManager;
	};
}
