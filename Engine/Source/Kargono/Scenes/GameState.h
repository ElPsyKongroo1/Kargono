#pragma once

#include "Kargono/Core/Base.h"
#include "Kargono/Core/WrappedData.h"
#include "AssetsPlugin/Asset.h"

#include <unordered_map>
#include <string>

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

		void SetName(std::string_view name)
		{
			m_Name = name;
		}
	public:
		std::string m_Name{};
		std::unordered_map<std::string, Ref<WrappedVariable>> m_Fields {};
	};

	class GameStateService
	{
	public:
		//=========================
		// Active Game State API
		//=========================
		static void SetActiveGameStateField(const std::string& fieldName, void* value)
		{
			if (!s_ActiveGameState)
			{
				KG_WARN("Attempt to set a field on active game state that is inactive");
				return;
			}

			s_ActiveGameState->SetField(fieldName, value);
		}
		static void* GetActiveGameStateField(const std::string& fieldName)
		{
			if (!s_ActiveGameState)
			{
				KG_WARN("Attempt to get a field on the active game state that is inactive");
				return nullptr;
			}

			return s_ActiveGameState->GetField(fieldName)->GetValue();
		}
	public:
		//=========================
		// Getter/Setter
		//=========================
		static void ClearActiveGameState()
		{
			s_ActiveGameState = nullptr;
			s_ActiveGameStateHandle = Assets::EmptyHandle;
		}
		static void SetActiveGameState(Ref<GameState> newGameState, Assets::AssetHandle newHandle)
		{
			s_ActiveGameState = newGameState;
			s_ActiveGameStateHandle = newHandle;
		}
		static Ref<GameState> GetActiveGameState()
		{
			return s_ActiveGameState;
		}
		static Assets::AssetHandle GetActiveGameStateHandle()
		{
			return s_ActiveGameStateHandle;
		}

	private:
		//=========================
		// Internal Fields
		//=========================
		static inline Ref<GameState> s_ActiveGameState{ nullptr };
		static inline Assets::AssetHandle s_ActiveGameStateHandle{ Assets::EmptyHandle };
	};
}
