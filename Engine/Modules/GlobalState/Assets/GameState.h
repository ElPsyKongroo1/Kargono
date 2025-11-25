#pragma once

#include "Kargono/Core/Base.h"
#include "Kargono/Core/WrappedData.h"
#include "Modules/Assets/AssetsCommon.h"
#include "Modules/Assets/Metadata.h"
#include "Modules/GlobalState/Module/GlobalStateModule.h"
#include "Modules/Assets/Module/AssetTag.h"

#include <unordered_map>
#include <string>

namespace Kargono::GlobalState
{
	class GameState
	{
	public:
		//==============================
		// Asset Config Info
		//==============================
		constexpr static FixedBufStr32 GetAssetName()
		{
			return "Game State";
		}
		constexpr static Assets::AssetFlags GetAssetFlags()
		{
			Assets::AssetFlags flags{};
			flags.ClearFlag(Assets::AssetFlag::HasAssetCache);
			flags.SetFlag(Assets::AssetFlag::RequireUniqueName);
			return flags;
		}
		constexpr static FixedBufStr16 GetFileExtension()
		{
			return ".kgstate";
		}

		static void CreateFromName(Assets::Metadata<GameState>& metadata);
	public:
		//==============================
		// Constructors/Destructors
		//==============================
		GameState() = default;
		~GameState() = default;
	public:
		//==============================
		// Serialization
		//==============================
		void Serialize(void* context);
		void Deserialize(void* context);
	public:
		//=========================
		// Modify Game State Fields
		//=========================
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
		Ref<WrappedVariable> GetField(std::string_view fieldName);
		void SetField(std::string_view fieldName, void* value);
		bool AddField(const std::string& fieldName, WrappedVarType fieldType);
		bool DeleteField(const std::string& fieldName);
	public:
		//=========================
		// Getters/Setters
		//=========================
		std::unordered_map<std::string, Ref<WrappedVariable>>& GetAllFields() { return m_Fields; }
		std::string GetName() { return m_Name; }
		void SetName(std::string_view name) { m_Name = name; }
	public:
		//=========================
		// Public Fields
		//=========================
		std::string m_Name{};
		std::unordered_map<std::string, Ref<WrappedVariable>> m_Fields{};
	};

	Register_Module_Type(GameState, Assets::AssetTag)
}
