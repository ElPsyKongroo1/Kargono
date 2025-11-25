#pragma once

#include "Modules/Assets/Module/AssetTag.h"
#include "Modules/Scripting/Module/ScriptingModule.h"
#include "Kargono/Core/FixedBufferString.h"

#include <vector>

namespace Kargono::Scripting
{
	struct CustomEnum
	{
	public:
		//==============================
		// Asset Config Info
		//==============================
		constexpr static FixedBufStr32 GetAssetName()
		{
			return "Custom Enum";
		}

		constexpr static Assets::AssetFlags GetAssetFlags()
		{
			Assets::AssetFlags flags{};
			flags.SetFlag(Assets::AssetFlag::HasAssetCache);
			flags.SetFlag(Assets::AssetFlag::RequireUniqueName);
			return flags;
		}

		constexpr static FixedBufStr16 GetFileExtension()
		{
			return ".kgenum";
		}
		static void CreateFromName(Assets::Metadata<CustomEnum>& metadata);

	public:
		//=========================
		// Constructors/Destructors
		//=========================
		CustomEnum() = default;
		~CustomEnum() = default;
	public:
		//==============================
		// Serialization
		//==============================
		void Serialize(void* context);
		void Deserialize(void* context);
	public:
		//=========================
		// Query State
		//=========================
		bool DoesContainIdentifier(const char* queryName);
	public:
		//=========================
		// Modify State
		//=========================
		bool RemoveIdentifier(const char* queryName);
		bool RemoveIdentifier(size_t indexToDelete);
		bool RenameIdentifier(size_t indexToModify, const char* newName);
	public:
		//=========================
		// Public Fields
		//=========================
		FixedBufStr32 m_EnumName{};
		std::vector<FixedBufStr32> m_EnumIdentifiers{};
	};

	Register_Module_Type(CustomEnum, Assets::AssetTag)
}
