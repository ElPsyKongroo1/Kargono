#pragma once

#include "Kargono/Core/Base.h"
#include "Kargono/Core/WrappedData.h"
#include "Modules/Assets/Asset.h"

#include <unordered_map>
#include <string>

namespace Kargono::Scenes
{
	class GameState
	{
	public:
		template<typename t_FieldType>
		t_FieldType& GetField(const std::string& fieldName)
		{
			if (!m_Fields.contains(fieldName))
			{
				KG_CRITICAL("Could not get field from game state {}", fieldName);
				return nullptr;
			}
			return m_Fields.at(fieldName)->GetWrappedValue<t_FieldType>();
		}

		Ref<WrappedVariable> GetField(std::string_view fieldName)
		{
			// TODO: This is horrendous. We shouldn't be using this in production anyways, but still... ughh
			std::string fieldNameString{ fieldName };
			if (!m_Fields.contains(fieldNameString))
			{
				KG_CRITICAL("Could not get field from game state {}", fieldName);
				return nullptr;
			}
			return m_Fields.at(fieldNameString);
		}

		void SetField(std::string_view fieldName, void* value)
		{
			// TODO: This is horrendous. We shouldn't be using this in production anyways, but still... ughh
			std::string fieldNameString{ fieldName };
			if (!m_Fields.contains(fieldNameString))
			{
				KG_CRITICAL("Could not get field from game state {}", fieldName);
				return;
			}

			m_Fields.at(fieldNameString)->SetValue(value);
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

	class GameStateContext
	{
	public:
		//=========================
		// Modify Active Game State
		//=========================
		void SetActiveGameState(Ref<GameState> newGameState, Assets::AssetHandle newHandle);
		void ClearActiveGameState();
	public:
		//=========================
		// Get Active Game State
		//=========================
		Assets::AssetHandle GetActiveGameStateHandle();
		Ref<GameState> GetActiveGameState();
	private:
		//=========================
		// Internal Fields
		//=========================
		Ref<GameState> m_ActiveGameState{ nullptr };
		Assets::AssetHandle m_ActiveGameStateHandle{ Assets::k_EmptyHandle };
	};

	class GameStateService // TODO: EWWWWW UGHHHHHHH
	{
	public:
		//==============================
		// Getters/Setters
		//==============================
		static GameStateContext& GetActiveContext() { return s_GameStateContext; }
	private:
		//==============================
		// Internal Fields
		//==============================
		static inline GameStateContext s_GameStateContext{};
	};
}
