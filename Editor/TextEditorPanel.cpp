#include "TextEditorPanel.h"

#include "Kargono.h"

namespace Kargono
{
	void TextEditorPanel::OnEditorUIRender()
	{
		ImGui::Begin("Text Editor");

		//ImGui::InputTextMultiline();

		ImGui::End();
	}
}
