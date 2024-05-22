#pragma once
#include "Kargono.h"

namespace Kargono
{
	class EntityClassEditor
	{
	public:
		EntityClassEditor();

		void OnEditorUIRender();

	private:
		void InitializeDisplayEntityClassScreen();

	public:
		void RefreshEntityScripts(Assets::AssetHandle handle);

	};
}
