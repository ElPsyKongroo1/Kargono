#include "Panels/EntityClassEditor.h"

#include "Kargono.h"
#include "EditorLayer.h"

namespace Kargono
{
	static EditorLayer* s_EditorLayer { nullptr };

	EntityClassEditor::EntityClassEditor()
	{
		s_EditorLayer = EditorLayer::GetCurrentLayer();
	}
	void EntityClassEditor::OnEditorUIRender()
	{
		UI::Editor::StartWindow("Class Editor", &s_EditorLayer->m_ShowClassEditor);

		UI::Editor::EndWindow();
	}
}
