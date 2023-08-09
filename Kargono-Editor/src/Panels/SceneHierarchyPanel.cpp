#include "SceneHierarchyPanel.h"
#include <imgui.h>
#include "Kargono/Scene/Components.h"

namespace Kargono
{
	SceneHierarchyPanel::SceneHierarchyPanel(const Ref<Scene> context)
	{
		SetContext(context);
	}
	void SceneHierarchyPanel::SetContext(const Ref<Scene>& context)
	{
		m_Context = context;
	}
	void SceneHierarchyPanel::OnImGuiRender()
	{
		ImGui::Begin("Scene Hierarchy");

		m_Context->m_Registry.each([&](auto entityID)
		{
			Entity entity{entityID, m_Context.get()};
			auto& tc = entity.GetComponent<TagComponent>();
			ImGui::Text("%s", tc.Tag.c_str());
		});

		ImGui::End();
	}
}
