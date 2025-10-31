#pragma once

#include "Modules/Assets/AssetManager.h"
#include "Modules/Scripting/ScriptingCommon.h"

namespace Kargono::Scripting { class Script; }

namespace Kargono::Assets
{
	class ScriptManager : public AssetManager<Scripting::Script>
	{
	public:
		
		virtual ~ScriptManager() = default;
	public:

		std::tuple<AssetHandle, bool> CreateNewScript(ScriptSpec& spec);
		bool SaveScript(AssetHandle scriptHandle, ScriptSpec& spec);
		void FillScriptMetadata(ScriptSpec& spec, Assets::AssetInfo& newAsset);

	};
}
