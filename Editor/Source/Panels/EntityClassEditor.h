#pragma once
#include "Kargono.h"

namespace Kargono
{
	class EntityClassEditor
	{
	public:
		EntityClassEditor();

		void OnEditorUIRender();

	public:
		void RefreshEntityScripts(Assets::AssetHandle handle);

	};
}
