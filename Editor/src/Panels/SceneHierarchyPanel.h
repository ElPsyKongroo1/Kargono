#pragma once

#include "Kargono.h"

namespace Kargono
{
	class SceneHierarchyPanel
	{
	public:
		SceneHierarchyPanel() = default;

		void OnImGuiRender();

		void SetSelectedEntity(Entity entity);
	private:
		void DrawEntityNode(Entity entity);
		void DrawComponents(Entity entity);
	private:
		template<typename T>
		void DisplayAddComponentEntry(const std::string& entryName);


	};
}
