#pragma once

#include "Kargono/Core/Base.h"
#include "Kargono/Core/WrappedData.h"
#include "Kargono/Assets/Asset.h"

#include <unordered_map>
#include <string>

namespace Kargono::Assets { class AssetManager; }

namespace Kargono::Scenes
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

		Ref<WrappedVariable> GetField(const std::string& fieldName)
		{
			if (!m_Fields.contains(fieldName))
			{
				KG_CRITICAL("Could not get field from game state {}", fieldName);
				return nullptr;
			}
			return m_Fields.at(fieldName);
		}

		void SetField(const std::string& fieldName, void* value)
		{
			if (!m_Fields.contains(fieldName))
			{
				KG_CRITICAL("Could not get field from game state {}", fieldName);
				return;
			}

			m_Fields.at(fieldName)->SetValue(value);
		}

		bool AddField(const std::string& fieldName, WrappedVarType fieldType)
		{
			if (m_Fields.contains(fieldName))
			{
				KG_WARN("Attempt to add field to Game State that already exists");
				return false;
			}
			Ref<WrappedVariable> newVariable = nullptr;

			switch (fieldType)
			{
				case WrappedVarType::UInteger16:
				{
					newVariable = CreateRef<WrappedUInteger16>();
					m_Fields.insert_or_assign(fieldName, newVariable);
					return true;
				}
				default:
				{
					KG_WARN("Could not add field to Game State. Invalid fieldType Given.");
					return false;
				}
			}
		}

		bool DeleteField(const std::string& fieldName)
		{
			if (!m_Fields.contains(fieldName))
			{
				KG_WARN("Attempt to delete field to Game State that does not exist");
				return false;
			}

			m_Fields.erase(fieldName);
			return true;
		}

		std::unordered_map<std::string, Ref<WrappedVariable>>& GetAllFields()
		{
			return m_Fields;
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
		static void SetActiveGameStateField(const std::string& fieldName, void* value)
		{
			if (!s_GameState)
			{
				KG_WARN("Attempt to set a field on active game state that is inactive");
				return;
			}

			s_GameState->SetField(fieldName, value);
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
