#pragma once
#include "Modules/EditorUI/Widgets/EditorUIWidget.h"

#include "Kargono/Core/Base.h"
#include "Kargono/Core/FixedString.h"
#include "Kargono/Core/Buffer.h"
#include "Kargono/Core/WrappedData.h"

#include <functional>

namespace Kargono::EditorUI
{
	//TODO: AHHHHH I HATE THIS. I WANT TO REMOVE THIS BUT I NEED TO REMOVE GAME STATES FIRST. 
	// TODO: AHHHHHHHHHHHHHHHHHHHHHHHHHHHHH.

	struct EditVariableWidget : public Widget
	{
	public:

		EditVariableWidget() : Widget() {}
	public:
		void RenderVariable();
	public:
		FixedString32 m_Label;
		Buffer FieldBuffer{};
		WrappedVarType VariableType{ WrappedVarType::Integer32 };
	public:
		void AllocateBuffer()
		{
			FieldBuffer.Allocate(400);
			FieldBuffer.SetDataToByte(0);
		}
	};
}

